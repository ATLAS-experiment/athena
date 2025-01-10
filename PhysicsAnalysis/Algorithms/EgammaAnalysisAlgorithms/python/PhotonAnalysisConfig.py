# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AthenaCommon.SystemOfUnits	import GeV
from AthenaConfiguration.Enums import LHCPeriod
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaCommon.Logging import logging

import ROOT

# E/gamma import(s).
from xAODEgamma.xAODEgammaParameters import xAOD

import PATCore.ParticleDataType

class PhotonCalibrationConfig (ConfigBlock) :
    """the ConfigBlock for the photon four-momentum correction"""

    def __init__ (self, containerName='') :
        super (PhotonCalibrationConfig, self).__init__ ()
        self.setBlockName('Photons')
        self.addOption ('containerName', containerName, type=str,
            noneAction='error',
            info="the name of the output container after calibration.")
        self.addOption ('ESModel', '', type=str,
            info="flag of egamma calibration recommendation.")
        self.addOption ('decorrelationModel', '1NP_v1', type=str,
            info="egamma energy scale decorrelationModel. The default is 1NP_v1. "
            "Supported Model: 1NP_v1, FULL_v1.")
        self.addOption ('postfix', '', type=str,
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here since the calibration is common to "
            "all photons.")
        self.addOption ('crackVeto', False, type=bool,
            info="whether to perform LAr crack veto based on the cluster eta, "
            "i.e. remove photons within 1.37<|eta|<1.52. "
            "The default is False.")
        self.addOption ('enableCleaning', True, type=bool,
            info="whether to enable photon cleaning (DFCommonPhotonsCleaning). "
            "The default is True.")
        self.addOption ('cleaningAllowLate', False, type=bool,
            info="whether to ignore timing information in cleaning "
            "(DFCommonPhotonsCleaningNoTime). The default is False.")
        self.addOption ('recomputeIsEM', False, type=bool,
            info="whether to recompute the photon shower shape fudge "
            "corrections (sets up an instance of CP::PhotonShowerShapeFudgeAlg). "
            "The default is False, i.e. to use derivation variables.")
        self.addOption ('recalibratePhyslite', True, type=bool,
            info="whether to run the CP::EgammaCalibrationAndSmearingAlg on "
            "PHYSLITE derivations. The default is True.")
        self.addOption ('minPt', 10*GeV, type=float,
            info="the minimum pT cut to apply to calibrated photons. "
            "The default is 10 GeV.")
        self.addOption ('maxEta', 2.37, type=float,
            info="maximum photon |eta| (float). The default is 2.37.")
        self.addOption ('forceFullSimConfig', False, type=bool,
            info="whether to force the tool to use the configuration meant for "
            "full simulation samples. Only for testing purposes. "
            "The default is False.")
        self.addOption ('splitCalibrationAndSmearing', False, type=bool,
            info="EXPERIMENTAL: This splits the EgammaCalibrationAndSmearingTool "
            " into two steps. The first step applies a baseline calibration that "
            "is not affected by systematics. The second step then applies the "
            "systematics dependent corrections.  The net effect is that the "
            "slower first step only has to be run once, while the second is run "
            "once per systematic. ATLASG-2358")
        self.addOption ('decorateTruth', False, type=bool,
            info="decorate truth particle information on the reconstructed one")


    def makeCalibrationAndSmearingAlg (self, config, name) :
        """Create the calibration and smearing algorithm

        Factoring this out into its own function, as we want to
        instantiate it in multiple places"""
        log = logging.getLogger('PhotonCalibrationConfig')

        # Set up the calibration and smearing algorithm:
        alg = config.createAlgorithm( 'CP::EgammaCalibrationAndSmearingAlg', name + self.postfix )
        config.addPrivateTool( 'calibrationAndSmearingTool',
                               'CP::EgammaCalibrationAndSmearingTool' )
        # Set default ESModel per period
        if self.ESModel:
            alg.calibrationAndSmearingTool.ESModel = self.ESModel
        else:
            if config.geometry() is LHCPeriod.Run2:
                alg.calibrationAndSmearingTool.ESModel = 'es2023_R22_Run2_v1'
            elif config.geometry() is LHCPeriod.Run3:
                alg.calibrationAndSmearingTool.ESModel = 'es2022_R22_PRE'
            elif config.geometry() is LHCPeriod.Run4:
                log.warning("No ESModel set for Run4, using Run3 model")
                alg.calibrationAndSmearingTool.ESModel = 'es2022_R22_PRE'
            else:
                raise ValueError (f"Can't set up the ElectronCalibrationConfig with {config.geometry().value}, "
                                  "there must be something wrong!")

        alg.calibrationAndSmearingTool.decorrelationModel = self.decorrelationModel
        alg.calibrationAndSmearingTool.useFastSim = (
            0 if self.forceFullSimConfig
            else int( config.dataType() is DataType.FastSim ))
        alg.egammas = config.readName (self.containerName)
        alg.egammasOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        return alg


    def makeAlgs (self, config) :

        log = logging.getLogger('PhotonCalibrationConfig')

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        if self.forceFullSimConfig:
            log.warning("You are running PhotonCalibrationConfig forcing full sim config")
            log.warning("This is only intended to be used for testing purposes")

        if config.isPhyslite() :
            config.setSourceName (self.containerName, "AnalysisPhotons")
        else :
            config.setSourceName (self.containerName, "Photons")

        cleaningWP = 'NoTime' if self.cleaningAllowLate else ''

        # Set up a shallow copy to decorate
        if config.wantCopy (self.containerName) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'PhotonShallowCopyAlg' + postfix )
            alg.input = config.readName (self.containerName)
            alg.output = config.copyName (self.containerName)

        # Set up the eta-cut on all photons prior to everything else
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PhotonEtaCutAlg' + postfix )
        alg.selectionDecoration = 'selectEta' + postfix + ',as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.selectionTool.maxEta = self.maxEta
        if self.crackVeto:
            alg.selectionTool.etaGapLow = 1.37
            alg.selectionTool.etaGapHigh = 1.52
        alg.selectionTool.useClusterEta = True
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration)

        # Setup shower shape fudge
        if self.recomputeIsEM and config.dataType() is DataType.FullSim:
            alg = config.createAlgorithm( 'CP::PhotonShowerShapeFudgeAlg',
                                          'PhotonShowerShapeFudgeAlg' + postfix )
            config.addPrivateTool( 'showerShapeFudgeTool',
                                    'ElectronPhotonVariableCorrectionTool' )
            if config.geometry is LHCPeriod.Run2: 
                alg.showerShapeFudgeTool.ConfigFile = \
              'EGammaVariableCorrection/TUNE25/ElPhVariableNominalCorrection.conf'
            if config.geometry is LHCPeriod.Run3:
                alg.showerShapeFudgeTool.ConfigFile = \
              'EGammaVariableCorrection/TUNE23/ElPhVariableNominalCorrection.conf'
            alg.photons = config.readName (self.containerName)
            alg.photonsOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        # Select photons only with good object quality.
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PhotonObjectQualityAlg' + postfix )
        alg.selectionDecoration = 'goodOQ,as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::EgammaIsGoodOQSelectionTool' )
        alg.selectionTool.Mask = xAOD.EgammaParameters.BADCLUSPHOTON
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration)

        # Select clean photons
        if self.enableCleaning:
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PhotonCleaningAlg' + postfix)
            config.addPrivateTool( 'selectionTool', 'CP::AsgFlagSelectionTool' )
            alg.selectionDecoration = 'isClean,as_bits'
            alg.selectionTool.selectionFlags = ['DFCommonPhotonsCleaning' + cleaningWP]
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', alg.selectionDecoration)

        # Change the origin of Photons from (0,0,0) to (0,0,z)
        # where z comes from the position of a vertex
        # Default the one tagged as Primary
        alg = config.createAlgorithm( 'CP::PhotonOriginCorrectionAlg',
                                      'PhotonOriginCorrectionAlg' + postfix )
        alg.photons = config.readName (self.containerName)
        alg.photonsOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')

        if not self.splitCalibrationAndSmearing :
            # Set up the calibration and smearing algorithm:
            alg = self.makeCalibrationAndSmearingAlg (config, 'PhotonCalibrationAndSmearingAlg')
            if config.isPhyslite() and not self.recalibratePhyslite :
                alg.skipNominal = True
        else:
            # This splits the EgammaCalibrationAndSmearingTool into two
            # steps. The first step applies a baseline calibration that
            # is not affected by systematics. The second step then
            # applies the systematics dependent corrections.  The net
            # effect is that the slower first step only has to be run
            # once, while the second is run once per systematic.
            #
            # For now (22 May 24) this has to happen in the same job, as
            # the output of the first step is not part of PHYSLITE, and
            # even for the nominal the output of the first and second
            # step are different.  In the future the plan is to put both
            # the output of the first and second step into PHYSLITE,
            # allowing to skip the first step when running on PHYSLITE.
            #
            # WARNING: All of this is experimental, see: ATLASG-2358

            # Set up the calibration algorithm:
            alg = self.makeCalibrationAndSmearingAlg (config, 'PhotonBaseCalibrationAlg')
            # turn off systematics for the calibration step
            alg.noToolSystematics = True
            # turn off smearing for the calibration step
            alg.calibrationAndSmearingTool.doSmearing = False

            # Set up the smearing algorithm:
            alg = self.makeCalibrationAndSmearingAlg (config, 'PhotonCalibrationSystematicsAlg')
            # turn off scale corrections for the smearing step
            alg.calibrationAndSmearingTool.doScaleCorrection = False
            alg.calibrationAndSmearingTool.useMVACalibration = False

        if self.minPt > 0:
            # Set up the the pt selection
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PhotonPtCutAlg' + postfix )
            alg.selectionDecoration = 'selectPt' + postfix + ',as_bits'
            config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
            alg.selectionTool.minPt = self.minPt
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', alg.selectionDecoration,
                                preselection=True)

        # Set up the isolation correction algorithm.
        alg = config.createAlgorithm( 'CP::EgammaIsolationCorrectionAlg',
                                      'PhotonIsolationCorrectionAlg' + postfix )
        config.addPrivateTool( 'isolationCorrectionTool',
                               'CP::IsolationCorrectionTool' )
        alg.isolationCorrectionTool.IsMC = config.dataType() is not DataType.Data
        alg.isolationCorrectionTool.AFII_corr = (
                0 if self.forceFullSimConfig
                else config.dataType() is DataType.FastSim)
        alg.egammas = config.readName (self.containerName)
        alg.egammasOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')

        # Additional decorations
        alg = config.createAlgorithm( 'CP::AsgEnergyDecoratorAlg', 'EnergyDecorator' + self.containerName + self.postfix )
        alg.particles = config.readName (self.containerName)

        config.addOutputVar (self.containerName, 'pt', 'pt')
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)
        config.addOutputVar (self.containerName, 'e_%SYS%', 'e')

        # decorate truth information on the reconstructed object:
        if self.decorateTruth and config.dataType() is not DataType.Data:
            config.addOutputVar (self.containerName, "truthType", "truth_type", noSys=True)
            config.addOutputVar (self.containerName, "truthOrigin", "truth_origin", noSys=True)


class PhotonWorkingPointConfig (ConfigBlock) :
    """the ConfigBlock for the photon working point

    This may at some point be split into multiple blocks (29 Aug 22)."""

    def __init__ (self, containerName='', selectionName='') :
        super (PhotonWorkingPointConfig, self).__init__ ()
        self.addOption ('containerName', containerName, type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('selectionName', selectionName, type=str,
            noneAction='error',
            info="the name of the photon selection to define (e.g. tight or "
            "loose).")
        self.addOption ('postfix', selectionName, type=str,
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here as selectionName is used internally.")
        self.addOption ('qualityWP', None, type=str,
            info="the ID WP (string) to use. Supported ID WPs: Tight, Medium and Loose.")
        self.addOption ('isolationWP', None, type=str,
            info="the ID WP (string) to use. Supported isolation WPs: "
            "FixedCutLoose, FixedCutTight, TightCaloOnly, NonIso.")
        self.addOption ('addSelectionToPreselection', True, type=bool,
            info="whether to retain only photons satisfying the working point "
            "requirements. The default is True.")
        self.addOption ('closeByCorrection', False, type=bool,
            info="whether to use close-by-corrected isolation working points")
        self.addOption ('recomputeIsEM', False, type=bool,
            info="whether to rerun the cut-based selection. The default is "
            "False, i.e. to use derivation flags.")
        self.addOption ('doFSRSelection', False, type=bool,
            info="whether to accept additional photons close to muons for the "
            "purpose of FSR corrections to these muons. Expert feature "
            "requested by the H4l analysis running on PHYSLITE. "
            "The default is False.")
        self.addOption ('noEffSF', False, type=bool,
            info="disables the calculation of efficiencies and scale factors. "
            "Experimental! only useful to test a new WP for which scale "
            "factors are not available. The default is False.")
        self.addOption ('forceFullSimConfig', False, type=bool,
            info="whether to force the tool to use the configuration meant "
            "for full simulation samples. Only for testing purposes. "
            "The default is False.")

    def makeAlgs (self, config) :

        log = logging.getLogger('PhotonWorkingPointConfig')

        # The setup below is inappropriate for Run 1
        if config.geometry() is LHCPeriod.Run1:
            raise ValueError ("Can't set up the PhotonWorkingPointConfig with %s, there must be something wrong!" % config.geometry().value)

        if self.forceFullSimConfig:
            log.warning("You are running PhotonWorkingPointConfig forcing full sim config")
            log.warning("This is only intended to be used for testing purposes")

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        if self.qualityWP == 'Tight' :
            quality = ROOT.egammaPID.PhotonTight
        elif self.qualityWP == 'Medium' :
            quality = ROOT.egammaPID.PhotonMedium
        elif self.qualityWP == 'Loose' :
            quality = ROOT.egammaPID.PhotonLoose
        else :
            raise Exception ('unknown photon quality working point "' + self.qualityWP + '" should be Tight, Medium or Loose')

        # Set up the photon selection algorithm:
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PhotonIsEMSelectorAlg' + postfix )
        alg.selectionDecoration = 'selectEM' + postfix + ',as_char'
        if self.recomputeIsEM:
            # Rerun the cut-based ID
            config.addPrivateTool( 'selectionTool', 'AsgPhotonIsEMSelector' )
            alg.selectionTool.isEMMask = quality
            if config.geometry() is LHCPeriod.Run2:
                if self.qualityWP == 'Tight':
                    alg.selectionTool.ConfigFile = 'ElectronPhotonSelectorTools/offline/mc20_20240510/PhotonIsEMTightSelectorCutDefs_pTdep_mc20_smooth.conf'
                elif self.qualityWP == 'Loose':
                    alg.selectionTool.ConfigFile = 'ElectronPhotonSelectorTools/offline/mc15_20150712/PhotonIsEMLooseSelectorCutDefs.conf'
                elif self.qualityWP == 'Medium':
                    alg.selectionTool.ConfigFile = 'ElectronPhotonSelectorTools/offline/mc20_20240510/PhotonIsEMMediumSelectorCutDefs_pTdep_smooth.conf'
            if config.geometry() is LHCPeriod.Run3:
                if self.qualityWP == 'Tight':
                    alg.selectionTool.ConfigFile = 'ElectronPhotonSelectorTools/offline/20180825/PhotonIsEMTightSelectorCutDefs.conf'
                elif self.qualityWP == 'Loose':
                    alg.selectionTool.ConfigFile = 'ElectronPhotonSelectorTools/offline/mc15_20150712/PhotonIsEMLooseSelectorCutDefs.conf'
                elif self.qualityWP == 'Medium':
                    raise ValueError('No Medium menu available for Run-3. Please get in contact with egamma')
        else:
            # Select from Derivation Framework flags
            config.addPrivateTool( 'selectionTool', 'CP::AsgFlagSelectionTool' )
            dfFlag = 'DFCommonPhotonsIsEM' + self.qualityWP
            alg.selectionTool.selectionFlags = [ dfFlag ]
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, self.selectionName)
        config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                             preselection=self.addSelectionToPreselection)

        # Set up the FSR selection
        if self.doFSRSelection :
            # save the flag set for the WP
            wpFlag = alg.selectionDecoration.split(",")[0]
            alg = config.createAlgorithm( 'CP::EgammaFSRForMuonsCollectorAlg', 'EgammaFSRForMuonsCollectorAlg' + postfix + '_ph') # added extra postfix to avoid name clash with electrons
            alg.selectionDecoration = wpFlag
            alg.ElectronOrPhotonContKey = config.readName (self.containerName)

        # Set up the isolation selection algorithm:
        if self.isolationWP != 'NonIso' :
            alg = config.createAlgorithm( 'CP::EgammaIsolationSelectionAlg',
                                          'PhotonIsolationSelectionAlg' + postfix )
            alg.selectionDecoration = 'isolated' + postfix + ',as_char'
            config.addPrivateTool( 'selectionTool', 'CP::IsolationSelectionTool' )
            alg.selectionTool.PhotonWP = self.isolationWP
            if self.closeByCorrection:
              alg.selectionTool.IsoDecSuffix = "CloseByCorr"
            alg.isPhoton = True
            alg.egammas = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                                 preselection=self.addSelectionToPreselection)

        # Set up the ID/reco photon efficiency correction algorithm:
        if config.dataType() is not DataType.Data and not self.noEffSF:
            alg = config.createAlgorithm( 'CP::PhotonEfficiencyCorrectionAlg',
                                          'PhotonEfficiencyCorrectionAlgID' + postfix )
            config.addPrivateTool( 'efficiencyCorrectionTool',
                                   'AsgPhotonEfficiencyCorrectionTool' )
            alg.scaleFactorDecoration = 'ph_id_effSF' + postfix + '_%SYS%'
            if config.dataType() is DataType.FastSim:
                alg.efficiencyCorrectionTool.ForceDataType = (
                    PATCore.ParticleDataType.Full if self.forceFullSimConfig else
                    PATCore.ParticleDataType.Fast)
            elif config.dataType() is DataType.FullSim:
                alg.efficiencyCorrectionTool.ForceDataType = \
                    PATCore.ParticleDataType.Full
            if config.geometry() >= LHCPeriod.Run2:
                alg.efficiencyCorrectionTool.MapFilePath = 'PhotonEfficiencyCorrection/2015_2025/rel22.2/2024_FinalRun2_Recommendation_v1/map1.txt'
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'ph_id_bad_eff' + postfix
            alg.photons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'id_effSF' + postfix)

        # Set up the ISO photon efficiency correction algorithm:
        if config.dataType() is not DataType.Data and self.isolationWP != 'NonIso' and not self.noEffSF:
            alg = config.createAlgorithm( 'CP::PhotonEfficiencyCorrectionAlg',
                                          'PhotonEfficiencyCorrectionAlgIsol' + postfix )
            config.addPrivateTool( 'efficiencyCorrectionTool',
                                   'AsgPhotonEfficiencyCorrectionTool' )
            alg.scaleFactorDecoration = 'ph_isol_effSF' + postfix + '_%SYS%'
            if config.dataType() is DataType.FastSim:
                alg.efficiencyCorrectionTool.ForceDataType = (
                    PATCore.ParticleDataType.Full if self.forceFullSimConfig else
                    PATCore.ParticleDataType.Fast)
            elif config.dataType() is DataType.FullSim:
                alg.efficiencyCorrectionTool.ForceDataType = \
                    PATCore.ParticleDataType.Full
            alg.efficiencyCorrectionTool.IsoKey = self.isolationWP.replace("FixedCut","")
            if config.geometry() >= LHCPeriod.Run2:
                alg.efficiencyCorrectionTool.MapFilePath = 'PhotonEfficiencyCorrection/2015_2025/rel22.2/2022_Summer_Prerecom_v1/map0.txt'
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'ph_isol_bad_eff' + postfix
            alg.photons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'isol_effSF' + postfix)





def makePhotonCalibrationConfig( seq, containerName,
                                 postfix = None,
                                 crackVeto = None,
                                 enableCleaning = None,
                                 cleaningAllowLate = None,
                                 recomputeIsEM = None,
                                 forceFullSimConfig = None):
    """Create photon calibration analysis algorithms

    This makes all the algorithms that need to be run first befor
    all working point specific algorithms and that can be shared
    between the working points.

    Keywrod arguments:
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      crackVeto -- Whether or not to perform eta crack veto
      enableCleaning -- Enable photon cleaning
      cleaningAllowLate -- Whether to ignore timing information in cleaning.
      recomputeIsEM -- Whether to rerun the cut-based selection. If not, use derivation flags
      forceFullSimConfig -- imposes full-sim config for FastSim for testing
    """

    config = PhotonCalibrationConfig (containerName)
    config.setOptionValue ('postfix', postfix)
    config.setOptionValue ('crackVeto', crackVeto)
    config.setOptionValue ('enableCleaning', enableCleaning)
    config.setOptionValue ('cleaningAllowLate', cleaningAllowLate)
    config.setOptionValue ('recomputeIsEM', recomputeIsEM)
    config.setOptionValue ('forceFullSimConfig', forceFullSimConfig)
    seq.append (config)



def makePhotonWorkingPointConfig( seq, containerName, workingPoint, selectionName,
                                  recomputeIsEM = None,
                                  noEffSF = None,
                                  forceFullSimConfig = None):
    """Create photon analysis algorithms for a single working point

    Keywrod arguments:
      workingPoint -- The working point to use
      selectionName -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      recomputeIsEM -- Whether to rerun the cut-based selection. If not, use derivation flags
      noEffSF -- Disables the calculation of efficiencies and scale factors
      forceFullSimConfig -- imposes full-sim config for FastSim for testing
    """

    config = PhotonWorkingPointConfig (containerName, selectionName)
    if workingPoint is not None :
        splitWP = workingPoint.split ('.')
        if len (splitWP) != 2 :
            raise ValueError ('working point should be of format "quality.isolation", not ' + workingPoint)
        config.setOptionValue ('qualityWP',     splitWP[0])
        config.setOptionValue ('isolationWP',   splitWP[1])
    config.setOptionValue ('recomputeIsEM', recomputeIsEM)
    config.setOptionValue ('noEffSF', noEffSF)
    config.setOptionValue ('forceFullSimConfig', forceFullSimConfig)
    seq.append (config)
