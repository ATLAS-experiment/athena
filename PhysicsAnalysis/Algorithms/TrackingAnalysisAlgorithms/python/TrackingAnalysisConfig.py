# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AthenaCommon.SystemOfUnits	import GeV
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaConfiguration.Enums import LHCPeriod
from Campaigns.Utils import Campaign
from AthenaCommon.Logging import logging

class PixelDEdxEqualizationBlock (ConfigBlock) :
    """the ConfigBlock for the Pixel ToT PID tool"""

    def __init__ (self, containerName='') :
        super (PixelDEdxEqualizationBlock, self).__init__ ()
        self.setBlockName('PixelDEdxEqualization')
        self.addOption ('containerName', containerName, type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('postfix', "", type=str,
            info="a postfix to apply to decorations and algorithm names.")
        self.addOption ('msosLink', "", type=str,
            info="Name of link from tracks to MSOSs.")
        self.addOption ('equalizeClusterMeasurements', False, type=bool,
            info="whether to equalize cluster level dE/dx measurements.")
        self.addOption ('equalizeTrackMeasurements', False, type=bool,
            info="whether to equalize track-level truncated mean dE/dx measurements (no pixel clusters required).")
        self.addOption ('tightClusterCleaning', False, type=bool,
            info="whether to perform extra cluster cleaning for dE/dx measurements (e.g. cluster size/shape).")
        self.addOption ('sfLocalFileName', "", type=str,
            info="Path to scale factor trees, overriding files stored in ASG calibration area.")
        self.addOption ('clusterSFTreeName', "cluster_SFs", type=str, # FIX! TBD
            info="Name of tree storing the cluster-level dE/dx equalization scale factors.")
        self.addOption ('trackSFTreeName', "track_SFs", type=str, # FIX! TBD
            info="Name of tree storing the track-level dE/dx equalization scale factors.")

        
    def makeAlgs (self, config) :
        alg = config.createAlgorithm( 'CP::PixelDEdxEqualizationAlg',
                                      'PixelDEdxEqualizationAlg' + self.postfix,
                                      reentrant=True)
        config.addPrivateTool( 'PixelDEdxEqualizationTool', 'CP::PixelDEdxEqualizationTool' )
        ### Algorithm properties
        alg.TrackContainerName = self.containerName
        alg.MSOSLink = self.msosLink
        alg.EqualizeClusterMeasurements = self.equalizeClusterMeasurements
        alg.EqualizeTrackMeasurements = self.equalizeTrackMeasurements
        alg.TightClusterCleaning = self.tightClusterCleaning
        ### Tool properties
        alg.PixelDEdxEqualizationTool.EqualizeClusterMeasurements = self.equalizeClusterMeasurements
        alg.PixelDEdxEqualizationTool.EqualizeTrackMeasurements = self.equalizeTrackMeasurements
        alg.PixelDEdxEqualizationTool.SFLocalFileName = self.sfLocalFileName
        alg.PixelDEdxEqualizationTool.ClusterSFTreeName = self.clusterSFTreeName
        alg.PixelDEdxEqualizationTool.TrackSFTreeName = self.trackSFTreeName
    

class InDetTrackCalibrationConfig (ConfigBlock):
    """the ConfigBlock for the track impact parameter correction"""

    def __init__ (self) :
        super (InDetTrackCalibrationConfig, self).__init__ ()
        self.setBlockName ('InDetTracks')
        self.addOption ('inputContainer', '', type=str,
            info="select track input container, by default set to InDetTrackParticles")
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the output container after calibration.")
        self.addOption ('postfix', '', type=str,
            info="a postfix to apply to decorations and algorithm names. Typically "
            "not needed here since the calibration is common to all tracks.")
        self.addOption ('runBiasing', True, type=bool,
            info="whether to run the InDetTrackBiasingTool. Allows the user to "
            "disable the tool if no recommendations are available. This should "
            "not be used in an analysis. The default is True.")
        self.addOption ('biasD0', None, type=float,
            info="a manual bias (float) to d0 in mm. Will be applied by the "
            "InDetTrackBiasingTool. Expert option in addition to the "
            "recommendations. The default is 0.")
        self.addOption ('biasZ0', None, type=float,
            info="a manual bias (float) to z0 in mm. Will be applied by the "
            "InDetTrackBiasingTool. Expert option in addition to the "
            "recommendations. The default is 0.")
        self.addOption ('biasQoverPsagitta', None, type=float,
            info="a manual bias (float) to QoverP in TeV^-1. Will be applied "
            "by the InDetTrackBiasingTool. Expert option in addition to the "
            "recommendations. The default is 0.")
        self.addOption ('customRunNumber', None, type=int,
            info="manually sets the runNumber (int) in the InDetTrackBiasingTool. "
            "Expert option leads to use of different recommendations. Default is "
            "retrieved from EventInfo")
        self.addOption ('calibFile', None, type=str,
            info="name (str) of the calibration file to use for the CTIDE "
            "calibration. Expert option to override the recommendations "
            "based on the campaign. The default is None.")
        self.addOption ('smearingToolSeed', None, type=int,
            info="random seed (int) to be used by the InDetTrackSmearingTool. "
            "Expert option. The default is 0.")
        self.addOption ('minPt', 0.5*GeV, type=float,
            info="the minimum pT cut to apply to calibrated tracks. "
            "The default is 0.5 GeV.")
        self.addOption ('maxEta', 2.5, type=float,
            info="maximum track |eta| (float). The default is 2.5.")
        self.addOption ('outputTrackSummaryInfo', False, type=bool,
            info="decorate track summary information on the reconstructed object")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName + self.postfix

    def makeAlgs (self, config) :
        log = logging.getLogger('InDetTrackCalibrationConfig')

        inputContainer = "InDetTrackParticles"
        if self.inputContainer:
            inputContainer = self.inputContainer
        config.setSourceName (self.containerName, inputContainer)

        # Set up a shallow copy to decorate
        if config.wantCopy (self.containerName) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'InDetTrackShallowCopyAlg' )
            alg.input = config.readName (self.containerName)
            alg.output = config.copyName (self.containerName)

        # Set up the eta-cut on all tracks prior to everything else
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'InDetTrackEtaCutAlg' )
        alg.selectionDecoration = 'selectEta' + self.postfix + ',as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.selectionTool.maxEta = self.maxEta
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration)

        # Set up the biasing algorithm. The recommendations is to bias MC instead of unbiasing Data:
        if config.dataType() is not DataType.Data:
            if not self.runBiasing:
                log.warning('Disabling the biasing tool for now. This should not '
                            'be used in an analysis.')
            else:
                if config.geometry() is LHCPeriod.Run3:
                    raise ValueError ('Recommendations are not yet available in Run 3.')
                elif config.geometry() is not LHCPeriod.Run2:
                    raise ValueError ('No recommendations found for geometry \"'
                                      + config.geometry().value + '\". Please check '
                                      'the configuration.')
                alg = config.createAlgorithm( 'CP::InDetTrackBiasingAlg', 'InDetTrackBiasingAlg' )
                config.addPrivateTool( 'biasingTool', 'InDet::InDetTrackBiasingTool' )
                if self.biasD0:
                    alg.biasingTool.biasD0 = self.biasD0
                if self.biasZ0:
                    alg.biasingTool.biasZ0 = self.biasZ0
                if self.biasQoverPsagitta:
                    alg.biasingTool.biasQoverPsagitta = self.biasQoverPsagitta
                if self.customRunNumber:
                    alg.biasingTool.runNumber = self.customRunNumber
                alg.inDetTracks = config.readName (self.containerName)
                alg.inDetTracksOut = config.copyName (self.containerName)
                alg.preselection = config.getPreselection (self.containerName, '')

        # Set up the smearing algorithm:
        if config.dataType() is not DataType.Data:
            alg = config.createAlgorithm( 'CP::InDetTrackSmearingAlg', 'InDetTrackSmearingAlg' )
            config.addPrivateTool( 'smearingTool', 'InDet::InDetTrackSmearingTool' )
            if self.smearingToolSeed:
                alg.smearingTool.Seed = self.smearingToolSeed
            if self.calibFile:
                alg.smearingTool.calibFileIP_CTIDE = self.calibFile
            else:
                if config.geometry() is LHCPeriod.Run2:
                    # Run 2 recommendations (MC20)
                    alg.smearingTool.calibFileIP_CTIDE = "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/d0z0_smearing_factors_Run2_v2.root"
                elif config.geometry() is LHCPeriod.Run3:
                    if config.campaign() is Campaign.MC23a:
                        # 2022 recommendations (MC23a)
                        alg.smearingTool.calibFileIP_CTIDE = "InDetTrackSystematicsTools/CalibData_25.2_2025-v00/2022_d0z0_smearing_factors_v2.root"
                    elif config.campaign() is Campaign.MC23d:
                        # 2023 recommendations (MC23d)
                        alg.smearingTool.calibFileIP_CTIDE = "InDetTrackSystematicsTools/CalibData_25.2_2025-v00/2023_d0z0_smearing_factors_v2.root"
                    elif config.campaign() is Campaign.MC23e:
                        # 2024 recommendations (MC23e)
                        alg.smearingTool.calibFileIP_CTIDE = "InDetTrackSystematicsTools/CalibData_25.2_2025-v00/2024_d0z0_smearing_factors.root"
                    else:
                        raise ValueError ('No recommendations found for capaign \"'
                                          + config.campaign().value + '\" in Run 3. '
                                          'Please check that the recommendations exist.')
                else:
                    raise ValueError ('No recommendations found for geometry \"'
                                      + config.geometry().value + '\". Please check '
                                      'the configuration.')
            alg.inDetTracks = config.readName (self.containerName)
            alg.inDetTracksOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        if self.minPt > 0 : # Set up the the pt selection
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'InDetTrackPtCutAlg' )
            alg.selectionDecoration = 'selectPt' + self.postfix + ',as_bits'
            config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
            alg.selectionTool.minPt = self.minPt
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', alg.selectionDecoration,
                                 preselection=True)

        alg = config.createAlgorithm( 'CP::InDetTrackMomentumDecoratorAlg', 'MomentumDecorator' )
        alg.inDetTracks = config.readName(self.containerName)

        config.addOutputVar (self.containerName, 'pt_%SYS%', 'pt')
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)
        config.addOutputVar (self.containerName, 'charge', 'charge', noSys=True)
        config.addOutputVar (self.containerName, 'qOverP', 'qOverP')
        config.addOutputVar (self.containerName, 'd0', 'd0')
        config.addOutputVar (self.containerName, 'z0', 'z0')
        config.addOutputVar (self.containerName, 'vz', 'vz', noSys=True)

        # decorate track summary information on the reconstructed object:
        if self.outputTrackSummaryInfo and config.dataType() is not DataType.Data:
            config.addOutputVar (self.containerName, 'numberOfInnermostPixelLayerHits', 'numberOfInnermostPixelLayerHits', noSys=True)
            config.addOutputVar (self.containerName, 'numberOfPixelDeadSensors', 'numberOfPixelDeadSensors', noSys=True)
            config.addOutputVar (self.containerName, 'numberOfPixelHits', 'numberOfPixelHits', noSys=True)
            config.addOutputVar (self.containerName, 'numberOfPixelHoles', 'numberOfPixelHoles', noSys=True)
            config.addOutputVar (self.containerName, 'numberOfPixelSharedHits', 'numberOfPixelSharedHits', noSys=True)
            config.addOutputVar (self.containerName, 'numberOfSCTDeadSensors', 'numberOfSCTDeadSensors', noSys=True)
            config.addOutputVar (self.containerName, 'numberOfSCTHits', 'numberOfSCTHits', noSys=True)
            config.addOutputVar (self.containerName, 'numberOfSCTHoles', 'numberOfSCTHoles', noSys=True)
            config.addOutputVar (self.containerName, 'numberOfSCTSharedHits', 'numberOfSCTSharedHits', noSys=True)
            config.addOutputVar (self.containerName, 'numberOfTRTHits', 'numberOfTRTHits', noSys=True)
            config.addOutputVar (self.containerName, 'numberOfTRTOutliers', 'numberOfTRTOutliers', noSys=True)


class InDetTrackWorkingPointConfig (ConfigBlock):
    """the ConfigBlock for the track working point"""

    def __init__ (self) :
        super (InDetTrackWorkingPointConfig, self).__init__ ()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('selectionName', '', type=str,
            noneAction='error',
            info="the name of the track selection to define (e.g. tightPrimary "
            "or loose).")
        self.addOption ('postfix', None, type=str,
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here as selectionName is used internally.")
        self.addOption ('cutLevel', None, type=str,
            noneAction='error',
            info="the selection WP (str) to use. Supported WPs for general "
            "use: `Loose` and `TightPrimary`. For expert studies, further "
            "WPs are available: `NoCut`, `LoosePrimary`, `LooseElectron`, "
            "`LooseMuon`, `LooseTau`, `MinBias`, `HILoose`, `HITight`, "
            "`HILooseOptimized`, `HITightOptimized`.")
        self.addOption ('additionalCuts', None, type=None,
            info="additional cuts to modify the selection WP. Only meant for "
            "expert studies of track selection. Passed as pairs of `cutName: value`. "
            "For an overview of available cuts, see twiki.cern.ch/twiki/bin/viewauth/"
            "AtlasProtected/InDetTrackSelectionTool#List_of_possible_cuts.")
        self.addOption ('runTruthFilter', True, type=bool,
            info="whether to run the TruthFilterTool. This tool is only compatible "
            "with the cut levels 'Loose' and 'TightPrimary'.")
        self.addOption ('calibFile', None, type=str,
            info="name (str) of the calibration file to use for efficiencies "
            "in the TruthFilter tool. Expert option to override the "
            "recommendations based on the campaign. The default is None.")
        self.addOption ('filterToolSeed', None, type=int,
            info="random seed (int) to be used by the InDetTrackTruthFilterTool. "
            "Expert option. The default is 0.")
        self.addOption ('fFakeLoose', None, type=float,
            info="the fraction of fake tracks (float) in the Loose working point. "
            "Will be used by the InDetTrackTruthFilterTool. Expert option to "
            "override the recommendations.")
        self.addOption ('fFakeTight', None, type=float,
            info="the fraction of fake tracks (float) in the TightPrimary working "
            "point. Will be used by the InDetTrackTruthFilterTool. Expert option "
            "to override the recommendations.")
        self.addOption ('trkEffSystScale', None, type=float,
            info="the track efficiency systematic scale (float). Will be used "
            "by the InDetTrackTruthFilterTool. Expert option to override the "
            "recommendations. Default is 1.0")
        self.addOption ('addSelectionToPreselection', True, type=bool,
            info="whether to retain only tracks satisfying the cutLevel "
            "requirements. The default is True.")

    def instanceName (self) :
        """Return the instance name for this block"""
        if self.postfix is not None:
            return self.containerName + self.selectionName + self.postfix
        else:
            return self.containerName + self.selectionName

    def makeAlgs (self, config) :
        log = logging.getLogger('InDetTrackWorkingPointConfig')

        selectionPostfix = self.selectionName
        if selectionPostfix != '' and selectionPostfix[0] != '_' :
            selectionPostfix = '_' + selectionPostfix

        postfix = self.postfix
        if postfix is None :
            postfix = self.selectionName
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        cutLevels = ["NoCut", "Loose", "LoosePrimary", "TightPrimary", "LooseMuon",
                     "LooseElectron", "LooseTau", "MinBias", "HILoose", "HITight",
                     "HILooseOptimized", "HITightOptimized"]
        alg = config.createAlgorithm( 'CP::InDetTrackSelectionAlg', 'InDetTrackSelectionAlg' )
        alg.selectionDecoration = 'selectTrack' + postfix + '_%SYS%,as_bits'
        config.addPrivateTool( 'selectionTool', 'InDet::InDetTrackSelectionTool')
        if self.cutLevel is None:
            log.warning("No selection WP chosen, not setting up InDetTrackSelectionTool.")
        elif self.cutLevel not in cutLevels:
            raise ValueError ('Invalid cut level: \"' + self.cutLevel + '\", has '
                              'to be one of: ' + ', '.join(cutLevels))
        elif self.cutLevel in ["Loose", "TightPrimary"]:
            alg.selectionTool.CutLevel = self.cutLevel
        else:
            log.warning('Using cut level: \"' + self.cutLevel + '\" that is not '
                        'meant for general use, but only expert studies.')
            alg.selectionTool.CutLevel = self.cutLevel
        if self.additionalCuts:
            for cutName, value in self.additionalCuts.items():
                setattr(alg.selectionTool, cutName, value)
        # Set up the truth filtering algorithm:
        if config.dataType() is not DataType.Data:
            if not self.runTruthFilter:
                log.warning('Disabling the TruthFilterTool.')
            else:
                config.addPrivateTool( 'filterTool', 'InDet::InDetTrackTruthFilterTool' )
                config.addPrivateTool( 'filterTool.trackOriginTool', 'InDet::InDetTrackTruthOriginTool' )
                # Set working point based on cut level
                if self.cutLevel == "Loose":
                    alg.filterWP = "LOOSE"
                elif self.cutLevel == "TightPrimary":
                    alg.filterWP = "TIGHT"
                else:
                    raise ValueError ('Attempting to set TruthFilter WP based on cut level: \"'
                                      + self.efficiencyWP + '\" that is not supported.')
                # Set calibFile and fake rates based on campaign
                if config.geometry() is LHCPeriod.Run2:
                    # Run 2 recommendations (MC20)
                    alg.filterTool.calibFileNomEff = "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/TrackingRecommendations_prelim_rel22.root"
                    alg.filterTool.fFakeLoose = 0.10
                    alg.filterTool.fFakeTight = 1.00
                elif config.geometry() is LHCPeriod.Run3:
                    if config.campaign() in [Campaign.MC23a, Campaign.MC23d, Campaign.MC23e]:
                        # 2022/23/24 recommendations (MC23a/d/e)
                        alg.filterTool.calibFileNomEff = "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/TrackingRecommendations_prelim_rel22.root"
                        alg.filterTool.fFakeLoose = 0.40
                        alg.filterTool.fFakeTight = 1.00
                    elif not (self.calibFile and self.fFakeLoose and self.fFakeTight):
                        raise ValueError ('No efficiency recommendations found for capaign \"'
                                          + config.campaign().value + '\" in Run 3. '
                                          'Please check that the recommendations exist.')
                elif not (self.calibFile and self.fFakeLoose and self.fFakeTight):
                    raise ValueError ('No efficiency recommendations found for geometry \"'
                                      + config.geometry().value + '\". Please check '
                                      'the configuration.')
                # Set custom calibFile, fake rates, or random seed
                if self.calibFile:
                    alg.filterTool.calibFileNomEff = self.calibFile
                if self.fFakeLoose:
                    alg.filterTool.fFakeLoose = self.fFakeLoose
                if self.fFakeTight:
                    alg.filterTool.fFakeTight = self.fFakeTight
                if self.filterToolSeed:
                    alg.filterTool.Seed = self.filterToolSeed
                if self.trkEffSystScale:
                    alg.filterTool.trkEffSystScale = self.trkEffSystScale
        alg.inDetTracks = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                             preselection=self.addSelectionToPreselection)
