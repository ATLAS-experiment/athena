# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class OverlapAnalysisConfig (ConfigBlock):
    """the ConfigBlock for the OverlapRemoval configuration"""

    def __init__ (self) :
        super (OverlapAnalysisConfig, self).__init__ ()
        self.setBlockName('OverlapRemoval')
        self.addOption ('inputLabel', '', type=str,
            info="any possible label used to pick up the selected objects with. This should not be a label already used elsewhere; try e.g. `preselectOR`.")
        self.addOption ('outputLabel', 'passesOR', type=str,
            info="decoration applied (internally) to the output objects, e.g. `passesOR`.")
        self.addOption ('selectionName', None, type=str,
            info="name of the common selection to which to append the OR decision, needed to distinguish between the various overlap removal strategies that may be set up. If left empty, apply the OR decision to the entire input containers.")
        self.addOption ('linkOverlapObjects', False, type=bool,
            info="whether to set up an element link between overlapping objects.")
        self.addOption ('enableUserPriority', False, type=bool,
            info="whether to use the user's custom priority ranking, instead of the recommended one. If set to `True`, will respect the priorities set with `inputLabel` (e.g. in SUSYTools, every object gets priority 2, but pre-selected jets get priority 1).")
        self.addOption ('forceOROutputFlags', False, type=bool,
            info="whether to add the OR decisions to the output in any case. As default, the OR decisions are only added if the OR is a standalone selection and not added to any other selection.")
        self.addOption ('forceOROutputFlagName', '', type=str,
            info="override the default output name for the OR decisions, which is otherwise automatically constructed dependent on the configuration.")
        self.addOption ('bJetLabel', '', type=str,
            info="flag to select b-jets with. If left empty, no b-jets are used in the overlap removal.")
        self.addOption ('InnerDR', 0.2, type=float,
            info="radius of the inner cone for removing jets (`EleJetORT`/`MuJetORT`).")
        self.addOption ('OuterDR', 0.4, type=float,
            info="radius of the outer cone for removing leptons (`EleJetORT`/`MuJetORT`)")
        self.addOption ('boostedLeptons', False, type=bool,
            info="whether to enable boosted lepton overlap removal (toggles on the property `UseSlidingDR` of the `ORUtils::EleJetOverlapTool` and `ORUtils::MuJetOverlapTool` tools).")
        self.addOption ('nominalOnly', False, type=bool,
            info="(experimental) toggle off the running of overlap removal on systematically-varied objects (instead, copy from nominal).",
            expertMode=True)
        self.addOption ('nominalOnlyUnifiedSelection', False, type=bool,
            info="(experimental) toggle off the running of overlap removal on systematically-varied objects (instead, copy from nominal), but consider the union of all systematically-varied object selections (not just nominal).",
            expertMode=True)
        self.addOption ('jets', "", type=str,
            info="the input jet container.")
        self.addOption ('fatJets', "", type=str,
            info="the input large-R jet container.")
        self.addOption ('electrons', "", type=str,
            info="the input electron container.")
        self.addOption ('muons', "", type=str,
            info="the input muon container.")
        self.addOption ('photons', "", type=str,
            info="the input photon container.")
        self.addOption ('taus', "", type=str,
            info="the input tau-jet container.")
        self.addOption ('antiTauIDTauLabel', '', type=str,
            info="flag to select the ID tau-jet for the tau-antitau-jet overlap removal.")
        self.addOption ('antiTauLabel', '', type=str,
            info="flag to select the anti-tau-jet for the tau-antitau-jet overlap removal.")
        self.addOption ('antiTauBJetLabel', '', type=str,
            info="flag to select b-jets for the tau-antitau-jet overlap removal.")
        self.addOption ('addToAllSelections', None, type=bool,
            info="add OR selection decision into all object selections. For most users, this should only be set to `True` if there is only one Overlap Removal setup, and to `False` otherwise. If set to `None`, this is handled automatically.")
        self.addOption ('addPreselection', None, type=bool,
            info="add preselection decorations without systematics. If set to `None`, will be turned on in case of multiple Overlap Removal setups.")
        self.addOption ('preselectLabel', None, type=str,
            info="label to use when adding preselection decorations. If set to `None`, uses the `outputLabel` instead.")
        self.addOption ('jetsSelectionName', None, type=str,
            info="name of the selection on the jet container to create/append the OR decision. If set to `None`, uses the `selectionName` instead.")
        self.addOption ('fatJetsSelectionName', None, type=str,
            info="name of the selection on the large-R jet container to create/append the OR decision. If set to `None`, uses the `selectionName` instead.")
        self.addOption ('electronsSelectionName', None, type=str,
            info="name of the selection on the electron container to create/append the OR decision. If set to `None`, uses the `selectionName` instead.")
        self.addOption ('muonsSelectionName', None, type=str,
            info="name of the selection on the muon container to create/append the OR decision. If set to `None`, uses the `selectionName` instead.")
        self.addOption ('photonsSelectionName', None, type=str,
            info="name of the selection on the photon container to create/append the OR decision. If set to `None`, uses the `selectionName` instead.")
        self.addOption ('tausSelectionName', None, type=str,
            info="name of the selection on the tau-jet container to create/append the OR decision. If set to `None`, uses the `selectionName` instead.")
        self.addOption ('doEleEleOR', False, type=bool,
            info="whether to perform the overlap removal amongst electrons.")
        self.addOption ('doEleMuOR', True, type=bool,
            info="whether to perform the overlap removal between electrons and muons.")
        self.addOption ('doEleJetOR', True, type=bool,
            info="whether to perform the overlap removal between electrons and jets.")
        self.addOption ('doMuJetOR', True, type=bool,
            info="whether to perform the overlap removal between muons and jets.")
        self.addOption ('doTauEleOR', True, type=bool,
            info="whether to perform the overlap removal between tau-jets and electrons.")
        self.addOption ('doTauMuOR', True, type=bool,
            info="whether to perform the overlap removal between tau-jets and muons.")
        self.addOption ('doTauJetOR', True, type=bool,
            info="whether to perform the overlap removal between tau-jets and jets.")
        self.addOption ('doTauAntiTauJetOR', False, type=bool,
            info="whether to perform the overlap removal between tau-jets and anti-tau-jets.")
        self.addOption ('doPhEleOR', True, type=bool,
            info="whether to perform the overlap removal between photons and electrons.")
        self.addOption ('doPhMuOR', True, type=bool,
            info="whether to perform the overlap removal between photons and muons.")
        self.addOption ('doPhJetOR', True, type=bool,
            info="whether to perform the overlap removal between photons and jets.")
        self.addOption ('doEleFatJetOR', True, type=bool,
            info="whether to perform the overlap removal between electrons and large-R jets.")
        self.addOption ('doJetFatJetOR', True, type=bool,
            info="whether to perform the overlap removal between jets and large-R jets.")
        self.addOption ('favourPhotonOverLepton', False, type=bool,
            info="whether to give priority to photons in the overlap removal between leptons and photons.")
        self.addOption ('allowNoPV', False, type=bool,
            info="whether to allow proceeding with overlap removal even when no primary vertex is found.")


    def instanceName (self) :
        """Return the instance name for this block"""
        if self.selectionName is not None:
            return self.selectionName
        return self.outputLabel

    def makeUnionPreselectionAlg(self, config, inputCollection):
        """
        Create a new selection for the inputCollection ('container.selection')
        that is the union over all systematics of 'selection'. This allows us
        to run nominal-only overlap removal while taking into account the impact
        of systematics on object acceptance.
        """
        container, selection = config.readNameAndSelection( inputCollection )
        alg = config.createAlgorithm( 'CP::AsgUnionSelectionAlg', 'UnionSelectionAlgForOR_' + inputCollection.split(".")[0] )
        alg.preselection = selection
        alg.particles = container
        alg.selectionDecoration = 'unifiedSelectForOR'

    def makeAlgs (self, config) :

        import logging, sys
        log = logging.getLogger('OverlapRemovalConfig')

        if self.addToAllSelections is None:
            # we resolve this automatically for the user:
            # - if there is only one OverlapRemoval config block setup registered,
            #   we set addToAllSections to True so that the OR decision is propagated
            #   to all relevant particles
            # - if there is more than one, then we set it to False
            # this is the desired behaviour in most cases! If not, set addToAllSelections
            # yourself :)
            numORblocks = OverlapAnalysisConfig.get_instance_count()
            if numORblocks == 1:
                # there is only one OR setup
                self.addToAllSelections = True
                if self.addPreselection is None:
                    self.addPreselection = False
            else:
                # there are more than one OR setups
                self.addToAllSelections = False
                if self.addPreselection is None:
                    self.addPreselection = True

        if self.selectionName is not None:
            selectionName = self.selectionName
            outputLabel = self.outputLabel + '_' + selectionName
            inputLabel = self.inputLabel + '_' + selectionName
            select_or_decoration = 'select_or_' + self.selectionName
        else:
            if self.addToAllSelections:
                selectionName = ""
                select_or_decoration = 'select_or'
            else:
                selectionName = self.outputLabel
                select_or_decoration = 'select_' + self.outputLabel
            outputLabel = self.outputLabel
            inputLabel = self.inputLabel

        # in case of multiple ORs, enforce that users explicitly specify the selection name for each input container: either create a new selection or attach the OR decision to an existing one
        if OverlapAnalysisConfig.get_instance_count() > 1:
            # warn about a potential misconfiguration
            if self.addToAllSelections:
                log.warning("More than one OverlapRemoval config scheduled but 'addToAllSelections' is set to True. This is probably not what you want.")

            if self.electrons and not self.electronsSelectionName:
                log.error("More than one OverlapRemoval config scheduled: please explicitly specify the selection name of the OR decision for electrons with 'electronsSelectionName'.")
                sys.exit(1)

            if self.muons and not self.muonsSelectionName:
                log.error("More than one OverlapRemoval config scheduled: please explicitly specify the selection name of the OR decision for muons with 'muonsSelectionName'.")
                sys.exit(1)

            if self.photons and not self.photonsSelectionName:
                log.error("More than one OverlapRemoval config scheduled: please explicitly specify the selection name of the OR decision for photons with 'photonsSelectionName'.")
                sys.exit(1)

            if self.taus and not self.tausSelectionName:
                log.error("More than one OverlapRemoval config scheduled: please explicitly specify the selection name of the OR decision for tau-jets with 'tausSelectionName'.")
                sys.exit(1)

            if self.jets and not self.jetsSelectionName:
                log.error("More than one OverlapRemoval config scheduled: please explicitly specify the selection name of the OR decision for jets with 'jetsSelectionName'.")
                sys.exit(1)

            if self.fatJets and not self.fatJetsSelectionName:
                log.error("More than one OverlapRemoval config scheduled: please explicitly specify the selection name of the OR decision for large-R jets with 'fatJetsSelectionName'.")
                sys.exit(1)

        # here the logic is:
        # - either the user has provided a specific selection name for the object, and we use that one
        # - or they haven't and then we use selectionName (which is either specified or '', i.e. everything)
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

        # helper function to decide whether to propagate OR flags to output
        def enableOutput(containerName, selectionName):
            # if we force the output, add them
            if self.forceOROutputFlags:
                return True

            # if selection name is '' the OR selection gets automatically applied everywhere, no point in adding to output
            if selectionName == '':
                return False

            # if OR decision is added to selection of input container, there is also no point in adding to output
            if selectionName in config.getSelectionNames(containerName.split(".")[0]):
                return False

            # otherwise, OR decision is standalone selection, so add it to output
            return True

        # the logic to define the output name of the OR decision is as follows:
        # - if the user specified a selection name for the container, use that to construct the output name
        # - otherwise, use the generic 'select_or_decoration' label defined above

        electrons = None
        if self.electrons != "" :
            if self.nominalOnlyUnifiedSelection:
                self.makeUnionPreselectionAlg(config, self.electrons)
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg','ORElectronsSelectAlg' )
            electrons, alg.preselection = config.readNameAndSelection (self.electrons)
            alg.particles = electrons
            alg.selectionDecoration = inputLabel + ',as_char'
            if enableOutput(self.electrons, electronsSelectionName):
                outputName = "select_" + self.electronsSelectionName if self.electronsSelectionName else select_or_decoration
                config.addOutputVar (self.electrons.split('.')[0],
                                    outputLabel + '_%SYS%',
                                    outputName if not self.forceOROutputFlagName else "select_" + self.forceOROutputFlagName,
                                    noSys=self.nominalOnly or self.nominalOnlyUnifiedSelection)
            if self.nominalOnlyUnifiedSelection:
                alg.preselection = 'unifiedSelectForOR'

        photons = None
        if self.photons != "" :
            if self.nominalOnlyUnifiedSelection:
                self.makeUnionPreselectionAlg(config, self.photons)
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg','ORPhotonsSelectAlg' )
            photons, alg.preselection = config.readNameAndSelection (self.photons)
            alg.particles = photons
            alg.selectionDecoration = inputLabel + ',as_char'
            if enableOutput(self.photons, photonsSelectionName):
                outputName = "select_" + self.photonsSelectionName if self.photonsSelectionName else select_or_decoration
                config.addOutputVar (self.photons.split('.')[0],
                                    outputLabel + '_%SYS%',
                                    outputName if not self.forceOROutputFlagName else "select_" + self.forceOROutputFlagName,
                                    noSys=self.nominalOnly or self.nominalOnlyUnifiedSelection)
            if self.nominalOnlyUnifiedSelection:
                alg.preselection = 'unifiedSelectForOR'

        muons = None
        if self.muons != "" :
            if self.nominalOnlyUnifiedSelection:
                self.makeUnionPreselectionAlg(config, self.muons)
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg','ORMuonsSelectAlg' )
            muons, alg.preselection = config.readNameAndSelection (self.muons)
            alg.particles = muons
            alg.selectionDecoration = inputLabel + ',as_char'
            if enableOutput(self.muons, muonsSelectionName):
                outputName = "select_" + self.muonsSelectionName if self.muonsSelectionName else select_or_decoration
                config.addOutputVar (self.muons.split('.')[0],
                                    outputLabel + '_%SYS%',
                                    outputName if not self.forceOROutputFlagName else "select_" + self.forceOROutputFlagName,
                                    noSys=self.nominalOnly or self.nominalOnlyUnifiedSelection)
            if self.nominalOnlyUnifiedSelection:
                alg.preselection = 'unifiedSelectForOR'

        taus = None
        if self.taus != "" :
            if self.nominalOnlyUnifiedSelection:
                self.makeUnionPreselectionAlg(config, self.taus)
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg','ORTausSelectAlg' )
            taus, alg.preselection = config.readNameAndSelection (self.taus)
            alg.particles = taus
            alg.selectionDecoration = inputLabel + ',as_char'
            if enableOutput(self.taus, tausSelectionName):
                outputName = "select_" + self.tausSelectionName if self.tausSelectionName else select_or_decoration
                config.addOutputVar (self.taus.split('.')[0],
                                    outputLabel + '_%SYS%',
                                    outputName if not self.forceOROutputFlagName else "select_" + self.forceOROutputFlagName,
                                    noSys=self.nominalOnly or self.nominalOnlyUnifiedSelection)
            if self.nominalOnlyUnifiedSelection:
                alg.preselection = 'unifiedSelectForOR'

        jets = None
        if self.jets != "" :
            if self.nominalOnlyUnifiedSelection:
                self.makeUnionPreselectionAlg(config, self.jets)
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg','ORJetsSelectAlg' )
            jets, alg.preselection = config.readNameAndSelection (self.jets)
            alg.particles = jets
            alg.selectionDecoration = inputLabel + ',as_char'
            if enableOutput(self.jets, jetsSelectionName):
                outputName = "select_" + self.jetsSelectionName if self.jetsSelectionName else select_or_decoration
                config.addOutputVar (self.jets.split('.')[0],
                                    outputLabel + '_%SYS%',
                                    outputName if not self.forceOROutputFlagName else "select_" + self.forceOROutputFlagName,
                                    noSys=self.nominalOnly or self.nominalOnlyUnifiedSelection)
            if self.nominalOnlyUnifiedSelection:
                alg.preselection = 'unifiedSelectForOR'

        fatJets = None
        if self.fatJets != "" :
            if self.nominalOnlyUnifiedSelection:
                self.makeUnionPreselectionAlg(config, self.fatJets)
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg','ORFatJetsSelectAlg' )
            fatJets, alg.preselection = config.readNameAndSelection (self.fatJets)
            alg.particles = fatJets
            alg.selectionDecoration = inputLabel + ',as_char'
            if enableOutput(self.fatJets, fatJetsSelectionName):
                outputName = "select_" + self.fatJetsSelectionName if self.fatJetsSelectionName else select_or_decoration
                config.addOutputVar (self.fatJets.split('.')[0],
                                    outputLabel + '_%SYS%',
                                    outputName if not self.forceOROutputFlagName else "select_" + self.forceOROutputFlagName,
                                    noSys=self.nominalOnly or self.nominalOnlyUnifiedSelection)
            if self.nominalOnlyUnifiedSelection:
                alg.preselection = 'unifiedSelectForOR'



        # Create the overlap removal algorithm:
        # Note: we use writeToOutput=False for the selections defined below as we add them manually to the output above already, otherwise they are picked up by createSelectionFlagBranches() of the OutputAnalysisConfig again
        alg = config.createAlgorithm( 'CP::OverlapRemovalAlg', 'OverlapRemovalAlg' )
        alg.OutputLabel = outputLabel
        if self.nominalOnly or self.nominalOnlyUnifiedSelection :
            alg.affectingSystematicsFilter = '.*'
        if electrons :
            alg.electrons = electrons
            alg.electronsDecoration = outputLabel + '_%SYS%,as_char'
            config.addSelection (self.electrons.split('.')[0], electronsSelectionName, alg.electronsDecoration, preselection=False, comesFrom='or', writeToOutput=False)
        if muons :
            alg.muons = muons
            alg.muonsDecoration = outputLabel + '_%SYS%,as_char'
            config.addSelection (self.muons.split('.')[0], muonsSelectionName, alg.muonsDecoration, preselection=False, comesFrom='or', writeToOutput=False)
        if taus :
            alg.taus = taus
            alg.tausDecoration = outputLabel + '_%SYS%,as_char'
            config.addSelection (self.taus.split('.')[0], tausSelectionName, alg.tausDecoration, preselection=False, comesFrom='or', writeToOutput=False)
        if jets :
            alg.jets = jets
            alg.jetsDecoration = outputLabel + '_%SYS%,as_char'
            config.addSelection (self.jets.split('.')[0], jetsSelectionName, alg.jetsDecoration, preselection=False, comesFrom='or', writeToOutput=False)
        if photons :
            alg.photons = photons
            alg.photonsDecoration = outputLabel + '_%SYS%,as_char'
            config.addSelection (self.photons.split('.')[0], photonsSelectionName, alg.photonsDecoration, preselection=False, comesFrom='or', writeToOutput=False)
        if fatJets :
            alg.fatJets = fatJets
            alg.fatJetsDecoration = outputLabel + '_%SYS%,as_char'
            config.addSelection (self.fatJets.split('.')[0], fatJetsSelectionName, alg.fatJetsDecoration, preselection=False, comesFrom='or', writeToOutput=False)

        # Create its main tool, and set its basic properties:
        config.addPrivateTool( 'overlapTool', 'ORUtils::OverlapRemovalTool' )
        alg.overlapTool.InputLabel = inputLabel
        alg.overlapTool.OutputLabel = outputLabel

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
            alg.overlapTool.EleEleORT.InputLabel = inputLabel
            alg.overlapTool.EleEleORT.OutputLabel = outputLabel
            alg.overlapTool.EleEleORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.EleEleORT.OutputPassValue = True

        # Set up the electron-muon overlap removal.
        if electrons and muons and self.doEleMuOR:
            config.addPrivateTool( 'overlapTool.EleMuORT',
                                   'ORUtils::EleMuSharedTrkOverlapTool' )
            alg.overlapTool.EleMuORT.InputLabel = inputLabel
            alg.overlapTool.EleMuORT.OutputLabel = outputLabel
            alg.overlapTool.EleMuORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.EleMuORT.OutputPassValue = True

        # Set up the electron-(narrow-)jet overlap removal.
        if electrons and jets and self.doEleJetOR:
            config.addPrivateTool( 'overlapTool.EleJetORT',
                                   'ORUtils::EleJetOverlapTool' )
            alg.overlapTool.EleJetORT.InputLabel = inputLabel
            alg.overlapTool.EleJetORT.OutputLabel = outputLabel
            alg.overlapTool.EleJetORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.EleJetORT.BJetLabel = self.bJetLabel
            alg.overlapTool.EleJetORT.InnerDR = self.InnerDR
            alg.overlapTool.EleJetORT.OuterDR = self.OuterDR
            alg.overlapTool.EleJetORT.UseSlidingDR = self.boostedLeptons
            alg.overlapTool.EleJetORT.EnableUserPriority = self.enableUserPriority
            alg.overlapTool.EleJetORT.OutputPassValue = True

        # Set up the muon-(narrow-)jet overlap removal.
        if muons and jets and self.doMuJetOR:
            config.addPrivateTool( 'overlapTool.MuJetORT',
                                   'ORUtils::MuJetOverlapTool' )
            alg.overlapTool.MuJetORT.InputLabel = inputLabel
            alg.overlapTool.MuJetORT.OutputLabel = outputLabel
            alg.overlapTool.MuJetORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.MuJetORT.BJetLabel = self.bJetLabel
            alg.overlapTool.MuJetORT.InnerDR = self.InnerDR
            alg.overlapTool.MuJetORT.OuterDR = self.OuterDR
            alg.overlapTool.MuJetORT.UseSlidingDR = self.boostedLeptons
            alg.overlapTool.MuJetORT.EnableUserPriority = self.enableUserPriority
            alg.overlapTool.MuJetORT.OutputPassValue = True
            alg.overlapTool.MuJetORT.AllowNoPV = self.allowNoPV

        # Set up the tau-electron overlap removal.
        if taus and electrons and self.doTauEleOR:
            config.addPrivateTool( 'overlapTool.TauEleORT',
                                   'ORUtils::DeltaROverlapTool' )
            alg.overlapTool.TauEleORT.InputLabel = inputLabel
            alg.overlapTool.TauEleORT.OutputLabel = outputLabel
            alg.overlapTool.TauEleORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.TauEleORT.DR = 0.2
            alg.overlapTool.TauEleORT.OutputPassValue = True

        # Set up the tau-muon overlap removal.
        if taus and muons and self.doTauMuOR:
            config.addPrivateTool( 'overlapTool.TauMuORT',
                                   'ORUtils::DeltaROverlapTool' )
            alg.overlapTool.TauMuORT.InputLabel = inputLabel
            alg.overlapTool.TauMuORT.OutputLabel = outputLabel
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

            alg.overlapTool.TauJetORT.InputLabel = inputLabel
            alg.overlapTool.TauJetORT.OutputLabel = outputLabel
            alg.overlapTool.TauJetORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.TauJetORT.DR = 0.2
            alg.overlapTool.TauJetORT.EnableUserPriority = self.enableUserPriority
            alg.overlapTool.TauJetORT.OutputPassValue = True

        # Set up the photon-electron overlap removal.
        if photons and electrons and self.doPhEleOR:
            config.addPrivateTool( 'overlapTool.PhoEleORT',
                                   'ORUtils::DeltaROverlapTool' )
            alg.overlapTool.PhoEleORT.InputLabel = inputLabel
            alg.overlapTool.PhoEleORT.OutputLabel = outputLabel
            alg.overlapTool.PhoEleORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.PhoEleORT.OutputPassValue = True
            if self.favourPhotonOverLepton:
                alg.overlapTool.PhoEleORT.SwapContainerPrecedence = True

        # Set up the photon-muon overlap removal.
        if photons and muons and self.doPhMuOR:
            config.addPrivateTool( 'overlapTool.PhoMuORT',
                                   'ORUtils::DeltaROverlapTool' )
            alg.overlapTool.PhoMuORT.InputLabel = inputLabel
            alg.overlapTool.PhoMuORT.OutputLabel = outputLabel
            alg.overlapTool.PhoMuORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.PhoMuORT.OutputPassValue = True
            if self.favourPhotonOverLepton:
                alg.overlapTool.PhoMuORT.SwapContainerPrecedence = True

        # Set up the photon-(narrow-)jet overlap removal.
        if photons and jets and self.doPhJetOR:
            config.addPrivateTool( 'overlapTool.PhoJetORT',
                                   'ORUtils::DeltaROverlapTool' )
            alg.overlapTool.PhoJetORT.InputLabel = inputLabel
            alg.overlapTool.PhoJetORT.OutputLabel = outputLabel
            alg.overlapTool.PhoJetORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.PhoJetORT.EnableUserPriority = self.enableUserPriority
            alg.overlapTool.PhoJetORT.OutputPassValue = True

        # Set up the electron-fat-jet overlap removal.
        if electrons and fatJets and self.doEleFatJetOR:
            config.addPrivateTool( 'overlapTool.EleFatJetORT',
                            'ORUtils::DeltaROverlapTool' )
            alg.overlapTool.EleFatJetORT.InputLabel = inputLabel
            alg.overlapTool.EleFatJetORT.OutputLabel = outputLabel
            alg.overlapTool.EleFatJetORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.EleFatJetORT.DR = 1.0
            alg.overlapTool.EleFatJetORT.OutputPassValue = True

        # Set up the (narrow-)jet-fat-jet overlap removal.
        if jets and fatJets and self.doJetFatJetOR:
            config.addPrivateTool( 'overlapTool.JetFatJetORT',
                                   'ORUtils::DeltaROverlapTool' )
            alg.overlapTool.JetFatJetORT.InputLabel = inputLabel
            alg.overlapTool.JetFatJetORT.OutputLabel = outputLabel
            alg.overlapTool.JetFatJetORT.LinkOverlapObjects = self.linkOverlapObjects
            alg.overlapTool.JetFatJetORT.DR = 1.0
            alg.overlapTool.JetFatJetORT.OutputPassValue = True

        if self.nominalOnly or self.nominalOnlyUnifiedSelection :
            if electrons :
                alg = config.createAlgorithm( 'CP::CopyNominalSelectionAlg', 'ORElectronsCopyAlg')
                alg.particles = electrons
                alg.selectionDecoration = outputLabel + '_%SYS%,as_char'
            if muons :
                alg = config.createAlgorithm( 'CP::CopyNominalSelectionAlg', 'ORMuonsCopyAlg')
                alg.particles = muons
                alg.selectionDecoration = outputLabel + '_%SYS%,as_char'
            if taus :
                alg = config.createAlgorithm( 'CP::CopyNominalSelectionAlg', 'ORTausCopyAlg')
                alg.particles = taus
                alg.selectionDecoration = outputLabel + '_%SYS%,as_char'
            if jets :
                alg = config.createAlgorithm( 'CP::CopyNominalSelectionAlg', 'ORJetsCopyAlg')
                alg.particles = jets
                alg.selectionDecoration = outputLabel + '_%SYS%,as_char'
            if photons :
                alg = config.createAlgorithm( 'CP::CopyNominalSelectionAlg', 'ORPhotonsCopyAlg')
                alg.particles = photons
                alg.selectionDecoration = outputLabel + '_%SYS%,as_char'
            if fatJets :
                alg = config.createAlgorithm( 'CP::CopyNominalSelectionAlg', 'ORFatJetsCopyAlg')
                alg.particles = fatJets
                alg.selectionDecoration = outputLabel + '_%SYS%,as_char'

        # provide a preselection if requested
        if self.addPreselection:
            if self.preselectLabel is not None :
                preselectLabel = self.preselectLabel
            else :
                preselectLabel = outputLabel

            if electrons :
                alg = config.createAlgorithm( 'CP::AsgUnionPreselectionAlg','ORElectronsPreselectionAlg')
                alg.particles = electrons
                alg.preselection = '&&'.join (config.getPreselection (self.electrons.split('.')[0], electronsSelectionName, asList=True)
                        + [outputLabel + '_%SYS%,as_char'])
                alg.selectionDecoration = preselectLabel
                config.addSelection (self.electrons.split('.')[0], electronsSelectionName, alg.selectionDecoration+',as_char', bits=1, preselection=True, comesFrom='or', writeToOutput=False)
            if muons :
                alg = config.createAlgorithm( 'CP::AsgUnionPreselectionAlg','ORMuonsPreselectionAlg')
                alg.particles = muons
                alg.preselection = '&&'.join (config.getPreselection (self.muons.split('.')[0], muonsSelectionName, asList=True)
                        + [outputLabel + '_%SYS%,as_char'])
                alg.selectionDecoration = preselectLabel
                config.addSelection (self.muons.split('.')[0], muonsSelectionName, alg.selectionDecoration+',as_char', bits=1, preselection=True, comesFrom='or', writeToOutput=False)
            if taus :
                alg = config.createAlgorithm( 'CP::AsgUnionPreselectionAlg','ORTausPreselectionAlg')
                alg.particles = taus
                alg.preselection = '&&'.join (config.getPreselection (self.taus.split('.')[0], tausSelectionName, asList=True)
                        + [outputLabel + '_%SYS%,as_char'])
                alg.selectionDecoration = preselectLabel
                config.addSelection (self.taus.split('.')[0], tausSelectionName, alg.selectionDecoration+',as_char', bits=1, preselection=True, comesFrom='or', writeToOutput=False)
            if jets :
                alg = config.createAlgorithm( 'CP::AsgUnionPreselectionAlg','ORJetsPreselectionAlg')
                alg.particles = jets
                alg.preselection = '&&'.join (config.getPreselection (self.jets.split('.')[0], jetsSelectionName, asList=True)
                        + [outputLabel + '_%SYS%,as_char'])
                alg.selectionDecoration = preselectLabel
                config.addSelection (self.jets.split('.')[0], jetsSelectionName, alg.selectionDecoration+',as_char', bits=1, preselection=True, comesFrom='or', writeToOutput=False)
            if photons :
                alg = config.createAlgorithm( 'CP::AsgUnionPreselectionAlg','ORPhotonsPreselectionAlg')
                alg.particles = photons
                alg.preselection = '&&'.join (config.getPreselection (self.photons.split('.')[0], photonsSelectionName, asList=True)
                        + [outputLabel + '_%SYS%,as_char'])
                alg.selectionDecoration = preselectLabel
                config.addSelection (self.photons.split('.')[0], photonsSelectionName, alg.selectionDecoration+',as_char', bits=1, preselection=True, comesFrom='or', writeToOutput=False)
            if fatJets :
                alg = config.createAlgorithm( 'CP::AsgUnionPreselectionAlg','ORFatJetsPreselectionAlg')
                alg.particles = fatJets
                alg.preselection = '&&'.join (config.getPreselection (self.fatJets.split('.')[0], fatJetsSelectionName, asList=True)
                        + [outputLabel + '_%SYS%,as_char'])
                alg.selectionDecoration = preselectLabel
                config.addSelection (self.fatJets.split('.')[0], fatJetsSelectionName, alg.selectionDecoration+',as_char', bits=1, preselection=True, comesFrom='or', writeToOutput=False)


