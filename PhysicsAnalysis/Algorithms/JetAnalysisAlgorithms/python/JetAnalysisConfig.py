# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration



# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigSequence import groupBlocks
from AnalysisAlgorithmsConfig.ConfigAccumulator import (
    DataType, ConfigDeprecationWarning, JetUncertaintyWarning)
from AthenaCommon.SystemOfUnits	import GeV
from AthenaConfiguration.Enums import LHCPeriod
import re
import warnings


def _resolveJetCollectionName(jetCollection):
    """Map the PHYSLITE jet collection aliases onto the actual collection names."""
    if jetCollection == "AnalysisJets":
        return "AntiKt4EMPFlowJets"
    elif jetCollection == "AnalysisLargeRJets":
        return "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets"
    return jetCollection


def _addJetUncertaintiesTool(config, alg, toolName, jetCollectionName, configFile,
                             calibArea, mcType, isData, pseudoData):
    """Add a private JetUncertaintiesTool to `alg` under property `toolName`
    and configure it."""
    config.addPrivateTool( toolName, 'JetUncertaintiesTool' )
    tool = getattr(alg, toolName)
    tool.JetDefinition = jetCollectionName[:-4]
    tool.ConfigFile = configFile
    if calibArea is not None:
        tool.CalibArea = calibArea
    tool.MCType = mcType
    tool.IsData = isData
    tool.PseudoDataJERsmearingMode = pseudoData
    return tool


def _parseJetCollection(jetCollection):
    """Parse a jet collection name into its components.

    Returns (radius, jetInput, trim, hasBTag) where:
      radius   -- the jet radius parameter (2, 4, 6, or 10)
      jetInput -- the input type string (e.g. 'EMPFlow', 'UFO', ...)
      trim     -- the trimming suffix string, or None
      hasBTag  -- whether the original collection name had a '_BTagging' suffix
    """
    hasBTag = False
    btIndex = jetCollection.find('_BTagging')
    if btIndex != -1:
        jetCollection = jetCollection[:btIndex]
        hasBTag = True

    jetCollectionName = _resolveJetCollectionName(jetCollection)

    collection_pattern = re.compile(
        r"AntiKt(\d+)(EMTopo|EMPFlow|LCTopo|TrackCaloCluster|UFO|Track|HI)"
        r"(TrimmedPtFrac5SmallR20|CSSKSoftDropBeta100Zcut10)?Jets")
    match = collection_pattern.match(jetCollectionName)
    if not match:
        raise ValueError(
            f"Jet collection {jetCollectionName} does not match expected pattern!")
    radius = int(match.group(1))
    if radius not in [2, 4, 6, 10]:
        raise ValueError(
            f"Jet collection has unsupported radius '{radius}'!")
    jetInput = match.group(2)
    trim = match.group(3)
    return radius, jetInput, trim, hasBTag


class PreJetAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the common preprocessing of jet sequences"""

    def __init__ (self) :
        super().__init__()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the output container after calibration.",
            meta={'role':'container'})
        self.addOption ('jetCollection', '', type=str,
            noneAction='error',
            info="the jet container to run on. It is interpreted to determine "
            "the correct config blocks to call for small- or large-R jets.")
        self.addOption('outputTruthLabelIDs', False, type=bool,
            info='enable or disable `HadronConeExclTruthLabelID` and `PartonTruthLabelID` decorations.')
        self.addOption ('runOriginalObjectLink', False, type=bool,
                info='sets up an instance of `CP::AsgOriginalObjectLinkAlg` to link systematically-varied containers to the base one.')
        self.addOption ('runGhostMuonAssociation', None, type=bool,
            info="whether to set up the jet-ghost-muon association algorithm "
            "`CP::JetGhostMuonAssociationAlg`. If left empty, automatically defaults to `False` for PHYSLITE and `True` otherwise.")
        self.addOption ('runTruthJetTagging', True, type=bool,
            info="whether to set up the jet truth tagging algorithm "
            "`CP::JetTruthTagAlg`.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs (self, config) :


        if config.isPhyslite() and self.jetCollection == 'AntiKt4EMPFlowJets' :
            config.setSourceName (self.containerName, "AnalysisJets", originalName = self.jetCollection)
        elif config.isPhyslite() and self.jetCollection == 'AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets' :
            config.setSourceName (self.containerName, "AnalysisLargeRJets", originalName = self.jetCollection)
        else :
            config.setSourceName (self.containerName, self.jetCollection, originalName = self.jetCollection)

        # Parse and store jet collection metadata for downstream blocks
        radius, jetInput, trim, hasBTag = _parseJetCollection(self.jetCollection)
        config.setContainerMeta(self.containerName, 'jetRadius', radius)
        config.setContainerMeta(self.containerName, 'jetInput', jetInput)
        config.setContainerMeta(self.containerName, 'jetTrim', trim)
        config.setContainerMeta(self.containerName, 'hasBTag', hasBTag)
        config.setContainerMeta(self.containerName, 'jetCollectionName',
                                _resolveJetCollectionName(self.jetCollection))

        # Relink original jets in case of b-tagging calibration
        if self.runOriginalObjectLink or hasBTag :
            alg = config.createAlgorithm( 'CP::AsgOriginalObjectLinkAlg',
                                          'JetOriginalObjectLinkAlg',
                                           reentrant=True )
            alg.baseContainerName = self.jetCollection
            alg.particles = config.readName (self.containerName)
            if config.wantCopy (self.containerName) :
                alg.particlesOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        # Set up the jet ghost muon association algorithm:
        if (self.runGhostMuonAssociation is None and not config.isPhyslite()) or \
           (self.runGhostMuonAssociation is True):
            alg = config.createAlgorithm( 'CP::JetGhostMuonAssociationAlg',
                                          'JetGhostMuonAssociationAlg' )
            alg.jets = config.readName (self.containerName)
            if config.isPhyslite():
                alg.muons = "AnalysisMuons"
            if config.wantCopy (self.containerName) :
                alg.jetsOut = config.copyName (self.containerName)
                taggerName = "GN2HLv01" if config.geometry() >= LHCPeriod.Run4 else "GN2v01"
                extraInputs = [
                    ( 'xAOD::JetContainer' , f"StoreGateSvc+{self.jetCollection}.DFCommonJets_jetClean_LooseBad"),
                    ( 'xAOD::JetContainer' , f"StoreGateSvc+{self.jetCollection}.DFCommonJets_jetClean_TightBad"),
                    ( 'xAOD::JetContainer' , f"StoreGateSvc+{self.jetCollection}.{taggerName}_pb"),
                    ( 'xAOD::JetContainer' , f"StoreGateSvc+{self.jetCollection}.{taggerName}_pc"),
                    ( 'xAOD::JetContainer' , f"StoreGateSvc+{self.jetCollection}.{taggerName}_ptau"),
                    ( 'xAOD::JetContainer' , f"StoreGateSvc+{self.jetCollection}.{taggerName}_pu")]
                config.setExtraInputs (extraInputs)

        if self.runTruthJetTagging and config.dataType() is not DataType.Data:
            # Decorate jets with isHS labels (required to retrieve Jvt SFs)
            alg = config.createAlgorithm( 'CP::JetDecoratorAlg', 'JetPileupLabelAlg' )
            config.addPrivateTool( 'decorator', 'JetPileupLabelingTool' )
            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)
            alg.decorator.RecoJetContainer = alg.jetsOut.replace ('%SYS%', 'NOSYS')
            alg.decorator.SuppressOutputDependence=True
            if self.jetCollection == 'AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets' :
                extraInputs = [
                    ( 'xAOD::JetContainer' , f"StoreGateSvc+{self.jetCollection}.GN2Xv01_phbb"),
                    ( 'xAOD::JetContainer' , f"StoreGateSvc+{self.jetCollection}.GN2Xv01_phcc"),
                    ( 'xAOD::JetContainer' , f"StoreGateSvc+{self.jetCollection}.GN2Xv01_pqcd"),
                    ( 'xAOD::JetContainer' , f"StoreGateSvc+{self.jetCollection}.GN2Xv01_ptop")]
                config.setExtraInputs (extraInputs)

        # Set up shallow copy if needed and not yet done
        if config.wantCopy (self.containerName) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'JetShallowCopyAlg' )
            alg.input = config.readName (self.containerName)
            alg.output = config.copyName (self.containerName)
            alg.outputType = 'xAOD::JetContainer'
            if self.jetCollection == 'AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets' :
                alg.declareDecorations = ['GN2Xv01_phbb', 'GN2Xv01_phcc',
                                          'GN2Xv01_pqcd', 'GN2Xv01_ptop']

        config.addOutputVar (self.containerName, 'pt', 'pt')
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)
        config.addOutputVar (self.containerName, 'charge', 'charge', noSys=True, enabled=False)


        if self.outputTruthLabelIDs and config.dataType() is not DataType.Data:
            config.addOutputVar (self.containerName, 'HadronConeExclTruthLabelID', 'HadronConeExclTruthLabelID', noSys=True, auxType="int")
            config.addOutputVar (self.containerName, 'PartonTruthLabelID', 'PartonTruthLabelID', noSys=True, auxType="int")



class SmallRJetAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the small-r jet sequence"""

    def __init__ (self) :
        super().__init__()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the output container after calibration.",
            meta={'role':'container'})
        self.addOption ('jetCollection', '', type=str,
            noneAction='error',
            info="the jet container to run on. It is interpreted to determine "
            "the correct config blocks to call for small- or large-R jets.")
        self.addOption ('runJvtUpdate', False, type=bool,
            info="whether to update the JVT.")
        self.addOption ('runNNJvtUpdate', False, type=bool,
            info="whether to update the NN-JVT.")
        self.addOption ('runJvtSelection', True, type=bool,
            info="whether to run JVT selection.")
        self.addOption ('runFJvtSelection', False, type=bool,
            info="whether to run forward JVT selection.")
        self.addOption ('jvtWP', "FixedEffPt", type=str,
            info="which Jvt WP to apply.",
            meta={'choices':(['FixedEffPt'],1)})
        self.addOption ('fJvtWP', "Loose", type=str,
            info="which fJvt WP to apply.",
            meta={'choices':(['Loose','Tight','Tighter'],1)})
        self.addOption ('runJvtEfficiency', True, type=bool,
            info="whether to calculate the JVT efficiency.")
        self.addOption ('runFJvtEfficiency', False, type=bool,
            info="whether to calculate the forward JVT efficiency.")
        self.addOption ('runUncertainties', True, type=bool,
            info="whether to configure JetUncertaintiesTool.", expertMode=True)
        self.addOption('uncertaintyAnalysisFiles', None, type=dict,
            info="dictionary of dsids with entry corresponding to file containing gluon fraction histograms needed to calculate an analysis specific "
            "jet flavor composition uncertainty.")
        self.addOption ('systematicsModelJES', "Category", type=str,
            info="the NP reduction scheme to use for JES: All, Global, Category, "
            "Scenario. The default is Category.")
        self.addOption ('systematicsModelJER', "Full", type=str,
            info="the NP reduction scheme to use for JER: All, Full, Simple. The "
            "default is Full.",
            meta={'choices':(['All','Full','Simple'],1)})
        self.addOption ('runJERsystematicsOnData', False, type=bool,
            info="whether to run the All/Full JER model variations also on data samples. Expert option!",
            expertMode=True)
        self.addOption ('recalibratePhyslite', True, type=bool,
            info="whether to run the `CP::JetCalibrationAlg` on PHYSLITE derivations.")
        # Calibration tool options
        self.addOption ('calibToolConfigFile', None, type=str,
            info="the name of the config file to use for the jet calibration "
            "tool. Expert option to override JetETmiss recommendations.",
            expertMode=True)
        self.addOption ('calibToolCalibArea', None, type=str,
            info="name of the CVMFS area to use for the jet calibration "
            "tool. Expert option to override JetETmiss recommendations.",
            expertMode=True)
        self.addOption ('calibToolCalibSeq', None, type=str,
            info="name of the sequence to use for the jet calibration "
            "tool (e.g. `JetArea_Residual_EtaJES_GSC`). Expert option to override "
            "JetETmiss recommendations.",
            expertMode=True)
        # Uncertainties tool options
        self.addOption ('uncertToolConfigPath', None, type=str,
            info="name (str) of the config file to use for the jet uncertainty "
            "tool. Expert option to override JetETmiss recommendations. The "
            "default is None.",
            expertMode=True)
        self.addOption ('uncertToolCalibArea', None, type=str,
            info="name (str) of the CVMFS area to use for the jet uncertainty "
            "tool. Expert option to override JetETmiss recommendations. The "
            "default is None.",
            expertMode=True)
        self.addOption ('uncertToolMCType', None, type=str,
            info="data type (str) to use for the jet uncertainty tool (e.g. "
            "'AF3' or 'MC16'). Expert option to override JetETmiss "
            "recommendations. The default is None.",
            expertMode=True)

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def getUncertaintyToolSettings(self, config):

        # Retrieve appropriate JES/JER recommendations for the JetUncertaintiesTool.
        # We do this separately from the tool declaration, as we may need to set uo
        # two such tools, but they have to be private.

        jetInput = config.getContainerMeta(self.containerName, 'jetInput', failOnMiss=True)
        # Config file:
        config_file = None
        if self.systematicsModelJES == "All" and self.systematicsModelJER == "All":
            config_file = "R4_AllNuisanceParameters_AllJERNP.config"
        elif "Scenario" in self.systematicsModelJES:
            if self.systematicsModelJER != "Simple":
                raise ValueError(
                    "Invalid uncertainty configuration - Scenario* systematicsModelJESs can "
                    "only be used together with the Simple systematicsModelJER")
            config_file = f"R4_{self.systematicsModelJES}_SimpleJER.config"
        elif self.systematicsModelJES in ["Global", "Category"] and self.systematicsModelJER in ["Simple", "Full"]:
            config_file = f"R4_{self.systematicsModelJES}Reduction_{self.systematicsModelJER}JER.config"
        else:
            raise ValueError(
                "Invalid combination of systematicsModelJES and systematicsModelJER settings: "
                f"systematicsModelJES: {self.systematicsModelJES}, systematicsModelJER: {self.systematicsModelJER}" )

        # Calibration area:
        calib_area = None
        if self.uncertToolCalibArea is not None:
            calib_area = self.uncertToolCalibArea

        # Expert override for config path:
        if self.uncertToolConfigPath is not None:
            config_file = self.uncertToolConfigPath
        else:
            if config.geometry() is LHCPeriod.Run2:
                if config.dataType() is DataType.FastSim:
                    config_file = "rel22/Fall2024_PreRec/" + config_file
                else:
                    if jetInput == "HI":
                        config_file = "HIJetUncertainties/Spring2023/HI" + config_file
                    else:
                        config_file = "rel22/Summer2023_PreRec/" + config_file
            else:
                if config.dataType() is DataType.FastSim:
                    config_file = "rel22/Winter2025_AF3_PreRec/" + config_file
                else:
                    if jetInput == "HI":
                        config_file = "HIJetUncertainties/Spring2023/HI" + config_file
                    else:
                        config_file = "rel22/Winter2025_PreRec/" + config_file

        # MC type:
        mc_type = None
        if self.uncertToolMCType is not None:
            mc_type = self.uncertToolMCType
        else:
            if config.geometry() is LHCPeriod.Run2:
                if config.dataType() is DataType.FastSim:
                    mc_type = "AF3"
                else:
                    mc_type = "MC20"
            else:
                if config.dataType() is DataType.FastSim:
                    mc_type = "MC23AF3"
                else:
                    if jetInput == "HI":
                        mc_type = "MC16"
                    else:
                        mc_type = "MC23"

        return config_file, calib_area, mc_type


    def createUncertaintyTool(self, jetUncertaintiesAlg, config, jetCollectionName, doPseudoData=False):

        # Create an instance of JetUncertaintiesTool, following JetETmiss recommendations.
        # To run Jet Energy Resolution (JER) uncertainties in the "Full" or "All" schemes,
        # we need two sets of tools: one configured as normal (MC), the other with the
        # exact same settings but pretending to run on data (pseudo-data).
        # This is achieved by passing "isPseudoData=True" to the arguments.

        # Retrieve the common configuration settings
        configFile, calibArea, mcType = self.getUncertaintyToolSettings(config)

        # The main tool for all JES+JER combinations
        tool = _addJetUncertaintiesTool(config, jetUncertaintiesAlg, 'uncertaintiesTool',
                                        jetCollectionName, configFile, calibArea, mcType,
                                        isData=(config.dataType() is DataType.Data),
                                        pseudoData=False)
        from PathResolver import PathResolver
        if self.uncertaintyAnalysisFiles is not None:
          if config.dsid() in self.uncertaintyAnalysisFiles:
            tool.AnalysisFile = PathResolver.FindCalibFile(self.uncertaintyAnalysisFiles[config.dsid()])

        if config.dataType() is DataType.Data and not (doPseudoData and self.runJERsystematicsOnData):
            # we don't want any systematics on data if we're not using the right JER model!
            jetUncertaintiesAlg.affectingSystematicsFilter = '.*'
        if config.dataType() is not DataType.Data and doPseudoData and not self.runJERsystematicsOnData:
            # The secondary tool for pseudo-data JER smearing
            _addJetUncertaintiesTool(config, jetUncertaintiesAlg, 'uncertaintiesToolPD',
                                     jetCollectionName, configFile, calibArea, mcType,
                                     isData=True, pseudoData=True)


    def makeAlgs (self, config) :

        # Self-select: only run for radius-4 jets
        if config.getContainerMeta(self.containerName, 'jetRadius', failOnMiss=True) != 4:
            return


        jetCollectionName = config.getContainerMeta(self.containerName, 'jetCollectionName', failOnMiss=True)

        jetInput = config.getContainerMeta(self.containerName, 'jetInput', failOnMiss=True)

        if jetInput not in ["EMTopo", "EMPFlow", "HI"]:
            raise ValueError(
                f"Unsupported input type '{jetInput}' for R=0.4 jets!" )

        if self.jvtWP not in ["FixedEffPt"]:
            raise ValueError(
                f"Unsupported NNJvt WP '{self.jvtWP}'" )

        if self.fJvtWP not in ["Loose", "Tight", "Tighter"]:
            raise ValueError(
                f"Unsupported fJvt WP '{self.fJvtWP}'" )

        if not config.isPhyslite() or self.recalibratePhyslite:
            # Create calibration tool as public so it can be shared
            # (e.g. with FTAG1LITE's JetCalibratedPtDecoratorAlg).
            # Must be created before the algorithm so EventLoop
            # initialises the tool first.
            calibToolName = 'JetCalibTool_' + jetCollectionName[:-4]
            calibTool = config.createPublicTool( 'JetCalibrationTool', calibToolName )
            calibTool.JetCollection = jetCollectionName[:-4]
            # Get the correct string to use in the config file name
            if jetInput == "EMPFlow":
                if config.geometry() is LHCPeriod.Run2:
                    configFile = "PreRec_R22_PFlow_ResPU_EtaJES_GSC_February23_230215.config"
                    calibTool.CalibArea = "00-04-82"
                elif config.geometry() >= LHCPeriod.Run3:
                    configFile = "AntiKt4EMPFlow_MC23a_PreRecR22_Phase2_CalibConfig_ResPU_EtaJES_GSC_241208_InSitu.config"
                    calibTool.CalibArea = "00-04-83"
            elif jetInput == "HI":
                if config.geometry() is LHCPeriod.Run2:
                    configFile = "JES_MC16_HI_Jan2021_5TeV.config"
                elif config.geometry() is LHCPeriod.Run3:
                    configFile = "AntiKt4HI_MC23_EtaJES_Run3PreRec_Run2VJet_Run3EtaInt_5p36TeV.config"
                    calibTool.CalibArea = "00-04-83"
                else:
                    raise ValueError(
                        f"Unsupported geometry '{config.geometry()}' for HI jet calibration!")
            else:
                if config.dataType() is DataType.FastSim:
                    configFile = f"JES_MC16Recommendation_AFII_{jetInput}_Apr2019_Rel21.config"
                else:
                    configFile = f"JES_MC16Recommendation_Consolidated_{jetInput}_Apr2019_Rel21.config"
            if self.calibToolCalibArea is not None:
                calibTool.CalibArea = self.calibToolCalibArea
            if self.calibToolConfigFile is not None:
                configFile = self.calibToolConfigFile
            calibTool.ConfigFile = configFile
            if config.dataType() is DataType.Data:
                if jetInput == "HI":
                    calibTool.CalibSequence = 'EtaJES_Insitu'
                else:
                    calibTool.CalibSequence = 'JetArea_Residual_EtaJES_GSC_Insitu'
            else:
                if jetInput == "EMPFlow":
                    calibTool.CalibSequence = 'JetArea_Residual_EtaJES_GSC'
                elif jetInput == "HI":
                    calibTool.CalibSequence = 'EtaJES'
                else:
                    calibTool.CalibSequence = 'JetArea_Residual_EtaJES_GSC_Smear'
            if self.calibToolCalibSeq is not None:
                calibTool.CalibSequence = self.calibToolCalibSeq
            calibTool.IsData = (config.dataType() is DataType.Data)
            # Prepare the jet calibration algorithm
            alg = config.createAlgorithm( 'CP::JetCalibrationAlg', 'JetCalibrationAlg' )
            alg.HIsetup = jetInput == "HI"
            alg.calibrationTool = f'{calibTool.getType()}/{calibTool.getName()}'
            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)

        # Jet uncertainties
        if self.runUncertainties:
            alg = config.createAlgorithm( 'CP::JetUncertaintiesAlg', 'JetUncertaintiesAlg' )
            self.createUncertaintyTool(alg, config, jetCollectionName, doPseudoData=( self.systematicsModelJER in ["Full","All"] ))
            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        # Set up the JVT update algorithm:
        if self.runJvtUpdate :
            alg = config.createAlgorithm( 'CP::JvtUpdateAlg', 'JvtUpdateAlg' )
            config.addPrivateTool( 'jvtTool', 'JetVertexTaggerTool' )
            alg.jvtTool.JetContainer = self.jetCollection
            alg.jvtTool.SuppressInputDependence=True
            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        if self.runNNJvtUpdate:
            assert jetInput=="EMPFlow", "NN JVT only defined for PFlow jets"
            alg = config.createAlgorithm( 'CP::JetDecoratorAlg', 'NNJvtUpdateAlg' )
            config.addPrivateTool( 'decorator', 'JetPileupTag::JetVertexNNTagger' )
            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)
            # Set this actually to the *output* collection
            alg.decorator.JetContainer = alg.jetsOut.replace ('%SYS%', 'NOSYS')
            alg.decorator.SuppressInputDependence=True
            alg.decorator.SuppressOutputDependence=True

        # Set up the jet efficiency scale factor calculation algorithm
        # Change the truthJetCollection property to AntiKt4TruthWZJets if preferred
        if self.runJvtSelection :
            assert jetInput=="EMPFlow", "NNJvt WPs and SFs only valid for PFlow jets"
            warnings.warn_explicit(
                "jvtWP, runJvtSelection and runJvtEfficiency"
                " are deprecated - please use a"
                " JVTWorkingPoint block instead.",
                ConfigDeprecationWarning, filename='', lineno=0)

            alg = config.createAlgorithm('CP::AsgSelectionAlg', 'JvtSelectionAlg')
            config.addPrivateTool('selectionTool', 'CP::NNJvtSelectionTool')
            alg.selectionTool.JetContainer = config.readName(self.containerName)
            alg.selectionTool.JvtMomentName = "NNJvt"
            alg.selectionTool.WorkingPoint = self.jvtWP
            alg.selectionTool.MaxPtForJvt = 60*GeV
            alg.selectionDecoration = "jvt_selection,as_char"
            alg.particles = config.readName(self.containerName)

            if self.runJvtEfficiency and config.dataType() is not DataType.Data:
                alg = config.createAlgorithm( 'CP::JvtEfficiencyAlg', 'JvtEfficiencyAlg' )
                config.addPrivateTool( 'efficiencyTool', 'CP::NNJvtEfficiencyTool' )
                alg.efficiencyTool.JetContainer = config.readName(self.containerName)
                alg.efficiencyTool.MaxPtForJvt = 60*GeV
                alg.efficiencyTool.WorkingPoint = self.jvtWP
                if config.geometry() is LHCPeriod.Run2:
                    alg.efficiencyTool.SFFile = "JetJvtEfficiency/May2024/NNJvtSFFile_Run2_EMPFlow.root"
                else:
                    alg.efficiencyTool.SFFile = "JetJvtEfficiency/May2024/NNJvtSFFile_Run3_EMPFlow.root"
                alg.selection = 'jvt_selection,as_char'
                alg.scaleFactorDecoration = 'jvt_effSF_%SYS%'
                alg.outOfValidity = 2
                alg.outOfValidityDeco = 'no_jvt'
                alg.skipBadEfficiency = False
                alg.jets = config.readName (self.containerName)
                alg.preselection = config.getPreselection (self.containerName, '')
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'jvtEfficiency')
            config.addSelection (self.containerName, 'baselineJvt', 'jvt_selection,as_char', preselection=False)

        if self.runFJvtSelection :
            assert jetInput=="EMPFlow", "fJvt WPs and SFs only valid for PFlow jets"
            warnings.warn_explicit(
                "fJvtWP, runFJvtSelection and runFJvtEfficiency"
                " are deprecated - please use a"
                " FJVTWorkingPoint block instead.",
                ConfigDeprecationWarning, filename='', lineno=0)

            alg = config.createAlgorithm('CP::AsgSelectionAlg', 'FJvtSelectionAlg')
            config.addPrivateTool('selectionTool', 'CP::FJvtSelectionTool')
            alg.selectionTool.JetContainer = config.readName(self.containerName)
            alg.selectionTool.JvtMomentName = "DFCommonJets_fJvt"
            alg.selectionTool.WorkingPoint = self.fJvtWP
            alg.selectionDecoration = "fjvt_selection,as_char"
            alg.particles = config.readName(self.containerName)

            if self.runFJvtEfficiency and config.dataType() is not DataType.Data:
                alg = config.createAlgorithm( 'CP::JvtEfficiencyAlg', 'FJvtEfficiencyAlg' )
                config.addPrivateTool( 'efficiencyTool', 'CP::FJvtEfficiencyTool' )
                alg.efficiencyTool.JetContainer = config.readName(self.containerName)
                alg.efficiencyTool.WorkingPoint = self.fJvtWP
                if config.geometry() is LHCPeriod.Run2:
                    alg.efficiencyTool.SFFile = "JetJvtEfficiency/May2024/fJvtSFFile_Run2_EMPFlow.root"
                else:
                    alg.efficiencyTool.SFFile = "JetJvtEfficiency/May2024/fJvtSFFile_Run3_EMPFlow.root"
                alg.selection = 'fjvt_selection,as_char'
                alg.scaleFactorDecoration = 'fjvt_effSF_%SYS%'
                alg.outOfValidity = 2
                alg.outOfValidityDeco = 'no_fjvt'
                alg.skipBadEfficiency = False
                alg.jets = config.readName (self.containerName)
                alg.preselection = config.getPreselection (self.containerName, '')
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'fjvtEfficiency')
            config.addSelection (self.containerName, 'baselineFJvt', 'fjvt_selection,as_char', preselection=False)

        # Additional decorations
        alg = config.createAlgorithm( 'CP::AsgEnergyDecoratorAlg', 'AsgEnergyDecoratorAlg' )
        alg.particles = config.readName (self.containerName)

        config.addOutputVar (self.containerName, 'e_%SYS%', 'e')


class RScanJetAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the r-scan jet sequence"""

    def __init__ (self) :
        super().__init__()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the output container after calibration.",
            meta={'role':'container'})
        self.addOption ('jetCollection', '', type=str,
            noneAction='error',
            info="the jet container to run on. It is interpreted to determine "
            "the correct config blocks to call for small- or large-R jets.")
        self.addOption ('recalibratePhyslite', True, type=bool,
            info="whether to run the CP::JetCalibrationAlg on PHYSLITE "
            "derivations. The default is True.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs (self, config) :

        # Self-select: only run for r-scan jets (radius 2 or 6)
        radius = config.getContainerMeta(self.containerName, 'jetRadius', failOnMiss=True)
        if radius not in [2, 6]:
            return

        jetCollectionName = config.getContainerMeta(self.containerName, 'jetCollectionName', failOnMiss=True)

        jetInput = config.getContainerMeta(self.containerName, 'jetInput', failOnMiss=True)

        if not config.isPhyslite() or self.recalibratePhyslite:
            if jetInput not in ["LCTopo", "HI"]:
                raise ValueError(
                    f"Unsupported input type '{jetInput}' for R-scan jets!" )
            # Create calibration tool before algorithm (EventLoop ordering)
            calibToolName = 'JetCalibTool_' + jetCollectionName[:-4]
            calibTool = config.createPublicTool( 'JetCalibrationTool', calibToolName )
            calibTool.JetCollection = jetCollectionName[:-4]
            if jetInput=="LCTopo":
                calibTool.ConfigFile = \
                    f"JES_MC16Recommendation_Rscan{radius}LC_Feb2022_R21.config"
                if config.dataType() is DataType.Data:
                    calibTool.CalibSequence = "JetArea_Residual_EtaJES_GSC_Insitu"
                else:
                    calibTool.CalibSequence = "JetArea_Residual_EtaJES_GSC_Smear"
            elif jetInput=="HI":
                calibTool.ConfigFile = \
                    "JES_MC16_HI_Jan2021_5TeV.config"
                if config.dataType() is DataType.Data:
                    calibTool.CalibSequence = "EtaJES_Insitu"
                else:
                    calibTool.CalibSequence = "EtaJES"
            calibTool.IsData = (config.dataType() is DataType.Data)
            # Prepare the jet calibration algorithm
            alg = config.createAlgorithm( 'CP::JetCalibrationAlg', 'JetCalibrationAlg' )
            alg.HIsetup = jetInput == "HI"
            alg.calibrationTool = f'{calibTool.getType()}/{calibTool.getName()}'
            alg.jets = config.readName (self.containerName)
            warnings.warn_explicit(
                "Uncertainties for R-Scan jets are not yet released!",
                JetUncertaintyWarning, filename='', lineno=0)


class LargeRJetAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the large-r jet sequence"""

    def __init__ (self) :
        super().__init__()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the output container after calibration.",
            meta={'role':'container'})
        self.addOption ('jetCollection', '', type=str,
            noneAction='error',
            info="the jet container to run on. It is interpreted to determine "
            "the correct config blocks to call for small- or large-R jets.")
        self.addOption ('recalibratePhyslite', True, type=bool,
            info="whether to run the CP::JetCalibrationAlg on PHYSLITE "
            "derivations. The default is True.")
        self.addOption ('runUncertainties', True, type=bool,
            info="whether to configure JetUncertaintiesTool.", expertMode=True )
        self.addOption ('systematicsModelJER', "Full", type=str,
            info="the NP reduction scheme to use for JER: Full, Simple. The default is Full.",
            meta={'choices':(['Simple','Full'],1)})
        self.addOption ('systematicsModelJMS', "Full", type=str,
            info="the NP reduction scheme to use for JMS: Full, Simple. The default is Full.",
            meta={'choices':(['Simple','Full'],1)})
        self.addOption ('systematicsModelJMR', "Full", type=str,
            info="the NP reduction scheme to use for JMR: Full, Simple. The default is Full.",
            meta={'choices':(['Full','Simple'],1)})
        self.addOption ('runJERsystematicsOnData', False, type=bool,
            info="whether to run the All/Full JER model variations also on data samples. Expert option!",
            expertMode=True)
        # Adding these options to override the jet uncertainty config file when we have new recommendations
        # Calibration tool options
        self.addOption ('calibToolConfigFile', None, type=str,
            info="name of the config file to use for the jet calibration "
            "tool. Expert option to override JetETmiss recommendations.",
            expertMode=True)
        self.addOption ('calibToolCalibArea', None, type=str,
            info="name of the CVMFS area to use for the jet calibration "
            "tool. Expert option to override JetETmiss recommendations.",
            expertMode=True)
        self.addOption ('calibToolCalibSeq', None, type=str,
            info="name of the sequence to use for the jet calibration "
            "tool (e.g. `JetArea_Residual_EtaJES_GSC`). Expert option to override "
            "JetETmiss recommendations.",
            expertMode=True)
        # Uncertainties tool options
        self.addOption ('uncertToolConfigPath', None, type=str,
            info="name of the config file to use for the JES, JER, and JMS uncertainty "
            "tool. Expert option to override JetETmiss recommendations.",
            expertMode=True)
        self.addOption ('uncertToolConfigPathJMR', None, type=str,
            info="name of the config file to use for the JMR uncertainty "
            "tool. Expert option to override JetETmiss recommendations.",
            expertMode=True)
        self.addOption ('uncertToolCalibArea', None, type=str,
            info="name (str) of the CVMFS area to use for the jet uncertainty "
            "tool. Expert option to override JetETmiss recommendations. The "
            "default is None.",
            expertMode=True)
        self.addOption ('uncertToolMCType', None, type=str,
            info="data type (str) to use for the jet uncertainty tool (e.g. "
            "'AF3' or 'MC16'). Expert option to override JetETmiss "
            "recommendations. The default is None.",
            expertMode=True)
        self.addOption ('minPt', 200.*GeV, type=float,
            info=r"the minimum $p_\mathrm{T}$ cut (in MeV) to apply to calibrated large-R jets.")
        self.addOption ('maxPt', 3000.*GeV, type=float,
            info=r"the maximum $p_\mathrm{T}$ cut (in MeV) to apply to calibrated large-R jets.")
        self.addOption ('maxEta', 0., type=float,
            info=r"the maximum $\vert\eta\vert$ cut to apply to calibrated large-R jets.")
        self.addOption ('maxRapidity', 2., type=float,
            info="the maximum rapidity cut to apply to calibrated large-R jets.")
        self.addOption ('minMass', 40.*GeV, type=float,
            info="the minimum mass cut (in MeV) to apply to calibrated large-R jets.")
        self.addOption ('maxMass', 600.*GeV, type=float,
            info="the maximum mass cut (in MeV) to apply to calibrated large-R jets.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def getUncertaintyToolSettings(self, config):
        # Retrieve appropriate JES/JER recommendations for the JetUncertaintiesTool.
        # We do this separately from the tool declaration, as we may need to set uo
        # two such tools, but they have to be private.


        # Config file:
        config_file = None
        if self.systematicsModelJER in ["Simple", "Full"] and self.systematicsModelJMS in ["Simple", "Full"]:
            config_file = f"R10_CategoryJES_{self.systematicsModelJER}JER_{self.systematicsModelJMS}JMS.config"
        else:
            raise ValueError(
                "Invalid request for systematicsModelJER/JMS settings: "
                f"systematicsModelJER = '{self.systematicsModelJER}', "
                f"systematicsModelJMS = '{self.systematicsModelJMS}'" )
        if self.uncertToolConfigPath is not None:
            # Expert override
            config_file = self.uncertToolConfigPath
        else:
            if config.geometry() in [LHCPeriod.Run2, LHCPeriod.Run3]:
                config_file = "rel22/Summer2025_PreRec/" + config_file
            else:
                warnings.warn_explicit(
                    "Uncertainties for UFO jets are not available for Run 4!",
                    JetUncertaintyWarning, filename='', lineno=0)


        # Calibration area:
        calib_area = None
        if self.uncertToolCalibArea is not None:
            calib_area = self.uncertToolCalibArea
                
        # MC type:
        if self.uncertToolMCType is not None:
            mc_type = self.uncertToolMCType
        else:
            if config.dataType() is DataType.FastSim:
                if config.geometry() is LHCPeriod.Run2:
                    mc_type = "MC20AF3"
                else:
                    mc_type = "MC23AF3"
            else:
                if config.geometry() is LHCPeriod.Run2:
                    mc_type = "MC20"
                else:
                    mc_type = "MC23"

        return config_file, calib_area, mc_type

    def createUncertaintyTool(self, jetUncertaintiesAlg, config, jetCollectionName, doPseudoData=False):
        '''
        Create instance(s) of JetUncertaintiesTool following JetETmiss recommendations.

        JER uncertainties under the "Full" scheme must be run on MC samples twice:
        1. Normal (MC) mode,
        2. Pseudodata (PD) mode, as if the events are Data.
        '''

        # Retrieve the common configuration settings
        configFile, calibArea, mcType = self.getUncertaintyToolSettings(config)

        # The main tool for all JER combinations
        _addJetUncertaintiesTool(config, jetUncertaintiesAlg, 'uncertaintiesTool',
                                 jetCollectionName, configFile, calibArea, mcType,
                                 isData=(config.dataType() is DataType.Data),
                                 pseudoData=False)

        # JER smearing on data 
        if config.dataType() is DataType.Data and not (doPseudoData and self.runJERsystematicsOnData):
            # we don't want any systematics on data if we're not using the right JER model!
            jetUncertaintiesAlg.affectingSystematicsFilter = '.*'

        if config.dataType() is not DataType.Data and doPseudoData and not self.runJERsystematicsOnData:
            # The secondary tool for pseudo-data JER smearing
            _addJetUncertaintiesTool(config, jetUncertaintiesAlg, 'uncertaintiesToolPD',
                                     jetCollectionName, configFile, calibArea, mcType,
                                     isData=True, pseudoData=True)

    def createFFSmearingTool(self, jetFFSmearingAlg, config):
        # Retrieve appropriate large-R jet mass resolution recommendations for the FFJetSmearingTool.

        # Config file:
        if self.systematicsModelJMR in ["Simple", "Full"]:
            config_file = f"R10_{self.systematicsModelJMR}JMR.config"
        else:
            raise ValueError(
                f"Invalid request for systematicsModelJMR settings: {self.systematicsModelJMR}"
            )

        # Expert override for config path:
        if self.uncertToolConfigPathJMR is not None:
            config_file = self.uncertToolConfigPathJMR
        else:
            config_file = "rel22/Summer2025_PreRec/" + config_file
            if config.geometry() is LHCPeriod.Run4:
                warnings.warn_explicit(
                    "Uncertainties for UFO jets are not for Run 4!",
                    JetUncertaintyWarning, filename='', lineno=0)

        # MC type:
        if config.geometry() is LHCPeriod.Run2:
            if config.dataType() is DataType.FastSim:
                mc_type = "MC20AF3"
            else:
                mc_type = "MC20"
        elif config.geometry() >= LHCPeriod.Run3:
            if config.dataType() is DataType.FastSim:
                mc_type = "MC23AF3"
            else:
                mc_type = "MC23"

        # Set up the FF smearing tool
        config.addPrivateTool( 'FFSmearingTool', 'CP::FFJetSmearingTool')
        jetFFSmearingAlg.FFSmearingTool.MassDef = "UFO"
        jetFFSmearingAlg.FFSmearingTool.MCType = mc_type
        jetFFSmearingAlg.FFSmearingTool.ConfigFile = config_file

    def makeAlgs (self, config) :

        # Self-select: only run for large-R (radius 10) jets
        if config.getContainerMeta(self.containerName, 'jetRadius', failOnMiss=True) != 10:
            return

        jetCollectionName = config.getContainerMeta(self.containerName, 'jetCollectionName', failOnMiss=True)

        jetInput = config.getContainerMeta(self.containerName, 'jetInput', failOnMiss=True)
        trim = config.getContainerMeta(self.containerName, 'jetTrim', failOnMiss=True)

        if jetInput not in ["UFO"]:
            raise ValueError(f"Invalid input type '{jetInput}' for large-R jets!" )

        if not trim:
            raise ValueError("Untrimmed large-R jets are not supported!")

        configFile = "JES_MC20PreRecommendation_R10_UFO_CSSK_SoftDrop_JMS_R21Insitu_26Nov2024.config"
        calibArea = "00-04-83"
        if self.calibToolConfigFile is not None:
            configFile = self.calibToolConfigFile

        if config.dataType() is not DataType.Data:
            calibSeq = "EtaJES_JMS"
        else:
            calibSeq = "EtaJES_JMS_Insitu"
        if self.calibToolCalibSeq is not None:
            calibSeq = self.calibToolCalibSeq

        if self.calibToolCalibArea is not None:
            calibArea = self.calibToolCalibArea

        if not config.isPhyslite() or self.recalibratePhyslite:
            # Create calibration tool before algorithm (EventLoop ordering)
            calibToolName = 'JetCalibTool_' + jetCollectionName[:-4]
            calibTool = config.createPublicTool( 'JetCalibrationTool', calibToolName )

            calibTool.JetCollection = jetCollectionName[:-4]

            calibTool.ConfigFile = configFile

            calibTool.CalibSequence = calibSeq

            if calibArea is not None:
                calibTool.CalibArea = calibArea

            calibTool.IsData = (config.dataType() is DataType.Data)
            # Prepare the jet calibration algorithm
            alg = config.createAlgorithm( 'CP::JetCalibrationAlg', 'JetCalibrationAlg' )
            alg.calibrationTool = f'{calibTool.getType()}/{calibTool.getName()}'
            alg.jets = config.readName(self.containerName)
            alg.jetsOut = config.copyName(self.containerName)

        # Jet uncertainties
        if config.dataType() in [DataType.FullSim, DataType.FastSim] and self.runUncertainties:
            alg = config.createAlgorithm( 'CP::JetUncertaintiesAlg', 'JetUncertaintiesAlg' )
            self.createUncertaintyTool(alg, config, jetCollectionName, doPseudoData=( self.systematicsModelJER in ["Full","All"] ))

            # R=1.0 jets have a validity range
            alg.outOfValidity = 2 # SILENT
            alg.outOfValidityDeco = 'outOfValidity'

            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        if config.dataType() is not DataType.Data:
            # set up the FF smearing algorithm
            alg = config.createAlgorithm( 'CP::JetFFSmearingAlg', 'JetFFSmearingAlg' )
            self.createFFSmearingTool(alg, config)
            alg.outOfValidity = 2 # SILENT
            alg.outOfValidityDeco = 'outOfValidityJMR'
            alg.jets = config.readName (self.containerName)
            alg.jetsOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        if self.minPt > 0 or self.maxPt > 0 or self.maxEta > 0 or self.maxRapidity > 0:
            # Set up the the pt-eta selection
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'JetPtEtaCutAlg' )
            alg.selectionDecoration = 'selectPtEta,as_bits'
            config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
            alg.selectionTool.minPt = self.minPt
            alg.selectionTool.maxPt = self.maxPt
            alg.selectionTool.maxEta = self.maxEta
            alg.selectionTool.maxRapidity = self.maxRapidity
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', alg.selectionDecoration,
                                 preselection=True)

        if self.minMass > 0 or self.maxMass > 0:
            # Set up the the mass selection
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'JetMassCutAlg' )
            alg.selectionDecoration = 'selectMass,as_bits'
            config.addPrivateTool( 'selectionTool', 'CP::AsgMassSelectionTool' )
            alg.selectionTool.minM = self.minMass
            alg.selectionTool.maxM = self.maxMass
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', alg.selectionDecoration,
                                 preselection=True)

        config.addOutputVar (self.containerName, 'm', 'm')

class _JvtWorkingPointBase (ConfigBlock) :
    """common base for the Jvt and fJvt working point config blocks

    The concrete blocks combine one of these bases with a parameter class
    (`_NNJvtParameters` or `_FJvtParameters`)."""

    def __init__ (self) :
        super().__init__()
        self.setBlockName(self.__class__.__name__)
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.",
            meta={'role':'containerRef'})
        self.addOption ('selectionName', '', type=str,
            noneAction='error',
            info="the name of the jet selection to define (e.g. `tight` or `loose`).",
            meta={'role':'selection'})
        self.addOption (self._wpOption, '', type=str,
            noneAction='error',
            info=self._wpInfo,
            meta={'choices':(self._wpChoices,1)})
        self.addOption ('useSuffix', True, type=bool,
            info="whether the working point name is to be used as suffix ."
            "Not to be disabled if multiple working points are scheduled.")

    def instanceName (self) :
        return self.containerName + '_' + self.selectionName

    def workingPoint (self) :
        return getattr(self, self._wpOption)

    def suffix (self) :
        return f"_{self.workingPoint()}" if self.useSuffix else ""


class _JvtWorkingPointSelectionBase (_JvtWorkingPointBase) :
    """common base for the Jvt and fJvt working point selection blocks"""

    def makeAlgs (self, config) :

        suffix = self.suffix()
        decorationName = f"{self._prefix}_selection{suffix},as_char"
        selectionName = self.selectionName

        alg = config.createAlgorithm('CP::AsgSelectionAlg', f'{self._algPrefix}SelectionAlg_{self.workingPoint()}')
        config.addPrivateTool('selectionTool', self._selectionTool)
        alg.selectionTool.JetContainer = config.readName(self.containerName)
        alg.selectionTool.JvtMomentName = self._momentName
        alg.selectionTool.WorkingPoint = self.workingPoint()
        if self._maxPtForJvt is not None:
            alg.selectionTool.MaxPtForJvt = self._maxPtForJvt
        alg.selectionDecoration = decorationName
        alg.particles = config.readName(self.containerName)

        config.addSelection (self.containerName, selectionName, decorationName, preselection=False)


class _JvtWorkingPointEfficiencyBase (_JvtWorkingPointBase) :
    """common base for the Jvt and fJvt working point efficiency blocks"""

    def __init__ (self) :
        super().__init__()
        self.addDependency('EventSelection', required=False)
        self.addDependency('EventSelectionMerger', required=False)
        self.addOption ('noEffSF', False, type=bool,
            info="disables the calculation of efficiencies and scale factors. "
            "Only useful to test a new WP for which scale factors are not available.",
            expertMode=True)

    def makeAlgs (self, config) :

        suffix = self.suffix()
        decorationName = f"{self._prefix}_selection{suffix},as_char"

        if not self.noEffSF and config.dataType() is not DataType.Data:
            alg = config.createAlgorithm( 'CP::JvtEfficiencyAlg', f'{self._algPrefix}EfficiencyAlg_{self.workingPoint()}' )
            config.addPrivateTool( 'efficiencyTool', self._efficiencyTool )
            alg.efficiencyTool.JetContainer = config.readName(self.containerName)
            if self._maxPtForJvt is not None:
                alg.efficiencyTool.MaxPtForJvt = self._maxPtForJvt
            alg.efficiencyTool.WorkingPoint = self.workingPoint()
            if config.geometry() is LHCPeriod.Run2:
                alg.efficiencyTool.SFFile = f"JetJvtEfficiency/May2024/{self._sfFileStem}_Run2_EMPFlow.root"
            else:
                alg.efficiencyTool.SFFile = f"JetJvtEfficiency/May2024/{self._sfFileStem}_Run3_EMPFlow.root"
            alg.selection = decorationName
            alg.scaleFactorDecoration = f'{self._prefix}_effSF{suffix}_%SYS%'
            alg.outOfValidity = 2
            alg.outOfValidityDeco = f'no_{self._prefix}{suffix}'
            alg.skipBadEfficiency = False
            alg.jets = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, f'{self._prefix}Efficiency{suffix}')


class _JvtWorkingPointEventEfficiencyBase (_JvtWorkingPointBase) :
    """common base for the Jvt and fJvt event working point efficiency blocks"""

    def __init__ (self) :
        super().__init__()
        self.addDependency(f'{self._algPrefix}WorkingPointEfficiencyConfig', required=True)
        self.addDependency('OverlapRemoval', required=False)
        self.addDependency('EventSelection', required=False)
        self.addDependency('EventSelectionMerger', required=False)
        self.addOption ('noEffSF', False, type=bool,
            info="disables the calculation of efficiencies and scale factors. "
            "Only useful to test a new WP for which scale factors are not available.",
            expertMode=True)
        self.addOption ('eventSF', True, type=bool,
            info="add calculation of event-level efficiency SF.")

    def makeAlgs (self, config) :

        if (not self.noEffSF and self.eventSF and
            config.dataType() is not DataType.Data):
            suffix = self.suffix()
            alg = config.createAlgorithm( 'CP::AsgEventScaleFactorAlg', f'{self._eventSFAlgPrefix}_{self.workingPoint()}' )
            preselection = config.getFullSelection (self.containerName, '')
            alg.preselection = preselection + f'&&no_{self._prefix}{suffix}' if preselection else f'no_{self._prefix}{suffix}'
            alg.scaleFactorInputDecoration = f'{self._prefix}_effSF{suffix}_%SYS%'
            alg.scaleFactorOutputDecoration = f'{self._prefix}_effSF{suffix}_%SYS%'
            alg.particles = config.readName (self.containerName)

            config.addOutputVar('EventInfo', alg.scaleFactorOutputDecoration, f'weight_{self._prefix}_effSF{suffix}')


class _NNJvtParameters :
    """the parameters specific to the (NN)Jvt working point blocks"""
    _prefix = 'jvt'
    _algPrefix = 'Jvt'
    _eventSFAlgPrefix = 'JvtEventScaleFactorAlg'
    _wpOption = 'jvtWP'
    _wpInfo = "the NNJvt WP to use. Supported WPs: `FixedEffPt`."
    _wpChoices = ['FixedEffPt']
    _selectionTool = 'CP::NNJvtSelectionTool'
    _efficiencyTool = 'CP::NNJvtEfficiencyTool'
    _momentName = "NNJvt"
    _maxPtForJvt = 60*GeV
    _sfFileStem = "NNJvtSFFile"


class _FJvtParameters :
    """the parameters specific to the fJvt working point blocks"""
    _prefix = 'fjvt'
    _algPrefix = 'FJvt'
    _eventSFAlgPrefix = 'ForwardJvtEventScaleFactorAlg'
    _wpOption = 'fjvtWP'
    _wpInfo = "the fJvt WP to use. Supported WPs: `Loose`, `Tight`, `Tighter`."
    _wpChoices = ['Loose','Tight','Tighter']
    _selectionTool = 'CP::FJvtSelectionTool'
    _efficiencyTool = 'CP::FJvtEfficiencyTool'
    _momentName = "DFCommonJets_fJvt"
    _maxPtForJvt = None
    _sfFileStem = "fJvtSFFile"


class JvtWorkingPointSelectionConfig (_NNJvtParameters, _JvtWorkingPointSelectionBase) :
    """the ConfigBlock for the Jvt working point selection"""


class JvtWorkingPointEfficiencyConfig (_NNJvtParameters, _JvtWorkingPointEfficiencyBase) :
    """the ConfigBlock for the Jvt working point efficiency"""


class JvtWorkingPointEventEfficiencyConfig (_NNJvtParameters, _JvtWorkingPointEventEfficiencyBase) :
    """the ConfigBlock for the event Jvt working point efficiency"""


class FJvtWorkingPointSelectionConfig (_FJvtParameters, _JvtWorkingPointSelectionBase) :
    """the ConfigBlock for the fJvt working point selection"""


class FJvtWorkingPointEfficiencyConfig (_FJvtParameters, _JvtWorkingPointEfficiencyBase) :
    """the ConfigBlock for the fJvt working point efficiency"""


class FJvtWorkingPointEventEfficiencyConfig (_FJvtParameters, _JvtWorkingPointEventEfficiencyBase) :
    """the ConfigBlock for the event fJvt working point efficiency"""


@groupBlocks
def Jets(seq):
    seq.append(PreJetAnalysisConfig())
    seq.append(SmallRJetAnalysisConfig())
    seq.append(RScanJetAnalysisConfig())
    seq.append(LargeRJetAnalysisConfig())

@groupBlocks
def JvtWorkingPoint(seq):
    seq.append(JvtWorkingPointSelectionConfig())
    seq.append(JvtWorkingPointEfficiencyConfig())
    seq.append(JvtWorkingPointEventEfficiencyConfig())

@groupBlocks
def FJvtWorkingPoint(seq):
    seq.append(FJvtWorkingPointSelectionConfig())
    seq.append(FJvtWorkingPointEfficiencyConfig())
    seq.append(FJvtWorkingPointEventEfficiencyConfig())
