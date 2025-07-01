# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AthenaCommon.SystemOfUnits	import GeV
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaConfiguration.Enums import LHCPeriod
from Campaigns.Utils import Campaign
from AthenaCommon.Logging import logging


class InDetTrackCalibrationConfig (ConfigBlock):
    """the ConfigBlock for the track impact parameter correction"""

    def __init__ (self, containerName='') :
        super (InDetTrackCalibrationConfig, self).__init__ ()
        self.setBlockName ('InDetTracks')
        self.addOption ('inputContainer', '', type=str,
            info="select track input container, by default set to InDetTrackParticles")
        self.addOption ('containerName', containerName, type=str,
            noneAction='error',
            info="the name of the output container after calibration.")
        self.addOption ('postfix', '', type=str,
            info="a postfix to apply to decorations and algorithm names. Typically "
            "not needed here since the calibration is common to all tracks.")
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

    def makeAlgs (self, config) :
        inputContainer = "InDetTrackParticles"
        if self.inputContainer:
            inputContainer = self.inputContainer
        config.setSourceName (self.containerName, inputContainer)

        # Set up a shallow copy to decorate
        if config.wantCopy (self.containerName) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'InDetTrackShallowCopyAlg' + self.postfix )
            alg.input = config.readName (self.containerName)
            alg.output = config.copyName (self.containerName)

        # Set up the eta-cut on all tracks prior to everything else
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'InDetTrackEtaCutAlg' + self.postfix )
        alg.selectionDecoration = 'selectEta' + self.postfix + ',as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.selectionTool.maxEta = self.maxEta
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration)

        if self.minPt > 0 : # Set up the the pt selection
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'InDetTrackPtCutAlg' + self.postfix )
            alg.selectionDecoration = 'selectPt' + self.postfix + ',as_bits'
            config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
            alg.selectionTool.minPt = self.minPt
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', alg.selectionDecoration,
                                 preselection=True)

        # Set up the smearing algorithm:
        if config.dataType() is not DataType.Data:
            alg = config.createAlgorithm( 'CP::InDetTrackSmearingAlg', 'InDetTrackSmearingAlg' + self.postfix )
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


        config.addOutputVar (self.containerName, 'pt', 'pt', noSys=True)
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)
        config.addOutputVar (self.containerName, 'charge', 'charge', noSys=True)
        config.addOutputVar (self.containerName, 'qOverP', 'qOverP', noSys=True)
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

    def __init__ (self, containerName='') :
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
        self.addOption ('addSelectionToPreselection', True, type=bool,
            info="whether to retain only tracks satisfying the cutLevel "
            "requirements. The default is True.")

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
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'InDetTrackSelectionAlg' + postfix )
        alg.selectionDecoration = 'selectTrack' + postfix + ',as_bits'
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
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                             preselection=self.addSelectionToPreselection)
