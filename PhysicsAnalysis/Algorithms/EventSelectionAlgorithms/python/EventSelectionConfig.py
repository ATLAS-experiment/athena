# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from functools import partial

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigSequence import groupBlocks
from AsgAnalysisAlgorithms.AsgAnalysisConfig import EventCutFlowBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType


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
    # keyword -> (container option, algorithm-name tag)
    _NOBJECT = {
        "EL_N":   ("electrons",  "NEL"),
        "MU_N":   ("muons",      "NMU"),
        "JET_N":  ("jets",       "NJET"),
        "PH_N":   ("photons",    "NPH"),
        "TAU_N":  ("taus",       "NTAU"),
        "LJET_N": ("largeRjets", "NLJET"),
    }

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
        }
        for kw, (attr, tag) in self._NOBJECT.items():
            d[kw] = partial(self._add_nobject, attr=attr, tag=tag)
        return d

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

    def _check_args(self, items, keyword, validCounts):
        """Validate the leading keyword and the number of arguments."""
        if items[0] != keyword:
            self.raise_misconfig(' '.join(items), keyword)
        if len(items) not in validCounts:
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
            value = int(test)
            if value == float(test):
                if not requirePositive or value >= 0:
                    return value
                else:
                    raise ValueError (f"[EventSelectionConfig] Misconfiguration! Int {test} us not positive!")
            else:
                raise ValueError (f"[EventSelectionConfig] Misconfiguration! {test} should be an int, not a float!")
        except ValueError:
            raise ValueError (f"[EventSelectionConfig] Misconfiguration! {test} should be an int, not {type(test)}")

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
        return

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

    def _maybe_dressed(self, alg, *specs):
        """Enable dressed kinematics when any of the given electron/muon
        containers is a truth container. Dressed kinematics only exist for
        truth electrons and muons, so only those specs should be passed here."""
        if any(spec and ("Particle" in spec or "Truth" in spec) for spec in specs):
            alg.useDressedProperties = self.useDressedProperties

    def _val_sign_count(self, items, config, alg, container):
        """Parse the trailing `[extraSel] value sign count` grammar (4 or 5
        tokens), applying the optional extra object selection in place.
        Returns (value, sign, count)."""
        if len(items) == 5:
            extraSel = self.check_string(items[1])
            alg.objectSelection = self.extendObjectSelection(
                config, container, alg.objectSelection, extraSel)
            i = 2
        else:  # len == 4, already validated by the caller
            i = 1
        return (self.check_float(items[i]),
                self.check_sign(items[i + 1]),
                self.check_int(items[i + 2]))

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
        if not spec:
            self.raise_missinginput(attr)
        if len(items) not in (4, 5):
            self.raise_misconfig(text, "number of arguments")
        thisalg = f'{self.selectionName}_{tag}_{self.step}'
        alg = config.createAlgorithm('CP::NObjectPtSelectorAlg', thisalg)
        alg.particles, alg.objectSelection = config.readNameAndSelection(spec)
        if attr in ("electrons", "muons"):
            self._maybe_dressed(alg, spec)
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        alg.minPt, alg.sign, alg.count = self._val_sign_count(
            items, config, alg, spec.split(".")[0])
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')

    def add_IMPORT(self, text, config):
        # this is used to import a previous selection
        items = text.split()
        self._check_args(items, "IMPORT", (2,))
        region = self.check_string(items[1])
        if not self.currentDecoration:
            self.currentDecoration = f'pass_{region}_%SYS%,as_char'
        else:
            self.currentDecoration = f'{self.currentDecoration},as_char&&pass_{region}_%SYS%'
        # for the cutflow, we need to retrieve all the cuts corresponding to this IMPORT
        imported_cuts = [cut for cut in config.getSelectionCutFlow('EventInfo', '') if cut.startswith(region)]
        self.cutflow += imported_cuts
        return

    def add_NBJET_selector(self, text, config):
        items = text.split()
        self._check_args(items, "JET_N_BTAG", (3, 4, 5))
        if not self.jets:
            self.raise_missinginput("jets")
        thisalg = f'{self.selectionName}_NBJET_{self.step}'
        alg = config.createAlgorithm('CP::NObjectPtSelectorAlg', thisalg)
        particles, selection = config.readNameAndSelection(self.jets)
        alg.particles = particles
        alg.objectSelection = f'{selection}&&{self.btagDecoration},as_char' if selection else f'{self.btagDecoration},as_char'
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        if len(items) == 3:
            alg.sign  = self.check_sign(items[1])
            alg.count = self.check_int(items[2])
        elif len(items) == 4:
            if ":" in text:
                btagger, btagWP = self.check_btagging(items[1])
                customBtag = f'ftag_select_{btagger}_{btagWP}'
                alg.objectSelection = f'{selection}&&{customBtag},as_char' if selection else f'{customBtag},as_char'
            else:
                extraSel = self.check_string(items[1])
                alg.objectSelection = self.extendObjectSelection(config, self.jets.split(".")[0], alg.objectSelection, extraSel)
            alg.sign  = self.check_sign(items[2])
            alg.count = self.check_int(items[3])
        elif len(items) == 5:
            extraSel = self.check_string(items[1])
            btagger, btagWP = self.check_btagging(items[2])
            customBtag = f'ftag_select_{btagger}_{btagWP}'
            alg.objectSelection = f'{selection}&&{customBtag},as_char' if selection else f'{customBtag},as_char'
            alg.objectSelection = self.extendObjectSelection(config, self.jets.split(".")[0], alg.objectSelection, extraSel)
            alg.sign  = self.check_sign(items[3])
            alg.count = self.check_int(items[4])
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_SUMNELNMU_selector(self, text, config):
        items = text.split()
        self._check_args(items, "SUM_EL_N_MU_N", (4, 5, 7))
        if not self.electrons and not self.muons:
            self.raise_missinginput("electrons or muons")
        thisalg = f'{self.selectionName}_SUMNELNMU_{self.step}'
        alg = config.createAlgorithm('CP::SumNLeptonPtSelectorAlg', thisalg)
        alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        self._maybe_dressed(alg, self.electrons, self.muons)
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        if len(items) == 4:
            alg.minPtEl = self.check_float(items[1])
            alg.minPtMu = self.check_float(items[1])
            alg.sign  = self.check_sign(items[2])
            alg.count = self.check_int(items[3])
        elif len(items) == 5:
            alg.minPtEl = self.check_float(items[1])
            alg.minPtMu = self.check_float(items[2])
            alg.sign  = self.check_sign(items[3])
            alg.count = self.check_int(items[4])
        elif len(items) == 7:
            extraSelEl = self.check_string(items[1])
            extraSelMu = self.check_string(items[2])
            alg.electronSelection = self.extendObjectSelection(config, self.electrons.split(".")[0], alg.electronSelection, extraSelEl)
            alg.muonSelection = self.extendObjectSelection(config, self.muons.split(".")[0], alg.muonSelection, extraSelMu)
            alg.minPtEl = self.check_float(items[3])
            alg.minPtMu = self.check_float(items[4])
            alg.sign  = self.check_sign(items[5])
            alg.count = self.check_int(items[6])
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_SUMNLEPTONS_selector(self, text, config):
        items = text.split()
        self._check_args(items, "SUM_EL_N_MU_N_TAU_N", (4, 6, 9))
        if not self.electrons and not self.muons and not self.taus:
            self.raise_missinginput("electrons, muons or taus")
        thisalg = f'{self.selectionName}_SUMNLEPTONS_{self.step}'
        alg = config.createAlgorithm('CP::SumNLeptonPtSelectorAlg', thisalg)
        alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        alg.taus, alg.tauSelection = config.readNameAndSelection(self.taus)
        self._maybe_dressed(alg, self.electrons, self.muons)
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        if len(items) == 4:
            alg.minPtEl = self.check_float(items[1])
            alg.minPtMu = self.check_float(items[1])
            alg.minPtTau = self.check_float(items[1])
            alg.sign  = self.check_sign(items[2])
            alg.count = self.check_int(items[3])
        elif len(items) == 6:
            alg.minPtEl = self.check_float(items[1])
            alg.minPtMu = self.check_float(items[2])
            alg.minPtTau = self.check_float(items[3])
            alg.sign  = self.check_sign(items[4])
            alg.count = self.check_int(items[5])
        elif len(items) == 9:
            extraSelEl = self.check_string(items[1])
            extraSelMu = self.check_string(items[2])
            extraSelTau = self.check_string(items[3])
            alg.electronSelection = self.extendObjectSelection(config, self.electrons.split(".")[0], alg.electronSelection, extraSelEl)
            alg.muonSelection = self.extendObjectSelection(config, self.muons.split(".")[0], alg.muonSelection, extraSelMu)
            alg.tauSelection = self.extendObjectSelection(config, self.taus.split(".")[0], alg.tauSelection, extraSelTau)
            alg.minPtEl = self.check_float(items[4])
            alg.minPtMu = self.check_float(items[5])
            alg.minPtTau = self.check_float(items[6])
            alg.sign  = self.check_sign(items[7])
            alg.count = self.check_int(items[8])
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_NLJETMASS_selector(self, text, config):
        items = text.split()
        self._check_args(items, "LJETMASS_N", (4, 5))
        thisalg = f'{self.selectionName}_NLJETMASS_{self.step}'
        alg = config.createAlgorithm('CP::NObjectMassSelectorAlg', thisalg)
        alg.particles, alg.objectSelection = config.readNameAndSelection(self.largeRjets)
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        alg.minMass, alg.sign, alg.count = self._val_sign_count(
            items, config, alg, self.largeRjets.split(".")[0])
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_NLJETMASSWINDOW_selector(self, text, config):
        items = text.split()
        self._check_args(items, "LJETMASSWINDOW_N", (5, 6, 7))
        thisalg = f'{self.selectionName}_NLJETMASSWINDOW_{self.step}'
        alg = config.createAlgorithm('CP::NLargeRJetMassWindowSelectorAlg', thisalg)
        alg.ljets, alg.ljetSelection = config.readNameAndSelection(self.largeRjets)
        vetoMode = items[-1] == 'veto' or items[-1] == 'VETO'
        if len(items) == 5 or (len(items) == 6 and vetoMode):
            alg.lowMass  = self.check_float(items[1])
            alg.highMass = self.check_float(items[2])
            alg.sign     = self.check_sign(items[3])
            alg.count    = self.check_int(items[4])
            alg.vetoMode = vetoMode
        elif (len(items) == 6 and not vetoMode) or len(items) == 7:
            extraSel = self.check_string(items[1])
            alg.ljetSelection = self.extendObjectSelection(config, self.largeRjets.split(".")[0], alg.ljetSelection, extraSel)
            alg.lowMass  = self.check_float(items[2])
            alg.highMass = self.check_float(items[3])
            alg.sign     = self.check_sign(items[4])
            alg.count    = self.check_int(items[5])
            alg.vetoMode = vetoMode
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_NJETGHOST_selector(self, text, config):
        items = text.split()
        self._check_args(items, "JET_N_GHOST", (4, 5))
        thisalg = f'{self.selectionName}_NJETGHOST_{self.step}'
        alg = config.createAlgorithm('CP::JetNGhostSelectorAlg', thisalg)
        alg.jets, alg.jetSelection = config.readNameAndSelection(self.jets)
        ghosts = self.check_ghosts(items[1])
        alg.ghost = ghosts[0]
        if len(ghosts) > 1 :
            alg.veto = ghosts[1]
        if len(items) == 4:
            alg.sign  = self.check_sign(items[2])
            alg.count = self.check_int(items[3])
        elif len(items) == 5:
            alg.minPt = self.check_float(items[2])
            alg.sign  = self.check_sign(items[3])
            alg.count = self.check_int(items[4])
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_NLJETGHOST_selector(self, text, config):
        items = text.split()
        self._check_args(items, "LJET_N_GHOST", (4, 5))
        thisalg = f'{self.selectionName}_NLJETGHOST_{self.step}'
        alg = config.createAlgorithm('CP::JetNGhostSelectorAlg', thisalg)
        alg.jets, alg.jetSelection = config.readNameAndSelection(self.largeRjets)
        ghosts = self.check_ghosts(items[1])
        alg.ghost = ghosts[0]
        if len(ghosts) > 1 :
            alg.veto = ghosts[1]
        if len(items) == 4:
            alg.sign  = self.check_sign(items[2])
            alg.count = self.check_int(items[3])
        elif len(items) == 5:
            alg.minPt = self.check_float(items[2])
            alg.sign  = self.check_sign(items[3])
            alg.count = self.check_int(items[4])
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_NOBJ_selector(self, text, config):
        items = text.split()
        self._check_args(items, "OBJ_N", (5,))
        thisalg = f'{self.selectionName}_NOBJ_{self.step}'
        alg = config.createAlgorithm('CP::NObjectPtSelectorAlg', thisalg)
        alg.particles, alg.objectSelection = config.readNameAndSelection(self.check_string(items[1]))
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        alg.minPt = self.check_float(items[2])
        alg.sign  = self.check_sign(items[3])
        alg.count = self.check_int(items[4])
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_MET_selector(self, text, config):
        items = text.split()
        self._check_args(items, "MET", (3,))
        if not self.met:
            self.raise_missinginput("MET")
        thisalg = f'{self.selectionName}_MET_{self.step}'
        alg = config.createAlgorithm('CP::MissingETSelectorAlg', thisalg)
        alg.met = config.readName(self.met)
        alg.metTerm = self.metTerm
        alg.sign = self.check_sign(items[1])
        alg.refMET = self.check_float(items[2])
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_MWT_selector(self, text, config):
        items = text.split()
        self._check_args(items, "MWT", (3,))
        if not self.electrons and not self.muons:
            self.raise_missinginput("electrons or muons")
        thisalg = f'{self.selectionName}_MWT_{self.step}'
        alg = config.createAlgorithm('CP::TransverseMassSelectorAlg', thisalg)
        alg.met = config.readName(self.met)
        alg.metTerm = self.metTerm
        alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        self._maybe_dressed(alg, self.electrons, self.muons)
        alg.sign = self.check_sign(items[1])
        alg.refMWT = self.check_float(items[2])
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_METMWT_selector(self, text, config):
        items = text.split()
        self._check_args(items, "MET+MWT", (3,))
        if not self.met:
            self.raise_missinginput("MET")
        if not self.electrons and not self.muons:
            self.raise_missinginput("electrons or muons")
        thisalg = f'{self.selectionName}_METMWT_{self.step}'
        alg = config.createAlgorithm('CP::MissingETPlusTransverseMassSelectorAlg', thisalg)
        alg.met = config.readName(self.met)
        alg.metTerm = self.metTerm
        alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        self._maybe_dressed(alg, self.electrons, self.muons)
        alg.sign = self.check_sign(items[1])
        alg.refMETMWT = self.check_float(items[2])
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_MLL_selector(self, text, config):
        items = text.split()
        self._check_args(items, "MLL", (3,))
        if not self.electrons and not self.muons:
            self.raise_missinginput("electrons or muons")
        thisalg = f'{self.selectionName}_MLL_{self.step}'
        alg = config.createAlgorithm('CP::DileptonInvariantMassSelectorAlg', thisalg)
        if self.electrons:
            alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        if self.muons:
            alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        self._maybe_dressed(alg, self.electrons, self.muons)
        alg.sign = self.check_sign(items[1])
        alg.refMLL = self.check_float(items[2])
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_MLLWINDOW_selector(self, text, config):
        items = text.split()
        self._check_args(items, "MLLWINDOW", (3, 4))
        if not self.electrons and not self.muons:
            self.raise_missinginput("electrons or muons")
        thisalg = f'{self.selectionName}_MLLWINDOW_{self.step}'
        alg = config.createAlgorithm('CP::DileptonInvariantMassWindowSelectorAlg', thisalg)
        if self.electrons:
            alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        if self.muons:
            alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        self._maybe_dressed(alg, self.electrons, self.muons)
        alg.lowMLL = self.check_float(items[1])
        alg.highMLL = self.check_float(items[2])
        alg.vetoMode = (len(items) == 4 and self.check_string(items[3]).lower() == "veto")
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def _add_charge(self, text, config, *, osMode, tag):
        """Builder shared by OS and SS: same algorithm, opposite charge mode."""
        items = text.split()
        if not items or len(items) > 4:
            self.raise_misconfig(text, "number of arguments")
        if not self.electrons and not self.muons and not self.taus:
            self.raise_missinginput("electrons or muons or taus")
        thisalg = f'{self.selectionName}_{tag}_{self.step}'
        alg = config.createAlgorithm('CP::ChargeSelectorAlg', thisalg)
        allLeptons = (len(items) == 1)
        if self.electrons and (allLeptons or "el" in items):
            self._route_lepton(alg, config, self.electrons,
                               ('electrons', 'electronSelection'),
                               ('truthElectrons', 'truthElectronSelection'))
        if self.muons and (allLeptons or "mu" in items):
            self._route_lepton(alg, config, self.muons,
                               ('muons', 'muonSelection'),
                               ('truthMuons', 'truthMuonSelection'))
        if self.taus and (allLeptons or "tau" in items):
            self._route_lepton(alg, config, self.taus,
                               ('taus', 'tauSelection'),
                               ('truthTaus', 'truthTauSelection'))
        alg.OS = osMode
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_MLL_OSSF_selector(self, text, config):
        items = text.split()
        self._check_args(items, "MLL_OSSF", (3, 4))
        if not self.electrons and not self.muons:
            self.raise_missinginput("electrons or muons")
        thisalg = f'{self.selectionName}_MLL_OSSF_{self.step}'
        alg = config.createAlgorithm('CP::DileptonOSSFInvariantMassWindowSelectorAlg', thisalg)
        if self.electrons:
            self._route_lepton(alg, config, self.electrons,
                               ('electrons', 'electronSelection'),
                               ('truthElectrons', 'truthElectronSelection'))
        if self.muons:
            self._route_lepton(alg, config, self.muons,
                               ('muons', 'muonSelection'),
                               ('truthMuons', 'truthMuonSelection'))
        self._maybe_dressed(alg, self.electrons, self.muons)
        alg.lowMll = self.check_float(items[1])
        alg.highMll = self.check_float(items[2])
        alg.vetoMode = (len(items) == 4 and self.check_string(items[3]).lower() == "veto")
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_EVENTFLAG(self, text, config):
        items = text.split()
        self._check_args(items, "EVENTFLAG", (2,))
        existingDecoration = self.check_string(items[1])
        self.setDecorationName(None, config, existingDecoration)
        return

    def add_GLOBALTRIGMATCH(self, text, config):
        items = text.split()
        self._check_args(items, "GLOBALTRIGMATCH", (1, 2))
        if len(items) == 1:
            self.setDecorationName(None, config, "globalTriggerMatch_%SYS%,as_char")
        else:
            postfix = self.check_string(items[1])
            self.setDecorationName(None, config, f"globalTriggerMatch{postfix}_%SYS%,as_char")
        return

    def add_RUNNUMBER(self, text, config):
        items = text.split()
        self._check_args(items, "RUN_NUMBER", (3,))
        thisalg = f'{self.selectionName}_RUN_NUMBER_{self.step}'
        alg = config.createAlgorithm('CP::RunNumberSelectorAlg', thisalg)
        alg.sign = self.check_sign(items[1])
        alg.runNumber = self.check_int(items[2])
        alg.useRandomRunNumber = config.dataType() is not DataType.Data
        alg.eventPreselection = self.checkDecorationName(self.currentDecoration)
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_SAVE(self, text, config):
        items = text.split()
        self._check_args(items, "SAVE", (1,))
        thisalg = f'{self.selectionName}_SAVE'
        alg = config.createAlgorithm('CP::SaveFilterAlg', thisalg)
        alg.FilterDescription = f'events passing < {self.selectionName} >'
        alg.eventDecisionOutputDecoration = f'ignore_{self.selectionName}_%SYS%'
        alg.selection = self.checkDecorationName(self.currentDecoration)
        alg.noFilter = True
        alg.selectionName = f'pass_{self.selectionName}_%SYS%,as_char' # this one is used as a selection
        alg.decorationName = f'ntuplepass_{self.selectionName}_%SYS%' # this one is saved to file
        config.addOutputVar('EventInfo', f'ntuplepass_{self.selectionName}_%SYS%', f'pass_{self.selectionName}')
        return


@groupBlocks
def EventSelection(seq):
    seq.append(EventSelectionConfig())
    seq.append(EventCutFlowBlock())
    seq.append(EventSelectionMergerConfig())
