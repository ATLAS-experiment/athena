# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class OverlapAnalysisConfig (ConfigBlock):
    """the ConfigBlock for the MET configuration"""

    def __init__ (self) :
        super (OverlapAnalysisConfig, self).__init__ ()
        self.addOption ('inputLabel', '', type=str)
        self.addOption ('outputLabel', 'passesOR', type=str)
        self.addOption ('selectionName', None, type=str)
        self.addOption ('linkOverlapObjects', False, type=bool)
        self.addOption ('enableUserPriority', False, type=bool)
        self.addOption ('bJetLabel', '', type=str)
        self.addOption ('boostedLeptons', False, type=bool)
        self.addOption ('postfix', '', type=str)
        self.addOption ('nominalOnly', False, type=bool)
        self.addOption ('jets', "", type=str)
        self.addOption ('fatJets', "", type=str)
        self.addOption ('electrons', "", type=str)
        self.addOption ('muons', "", type=str)
        self.addOption ('photons', "", type=str)
        self.addOption ('taus', "", type=str)
        self.addOption ('antiTauIDTauLabel', '', type=str)
        self.addOption ('antiTauLabel', '', type=str)
        self.addOption ('antiTauBJetLabel', '', type=str)
        self.addOption ('addToAllSelections', True, type=bool,
                        info='add OR selection decision into all object selections')
        self.addOption ('addPreselection', False, type=bool,
                        info='add preselection decorations without systematics')
        self.addOption ('preselectLabel', None, type=str,
                        info='label for preselection decorations')
        self.addOption ('jetsSelectionName', None, type=str)
        self.addOption ('fatJetsSelectionName', None, type=str)
        self.addOption ('electronsSelectionName', None, type=str)
        self.addOption ('muonsSelectionName', None, type=str)
        self.addOption ('photonsSelectionName', None, type=str)
        self.addOption ('tausSelectionName', None, type=str)
        self.addOption ('doEleEleOR', False, type=bool)
        self.addOption ('doEleMuOR', True, type=bool)
        self.addOption ('doEleJetOR', True, type=bool)
        self.addOption ('doMuJetOR', True, type=bool)
        self.addOption ('doTauEleOR', True, type=bool)
        self.addOption ('doTauMuOR', True, type=bool)
        self.addOption ('doTauJetOR', True, type=bool)
        self.addOption ('doTauAntiTauJetOR', False, type=bool)
        self.addOption ('doPhEleOR', True, type=bool)
        self.addOption ('doPhMuOR', True, type=bool)
        self.addOption ('doPhJetOR', True, type=bool)
        self.addOption ('doEleFatJetOR', True, type=bool)
        self.addOption ('doJetFatJetOR', True, type=bool)


    def makeAlgs (self, config) :

        postfix = self.postfix

        if self.addToAllSelections and self.selectionName is not None:
            raise Exception('When addToAllSelections=True, you cannot configure selectionName!')

        if self.selectionName is not None:
            selectionName = self.selectionName
        else:
            if self.addToAllSelections:
                selectionName = ""
            else:
                selectionName = self.outputLabel

        if self.jetsSelectionName is not None:
            jetsSelectionName = self.jetsSelectionName
        else:
            jetsSelectionName = selectionName
        if self.fatJetsSelectionName is not None:
            fatJetsSelectionName = self.fatJetsSelectionName
        else:
            fatJetsSelectionName = selectionName
        if self.electronsSelectionName is not None:
            electronsSelectionName = self.electronsSelectionName
        else:
            electronsSelectionName = selectionName
        if self.muonsSelectionName is not None:
            muonsSelectionName = self.muonsSelectionName
        else:
            muonsSelectionName = selectionName
        if self.photonsSelectionName is not None:
            photonsSelectionName = self.photonsSelectionName
        else:
            photonsSelectionName = selectionName
        if self.tausSelectionName is not None:
            tausSelectionName = self.tausSelectionName
        else:
            tausSelectionName = selectionName

        # For now we have to decorate our selections on the objects in
        # separate algorithms beforehand, so that the overlap
        # algorithm can read them.  This should probably be changed at
        # some point.  For one, this seems like an unnecessary
        # complication in the configuration.  For another, it requires
        # that all selection systematics applied so far have dedicated
        # shallow copies for all objects and systematics, i.e. be
        # kinematic systematics.  While that is technically the case
        # right now, I'd prefer if I didn't force that.

        electrons = None
        if self.electrons != "" :
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg','ORElectronsSelectAlg' + postfix )
            electrons, alg.preselection = config.readNameAndSelection (self.electrons)
            alg.particles = electrons
            alg.selectionDecoration = self.inputLabel + ',as_char'
            # if  OR added to all selections, don't need standalone selection flag
            if self.selectionName != '':
                config.addOutputVar (self.electrons.split('.')[0], self.outputLabel + '_%SYS%', 'select_or', noSys=self.nominalOnly)

        photons = None
        if self.photons != "" :
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg','ORPhotonsSelectAlg' + postfix )
            photons, alg.preselection = config.readNameAndSelection (self.photons)
            alg.particles = photons
            alg.selectionDecoration = self.inputLabel + ',as_char'
            if self.selectionName != '':
                config.addOutputVar (self.photons.split('.')[0], self.outputLabel + '_%SYS%', 'select_or', noSys=self.nominalOnly)

        muons = None
        if self.muons != "" :
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg','ORMuonsSelectAlg' + postfix )
            muons, alg.preselection = config.readNameAndSelection (self.muons)
            alg.particles = muons
            alg.selectionDecoration = self.inputLabel + ',as_char'
            if self.selectionName != '':
                config.addOutputVar (self.muons.split('.')[0], self.outputLabel + '_%SYS%', 'select_or', noSys=self.nominalOnly)

        taus = None
        if self.taus != "" :
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg','ORTausSelectAlg' + postfix )
            taus, alg.preselection = config.readNameAndSelection (self.taus)
            alg.particles = taus
            alg.selectionDecoration = self.inputLabel + ',as_char'
            if self.selectionName != '':
                config.addOutputVar (self.taus.split('.')[0], self.outputLabel + '_%SYS%', 'select_or', noSys=self.nominalOnly)

        jets = None
        if self.jets != "" :
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg','ORJetsSelectAlg' + postfix )
            jets, alg.preselection = config.readNameAndSelection (self.jets)
            alg.particles = jets
            alg.selectionDecoration = self.inputLabel + ',as_char'
            if self.selectionName != '':
                config.addOutputVar (self.jets.split('.')[0], self.outputLabel + '_%SYS%', 'select_or', noSys=self.nominalOnly)

        fatJets = None
        if self.fatJets != "" :
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg','ORFatJetsSelectAlg' + postfix )
            fatJets, alg.preselection = config.readNameAndSelection (self.fatJets)
            alg.particles = fatJets
            alg.selectionDecoration = self.inputLabel + ',as_char'
            if self.selectionName != '':
                config.addOutputVar (self.fatJets.split('.')[0], self.outputLabel + '_%SYS%', 'select_or', noSys=self.nominalOnly)


        # Create the overlap removal algorithm:
        alg = config.createAlgorithm( 'CP::OverlapRemovalAlg', 'OverlapRemovalAlg' + postfix )
        alg.OutputLabel = self.outputLabel
        if self.nominalOnly :
            alg.affectingSystematicsFilter = '.*'
        if electrons :
            alg.electrons = electrons
            alg.electronsDecoration = self.outputLabel + '_%SYS%,as_char'
            config.addSelection (self.electrons.split('.')[0], electronsSelectionName, alg.electronsDecoration, preselection=False, comesFrom='or')
        if muons :
            alg.muons = muons
            alg.muonsDecoration = self.outputLabel + '_%SYS%,as_char'
            config.addSelection (self.muons.split('.')[0], muonsSelectionName, alg.muonsDecoration, preselection=False, comesFrom='or')
        if taus :
            alg.taus = taus
            alg.tausDecoration = self.outputLabel + '_%SYS%,as_char'
            config.addSelection (self.taus.split('.')[0], tausSelectionName, alg.tausDecoration, preselection=False, comesFrom='or')
        if jets :
            alg.jets = jets
            alg.jetsDecoration = self.outputLabel + '_%SYS%,as_char'
            config.addSelection (self.jets.split('.')[0], jetsSelectionName, alg.jetsDecoration, preselection=False, comesFrom='or')
        if photons :
            alg.photons = photons
            alg.photonsDecoration = self.outputLabel + '_%SYS%,as_char'
            config.addSelection (self.photons.split('.')[0], photonsSelectionName, alg.photonsDecoration, preselection=False, comesFrom='or')
        if fatJets :
            alg.fatJets = fatJets
            alg.fatJetsDecoration = self.outputLabel + '_%SYS%,as_char'
            config.addSelection (self.fatJets.split('.')[0], fatJetsSelectionName, alg.fatJetsDecoration, preselection=False, comesFrom='or')

        # Create its main tool, and set its basic properties:
        config.addPrivateTool( 'overlapTool', 'ORUtils::OverlapRemovalTool' )
        alg.overlapTool.InputLabel = self.inputLabel
        alg.overlapTool.OutputLabel = self.outputLabel

        # By default the OverlapRemovalTool would flag objects that need to be
        # suppressed, with a "true" value. But since the analysis algorithms expect
        # the opposite behaviour from selection flags, we need to tell the tool
        # explicitly to use the "true" flag on objects that pass the overlap
        # removal.
        alg.overlapTool.OutputPassValue = True

        # Set up the electron-electron overlap removal, if requested.
        if electrons and self.doEleEleOR:
            config.addPrivateTool( 'overlapTool.EleEleORT',
                                   'ORUtils::EleEleOverlapTool' )
            alg.overlapTool.EleEleORT.InputLabel = self.inputLabel
            alg.overlapTool.EleEleORT.OutputLabel = self.outputLabel
            alg.overlapTool.EleEleORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.EleEleORT.OutputPassValue = True

        # Set up the electron-muon overlap removal.
        if electrons and muons and self.doEleMuOR:
            config.addPrivateTool( 'overlapTool.EleMuORT',
                                   'ORUtils::EleMuSharedTrkOverlapTool' )
            alg.overlapTool.EleMuORT.InputLabel = self.inputLabel
            alg.overlapTool.EleMuORT.OutputLabel = self.outputLabel
            alg.overlapTool.EleMuORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.EleMuORT.OutputPassValue = True

        # Set up the electron-(narrow-)jet overlap removal.
        if electrons and jets and self.doEleJetOR:
            config.addPrivateTool( 'overlapTool.EleJetORT',
                                   'ORUtils::EleJetOverlapTool' )
            alg.overlapTool.EleJetORT.InputLabel = self.inputLabel
            alg.overlapTool.EleJetORT.OutputLabel = self.outputLabel
            alg.overlapTool.EleJetORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.EleJetORT.BJetLabel = self.bJetLabel
            alg.overlapTool.EleJetORT.UseSlidingDR = self.boostedLeptons
            alg.overlapTool.EleJetORT.EnableUserPriority = self.enableUserPriority
            alg.overlapTool.EleJetORT.OutputPassValue = True

        # Set up the muon-(narrow-)jet overlap removal.
        if muons and jets and self.doMuJetOR:
            config.addPrivateTool( 'overlapTool.MuJetORT',
                                   'ORUtils::MuJetOverlapTool' )
            alg.overlapTool.MuJetORT.InputLabel = self.inputLabel
            alg.overlapTool.MuJetORT.OutputLabel = self.outputLabel
            alg.overlapTool.MuJetORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.MuJetORT.BJetLabel = self.bJetLabel
            alg.overlapTool.MuJetORT.UseSlidingDR = self.boostedLeptons
            alg.overlapTool.MuJetORT.EnableUserPriority = self.enableUserPriority
            alg.overlapTool.MuJetORT.OutputPassValue = True

        # Set up the tau-electron overlap removal.
        if taus and electrons and self.doTauEleOR:
            config.addPrivateTool( 'overlapTool.TauEleORT',
                                   'ORUtils::DeltaROverlapTool' )
            alg.overlapTool.TauEleORT.InputLabel = self.inputLabel
            alg.overlapTool.TauEleORT.OutputLabel = self.outputLabel
            alg.overlapTool.TauEleORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.TauEleORT.DR = 0.2
            alg.overlapTool.TauEleORT.OutputPassValue = True

        # Set up the tau-muon overlap removal.
        if taus and muons and self.doTauMuOR:
            config.addPrivateTool( 'overlapTool.TauMuORT',
                                   'ORUtils::DeltaROverlapTool' )
            alg.overlapTool.TauMuORT.InputLabel = self.inputLabel
            alg.overlapTool.TauMuORT.OutputLabel = self.outputLabel
            alg.overlapTool.TauMuORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.TauMuORT.DR = 0.2
            alg.overlapTool.TauMuORT.OutputPassValue = True

        # Set up the tau-(narrow-)jet overlap removal.
        if taus and jets and self.doTauJetOR:
            if self.doTauAntiTauJetOR:
                config.addPrivateTool( 'overlapTool.TauJetORT',
                                       'ORUtils::TauAntiTauJetOverlapTool' )
                alg.overlapTool.TauJetORT.TauLabel = self.antiTauIDTauLabel
                alg.overlapTool.TauJetORT.AntiTauLabel = self.antiTauLabel
                alg.overlapTool.TauJetORT.BJetLabel = self.antiTauBJetLabel
            else:
                config.addPrivateTool( 'overlapTool.TauJetORT',
                                       'ORUtils::DeltaROverlapTool' )

            alg.overlapTool.TauJetORT.InputLabel = self.inputLabel
            alg.overlapTool.TauJetORT.OutputLabel = self.outputLabel
            alg.overlapTool.TauJetORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.TauJetORT.DR = 0.2
            alg.overlapTool.TauJetORT.EnableUserPriority = self.enableUserPriority
            alg.overlapTool.TauJetORT.OutputPassValue = True

        # Set up the photon-electron overlap removal.
        if photons and electrons and self.doPhEleOR:
            config.addPrivateTool( 'overlapTool.PhoEleORT',
                                   'ORUtils::DeltaROverlapTool' )
            alg.overlapTool.PhoEleORT.InputLabel = self.inputLabel
            alg.overlapTool.PhoEleORT.OutputLabel = self.outputLabel
            alg.overlapTool.PhoEleORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.PhoEleORT.OutputPassValue = True

        # Set up the photon-muon overlap removal.
        if photons and muons and self.doPhMuOR:
            config.addPrivateTool( 'overlapTool.PhoMuORT',
                                   'ORUtils::DeltaROverlapTool' )
            alg.overlapTool.PhoMuORT.InputLabel = self.inputLabel
            alg.overlapTool.PhoMuORT.OutputLabel = self.outputLabel
            alg.overlapTool.PhoMuORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.PhoMuORT.OutputPassValue = True

        # Set up the photon-(narrow-)jet overlap removal.
        if photons and jets and self.doPhJetOR:
            config.addPrivateTool( 'overlapTool.PhoJetORT',
                                   'ORUtils::DeltaROverlapTool' )
            alg.overlapTool.PhoJetORT.InputLabel = self.inputLabel
            alg.overlapTool.PhoJetORT.OutputLabel = self.outputLabel
            alg.overlapTool.PhoJetORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.PhoJetORT.EnableUserPriority = self.enableUserPriority
            alg.overlapTool.PhoJetORT.OutputPassValue = True

        # Set up the electron-fat-jet overlap removal.
        if electrons and fatJets and self.doEleFatJetOR:
            config.addPrivateTool( 'overlapTool.EleFatJetORT',
                            'ORUtils::DeltaROverlapTool' )
            alg.overlapTool.EleFatJetORT.InputLabel = self.inputLabel
            alg.overlapTool.EleFatJetORT.OutputLabel = self.outputLabel
            alg.overlapTool.EleFatJetORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.EleFatJetORT.DR = 1.0
            alg.overlapTool.EleFatJetORT.OutputPassValue = True

        # Set up the (narrow-)jet-fat-jet overlap removal.
        if jets and fatJets and self.doJetFatJetOR:
            config.addPrivateTool( 'overlapTool.JetFatJetORT',
                                   'ORUtils::DeltaROverlapTool' )
            alg.overlapTool.JetFatJetORT.InputLabel = self.inputLabel
            alg.overlapTool.JetFatJetORT.OutputLabel = self.outputLabel
            alg.overlapTool.JetFatJetORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.JetFatJetORT.DR = 1.0
            alg.overlapTool.JetFatJetORT.OutputPassValue = True
        
        if self.nominalOnly :
            if electrons :
                alg = config.createAlgorithm( 'CP::CopyNominalSelectionAlg', 'ORElectronsCopyAlg' + postfix)
                alg.particles = electrons
                alg.selectionDecoration = self.outputLabel + '_%SYS%,as_char'
            if muons :
                alg = config.createAlgorithm( 'CP::CopyNominalSelectionAlg', 'ORMuonsCopyAlg' + postfix)
                alg.particles = muons
                alg.selectionDecoration = self.outputLabel + '_%SYS%,as_char'
            if taus :
                alg = config.createAlgorithm( 'CP::CopyNominalSelectionAlg', 'ORTausCopyAlg' + postfix)
                alg.particles = taus
                alg.selectionDecoration = self.outputLabel + '_%SYS%,as_char'
            if jets :
                alg = config.createAlgorithm( 'CP::CopyNominalSelectionAlg', 'ORJetsCopyAlg' + postfix)
                alg.particles = jets
                alg.selectionDecoration = self.outputLabel + '_%SYS%,as_char'
            if photons :
                alg = config.createAlgorithm( 'CP::CopyNominalSelectionAlg', 'ORPhotonsCopyAlg' + postfix)
                alg.particles = photons
                alg.selectionDecoration = self.outputLabel + '_%SYS%,as_char'
            if fatJets :
                alg = config.createAlgorithm( 'CP::CopyNominalSelectionAlg', 'ORFatJetsCopyAlg' + postfix)
                alg.particles = fatJets
                alg.selectionDecoration = self.outputLabel + '_%SYS%,as_char'

        # provide a preselection if requested
        if self.addPreselection:
            if self.preselectLabel is not None :
                preselectLabel = self.preselectLabel
            else :
                preselectLabel = self.outputLabel

            if electrons :
                alg = config.createAlgorithm( 'CP::AsgUnionPreselectionAlg','ORElectronsPreselectionAlg' + postfix )
                alg.particles = electrons
                alg.preselection = '&&'.join (config.getPreselection (self.electrons.split('.')[0], electronsSelectionName, asList=True)
                        + [self.outputLabel + '_%SYS%,as_char'])
                alg.selectionDecoration = preselectLabel
                config.addSelection (self.electrons.split('.')[0], electronsSelectionName, alg.selectionDecoration+',as_char', bits=1, preselection=True, comesFrom='or')
            if muons :
                alg = config.createAlgorithm( 'CP::AsgUnionPreselectionAlg','ORMuonsPreselectionAlg' + postfix )
                alg.particles = muons
                alg.preselection = '&&'.join (config.getPreselection (self.muons.split('.')[0], muonsSelectionName, asList=True)
                        + [self.outputLabel + '_%SYS%,as_char'])
                alg.selectionDecoration = preselectLabel
                config.addSelection (self.muons.split('.')[0], muonsSelectionName, alg.selectionDecoration+',as_char', bits=1, preselection=True, comesFrom='or')
            if taus :
                alg = config.createAlgorithm( 'CP::AsgUnionPreselectionAlg','ORTausPreselectionAlg' + postfix )
                alg.particles = taus
                alg.preselection = '&&'.join (config.getPreselection (self.taus.split('.')[0], tausSelectionName, asList=True)
                        + [self.outputLabel + '_%SYS%,as_char'])
                alg.selectionDecoration = preselectLabel
                config.addSelection (self.taus.split('.')[0], tausSelectionName, alg.selectionDecoration+',as_char', bits=1, preselection=True, comesFrom='or')
            if jets :
                alg = config.createAlgorithm( 'CP::AsgUnionPreselectionAlg','ORJetsPreselectionAlg' + postfix )
                alg.particles = jets
                alg.preselection = '&&'.join (config.getPreselection (self.jets.split('.')[0], jetsSelectionName, asList=True)
                        + [self.outputLabel + '_%SYS%,as_char'])
                alg.selectionDecoration = preselectLabel
                config.addSelection (self.jets.split('.')[0], jetsSelectionName, alg.selectionDecoration+',as_char', bits=1, preselection=True, comesFrom='or')
            if photons :
                alg = config.createAlgorithm( 'CP::AsgUnionPreselectionAlg','ORPhotonsPreselectionAlg' + postfix )
                alg.particles = photons
                alg.preselection = '&&'.join (config.getPreselection (self.photons.split('.')[0], photonsSelectionName, asList=True)
                        + [self.outputLabel + '_%SYS%,as_char'])
                alg.selectionDecoration = preselectLabel
                config.addSelection (self.photons.split('.')[0], photonsSelectionName, alg.selectionDecoration+',as_char', bits=1, preselection=True, comesFrom='or')
            if fatJets :
                alg = config.createAlgorithm( 'CP::AsgUnionPreselectionAlg','ORFatJetsPreselectionAlg' + postfix )
                alg.particles = fatJets
                alg.preselection = '&&'.join (config.getPreselection (self.fatJets.split('.')[0], fatJetsSelectionName, asList=True)
                        + [self.outputLabel + '_%SYS%,as_char'])
                alg.selectionDecoration = preselectLabel
                config.addSelection (self.fatJets.split('.')[0], fatJetsSelectionName, alg.selectionDecoration+',as_char', bits=1, preselection=True, comesFrom='or')



def makeOverlapAnalysisConfig( seq,
                               inputLabel = None, outputLabel = None,
                               linkOverlapObjects = None,
                               doEleEleOR = None, electrons = None,
                               muons = None, jets = None,
                               taus = None, doTauAntiTauJetOR = None,
                               photons = None, fatJets = None,
                               enableUserPriority = None,
                               bJetLabel = None,
                               antiTauIDTauLabel = None, antiTauLabel = None,
                               antiTauBJetLabel = None,
                               boostedLeptons = None,
                               postfix = None):
    """Function creating the overlap removal algorithm sequence

    The function sets up a multi-input/multi-output analysis algorithm sequnce,
    which needs to be used in a quite particular way. First off you need to set
    the arguments of this function correctly.

    Function keyword arguments:
      inputLabel -- Any possible label used to pick up the selected objects
                    with.  This should not be a label already used otherwise.
      outputLabel -- Decoration put on the output variables. Set to "true" for
                     objects passing the overlap removal.
      linkOverlapObjects -- Set up an element link between overlapping objects
      doEleEleOR -- Set up electron-electron overlap removal
      doTauAntiTauJetOR -- Set up Tau-AntiTau-Jet overlap removal
      enableUserPriority -- If enabled, the Ele-, Mu-, Tau- and PhoJetOR tools
                            will respect the user priority in the inputLabel.
                            E.g. SUSYTools assigns all signal objects the
                            priority 2 and pre-selected jets the priority 1.
      bJetLabel -- Flag to select b-jets with for lepton OR.
                   If left empty, no b-jets are used in the overlap removal.
      antiTauIDTauLabel -- Flag to select ID tau with for Tau-AntiTau-Jet OR.
      antiTauLabel -- Flag to select antiTau with. Required for Tau-AntiTau-Jet OR.
      antiTauBJetLabel -- Flag to select b-jets with for Tau-AntiTau-Jet OR.
      boostedLeptons -- Set to True to enable boosted lepton overlap removal
    """

    config = OverlapAnalysisConfig()
    config.setOptionValue ('inputLabel', inputLabel)
    config.setOptionValue ('outputLabel', outputLabel)
    config.setOptionValue ('linkOverlapObjects', linkOverlapObjects)
    config.setOptionValue ('doEleEleOR', doEleEleOR)
    config.setOptionValue ('electrons', electrons)
    config.setOptionValue ('muons', muons)
    config.setOptionValue ('jets', jets)
    config.setOptionValue ('taus', taus)
    config.setOptionValue ('doTauAntiTauJetOR', doTauAntiTauJetOR)
    config.setOptionValue ('photons', photons)
    config.setOptionValue ('fatJets', fatJets)
    config.setOptionValue ('enableUserPriority', enableUserPriority)
    config.setOptionValue ('bJetLabel', bJetLabel)
    config.setOptionValue ('antiTauIDTauLabel', antiTauIDTauLabel)
    config.setOptionValue ('antiTauLabel', antiTauLabel)
    config.setOptionValue ('antiTauBJetLabel', antiTauBJetLabel)
    config.setOptionValue ('boostedLeptons', boostedLeptons)
    config.setOptionValue ('postfix', postfix)
    seq.append (config)
