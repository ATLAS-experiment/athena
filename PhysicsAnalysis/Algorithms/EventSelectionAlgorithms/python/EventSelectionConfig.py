# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AsgAnalysisAlgorithms.AsgAnalysisConfig import makeEventCutFlowConfig

class EventSelectionMergerConfig(ConfigBlock):
    """ConfigBlock for merging the output of various selection streams"""

    def __init__(self):
        super(EventSelectionMergerConfig, self).__init__('EventSelectionMerger')
        self.addOption('selections', [], type=list)
        self.addOption('noFilter', False, type=bool)

    def makeAlgs(self, config):
        alg = config.createAlgorithm('CP::SaveFilterAlg', 'EventSelectionMerger')
        alg.FilterDescription = 'events passing at least one EventSelection algorithm'
        alg.eventDecisionOutputDecoration = 'ignore_anySelection_%SYS%'
        alg.selection = '||'.join(self.selections)
        alg.noFilter = self.noFilter
        alg.selectionName = 'pass_anySelection_%SYS%'
        alg.decorationName = 'ntuplepass_anySelection_%SYS%'

class EventSelectionConfig(ConfigBlock):
    """ConfigBlock for interpreting text-based event selections"""

    def __init__(self, name):
        super(EventSelectionConfig, self).__init__('EventSelection_'+name)
        self.addOption('electrons', "", type=str)
        self.addOption('muons', "", type=str)
        self.addOption('jets', "", type=str)
        self.addOption('photons', "", type=str)
        self.addOption('taus', "", type=str)
        self.addOption('met', "", type=str)
        self.addOption('btagDecoration', "", type=str)
        self.addOption('preselection', "", type=str)
        self.addOption('selectionCuts', "", type=str)
        self.addOption('noFilter', False, type=bool)
        self.addOption('debugMode', False, type=bool)
        self.step = 0
        self.currentDecoration = ''
        self.cutflow = []
        self.name = name

    def makeAlgs(self, config):
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
        config.addEventCutFlow(self.name, self.getCutflow())

    def interpret(self, text, cfg):
        text = text.strip()
        if not text:
            return
        if text.startswith("#"):
            return
        self.step += 1
        if "EL_N" in text.split():
            self.add_NEL_selector(text, cfg)
        elif "MU_N" in text.split():
            self.add_NMU_selector(text, cfg)
        elif "JET_N" in text.split():
            self.add_NJET_selector(text, cfg)
        elif "JET_N_BTAG" in text.split():
            self.add_NBJET_selector(text, cfg)
        elif "PH_N" in text.split():
            self.add_NPH_selector(text, cfg)
        elif "TAU_N" in text.split():
            self.add_NTAU_selector(text, cfg)
        elif "MET" in text.split():
            self.add_MET_selector(text, cfg)
        elif "MWT" in text.split():
            self.add_MWT_selector(text, cfg)
        elif "MET+MWT" in text.split():
            self.add_METMWT_selector(text, cfg)
        elif "MLL" in text.split():
            self.add_MLL_selector(text, cfg)
        elif "MLLWINDOW" in text.split():
            self.add_MLLWINDOW_selector(text, cfg)
        elif "OS" in text.split():
            self.add_OS_selector(text, cfg)
        elif "SS" in text.split():
            self.add_SS_selector(text, cfg)
        elif "SAVE" in text.split():
            self.add_SAVE(text, cfg)
        elif "IMPORT" in text.split():
            self.add_IMPORT(text, cfg)
        else:
            raise ValueError (f"[EventSelectionConfig] The following selection cut is not recognised! --> {text}")

    def raise_misconfig(self, text, keyword):
        raise ValueError (f"[EventSelectionConfig] Misconfiguration! Check {keyword} in: {text}")

    def raise_missinginput(self, collection):
        raise ValueError (f"[EventSelectionConfig] Misconfiguration! Missing input collection for {collection}")

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

    def getCutflow(self):
        return self.cutflow

    def setDecorationName(self, algorithm, config, decoration):
        algorithm.decorationName = f'{decoration}'
        self.currentDecoration = decoration
        self.cutflow.append( decoration )
        if self.debugMode:
            config.addOutputVar('EventInfo', decoration, decoration.split("_%SYS%")[0])
        config.addSelection('EventInfo', '', decoration)
        return

    def add_IMPORT(self, text, config):
        # this is used to import a previous selection
        items = text.split()
        if items[0] != "IMPORT":
            self.raise_misconfig(text, "IMPORT")
        if len(items) != 2:
            self.raise_misconfig(text, "number of arguments")
        region = self.check_string(items[1])
        if not self.currentDecoration:
            self.currentDecoration = f'pass_{region}_%SYS%'
        else:
            self.currentDecoration = f'{self.currentDecoration}&&pass_{region}_%SYS%'
        # for the cutflow, we need to retrieve all the cuts corresponding to this IMPORT
        imported_cuts = [cut for cut in config.getSelectionCutFlow('EventInfo', '') if cut.startswith(region)]
        self.cutflow += imported_cuts
        return

    def add_NEL_selector(self, text, config):
        items = text.split()
        if items[0] != "EL_N":
            self.raise_misconfig(text, "EL_N")
        if len(items) != 4 and len(items) != 5:
            self.raise_misconfig(text, "number of arguments")
        if not self.electrons:
            self.raise_missinginput("electrons")
        thisalg = f'{self.name}_NEL_{self.step}'
        alg = config.createAlgorithm('CP::NObjectPtSelectorAlg', thisalg)
        alg.particles, alg.objectSelection = config.readNameAndSelection(self.electrons)
        alg.eventPreselection = f'{self.currentDecoration}'
        if len(items) == 4:
            alg.minPt = self.check_float(items[1])
            alg.sign  = self.check_sign(items[2])
            alg.count = self.check_int(items[3])
        elif len(items) == 5:
            extraSel  = self.check_string(items[1])
            if alg.objectSelection:
                alg.objectSelection += "&&" + config.getFullSelection(self.electrons.split(".")[0], extraSel)
            else:
                alg.objectSElection = config.getFullSelection(self.electrons.split(".")[0], extraSel)
            alg.minPt = self.check_float(items[2])
            alg.sign  = self.check_sign(items[3])
            alg.count = self.check_int(items[4])
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_NMU_selector(self, text, config):
        items = text.split()
        if items[0] != "MU_N":
            self.raise_misconfig(text, "MU_N")
        if len(items) != 4 and len(items) != 5:
            self.raise_misconfig(text, "number of arguments")
        if not self.muons:
            self.raise_missinginput("muons")
        thisalg = f'{self.name}_NMU_{self.step}'
        alg = config.createAlgorithm('CP::NObjectPtSelectorAlg', thisalg)
        alg.particles, alg.objectSelection = config.readNameAndSelection(self.muons)
        alg.eventPreselection = f'{self.currentDecoration}'
        if len(items) == 4:
            alg.minPt = self.check_float(items[1])
            alg.sign  = self.check_sign(items[2])
            alg.count = self.check_int(items[3])
        elif len(items) == 5:
            extraSel  = self.check_string(items[1])
            if alg.objectSelection:
                alg.objectSelection += "&&" + config.getFullSelection(self.muons.split(".")[0], extraSel)
            else:
                alg.objectSElection = config.getFullSelection(self.muons.split(".")[0], extraSel)
            alg.minPt = self.check_float(items[2])
            alg.sign  = self.check_sign(items[3])
            alg.count = self.check_int(items[4])
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_NJET_selector(self, text, config):
        items = text.split()
        if items[0] != "JET_N":
            self.raise_misconfig(text, "JET_N")
        if len(items) != 4 and len(items) != 5:
            self.raise_misconfig(text, "number of arguments")
        if not self.jets:
            self.raise_missinginput("jets")
        thisalg = f'{self.name}_NJET_{self.step}'
        alg = config.createAlgorithm('CP::NObjectPtSelectorAlg', thisalg)
        alg.particles, alg.objectSelection = config.readNameAndSelection(self.jets)
        alg.eventPreselection = f'{self.currentDecoration}'
        if len(items) == 4:
            alg.minPt = self.check_float(items[1])
            alg.sign  = self.check_sign(items[2])
            alg.count = self.check_int(items[3])
        elif len(items) == 5:
            extraSel  = self.check_string(items[1])
            if alg.objectSelection:
                alg.objectSelection += "&&" + config.getFullSelection(self.jets.split(".")[0], extraSel)
            else:
                alg.objectSElection = config.getFullSelection(self.jets.split(".")[0], extraSel)
            alg.minPt = self.check_float(items[2])
            alg.sign  = self.check_sign(items[3])
            alg.count = self.check_int(items[4])
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_NBJET_selector(self, text, config):
        items = text.split()
        if items[0] != "JET_N_BTAG":
            self.raise_misconfig(text, "JET_N_BTAG")
        if len(items) != 3 and len(items) != 4:
            self.raise_misconfig(text, "number of arguments")
        if not self.jets:
            self.raise_missinginput("jets")
        thisalg = f'{self.name}_NBJET_{self.step}'
        alg = config.createAlgorithm('CP::NObjectPtSelectorAlg', thisalg)
        particles, selection = config.readNameAndSelection(self.jets)
        alg.particles = particles
        alg.objectSelection = f'{selection}&&{self.btagDecoration},as_char'
        alg.eventPreselection = f'{self.currentDecoration}'
        alg.minPt = 25000.
        if len(items) == 3:
            alg.sign  = self.check_sign(items[1])
            alg.count = self.check_int(items[2])
        elif len(items) == 4:
            btagger, btagWP = self.check_btagging(items[1])
            customBtag = f'ftag_select_{btagger}_{btagWP}'
            alg.objectSelection = f'{selection}&&{customBtag},as_char'
            alg.sign  = self.check_sign(items[2])
            alg.count = self.check_int(items[3])
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_NPH_selector(self, text, config):
        items = text.split()
        if items[0] != "PH_N":
            self.raise_misconfig(text, "PH_N")
        if len(items) != 4 and len(items) != 5:
            self.raise_misconfig(text, "number of arguments")
        if not self.photons:
            self.raise_missinginput("photons")
        thisalg = f'{self.name}_NPH_{self.step}'
        alg = config.createAlgorithm('CP::NObjectPtSelectorAlg', thisalg)
        alg.particles, alg.objectSelection = config.readNameAndSelection(self.photons)
        alg.eventPreselection = f'{self.currentDecoration}'
        if len(items) == 4:
            alg.minPt = self.check_float(items[1])
            alg.sign  = self.check_sign(items[2])
            alg.count = self.check_int(items[3])
        elif len(items) == 5:
            extraSel  = self.check_string(items[1])
            if alg.objectSelection:
                alg.objectSelection += "&&" + config.getFullSelection(self.photons.split(".")[0], extraSel)
            else:
                alg.objectSElection = config.getFullSelection(self.photons.split(".")[0], extraSel)
            alg.minPt = self.check_float(items[2])
            alg.sign  = self.check_sign(items[3])
            alg.count = self.check_int(items[4])
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_NTAU_selector(self, text, config):
        items = text.split()
        if items[0] != "TAU_N":
            self.raise_misconfig(text, "TAU_N")
        if len(items) != 4 and len(items) != 5:
            self.raise_misconfig(text, "number of arguments")
        if not self.taus:
            self.raise_missinginput("taus")
        thisalg = f'{self.name}_NTAU_{self.step}'
        alg = config.createAlgorithm('CP::NObjectPtSelectorAlg', thisalg)
        alg.particles, alg.objectSelection = config.readNameAndSelection(self.taus)
        alg.eventPreselection = f'{self.currentDecoration}'
        if len(items) == 4:
            alg.minPt = self.check_float(items[1])
            alg.sign  = self.check_sign(items[2])
            alg.count = self.check_int(items[3])
        elif len(items) == 5:
            extraSel  = self.check_string(items[1])
            if alg.objectSelection:
                alg.objectSelection += "&&" + config.getFullSelection(self.taus.split(".")[0], extraSel)
            else:
                alg.objectSElection = config.getFullSelection(self.taus.split(".")[0], extraSel)
            alg.minPt = self.check_float(items[2])
            alg.sign  = self.check_sign(items[3])
            alg.count = self.check_int(items[4])
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return
    
    def add_MET_selector(self, text, config):
        items = text.split()
        if items[0] != "MET":
            self.raise_misconfig(text, "MET")
        if len(items) != 3:
            self.raise_misconfig(text, "number of arguments")
        if not self.met:
            self.raise_missinginput("MET")
        thisalg = f'{self.name}_MET_{self.step}'
        alg = config.createAlgorithm('CP::MissingETSelectorAlg', thisalg)
        alg.met = config.readName(self.met)
        alg.sign = self.check_sign(items[1])
        alg.refMET = self.check_float(items[2])
        alg.eventPreselection = f'{self.currentDecoration}'
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return
    
    def add_MWT_selector(self, text, config):
        items = text.split()
        if items[0] != "MWT":
            self.raise_misconfig(text, "MWT")
        if len(items) != 3:
            self.raise_misconfig(text, "number of arguments")
        if not self.electrons and not self.muons:
            self.raise_missinginput("electrons or muons")
        thisalg = f'{self.name}_MWT_{self.step}'
        alg = config.createAlgorithm('CP::TransverseMassSelectorAlg', thisalg)
        alg.met = config.readName(self.met)
        alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        alg.sign = self.check_sign(items[1])
        alg.refMWT = self.check_float(items[2])
        alg.eventPreselection = f'{self.currentDecoration}'
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_METMWT_selector(self, text, config):
        items = text.split()
        if items[0] != "MET+MWT":
            self.raise_misconfig(text, "MET+MWT")
        if len(items) != 3:
            self.raise_misconfig(text, "number of arguments")
        if not self.met:
            self.raise_missinginput("MET")
        if not self.electrons and not self.muons:
            self.raise_missinginput("electrons or muons")
        thisalg = f'{self.name}_METMWT_{self.step}'
        alg = config.createAlgorithm('CP::MissingETPlusTransverseMassSelectorAlg', thisalg)
        alg.met = config.readName(self.met)
        alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        alg.sign = self.check_sign(items[1])
        alg.refMETMWT = self.check_float(items[2])
        alg.eventPreselection = f'{self.currentDecoration}'
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_MLL_selector(self, text, config):
        items = text.split()
        if items[0] != "MLL":
            self.raise_misconfig(text, "MLL")
        if len(items) != 3:
            self.raise_misconfig(text, "number of arguments")
        if not self.electrons and not self.muons:
            self.raise_missinginput("electrons or muons")
        thisalg = f'{self.name}_MLL_{self.step}'
        alg = config.createAlgorithm('CP::DileptonInvariantMassSelectorAlg', thisalg)
        if self.electrons:
            alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        if self.muons:
            alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        alg.sign = self.check_sign(items[1])
        alg.refMLL = self.check_float(items[2])
        alg.eventPreselection = f'{self.currentDecoration}'
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_MLLWINDOW_selector(self, text, config):
        items = text.split()
        if items[0] != "MLLWINDOW":
            self.raise_misconfig(text, "MLLWINDOW")
        if len(items) != 3:
            self.raise_misconfig(text, "number of arguments")
        if not self.electrons and not self.muons:
            self.raise_missinginput("electrons or muons")
        thisalg = f'{self.name}_MLLWINDOW_{self.step}'
        alg = config.createAlgorithm('CP::DileptonInvariantMassWindowSelectorAlg', thisalg)
        if self.electrons:
            alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        if self.muons:
            alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        alg.lowMLL = self.check_float(items[1])
        alg.highMLL = self.check_float(items[2])
        # if high<low we are trying to veto events in that window; otherwise we select them
        alg.vetoMode = alg.highMLL < alg.lowMLL
        alg.eventPreselection = f'{self.currentDecoration}'
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_OS_selector(self, text, config):
        items = text.split()
        if len(items) != 1:
            self.raise_misconfig(text, "number of arguments")
        if not self.electrons and not self.muons:
            self.raise_missinginput("electrons or muons")
        thisalg = f'{self.name}_OS_{self.step}'
        alg = config.createAlgorithm('CP::ChargeSelectorAlg', thisalg)
        if self.electrons:
            alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        if self.muons:
            alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        alg.OS = True
        alg.eventPreselection = f'{self.currentDecoration}'
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_SS_selector(self, text, config):
        items = text.split()
        if len(items) != 1:
            self.raise_misconfig(text, "number of arguments")
        if not self.electrons and not self.muons:
            self.raise_missinginput("electrons or muons")
        thisalg = f'{self.name}_SS_{self.step}'
        alg = config.createAlgorithm('CP::ChargeSelectorAlg', thisalg)
        if self.electrons:
            alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        if self.muons:
            alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        alg.OS = False
        alg.eventPreselection = f'{self.currentDecoration}'
        self.setDecorationName(alg, config, f'{thisalg}_%SYS%')
        return

    def add_SAVE(self, text, config):
        items = text.split()
        if items[0] != "SAVE":
            self.raise_misconfig(text, "SAVE")
        if len(items) != 1:
            self.raise_misconfig(text, "number of arguments")
        thisalg = f'{self.name}_SAVE'
        alg = config.createAlgorithm('CP::SaveFilterAlg', thisalg)
        alg.FilterDescription = f'events passing < {self.name} >'
        alg.eventDecisionOutputDecoration = f'ignore_{self.name}_%SYS%'
        alg.selection = f'{self.currentDecoration}'
        alg.noFilter = self.noFilter
        alg.selectionName = f'pass_{self.name}_%SYS%' # this one is used as a selection
        alg.decorationName = f'ntuplepass_{self.name}_%SYS%' # this one is saved to file
        config.addOutputVar('EventInfo', f'ntuplepass_{self.name}_%SYS%', f'pass_{self.name}')
        return

def makeEventSelectionConfig(seq,
                             name,
                             electrons=None, muons=None, jets=None,
                             photons=None, taus=None, met=None,
                             btagDecoration=None, preselection=None,
                             selectionCuts=None, noFilter=None,
                             debugMode=None, cutFlowHistograms=None):
    """Create an event selection config block

    Keyword arguments:
        name -- the name defining this selection
        electrons -- the electron container and selection
        muons -- the muon container and selection
        jets -- the jet container and selection
        photons -- the photon container and selection
        taus -- the tau-jet container and selection
        met -- the MET container
        btagDecoration -- the b-tagging decoration to use when defining b-jets
        preselection -- optional event-wise selection flag to start from
        selectionCuts -- a string listing one selection cut per line
        noFilter -- whether to disable the event filter
        debugMode -- enables saving all intermediate decorations
        cutFlowHistograms -- whether to toggle event cutflow histograms per systematic
    """

    config = EventSelectionConfig(name)
    config.setOptionValue ('electrons', electrons, noneAction='ignore')
    config.setOptionValue ('muons', muons, noneAction='ignore')
    config.setOptionValue ('jets', jets, noneAction='ignore')
    config.setOptionValue ('photons', photons, noneAction='ignore')
    config.setOptionValue ('taus', taus, noneAction='ignore')
    config.setOptionValue ('met', met, noneAction='ignore')
    config.setOptionValue ('btagDecoration', btagDecoration, noneAction='ignore')
    config.setOptionValue ('preselection', preselection, noneAction='ignore')
    config.setOptionValue ('selectionCuts', selectionCuts)
    config.setOptionValue ('noFilter', noFilter, noneAction='ignore')
    config.setOptionValue ('debugMode', debugMode, noneAction='ignore')
    seq.append(config)

    # add event cutflow algorithm
    if cutFlowHistograms:
        makeEventCutFlowConfig(seq, 'EventInfo', selectionName='', postfix=name,
                               customSelections=name)

def makeMultipleEventSelectionConfigs(seq,
                                      electrons=None, muons=None, jets=None,
                                      photons=None, taus=None, met=None,
                                      btagDecoration=None, preselection=None,
                                      selectionCutsDict=None, noFilter=None,
                                      debugMode=None, cutFlowHistograms=None):
    """Create multiple event selection config blocks

       Keyword arguments:
        electrons -- the electron container and selection
        muons -- the muon container and selection
        jets -- the jet container and selection
        photons -- the photon container and selection
        taus -- the tau-jet container and selection
        met -- the MET container
        btagDecoration -- the b-tagging decoration to use when defining b-jets
        preselection -- optional event-wise selection flag to start from
        selectionCutsDict -- a dictionary with key the name of the selection and value a string listing one selection cut per line
        noFilter -- whether to disable the event filter
        debugMode -- enables saving all intermediate decorations
        cutFlowHistograms -- whether to toggle event cutflow histograms per region and per systematic
    """

    # handle the case where a user is only providing one selection
    if len(list(selectionCutsDict.keys())) == 1:
        name, selectionCuts = list(selectionCutsDict.items())[0]
        makeEventSelectionConfig(seq, name, electrons, muons, jets, photons, taus, met, btagDecoration, preselection, selectionCuts, noFilter=noFilter, debugMode=debugMode, cutFlowHistograms=cutFlowHistograms)
        return

    # first, we generate all the individual event selections
    # !!! it's important to pass noFilter=True, to avoid applying the individual filters in series
    for name, selectionCuts in selectionCutsDict.items():
        makeEventSelectionConfig(seq, name, electrons, muons, jets, photons, taus, met, btagDecoration, preselection, selectionCuts, noFilter=True, debugMode=debugMode, cutFlowHistograms=cutFlowHistograms)

    # now we are ready to collect all the filters and apply their logical OR
    # !!! subregions (name starts with "SUB") are not used in the final filtering
    config = EventSelectionMergerConfig()
    config.setOptionValue ('selections', [f'pass_{name}_%SYS%' for name in selectionCutsDict.keys() if not name.startswith("SUB")])
    config.setOptionValue ('noFilter', noFilter)
    seq.append(config)
