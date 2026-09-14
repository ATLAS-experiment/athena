# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import re
import warnings
from copy import deepcopy
from dataclasses import dataclass
from functools import partial

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigSequence import groupBlocks
from AsgAnalysisAlgorithms.AsgAnalysisConfig import EventCutFlowBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType, ConfigDeprecationWarning


class UnavailableFeatureError(ValueError):
    """Raised when an EXPR cut uses a syntactically valid but unimplemented
    feature (an unknown variable or collection). Subclasses ValueError so the
    framework's existing tolerance and `except ValueError` still apply."""


class InconsistentSettingsError(ValueError):
    """Raised when an EXPR cut is internally inconsistent or ill-typed (wrong
    operand count, an operation undefined for the given object, etc.)."""


@dataclass(frozen=True)
class _ExprCollection:
    option: str
    apply_btag: bool = False
    is_met: bool = False


@dataclass(frozen=True)
class _ExprVariable:
    min_operands: int
    max_operands: int | None
    separator: str | None
    needs_eta: bool
    met_ok: bool


class _ExprParser:
    """Small recursive-descent parser for EXPR selector expressions."""

    def __init__(self, token_re):
        self._token_re = token_re
        self._tokens = []
        self._pos = 0

    def tokenize(self, text):
        tokens, pos = [], 0
        while pos < len(text):
            match = self._token_re.match(text, pos)
            if not match:
                raise InconsistentSettingsError(
                    f"[EventSelectionConfig] EXPR: cannot parse near '{text[pos:]}'")
            pos = match.end()
            if match.lastgroup != "WS":
                tokens.append((match.lastgroup, match.group()))
        tokens.append(("END", ""))
        return tokens

    def _peek(self):
        return self._tokens[self._pos]

    def _advance(self):
        token = self._tokens[self._pos]
        self._pos += 1
        return token

    def _expect(self, kind):
        token = self._advance()
        if token[0] != kind:
            raise InconsistentSettingsError(
                f"[EventSelectionConfig] EXPR: expected {kind}, got '{token[1]}'")
        return token

    def parse(self, tokens):
        self._tokens = tokens
        self._pos = 0
        variable, separator, operands = self._parse_funcall()
        sign = self._parse_sign()
        ref_value = self._parse_value()
        self._expect("END")
        return variable, separator, operands, sign, ref_value

    def _parse_funcall(self):
        variable = self._expect("ID")[1]
        self._expect("LP")
        operands = [self._parse_operand()]
        separator = None
        while self._peek()[0] in ("COMMA", "PLUS"):
            sep = "," if self._advance()[0] == "COMMA" else "+"
            if separator is None:
                separator = sep
            elif sep != separator:
                raise InconsistentSettingsError(
                    "[EventSelectionConfig] EXPR: cannot mix ',' and '+' separators")
            operands.append(self._parse_operand())
        self._expect("RP")
        return variable, separator, operands

    def _parse_operand(self):
        coll = self._expect("ID")[1]
        index = None
        if self._peek()[0] == "LB":
            self._advance()
            index = int(self._expect("NUM")[1])
            self._expect("RB")
        return coll, index

    def _parse_sign(self):
        token = self._advance()
        if token[0] not in ("LT", "GT", "EQ", "GE", "LE"):
            raise InconsistentSettingsError(
                f"[EventSelectionConfig] EXPR: expected a comparison operator, got '{token[1]}'")
        return token[0]

    def _parse_value(self):
        negative = False
        if self._peek()[0] == "MINUS":
            self._advance()
            negative = True
        value = float(self._expect("NUM")[1])
        return -value if negative else value



class EventSelectionMergerConfig(ConfigBlock):
    """ConfigBlock for merging the output of various selection streams"""

    def __init__(self):
        super(EventSelectionMergerConfig, self).__init__()
        self._instance_number = EventSelectionMergerConfig.get_instance_count()
        self.setBlockName('EventSelectionMerger')
        self.addDependency('EventSelection', required=True)
        self.addOption('noFilter', False, type=bool,
            info="do not apply an event filter, i.e. setting it to `False` "
            "removes events not passing the full list of selection cuts.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return '' # There is only ever one instance of this block

    def makeAlgs(self, config):
        # Only the first instance runs; all others are no-ops
        if self._instance_number != 1:
            return

        selections = config.getContainerMeta('EventInfo', 'eventSelectionNames',
                                             failOnMiss=True)
        selections = [sel for sel in selections if not sel.startswith("pass_SUB")]

        alg = config.createAlgorithm('CP::SaveFilterAlg',
                                     'EventSelectionMerger' + selections[0].split("_%SYS%")[0])
        alg.FilterDescription = 'events passing at least one EventSelection'
        alg.eventDecisionOutputDecoration = 'ignore_anySelection_%SYS%'
        alg.selection = '||'.join([sel + ',as_char' for sel in selections])
        alg.noFilter = self.noFilter
        alg.selectionName = 'pass_anySelection_%SYS%'
        alg.decorationName = 'ntuplepass_anySelection_%SYS%'


class EventSelectionConfig(ConfigBlock):
    """ConfigBlock for interpreting text-based event selections"""

    # N-object pT selectors that differ only by source container.
    # keyword -> (container option, algorithm-name tag, plural noun)
    _NOBJECT = {
        "EL_N":   ("electrons",  "NEL",   "electrons"),
        "MU_N":   ("muons",      "NMU",   "muons"),
        "JET_N":  ("jets",       "NJET",  "jets"),
        "PH_N":   ("photons",    "NPH",   "photons"),
        "TAU_N":  ("taus",       "NTAU",  "tau-jets"),
        "LJET_N": ("largeRjets", "NLJET", "large-R jets"),
    }
    _KEYWORD_SPECS = None

    def __init__(self):
        super(EventSelectionConfig, self).__init__()
        self.setBlockName('EventSelection')
        self.addOption('selectionName', '', type=str,
            noneAction='error',
            info="the name of the event selection, used to uniquely identify "
            "the `EventSelectionConfig` block.")
        self.addOption('electrons', "", type=str,
            info="the input electron container, with a possible selection, in "
            "the format `container` or `container.selection`.")
        self.addOption('muons', "", type=str,
            info="the input muon container, with a possible selection, in the "
            "format `container` or `container.selection`.")
        self.addOption('jets', "", type=str,
            info="the input jet container, with a possible selection, in the "
            "format `container` or `container.selection`.")
        self.addOption('largeRjets', "", type=str,
            info="the large-R jet container, with a possible selection, in "
            "the format `container` or `container.selection`.")
        self.addOption('photons', "", type=str,
            info="the input photon container, with a possible selection, in "
            "the format `container` or `container.selection`.")
        self.addOption('taus', "", type=str,
            info="the input tau-jet container, with a possible selection, in "
            "the format `container` or `container.selection`.")
        self.addOption('met', "", type=str,
            info="the input MET container.")
        self.addOption('metTerm', "Final", type=str,
            info="the MET term to use when computing MET-based quantities.")
        self.addOption('btagDecoration', "", type=str,
            info="the b-tagging decoration to use when defining b-jets.")
        self.addOption('preselection', "", type=str,
            info="the event-wise selection flag to start this event selection "
            "from.")
        self.addOption('selectionCuts', "", type=str,
            noneAction='error',
            info="a single string listing one selection cut per line. "
            "See [available keywords](https://topcptoolkit.docs.cern.ch/latest/settings/eventselection/#available-keywords).")
        self.addOption('debugMode', False, type=bool,
            info="whether to create an output branch for every single line "
            "of the selection cuts. Setting it to `False` only saves the"
            " final decision.")
        self.addOption('useDressedProperties', True, type=bool,
            info="whether to use dressed truth electron and truth muon "
            "kinematics rather than simple 4-vector kinematics.")
        self.step = 0
        self.currentDecoration = ''
        self.cutflow = []
        self._dispatch = self._build_dispatch()

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.selectionName

    def _build_dispatch(self):
        """Map each keyword to its handler. Dispatch is an exact lookup on the
        first token, which removes the ordering fragility of token-membership."""
        d = {
            "JET_N_BTAG":          self.add_NBJET_selector,
            "JET_N_GHOST":         self.add_NJETGHOST_selector,
            "LJET_N_GHOST":        self.add_NLJETGHOST_selector,
            "LJETMASS_N":          self.add_NLJETMASS_selector,
            "LJETMASSWINDOW_N":    self.add_NLJETMASSWINDOW_selector,
            "OBJ_N":               self.add_NOBJ_selector,
            "SUM_EL_N_MU_N":       self.add_SUMNELNMU_selector,
            "SUM_EL_N_MU_N_TAU_N": self.add_SUMNLEPTONS_selector,
            "MET":                 self.add_MET_selector,
            "MWT":                 self.add_MWT_selector,
            "MET+MWT":             self.add_METMWT_selector,
            "MLL":                 self.add_MLL_selector,
            "MLLWINDOW":           self.add_MLLWINDOW_selector,
            "MLL_OSSF":            self.add_MLL_OSSF_selector,
            "OS":                  partial(self._add_charge, osMode=True,  tag="OS"),
            "SS":                  partial(self._add_charge, osMode=False, tag="SS"),
            "SAVE":                self.add_SAVE,
            "IMPORT":              self.add_IMPORT,
            "EVENTFLAG":           self.add_EVENTFLAG,
            "GLOBALTRIGMATCH":     self.add_GLOBALTRIGMATCH,
            "RUN_NUMBER":          self.add_RUNNUMBER,
            "EXPR":                self.add_EXPR_selector,
            "EVENTVAR":            self.add_EVENTVAR_selector,
        }
        for kw, (attr, tag, _noun) in self._NOBJECT.items():
            d[kw] = partial(self._add_nobject, attr=attr, tag=tag)
        return d

    # ------------------------------------------------------------------ #
    #  keyword specification (shared with the GUI and the parser)        #
    # ------------------------------------------------------------------ #

    @classmethod
    def _keywordSpecs(cls) -> dict:
        """Machine-readable description of every `selectionCuts` keyword.

        Spec fields:
          * `info` (str, required): one-line description of the keyword.
          * `args` (list): argument descriptors, in token order.
          * `forms` (list of arg-lists): used instead of `args` for keywords
            offering several fixed shapes that are not "required + optionals".
          * `freeText` (True): everything after the keyword is a single
            expression (EXPR only).
          * `grammar` (dict): the vocabulary of that expression (EXPR only).
          * `deprecated` (True): the keyword is deprecated (SAVE only).

        Argument descriptor fields:
          * `name` (str), `type` (str), and optionally `optional` (True),
            `choices` (list), `pattern` (regex str), `signed` (True).
          * `type` is one of 'str', 'float', 'int', 'sign', 'region', 'flag',
            'container', where:
              - 'sign'      is one of `<` `>` `==` `>=` `<=`
              - 'region'    is the `selectionName` of another EventSelection
              - 'flag'      is a literal token, present or absent; the literal
                            is the arg's `name`, matched case-insensitively
              - 'container' is a container reference, in the format `Name` or
                            `Name.selection`
          * `signed` (True) marks a float that may be negative; every other
            float goes through `check_float(requirePositive=True)`.

        Filling rule (implemented by `parseArgs`):
          1. `flag` arguments are lifted out of the token list first, by
             case-insensitive match against the arg name.
          2. Of what remains, required arguments are matched first; optional
             ones are filled left-to-right with the surplus tokens, skipping an
             optional whose `pattern` the candidate token does not match.
          3. With `forms`, the first form whose token count and patterns fit
             wins.
        """
        if cls._KEYWORD_SPECS is not None:
            return cls._KEYWORD_SPECS

        specs = {}
        for kw, (_attr, _tag, noun) in cls._NOBJECT.items():
            specs[kw] = {
                'info': f'Count {noun} above a pT threshold',
                'args': [
                    {'name': 'sel',   'type': 'str', 'optional': True},
                    {'name': 'ptmin', 'type': 'float'},
                    {'name': 'sign',  'type': 'sign'},
                    {'name': 'count', 'type': 'int'},
                ],
            }
        specs.update({
            'JET_N_BTAG': {
                'info': 'Count b-tagged jets above the default b-tagging working '
                        'point, or above a custom one given as `tagger:WP`',
                'args': [
                    {'name': 'sel',   'type': 'str', 'optional': True, 'pattern': '^[^:]+$'},
                    {'name': 'btag',  'type': 'str', 'optional': True, 'pattern': '^[^:]+:[^:]+$'},
                    {'name': 'sign',  'type': 'sign'},
                    {'name': 'count', 'type': 'int'},
                ],
            },
            'JET_N_GHOST': {
                'info': 'Count jets ghost-associated to a given particle, e.g. `B`, '
                        'or `B!C` to also veto a second ghost association',
                'args': [
                    {'name': 'ghost', 'type': 'str', 'pattern': '^[A-Za-z]+(![A-Za-z]+)?$'},
                    {'name': 'ptmin', 'type': 'float', 'optional': True},
                    {'name': 'sign',  'type': 'sign'},
                    {'name': 'count', 'type': 'int'},
                ],
            },
            'LJET_N_GHOST': {
                'info': 'Count large-R jets ghost-associated to a given particle, e.g. '
                        '`B`, or `B!C` to also veto a second ghost association',
                'args': [
                    {'name': 'ghost', 'type': 'str', 'pattern': '^[A-Za-z]+(![A-Za-z]+)?$'},
                    {'name': 'ptmin', 'type': 'float', 'optional': True},
                    {'name': 'sign',  'type': 'sign'},
                    {'name': 'count', 'type': 'int'},
                ],
            },
            'LJETMASS_N': {
                'info': 'Count large-R jets above a mass threshold',
                'args': [
                    {'name': 'sel',     'type': 'str', 'optional': True},
                    {'name': 'minMass', 'type': 'float'},
                    {'name': 'sign',    'type': 'sign'},
                    {'name': 'count',   'type': 'int'},
                ],
            },
            'LJETMASSWINDOW_N': {
                'info': 'Count large-R jets inside (or, with `veto`, outside) a mass window',
                'args': [
                    {'name': 'sel',      'type': 'str', 'optional': True},
                    {'name': 'lowMass',  'type': 'float'},
                    {'name': 'highMass', 'type': 'float'},
                    {'name': 'sign',     'type': 'sign'},
                    {'name': 'count',    'type': 'int'},
                    {'name': 'veto',     'type': 'flag', 'optional': True},
                ],
            },
            'OBJ_N': {
                'info': 'Count objects of an arbitrary container above a pT threshold',
                'args': [
                    {'name': 'container', 'type': 'container'},
                    {'name': 'ptmin',     'type': 'float'},
                    {'name': 'sign',      'type': 'sign'},
                    {'name': 'count',     'type': 'int'},
                ],
            },
            'SUM_EL_N_MU_N': {
                'info': 'Count electrons and muons together, above a common or '
                        'per-flavour pT threshold',
                'forms': [
                    [
                        {'name': 'ptmin', 'type': 'float'},
                        {'name': 'sign',  'type': 'sign'},
                        {'name': 'count', 'type': 'int'},
                    ],
                    [
                        {'name': 'ptEl',  'type': 'float'},
                        {'name': 'ptMu',  'type': 'float'},
                        {'name': 'sign',  'type': 'sign'},
                        {'name': 'count', 'type': 'int'},
                    ],
                    [
                        {'name': 'selEl', 'type': 'str'},
                        {'name': 'selMu', 'type': 'str'},
                        {'name': 'ptEl',  'type': 'float'},
                        {'name': 'ptMu',  'type': 'float'},
                        {'name': 'sign',  'type': 'sign'},
                        {'name': 'count', 'type': 'int'},
                    ],
                ],
            },
            'SUM_EL_N_MU_N_TAU_N': {
                'info': 'Count electrons, muons and tau-jets together, above a common '
                        'or per-flavour pT threshold',
                'forms': [
                    [
                        {'name': 'ptmin', 'type': 'float'},
                        {'name': 'sign',  'type': 'sign'},
                        {'name': 'count', 'type': 'int'},
                    ],
                    [
                        {'name': 'ptEl',  'type': 'float'},
                        {'name': 'ptMu',  'type': 'float'},
                        {'name': 'ptTau', 'type': 'float'},
                        {'name': 'sign',  'type': 'sign'},
                        {'name': 'count', 'type': 'int'},
                    ],
                    [
                        {'name': 'selEl',  'type': 'str'},
                        {'name': 'selMu',  'type': 'str'},
                        {'name': 'selTau', 'type': 'str'},
                        {'name': 'ptEl',   'type': 'float'},
                        {'name': 'ptMu',   'type': 'float'},
                        {'name': 'ptTau',  'type': 'float'},
                        {'name': 'sign',   'type': 'sign'},
                        {'name': 'count',  'type': 'int'},
                    ],
                ],
            },
            'MET': {
                'info': 'Cut on the missing transverse energy',
                'args': [
                    {'name': 'sign',   'type': 'sign'},
                    {'name': 'refMET', 'type': 'float'},
                ],
            },
            'MWT': {
                'info': 'Cut on the transverse mass of the leading lepton and MET',
                'args': [
                    {'name': 'sign',   'type': 'sign'},
                    {'name': 'refMWT', 'type': 'float'},
                ],
            },
            'MET+MWT': {
                'info': 'Cut on the sum of the missing transverse energy and the '
                        'transverse mass',
                'args': [
                    {'name': 'sign',      'type': 'sign'},
                    {'name': 'refMETMWT', 'type': 'float'},
                ],
            },
            'MLL': {
                'info': 'Cut on the dilepton invariant mass',
                'args': [
                    {'name': 'sign',   'type': 'sign'},
                    {'name': 'refMLL', 'type': 'float'},
                ],
            },
            'MLLWINDOW': {
                'info': 'Require the dilepton invariant mass inside (or, with `veto`, '
                        'outside) a mass window',
                'args': [
                    {'name': 'lowMLL',  'type': 'float'},
                    {'name': 'highMLL', 'type': 'float'},
                    {'name': 'veto',    'type': 'flag', 'optional': True},
                ],
            },
            'MLL_OSSF': {
                'info': 'Require the opposite-sign same-flavour dilepton invariant mass '
                        'inside (or, with `veto`, outside) a mass window',
                'args': [
                    {'name': 'lowMll',  'type': 'float'},
                    {'name': 'highMll', 'type': 'float'},
                    {'name': 'veto',    'type': 'flag', 'optional': True},
                ],
            },
            'OS': {
                'info': 'Require an opposite-sign lepton pair; without any flag, all '
                        'available lepton flavours are considered',
                'args': [
                    {'name': 'el',  'type': 'flag', 'optional': True},
                    {'name': 'mu',  'type': 'flag', 'optional': True},
                    {'name': 'tau', 'type': 'flag', 'optional': True},
                ],
            },
            'SS': {
                'info': 'Require a same-sign lepton pair; without any flag, all '
                        'available lepton flavours are considered',
                'args': [
                    {'name': 'el',  'type': 'flag', 'optional': True},
                    {'name': 'mu',  'type': 'flag', 'optional': True},
                    {'name': 'tau', 'type': 'flag', 'optional': True},
                ],
            },
            'IMPORT': {
                'info': 'Import all the cuts of a previously defined event selection',
                'args': [
                    {'name': 'region', 'type': 'region'},
                ],
            },
            'EVENTFLAG': {
                'info': 'Require an existing event-wise decoration to be true',
                'args': [
                    {'name': 'decoration', 'type': 'str'},
                ],
            },
            'GLOBALTRIGMATCH': {
                'info': 'Require the global trigger matching decision, optionally for '
                        'a given trigger-configuration postfix',
                'args': [
                    {'name': 'postfix', 'type': 'str', 'optional': True},
                ],
            },
            'RUN_NUMBER': {
                'info': 'Cut on the (random) run number',
                'args': [
                    {'name': 'sign',      'type': 'sign'},
                    {'name': 'runNumber', 'type': 'int'},
                ],
            },
            'EVENTVAR': {
                'info': 'Cut on an existing EventInfo scalar variable, e.g. a DNN or '
                        'BDT discriminant',
                'args': [
                    {'name': 'type',  'type': 'str', 'choices': sorted(cls._EVENTVAR_TYPES)},
                    {'name': 'name',  'type': 'str'},
                    {'name': 'sign',  'type': 'sign'},
                    {'name': 'value', 'type': 'float', 'signed': True},
                ],
            },
            'EXPR': {
                'info': 'Cut on a generic object-kinematic expression, e.g. '
                        '`dR(el[0],jet[0]) > 0.4`',
                'freeText': True,
                'grammar': {
                    'collections': sorted(cls._EXPR_COLL),
                    'variables': sorted(cls._EXPR_VARS),
                },
            },
            'SAVE': {
                'info': 'Deprecated and ignored: the event filter is now emitted '
                        'automatically at the end of every event selection',
                'deprecated': True,
                'args': [],
            },
        })
        cls._KEYWORD_SPECS = specs
        return cls._KEYWORD_SPECS

    @classmethod
    def keywordSpecs(cls) -> dict:
        """Return an independent copy of the keyword specification table."""
        return deepcopy(cls._keywordSpecs())

    @classmethod
    def _argCounts(cls, spec):
        """Return the set of valid total token counts (leading keyword included)
        allowed by a keyword specification, or None when the keyword takes free
        text and no count check applies."""
        if spec.get('freeText'):
            return None
        forms = spec.get('forms') or [spec.get('args', [])]
        counts = set()
        for form in forms:
            min_args = sum(1 for arg in form if not arg.get('optional'))
            max_args = len(form)
            counts.update(n + 1 for n in range(min_args, max_args + 1))
        return counts

    def makeAlgs(self, config):
        existing = config.getContainerMeta('EventInfo', 'eventSelectionNames', defaultValue=[])
        config.setContainerMeta('EventInfo', 'eventSelectionNames',
                                existing + [f'pass_{self.selectionName}_%SYS%'], allowOverwrite=True)

        # need to re-initialize here to deal with multiple passes
        self.step = 0
        # initialize the pre-selection
        self.currentDecoration = self.preselection
        # re-initialize the cutflow
        self.cutflow = []
        # read the selection cuts
        if self.selectionCuts is None:
            raise ValueError ("[EventSelectionConfig] You must provide the 'selectionCuts' option to 'EventSelectionConfig': "
                              "a single string where each line represents a different selection cut to apply in order.")
        for line in self.selectionCuts.split("\n"):
            self.interpret(line, config)
        # the event filter is always created automatically at the end of the
        # block; an explicit SAVE line only triggers a deprecation warning
        self._emit_save(config)
        config.addEventCutFlow(self.selectionName, self.getCutflow())

    def interpret(self, text, cfg):
        text = text.strip()
        if not text or text.startswith("#"):
            return
        self.step += 1
        keyword = text.split()[0]
        handler = self._dispatch.get(keyword)
        if handler is None:
            raise ValueError (f"[EventSelectionConfig] The following selection cut is not recognised! --> {text}")
        handler(text, cfg)

    # ------------------------------------------------------------------ #
    #  validation helpers                                                #
    # ------------------------------------------------------------------ #

    def raise_misconfig(self, text, keyword):
        raise ValueError (f"[EventSelectionConfig] Misconfiguration! Check {keyword} in: {text}")

    def raise_missinginput(self, collection):
        raise ValueError (f"[EventSelectionConfig] Misconfiguration! Missing input collection for {collection}")

    def _check_args(self, items, keyword):
        """Validate the leading keyword and the number of arguments, the latter
        derived from the keyword specification table."""
        if items[0] != keyword:
            self.raise_misconfig(' '.join(items), keyword)
        validCounts = self._argCounts(self._keywordSpecs()[keyword])
        if validCounts is not None and len(items) not in validCounts:
            self.raise_misconfig(' '.join(items), "number of arguments")

    def _extract_flags(self, form, tokens):
        """Remove flag tokens from `tokens` and return their boolean values."""
        tokens = list(tokens)
        values = {}
        for arg in form:
            if arg.get('type') != 'flag':
                continue
            values[arg['name']] = False
            for i, token in enumerate(tokens):
                if token.lower() == arg['name'].lower():
                    del tokens[i]
                    values[arg['name']] = True
                    break
        return tokens, values

    def _match_form(self, form, tokens):
        """Match one keyword argument form, returning values or None."""
        tokens, values = self._extract_flags(form, tokens)
        positional = [arg for arg in form if arg.get('type') != 'flag']
        nRequired = sum(1 for arg in positional if not arg.get('optional'))
        if len(tokens) < nRequired or len(tokens) > len(positional):
            return None

        budget = len(tokens) - nRequired
        cursor = 0
        for arg in positional:
            if not arg.get('optional'):
                values[arg['name']] = tokens[cursor]
                cursor += 1
                continue
            pattern = arg.get('pattern')
            if budget > 0 and (pattern is None or re.fullmatch(pattern, tokens[cursor])):
                values[arg['name']] = tokens[cursor]
                cursor += 1
                budget -= 1
            else:
                values[arg['name']] = ''
        return values if cursor == len(tokens) else None

    def parseArgs(self, keyword, items):
        """Turn the already-split token list `items` (leading keyword included)
        into a dict of argument name -> value, driven by `keywordSpecs`, which
        is authoritative for the argument grammar."""
        self._check_args(items, keyword)
        spec = self._keywordSpecs()[keyword]
        if spec.get('freeText'):
            return {'text': ' '.join(items[1:])}
        for form in (spec.get('forms') or [spec.get('args', [])]):
            values = self._match_form(form, items[1:])
            if values is not None:
                return values
        self.raise_misconfig(' '.join(items), "number of arguments")

    def check_float(self, test, requirePositive=True):
        try:
            value = float(test)
            if not requirePositive or value >= 0:
                return value
            else:
                raise ValueError (f"[EventSelectionConfig] Misconfiguration! Float {test} is not positive!")
        except ValueError:
            raise ValueError (f"[EventSelectionConfig] Misconfiguration! {test} should be a float, not {type(test)}!")

    def check_int(self, test, requirePositive=True):
        try:
            numeric = float(test)
            value = int(numeric)
        except (TypeError, ValueError):
            raise ValueError (f"[EventSelectionConfig] Misconfiguration! {test} should be an int, not {type(test)}")
        if value != numeric:
            raise ValueError (f"[EventSelectionConfig] Misconfiguration! {test} should be an int, not a float!")
        if requirePositive and value < 0:
            raise ValueError (f"[EventSelectionConfig] Misconfiguration! Int {test} is not positive!")
        return value

    def check_string(self, test):
        if not isinstance(test, str):
            raise ValueError (f"[EventSelectionConfig] Misconfiguration! {test} should be a string, not a number!")
        else:
            return test

    def check_sign(self, test):
        mapping = {
            "<" : "LT",
            ">" : "GT",
            "==": "EQ",
            ">=": "GE",
            "<=": "LE"
        }
        try:
            return mapping[test]
        except KeyError:
            raise KeyError (f"[EventSelectionConfig] Misconfiguration! {test} should be one of {list(mapping.keys())}")

    def check_btagging(self, test):
        test = test.split(":")
        if len(test) != 2:
            raise ValueError (f"[EventSelectionConfig] Misconfiguration! {test} should be provided as 'btagger:btagWP'")
        else:
            return test

    def check_ghosts(self, test):
        test = self.check_string(test)
        values = test.split("!")
        ghost_map = {
            "B": "GhostBHadronsFinalCount",
            "C": "GhostCHadronsFinalCount",
            "T": "GhostTQuarksFinalCount",
            "W": "GhostWBosonsCount",
            "Z": "GhostZBosonsCount",
            "H": "GhostHBosonsCount",
            "TAU": "GhostTausFinalCount"
        }
        return [ghost_map.get(value.upper(), value) for value in values]

    # ------------------------------------------------------------------ #
    #  decoration / selection bookkeeping                                #
    # ------------------------------------------------------------------ #

    def getCutflow(self):
        return self.cutflow

    def setDecorationName(self, algorithm, config, decoration):
        self.cutflow.append( decoration )
        if algorithm is not None:
            algorithm.decorationName = f'{decoration},as_char'
            self.currentDecoration = decoration
            if self.debugMode:
                config.addOutputVar('EventInfo', decoration, decoration.split("_%SYS%")[0])
        else:
            if self.currentDecoration:
                self.currentDecoration += '&&' + decoration
            else:
                self.currentDecoration = decoration
        config.addSelection('EventInfo', '', decoration)

    def checkDecorationName(self, decoration):
        if decoration == '':
            return decoration
        decoration = decoration.split("&&")
        decoration = [sub + ',as_char' if ',as_char' not in sub else sub for sub in decoration]
        return '&&'.join(decoration)

    def extendObjectSelection(self, config, container, oldSelection, newSelection):
        if oldSelection:
            return oldSelection + "&&" + config.getFullSelection(container, newSelection)
        else:
            return config.getFullSelection(container, newSelection)

    # ------------------------------------------------------------------ #
    #  shared selector helpers                                           #
    # ------------------------------------------------------------------ #

    def _require_inputs(self, *names, message=None):
        """Require at least one of the named input options to be configured."""
        if not any(getattr(self, name) for name in names):
            self.raise_missinginput(message or " or ".join(names))

    def _finish_selector(self, alg, config, name):
        """Apply the common event-preselection and output decoration."""
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        self.setDecorationName(alg, config, f'{name}_%SYS%')

    def _configure_leptons(self, alg, config, leptons):
        """Configure reco/truth lepton handles from explicit metadata."""
        for spec, reco, truth in leptons:
            if spec:
                self._route_lepton(alg, config, spec, reco, truth)
        self._maybe_dressed(alg, *(spec for spec, _reco, _truth in leptons))

    def _maybe_dressed(self, alg, *specs):
        """Enable dressed kinematics when any of the given electron/muon
        containers is a truth container. Dressed kinematics only exist for
        truth electrons and muons, so only those specs should be passed here."""
        if any(spec and ("Particle" in spec or "Truth" in spec) for spec in specs):
            alg.useDressedProperties = self.useDressedProperties

    def _route_lepton(self, alg, config, spec, reco, truth):
        """Assign (name, selection) to the reco or truth handles of `alg`
        depending on whether `spec` points to a truth container.
        `reco`/`truth` are (nameAttr, selectionAttr) pairs."""
        name, sel = config.readNameAndSelection(spec)
        nameAttr, selAttr = truth if ("Particle" in spec or "Truth" in spec) else reco
        setattr(alg, nameAttr, name)
        setattr(alg, selAttr, sel)

    # ------------------------------------------------------------------ #
    #  selector builders                                                 #
    # ------------------------------------------------------------------ #

    def _add_nobject(self, text, config, *, attr, tag):
        """Generic builder for the N-object pT selectors (EL_N, MU_N, JET_N,
        PH_N, TAU_N, LJET_N): identical except for the source container, which
        is always required since the cut acts on it."""
        items = text.split()
        spec = getattr(self, attr)
        self._require_inputs(attr)
        # the keyword is not known to this generic builder: read it back
        args = self.parseArgs(items[0], items)
        thisalg = f'{self.selectionName}_{tag}_{self.step}'
        alg = config.createAlgorithm('CP::NObjectPtSelectorAlg', thisalg)
        alg.particles, alg.objectSelection = config.readNameAndSelection(spec)
        if attr in ("electrons", "muons"):
            self._maybe_dressed(alg, spec)
        if args['sel']:
            alg.objectSelection = self.extendObjectSelection(
                config, spec.split(".")[0], alg.objectSelection,
                self.check_string(args['sel']))
        alg.minPt = self.check_float(args['ptmin'])
        alg.sign  = self.check_sign(args['sign'])
        alg.count = self.check_int(args['count'])
        self._finish_selector(alg, config, thisalg)

    def add_IMPORT(self, text, config):
        # this is used to import a previous selection
        args = self.parseArgs("IMPORT", text.split())
        region = self.check_string(args['region'])
        if not self.currentDecoration:
            self.currentDecoration = f'pass_{region}_%SYS%,as_char'
        else:
            self.currentDecoration = f'{self.currentDecoration},as_char&&pass_{region}_%SYS%'
        # for the cutflow, we need to retrieve all the cuts corresponding to this IMPORT
        imported_cuts = [cut for cut in config.getSelectionCutFlow('EventInfo', '') if cut.startswith(region)]
        self.cutflow += imported_cuts

    def add_NBJET_selector(self, text, config):
        args = self.parseArgs("JET_N_BTAG", text.split())
        self._require_inputs("jets")
        thisalg = f'{self.selectionName}_NBJET_{self.step}'
        alg = config.createAlgorithm('CP::NObjectPtSelectorAlg', thisalg)
        particles, selection = config.readNameAndSelection(self.jets)
        alg.particles = particles
        alg.objectSelection = f'{selection}&&{self.btagDecoration},as_char' if selection else f'{self.btagDecoration},as_char'
        if args['btag']:
            btagger, btagWP = self.check_btagging(args['btag'])
            customBtag = f'ftag_select_{btagger}_{btagWP}'
            alg.objectSelection = f'{selection}&&{customBtag},as_char' if selection else f'{customBtag},as_char'
        if args['sel']:
            extraSel = self.check_string(args['sel'])
            alg.objectSelection = self.extendObjectSelection(config, self.jets.split(".")[0], alg.objectSelection, extraSel)
        alg.sign  = self.check_sign(args['sign'])
        alg.count = self.check_int(args['count'])
        self._finish_selector(alg, config, thisalg)

    def add_SUMNELNMU_selector(self, text, config):
        args = self.parseArgs("SUM_EL_N_MU_N", text.split())
        self._require_inputs("electrons", "muons")
        thisalg = f'{self.selectionName}_SUMNELNMU_{self.step}'
        alg = config.createAlgorithm('CP::SumNLeptonPtSelectorAlg', thisalg)
        alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        self._maybe_dressed(alg, self.electrons, self.muons)
        if args.get('ptmin') is not None:
            # a single threshold, applied to every flavour
            alg.minPtEl = self.check_float(args['ptmin'])
            alg.minPtMu = self.check_float(args['ptmin'])
        else:
            if args.get('selEl'):
                extraSelEl = self.check_string(args['selEl'])
                alg.electronSelection = self.extendObjectSelection(config, self.electrons.split(".")[0], alg.electronSelection, extraSelEl)
            if args.get('selMu'):
                extraSelMu = self.check_string(args['selMu'])
                alg.muonSelection = self.extendObjectSelection(config, self.muons.split(".")[0], alg.muonSelection, extraSelMu)
            alg.minPtEl = self.check_float(args['ptEl'])
            alg.minPtMu = self.check_float(args['ptMu'])
        alg.sign  = self.check_sign(args['sign'])
        alg.count = self.check_int(args['count'])
        self._finish_selector(alg, config, thisalg)

    def add_SUMNLEPTONS_selector(self, text, config):
        args = self.parseArgs("SUM_EL_N_MU_N_TAU_N", text.split())
        self._require_inputs("electrons", "muons", "taus", message="electrons, muons or taus")
        thisalg = f'{self.selectionName}_SUMNLEPTONS_{self.step}'
        alg = config.createAlgorithm('CP::SumNLeptonPtSelectorAlg', thisalg)
        alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        alg.taus, alg.tauSelection = config.readNameAndSelection(self.taus)
        self._maybe_dressed(alg, self.electrons, self.muons)
        if args.get('ptmin') is not None:
            # a single threshold, applied to every flavour
            alg.minPtEl = self.check_float(args['ptmin'])
            alg.minPtMu = self.check_float(args['ptmin'])
            alg.minPtTau = self.check_float(args['ptmin'])
        else:
            if args.get('selEl'):
                extraSelEl = self.check_string(args['selEl'])
                alg.electronSelection = self.extendObjectSelection(config, self.electrons.split(".")[0], alg.electronSelection, extraSelEl)
            if args.get('selMu'):
                extraSelMu = self.check_string(args['selMu'])
                alg.muonSelection = self.extendObjectSelection(config, self.muons.split(".")[0], alg.muonSelection, extraSelMu)
            if args.get('selTau'):
                extraSelTau = self.check_string(args['selTau'])
                alg.tauSelection = self.extendObjectSelection(config, self.taus.split(".")[0], alg.tauSelection, extraSelTau)
            alg.minPtEl = self.check_float(args['ptEl'])
            alg.minPtMu = self.check_float(args['ptMu'])
            alg.minPtTau = self.check_float(args['ptTau'])
        alg.sign  = self.check_sign(args['sign'])
        alg.count = self.check_int(args['count'])
        self._finish_selector(alg, config, thisalg)

    def add_NLJETMASS_selector(self, text, config):
        args = self.parseArgs("LJETMASS_N", text.split())
        thisalg = f'{self.selectionName}_NLJETMASS_{self.step}'
        alg = config.createAlgorithm('CP::NObjectMassSelectorAlg', thisalg)
        alg.particles, alg.objectSelection = config.readNameAndSelection(self.largeRjets)
        if args['sel']:
            alg.objectSelection = self.extendObjectSelection(
                config, self.largeRjets.split(".")[0], alg.objectSelection,
                self.check_string(args['sel']))
        alg.minMass = self.check_float(args['minMass'])
        alg.sign    = self.check_sign(args['sign'])
        alg.count   = self.check_int(args['count'])
        self._finish_selector(alg, config, thisalg)

    def add_NLJETMASSWINDOW_selector(self, text, config):
        args = self.parseArgs("LJETMASSWINDOW_N", text.split())
        thisalg = f'{self.selectionName}_NLJETMASSWINDOW_{self.step}'
        alg = config.createAlgorithm('CP::NLargeRJetMassWindowSelectorAlg', thisalg)
        alg.ljets, alg.ljetSelection = config.readNameAndSelection(self.largeRjets)
        if args['sel']:
            extraSel = self.check_string(args['sel'])
            alg.ljetSelection = self.extendObjectSelection(config, self.largeRjets.split(".")[0], alg.ljetSelection, extraSel)
        alg.lowMass  = self.check_float(args['lowMass'])
        alg.highMass = self.check_float(args['highMass'])
        alg.sign     = self.check_sign(args['sign'])
        alg.count    = self.check_int(args['count'])
        alg.vetoMode = args['veto']
        self._finish_selector(alg, config, thisalg)

    def add_NJETGHOST_selector(self, text, config):
        args = self.parseArgs("JET_N_GHOST", text.split())
        thisalg = f'{self.selectionName}_NJETGHOST_{self.step}'
        alg = config.createAlgorithm('CP::JetNGhostSelectorAlg', thisalg)
        alg.jets, alg.jetSelection = config.readNameAndSelection(self.jets)
        ghosts = self.check_ghosts(args['ghost'])
        alg.ghost = ghosts[0]
        if len(ghosts) > 1 :
            alg.veto = ghosts[1]
        if args['ptmin']:
            alg.minPt = self.check_float(args['ptmin'])
        alg.sign  = self.check_sign(args['sign'])
        alg.count = self.check_int(args['count'])
        self._finish_selector(alg, config, thisalg)

    def add_NLJETGHOST_selector(self, text, config):
        args = self.parseArgs("LJET_N_GHOST", text.split())
        thisalg = f'{self.selectionName}_NLJETGHOST_{self.step}'
        alg = config.createAlgorithm('CP::JetNGhostSelectorAlg', thisalg)
        alg.jets, alg.jetSelection = config.readNameAndSelection(self.largeRjets)
        ghosts = self.check_ghosts(args['ghost'])
        alg.ghost = ghosts[0]
        if len(ghosts) > 1 :
            alg.veto = ghosts[1]
        if args['ptmin']:
            alg.minPt = self.check_float(args['ptmin'])
        alg.sign  = self.check_sign(args['sign'])
        alg.count = self.check_int(args['count'])
        self._finish_selector(alg, config, thisalg)

    def add_NOBJ_selector(self, text, config):
        args = self.parseArgs("OBJ_N", text.split())
        thisalg = f'{self.selectionName}_NOBJ_{self.step}'
        alg = config.createAlgorithm('CP::NObjectPtSelectorAlg', thisalg)
        alg.particles, alg.objectSelection = config.readNameAndSelection(self.check_string(args['container']))
        alg.minPt = self.check_float(args['ptmin'])
        alg.sign  = self.check_sign(args['sign'])
        alg.count = self.check_int(args['count'])
        self._finish_selector(alg, config, thisalg)

    def add_MET_selector(self, text, config):
        args = self.parseArgs("MET", text.split())
        self._require_inputs("met", message="MET")
        thisalg = f'{self.selectionName}_MET_{self.step}'
        alg = config.createAlgorithm('CP::MissingETSelectorAlg', thisalg)
        alg.met = config.readName(self.met)
        alg.metTerm = self.metTerm
        alg.sign = self.check_sign(args['sign'])
        alg.refMET = self.check_float(args['refMET'])
        self._finish_selector(alg, config, thisalg)

    def add_MWT_selector(self, text, config):
        args = self.parseArgs("MWT", text.split())
        self._require_inputs("electrons", "muons")
        thisalg = f'{self.selectionName}_MWT_{self.step}'
        alg = config.createAlgorithm('CP::TransverseMassSelectorAlg', thisalg)
        alg.met = config.readName(self.met)
        alg.metTerm = self.metTerm
        alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        self._maybe_dressed(alg, self.electrons, self.muons)
        alg.sign = self.check_sign(args['sign'])
        alg.refMWT = self.check_float(args['refMWT'])
        self._finish_selector(alg, config, thisalg)

    def add_METMWT_selector(self, text, config):
        args = self.parseArgs("MET+MWT", text.split())
        self._require_inputs("met", message="MET")
        self._require_inputs("electrons", "muons")
        thisalg = f'{self.selectionName}_METMWT_{self.step}'
        alg = config.createAlgorithm('CP::MissingETPlusTransverseMassSelectorAlg', thisalg)
        alg.met = config.readName(self.met)
        alg.metTerm = self.metTerm
        alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        self._maybe_dressed(alg, self.electrons, self.muons)
        alg.sign = self.check_sign(args['sign'])
        alg.refMETMWT = self.check_float(args['refMETMWT'])
        self._finish_selector(alg, config, thisalg)

    def add_MLL_selector(self, text, config):
        args = self.parseArgs("MLL", text.split())
        self._require_inputs("electrons", "muons")
        thisalg = f'{self.selectionName}_MLL_{self.step}'
        alg = config.createAlgorithm('CP::DileptonInvariantMassSelectorAlg', thisalg)
        self._configure_leptons(alg, config, (
            (self.electrons, ('electrons', 'electronSelection'),
             ('truthElectrons', 'truthElectronSelection')),
            (self.muons, ('muons', 'muonSelection'),
             ('truthMuons', 'truthMuonSelection')),
        ))
        alg.sign = self.check_sign(args['sign'])
        alg.refMLL = self.check_float(args['refMLL'])
        self._finish_selector(alg, config, thisalg)

    def add_MLLWINDOW_selector(self, text, config):
        args = self.parseArgs("MLLWINDOW", text.split())
        self._require_inputs("electrons", "muons")
        thisalg = f'{self.selectionName}_MLLWINDOW_{self.step}'
        alg = config.createAlgorithm('CP::DileptonInvariantMassWindowSelectorAlg', thisalg)
        self._configure_leptons(alg, config, (
            (self.electrons, ('electrons', 'electronSelection'),
             ('truthElectrons', 'truthElectronSelection')),
            (self.muons, ('muons', 'muonSelection'),
             ('truthMuons', 'truthMuonSelection')),
        ))
        alg.lowMLL = self.check_float(args['lowMLL'])
        alg.highMLL = self.check_float(args['highMLL'])
        alg.vetoMode = args['veto']
        self._finish_selector(alg, config, thisalg)

    def _add_charge(self, text, config, *, osMode, tag):
        """Builder shared by OS and SS: same algorithm, opposite charge mode."""
        args = self.parseArgs(tag, text.split())
        self._require_inputs("electrons", "muons", "taus")
        thisalg = f'{self.selectionName}_{tag}_{self.step}'
        alg = config.createAlgorithm('CP::ChargeSelectorAlg', thisalg)
        allLeptons = not (args['el'] or args['mu'] or args['tau'])
        if self.electrons and (allLeptons or args['el']):
            self._route_lepton(alg, config, self.electrons,
                               ('electrons', 'electronSelection'),
                               ('truthElectrons', 'truthElectronSelection'))
        if self.muons and (allLeptons or args['mu']):
            self._route_lepton(alg, config, self.muons,
                               ('muons', 'muonSelection'),
                               ('truthMuons', 'truthMuonSelection'))
        if self.taus and (allLeptons or args['tau']):
            self._route_lepton(alg, config, self.taus,
                               ('taus', 'tauSelection'),
                               ('truthTaus', 'truthTauSelection'))
        alg.OS = osMode
        self._finish_selector(alg, config, thisalg)

    def add_MLL_OSSF_selector(self, text, config):
        args = self.parseArgs("MLL_OSSF", text.split())
        self._require_inputs("electrons", "muons")
        thisalg = f'{self.selectionName}_MLL_OSSF_{self.step}'
        alg = config.createAlgorithm('CP::DileptonOSSFInvariantMassWindowSelectorAlg', thisalg)
        self._configure_leptons(alg, config, (
            (self.electrons, ('electrons', 'electronSelection'),
             ('truthElectrons', 'truthElectronSelection')),
            (self.muons, ('muons', 'muonSelection'),
             ('truthMuons', 'truthMuonSelection')),
        ))
        alg.lowMll = self.check_float(args['lowMll'])
        alg.highMll = self.check_float(args['highMll'])
        alg.vetoMode = args['veto']
        self._finish_selector(alg, config, thisalg)

    def add_EVENTFLAG(self, text, config):
        args = self.parseArgs("EVENTFLAG", text.split())
        existingDecoration = self.check_string(args['decoration'])
        self.setDecorationName(None, config, existingDecoration)

    def add_GLOBALTRIGMATCH(self, text, config):
        args = self.parseArgs("GLOBALTRIGMATCH", text.split())
        postfix = self.check_string(args['postfix'])
        self.setDecorationName(None, config, f"globalTriggerMatch{postfix}_%SYS%,as_char")

    def add_RUNNUMBER(self, text, config):
        args = self.parseArgs("RUN_NUMBER", text.split())
        thisalg = f'{self.selectionName}_RUN_NUMBER_{self.step}'
        alg = config.createAlgorithm('CP::RunNumberSelectorAlg', thisalg)
        alg.sign = self.check_sign(args['sign'])
        alg.runNumber = self.check_int(args['runNumber'])
        alg.useRandomRunNumber = config.dataType() is not DataType.Data
        self._finish_selector(alg, config, thisalg)

    # ------------------------------------------------------------------ #
    #  EXPR: generic object-kinematic expression cuts                    #
    # ------------------------------------------------------------------ #

    # collection token -> metadata
    _EXPR_COLL = {
        "jet":  _ExprCollection("jets"),
        "bjet": _ExprCollection("jets", apply_btag=True),
        "el":   _ExprCollection("electrons"),
        "mu":   _ExprCollection("muons"),
        "tau":  _ExprCollection("taus"),
        "ph":   _ExprCollection("photons"),
        "ljet": _ExprCollection("largeRjets"),
        "met":  _ExprCollection("met", is_met=True),
    }

    # variable -> metadata
    _EXPR_VARS = {
        "dR":   _ExprVariable(2, 2, ",", True, False),
        "dEta": _ExprVariable(2, 2, ",", True, False),
        "dPhi": _ExprVariable(2, 2, ",", False, True),
        "m":    _ExprVariable(1, None, "+", False, False),
        "e":    _ExprVariable(1, None, "+", False, False),
        "pt":   _ExprVariable(1, None, "+", False, True),
        "eta":  _ExprVariable(1, 1, None, True, False),
        "phi":  _ExprVariable(1, 1, None, False, True),
    }

    _EXPR_TOKEN_RE = re.compile(r"""
          (?P<NUM>\d+\.\d+(?:[eE][+-]?\d+)?|\d+[eE][+-]?\d+|\d+)
        | (?P<GE>>=) | (?P<LE><=) | (?P<EQ>==) | (?P<LT><) | (?P<GT>>)
        | (?P<ID>[A-Za-z_][A-Za-z0-9_]*)
        | (?P<LP>\() | (?P<RP>\)) | (?P<LB>\[) | (?P<RB>\])
        | (?P<COMMA>,) | (?P<PLUS>\+) | (?P<MINUS>-)
        | (?P<WS>\s+)
    """, re.VERBOSE)


    def _expr_tokenize(self, text):
        return _ExprParser(self._EXPR_TOKEN_RE).tokenize(text)

    def _expr_parse(self, tokens):
        return _ExprParser(self._EXPR_TOKEN_RE).parse(tokens)

    def _expr_validate(self, variable, separator, operands):
        if variable not in self._EXPR_VARS:
            raise UnavailableFeatureError(
                f"[EventSelectionConfig] EXPR: variable '{variable}' is not available. "
                "Please request it from the EventSelectionAlgorithms developers.")
        var_spec = self._EXPR_VARS[variable]
        minN = var_spec.min_operands
        maxN = var_spec.max_operands
        sep = var_spec.separator
        metOk = var_spec.met_ok
        n = len(operands)
        if n < minN or (maxN is not None and n > maxN):
            expected = f"{minN}" if maxN == minN else (f"{minN}+" if maxN is None else f"{minN}-{maxN}")
            raise InconsistentSettingsError(
                f"[EventSelectionConfig] EXPR: '{variable}' takes {expected} operand(s), got {n}")
        if n > 1 and separator != sep:
            want = {",": "','", "+": "'+'"}.get(sep, str(sep))
            raise InconsistentSettingsError(
                f"[EventSelectionConfig] EXPR: '{variable}' operands must be separated by {want}")
        for coll, index in operands:
            if coll not in self._EXPR_COLL:
                raise UnavailableFeatureError(
                    f"[EventSelectionConfig] EXPR: collection '{coll}' is not available. "
                    "Please request it from the EventSelectionAlgorithms developers.")
            coll_spec = self._EXPR_COLL[coll]
            isMET = coll_spec.is_met
            if isMET:
                if not metOk:
                    raise InconsistentSettingsError(
                        f"[EventSelectionConfig] EXPR: 'met' is not valid for '{variable}'")
                if index is not None:
                    raise InconsistentSettingsError(
                        "[EventSelectionConfig] EXPR: 'met' cannot be indexed")
                if separator == "+":
                    raise InconsistentSettingsError(
                        "[EventSelectionConfig] EXPR: 'met' cannot be combined in a sum")
            elif index is None:
                raise InconsistentSettingsError(
                    f"[EventSelectionConfig] EXPR: '{coll}' must be indexed, e.g. {coll}[0]")

    def add_EXPR_selector(self, text, config):
        body = text[len("EXPR"):].strip()
        if not body:
            self.raise_misconfig(text, "EXPR expression")
        variable, separator, operands, sign, refValue = self._expr_parse(self._expr_tokenize(body))
        self._expr_validate(variable, separator, operands)

        thisalg = f'{self.selectionName}_EXPR_{self.step}'
        alg = config.createAlgorithm('CP::ObjectKinematicSelectorAlg', thisalg)
        alg.variable = variable
        alg.sign = sign
        alg.refValue = refValue

        operandKinds, collections, selections, indices = [], [], [], []
        usesMET = False
        for coll, index in operands:
            coll_spec = self._EXPR_COLL[coll]
            opt = coll_spec.option
            applyBtag = coll_spec.apply_btag
            isMET = coll_spec.is_met
            container = getattr(self, opt)
            if not container:
                self.raise_missinginput(opt)
            if isMET:
                operandKinds.append("MET")
                usesMET = True
                continue
            operandKinds.append("PARTICLE")
            name, selection = config.readNameAndSelection(container)
            if applyBtag:
                if not self.btagDecoration:
                    self.raise_missinginput("btagDecoration")
                selection = (f'{selection}&&{self.btagDecoration},as_char'
                             if selection else f'{self.btagDecoration},as_char')
            collections.append(name)
            selections.append(selection)
            indices.append(index)

        alg.operandKinds = operandKinds
        alg.collections = collections
        alg.selections = selections
        alg.indices = indices
        if usesMET:
            if not self.met:
                self.raise_missinginput("met")
            alg.met = config.readName(self.met)
            alg.metTerm = self.metTerm
        self._finish_selector(alg, config, thisalg)

    # EventInfo scalar (e.g. a DNN/BDT discriminant) -> threshold cut.
    # The configurable stored type maps to the matching typed handle on the alg;
    # the Python side populates exactly one of them.
    _EVENTVAR_TYPES = {"float": "floatVariable", "int": "intVariable", "double": "doubleVariable"}

    def add_EVENTVAR_selector(self, text, config):
        # EVENTVAR <type> <varname> <sign> <threshold>
        args = self.parseArgs("EVENTVAR", text.split())
        valueType = self.check_string(args['type'])
        if valueType not in self._EVENTVAR_TYPES:
            self.raise_misconfig(text, f"value type (one of {sorted(self._EVENTVAR_TYPES)})")
        varname = self.check_string(args['name'])
        thisalg = f'{self.selectionName}_EVENTVAR_{self.step}'
        alg = config.createAlgorithm('CP::EventScalarSelectorAlg', thisalg)
        # the variable name is given bare; the systematics suffix is appended here
        setattr(alg, self._EVENTVAR_TYPES[valueType], f'{varname}_%SYS%')
        alg.sign = self.check_sign(args['sign'])
        alg.refValue = self.check_float(args['value'], requirePositive=False)  # discriminants may be negative
        self._finish_selector(alg, config, thisalg)

    def add_SAVE(self, text, config):
        # SAVE is deprecated: the event filter is now emitted automatically at
        # the end of the block (see makeAlgs). The keyword is accepted only to
        # warn existing configs; it performs no operation itself.
        self.parseArgs("SAVE", text.split())
        warnings.warn(
            "[EventSelectionConfig] The 'SAVE' keyword is deprecated: the event "
            "filter is now created automatically at the end of each EventSelection "
            f"block. Please remove the 'SAVE' line from selection '{self.selectionName}'.",
            category=ConfigDeprecationWarning, stacklevel=2)

    def _emit_save(self, config):
        """Create the SaveFilterAlg that turns the accumulated event selection
        into a named, persisted selection (and ntuple branch). Called once per
        block, automatically at the end of makeAlgs."""
        thisalg = f'{self.selectionName}_SAVE'
        alg = config.createAlgorithm('CP::SaveFilterAlg', thisalg)
        alg.FilterDescription = f'events passing < {self.selectionName} >'
        alg.eventDecisionOutputDecoration = f'ignore_{self.selectionName}_%SYS%'
        alg.selection = self.checkDecorationName(self.currentDecoration)
        alg.noFilter = True
        alg.selectionName = f'pass_{self.selectionName}_%SYS%,as_char' # this one is used as a selection
        alg.decorationName = f'ntuplepass_{self.selectionName}_%SYS%' # this one is saved to file
        config.addOutputVar('EventInfo', f'ntuplepass_{self.selectionName}_%SYS%', f'pass_{self.selectionName}')


@groupBlocks
def EventSelection(seq):
    seq.append(EventSelectionConfig())
    seq.append(EventCutFlowBlock())
    seq.append(EventSelectionMergerConfig())
