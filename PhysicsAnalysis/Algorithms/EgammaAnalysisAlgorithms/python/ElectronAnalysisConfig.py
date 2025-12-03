# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AthenaCommon.SystemOfUnits	import GeV
from AthenaConfiguration.Enums import LHCPeriod
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from TrackingAnalysisAlgorithms.TrackingAnalysisConfig import InDetTrackCalibrationConfig
from TrigGlobalEfficiencyCorrection.TriggerLeg_DictHelpers import TriggerDict, MapKeysDict
from AthenaCommon.Logging import logging

# E/gamma import(s).
from xAODEgamma.xAODEgammaParameters import xAOD

import PATCore.ParticleDataType


class ElectronCalibrationConfig (ConfigBlock) :
    """the ConfigBlock for the electron four-momentum correction"""

    def __init__ (self) :
        super (ElectronCalibrationConfig, self).__init__ ()
        self.setBlockName('Electrons')
        self.addOption ('inputContainer', '', type=str,  
            info="select electron input container, by default set to Electrons")
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the output container after calibration.")
        self.addOption ('ESModel', '', type=str,
            info="flag of egamma calibration recommendation.")
        self.addOption ('decorrelationModel', '1NP_v1', type=str,
            info="egamma energy scale decorrelationModel. The default is 1NP_v1. "
            "Supported Model: 1NP_v1, FULL_v1.")
        self.addOption ('postfix', '', type=str,
            info="a postfix to apply to decorations and algorithm names. Typically "
            "not needed here since the calibration is common to all electrons.")
        self.addOption ('crackVeto', False, type=bool,
            info="whether to perform LAr crack veto based on the cluster eta, "
            "i.e. remove electrons within 1.37<|eta|<1.52. The default "
            "is False.")
        self.addOption ('isolationCorrection', True, type=bool,
            info="whether or not to perform isolation corrections (leakage "
            "corrections), i.e. set up an instance of "
            "CP::EgammaIsolationCorrectionAlg.")
        self.addOption ('recalibratePhyslite', True, type=bool,
            info="whether to run the CP::EgammaCalibrationAndSmearingAlg on "
            "PHYSLITE derivations. The default is True.")
        self.addOption ('minPt', 4.5*GeV, type=float,
            info="the minimum pT cut to apply to calibrated electrons. "
            "The default is 4.5 GeV.")
        self.addOption ('maxEta', 2.47, type=float,
            info="maximum electron |eta| (float). The default is 2.47.")
        self.addOption ('forceFullSimConfigForP4', False, type=bool,
            info="whether to force the tool to use the configuration meant for "
            "full simulation samples for P4 corrections. Only for testing purposes. "
            "The default is False.")
        self.addOption ('forceFullSimConfigForIso', False, type=bool,
            info="whether to force the tool to use the configuration meant for "
            "full simulation samples for isolation corrections. Only for testing purposes. "
            "The default is False.")
        self.addOption ('splitCalibrationAndSmearing', False, type=bool,
            info="EXPERIMENTAL: This splits the EgammaCalibrationAndSmearingTool "
            " into two steps. The first step applies a baseline calibration that "
            "is not affected by systematics. The second step then applies the "
            "systematics dependent corrections.  The net effect is that the "
            "slower first step only has to be run once, while the second is run "
            "once per systematic. ATLASG-2358",
            expertMode=True)
        self.addOption ('runTrackBiasing', False, type=bool,
            info="EXPERIMENTAL: This enables the InDetTrackBiasingTool, for "
            "tracks associated to Electrons. The the tool does not have run 3 "
            "recommendations yet.",
            expertMode=True)

        self.addOption ('decorateTruth', False, type=bool,
            info="decorate truth particle information on the reconstructed one")
        self.addOption ('decorateCaloClusterEta', False, type=bool,
            info="decorate the calo cluster eta on the reconstructed one")
        self.addOption ('writeTrackD0Z0', False, type = bool,
            info="save the d0 significance and z0sinTheta variables so they can be written out")
        self.addOption ('decorateEmva', False, type=bool,
            info="decorate E_mva_only on the objects (needed for columnar tools/PHYSLITE)")

        self.addOption ('decorateSamplingPattern', False, type=bool,
            info="add samplingPattern decorations to clusters as part of PHYSLITE")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName + self.postfix

    def makeCalibrationAndSmearingAlg (self, config, name) :
        """Create the calibration and smearing algorithm

        Factoring this out into its own function, as we want to
        instantiate it in multiple places"""
        log = logging.getLogger('ElectronCalibrationConfig')

        # Set up the calibration and smearing algorithm:
        alg = config.createAlgorithm( 'CP::EgammaCalibrationAndSmearingAlg', name )
        config.addPrivateTool( 'calibrationAndSmearingTool',
                            'CP::EgammaCalibrationAndSmearingTool' )
        # Set default ESModel per period
        if self.ESModel:
            alg.calibrationAndSmearingTool.ESModel = self.ESModel
        else:
            if config.geometry() is LHCPeriod.Run2:
                alg.calibrationAndSmearingTool.ESModel = 'es2023_R22_Run2_v1'
            elif config.geometry() is LHCPeriod.Run3:
                alg.calibrationAndSmearingTool.ESModel = 'es2024_Run3_v0'
            elif config.geometry() is LHCPeriod.Run4:
                log.warning("No ESModel set for Run4, using Run 3 model instead")
                alg.calibrationAndSmearingTool.ESModel = 'es2024_Run3_v0'
            else:
                raise ValueError (f"Can't set up the ElectronCalibrationConfig with {config.geometry().value}, "
                                  "there must be something wrong!")

        alg.calibrationAndSmearingTool.decorrelationModel = self.decorrelationModel
        alg.calibrationAndSmearingTool.useFastSim = (
            0 if self.forceFullSimConfigForP4
            else int( config.dataType() is DataType.FastSim ))
        alg.calibrationAndSmearingTool.decorateEmva = self.decorateEmva
        alg.egammas = config.readName (self.containerName)
        alg.egammasOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        return alg


    def makeAlgs (self, config) :

        log = logging.getLogger('ElectronCalibrationConfig')

        if self.forceFullSimConfigForP4:
            log.warning("You are running ElectronCalibrationConfig forcing full sim config")
            log.warning(" This is only intended to be used for testing purposes")

        inputContainer = "AnalysisElectrons" if config.isPhyslite() else "Electrons"
        if self.inputContainer:
            inputContainer = self.inputContainer
        config.setSourceName (self.containerName, inputContainer)

        # Decorate calo cluster eta if required
        if self.decorateCaloClusterEta:
            alg = config.createAlgorithm( 'CP::EgammaCaloClusterEtaAlg',
                                          'ElectronEgammaCaloClusterEtaAlg',
                                           reentrant=True )
            alg.particles = config.readName(self.containerName)
            config.addOutputVar (self.containerName, 'caloEta2', 'caloEta2', noSys=True)

        if self.decorateSamplingPattern:
            config.createAlgorithm( 'CP::EgammaSamplingPatternDecoratorAlg', 'EgammaSamplingPatternDecoratorAlg' )

        # Set up a shallow copy to decorate
        if config.wantCopy (self.containerName) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'ElectronShallowCopyAlg' )
            alg.input = config.readName (self.containerName)
            alg.output = config.copyName (self.containerName)

        # Set up the eta-cut on all electrons prior to everything else
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronEtaCutAlg' )
        alg.selectionDecoration = 'selectEta' + self.postfix + ',as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.selectionTool.maxEta = self.maxEta
        if self.crackVeto:
            alg.selectionTool.etaGapLow = 1.37
            alg.selectionTool.etaGapHigh = 1.52
        alg.selectionTool.useClusterEta = True
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration)

        # Select electrons only with good object quality.
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronObjectQualityAlg' )
        alg.selectionDecoration = 'goodOQ' + self.postfix + ',as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::EgammaIsGoodOQSelectionTool' )
        alg.selectionTool.Mask = xAOD.EgammaParameters.BADCLUSELECTRON
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration)

        if not self.splitCalibrationAndSmearing :
            # Set up the calibration and smearing algorithm:
            alg = self.makeCalibrationAndSmearingAlg (config, 'ElectronCalibrationAndSmearingAlg')
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
            alg = self.makeCalibrationAndSmearingAlg (config, 'ElectronBaseCalibrationAlg')
            # turn off systematics for the calibration step
            alg.noToolSystematics = True
            # turn off smearing for the calibration step
            alg.calibrationAndSmearingTool.doSmearing = False

            # Set up the smearing algorithm:
            alg = self.makeCalibrationAndSmearingAlg (config, 'ElectronCalibrationSystematicsAlg')
            # turn off scale corrections for the smearing step
            alg.calibrationAndSmearingTool.doScaleCorrection = False
            alg.calibrationAndSmearingTool.useMVACalibration = False
            alg.calibrationAndSmearingTool.decorateEmva = False
        
        if self.minPt > 0 :
            # Set up the the pt selection
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronPtCutAlg' )
            alg.selectionDecoration = 'selectPt' + self.postfix + ',as_bits'
            config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
            alg.selectionTool.minPt = self.minPt
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', alg.selectionDecoration,
                                preselection=True)

        # Set up the isolation correction algorithm:
        if self.isolationCorrection:
            alg = config.createAlgorithm( 'CP::EgammaIsolationCorrectionAlg',
                                          'ElectronIsolationCorrectionAlg' )
            config.addPrivateTool( 'isolationCorrectionTool',
                                   'CP::IsolationCorrectionTool' )
            alg.isolationCorrectionTool.IsMC = config.dataType() is not DataType.Data
            alg.isolationCorrectionTool.AFII_corr = (
                0 if self.forceFullSimConfigForIso
                else config.dataType() is DataType.FastSim)
            alg.isolationCorrectionTool.FixTimingIssueInCore = True
            alg.isolationCorrectionTool.ToolVer = "REL22"
            alg.isolationCorrectionTool.CorrFile = "IsolationCorrections/v6/isolation_ptcorrections_rel22_mc20.root"
            alg.egammas = config.readName (self.containerName)
            alg.egammasOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
        else:
            log.warning("You are not applying the isolation corrections")
            log.warning("This is only intended to be used for testing purposes")
            
        # Additional decorations
        if self.writeTrackD0Z0:
            alg = config.createAlgorithm( 'CP::AsgLeptonTrackDecorationAlg',
                                          'LeptonTrackDecorator' )
            if config.dataType() is not DataType.Data:
                if self.runTrackBiasing:
                    InDetTrackCalibrationConfig.makeTrackBiasingTool(config, alg)
                InDetTrackCalibrationConfig.makeTrackSmearingTool(config, alg)
            alg.particles = config.readName (self.containerName)

        alg = config.createAlgorithm( 'CP::AsgEnergyDecoratorAlg', 'EnergyDecorator' )
        alg.particles = config.readName(self.containerName)

        config.addOutputVar (self.containerName, 'pt', 'pt')
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)
        config.addOutputVar (self.containerName, 'e_%SYS%', 'e')
        config.addOutputVar (self.containerName, 'charge', 'charge', noSys=True)

        if self.writeTrackD0Z0:
            config.addOutputVar (self.containerName, 'd0_%SYS%', 'd0')
            config.addOutputVar (self.containerName, 'd0sig_%SYS%', 'd0sig')
            config.addOutputVar (self.containerName, 'z0_%SYS%', 'z0')
            config.addOutputVar (self.containerName, 'z0sintheta_%SYS%', 'z0sintheta')
            config.addOutputVar (self.containerName, 'z0sinthetasig_%SYS%', 'z0sinthetasig')

        # decorate truth information on the reconstructed object:
        if self.decorateTruth and config.dataType() is not DataType.Data:
            config.addOutputVar (self.containerName, "truthType", "truth_type", noSys=True)
            config.addOutputVar (self.containerName, "truthOrigin", "truth_origin", noSys=True)

            config.addOutputVar (self.containerName, "firstEgMotherPdgId", "truth_firstEgMotherPdgId", noSys=True)
            config.addOutputVar (self.containerName, "firstEgMotherTruthOrigin", "truth_firstEgMotherTruthOrigin", noSys=True)
            config.addOutputVar (self.containerName, "firstEgMotherTruthType", "truth_firstEgMotherTruthType", noSys=True)


class ElectronWorkingPointConfig (ConfigBlock) :
    """the ConfigBlock for the electron working point

    This may at some point be split into multiple blocks (29 Aug 22)."""

    def __init__ (self) :
        super (ElectronWorkingPointConfig, self).__init__ ()
        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the input container.")
        self.addOption ('selectionName', '', type=str,
            noneAction='error',
            info="the name of the electron selection to define (e.g. tight or "
            "loose).")
        self.addOption ('postfix', None, type=str,
            info="a postfix to apply to decorations and algorithm names. "
            "Typically not needed here as selectionName is used internally.")
        self.addOption ('trackSelection', True, type=bool,
            info="whether or not to set up an instance of "
            "CP::AsgLeptonTrackSelectionAlg, with the recommended d_0 and "
            "z_0 sin(theta) cuts. The default is True.")
        self.addOption ('maxD0Significance', 5, type=float,
            info="maximum d0 significance used for the trackSelection"
            "The default is 5")
        self.addOption ('maxDeltaZ0SinTheta', 0.5, type=float,
            info="maximum z0sinTheta in mm used for the trackSelection"
            "The default is 0.5 mm")
        self.addOption ('identificationWP', None, type=str,
            info="the ID WP (string) to use. Supported ID WPs: TightLH, "
            "MediumLH, LooseBLayerLH, TightDNN, MediumDNN, LooseDNN, "
            "TightNoCFDNN, MediumNoCFDNN, VeryLooseNoCF97DNN, NoID.",
            expertMode=["NoID"])
        self.addOption ('isolationWP', None, type=str,
            info="the isolation WP (string) to use. Supported isolation WPs: "
            "HighPtCaloOnly, Loose_VarRad, Tight_VarRad, TightTrackOnly_"
            "VarRad, TightTrackOnly_FixedRad, NonIso.")
        self.addOption ('convSelection', None, type=str,
            info="enter additional selection (string) to use. To be used with "
            "TightLH or will crash. Supported keywords:"
            "Veto, MatConv, GammaStar.")
        self.addOption ('addSelectionToPreselection', True, type=bool,
            info="whether to retain only electrons satisfying the working point "
            "requirements. The default is True.")
        self.addOption ('closeByCorrection', False, type=bool,
            info="whether to use close-by-corrected isolation working points")
        self.addOption ('recomputeID', False, type=bool,
            info="whether to rerun the ID LH/DNN. The default is False, i.e. to use "
            "derivation flags.")
        self.addOption ('chargeIDSelectionRun2', False, type=bool,
            info="whether to run the ECIDS tool. Only available for run 2. "
            "The default is False.")
        self.addOption ('recomputeChargeID', False, type=bool,
            info="whether to rerun the ECIDS. The default is False, i.e. to use "
            "derivation flags.")
        self.addOption ('doFSRSelection', False, type=bool,
            info="whether to accept additional electrons close to muons for "
            "the purpose of FSR corrections to these muons. Expert feature "
            "requested by the H4l analysis running on PHYSLITE. "
            "The default is False.",
            expertMode=True)
        self.addOption ('noEffSF', False, type=bool,
            info="disables the calculation of efficiencies and scale factors. "
            "Experimental! only useful to test a new WP for which scale "
            "factors are not available. The default is False.",
            expertMode=True)
        self.addOption ('saveDetailedSF', True, type=bool,
            info="save all the independent detailed object scale factors. "
            "The default is True.")
        self.addOption ('saveCombinedSF', False, type=bool,
            info="save the combined object scale factor. "
            "The default is False.")
        self.addOption ('forceFullSimConfig', False, type=bool,
            info="whether to force the tool to use the configuration meant for "
            "full simulation samples. Only for testing purposes. "
            "The default is False.")
        self.addOption ('correlationModelId', 'SIMPLIFIED', type=str,
            info="the correlation model (string) to use for ID scale factors "
            "Supported models: SIMPLIFIED (default), FULL, TOTAL, TOYS")
        self.addOption ('correlationModelIso', 'SIMPLIFIED', type=str,
            info="the correlation model (string) to use for isolation scale factors "
            "Supported models: SIMPLIFIED (default), FULL, TOTAL, TOYS")
        self.addOption ('correlationModelReco', 'SIMPLIFIED', type=str,
            info="the correlation model (string) to use for reconstruction scale factors "
            "Supported models: SIMPLIFIED (default), FULL, TOTAL, TOYS")
        self.addOption('addChargeMisIDSF', False, type=bool,
            info="Adds scale factors for charge-misID.")

    def instanceName (self) :
        """Return the instance name for this block"""
        if self.postfix is not None :
            return self.containerName + '_' + self.selectionName + self.postfix
        return self.containerName + '_' + self.selectionName

    def makeAlgs (self, config) :

        log = logging.getLogger('ElectronWorkingPointConfig')

        if self.forceFullSimConfig:
            log.warning("You are running ElectronWorkingPointConfig forcing full sim config")
            log.warning("This is only intended to be used for testing purposes")

        selectionPostfix = self.selectionName
        if selectionPostfix != '' and selectionPostfix[0] != '_' :
            selectionPostfix = '_' + selectionPostfix

        # The setup below is inappropriate for Run 1
        if config.geometry() is LHCPeriod.Run1:
            raise ValueError ("Can't set up the ElectronWorkingPointConfig with %s, there must be something wrong!" % config.geometry().value)

        postfix = self.postfix
        if postfix is None :
            postfix = self.selectionName
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        # Set up the track selection algorithm:
        if self.trackSelection :
            alg = config.createAlgorithm( 'CP::AsgLeptonTrackSelectionAlg',
                                          'ElectronTrackSelectionAlg',
                                          reentrant=True )
            alg.selectionDecoration = 'trackSelection' + postfix + ',as_bits'
            alg.maxD0Significance = self.maxD0Significance
            alg.maxDeltaZ0SinTheta = self.maxDeltaZ0SinTheta
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            if self.trackSelection :
                config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                                     preselection=self.addSelectionToPreselection)

        if 'LH' in self.identificationWP:
            # Set up the likelihood ID selection algorithm
            # It is safe to do this before calibration, as the cluster E is used
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronLikelihoodAlg' )
            alg.selectionDecoration = 'selectLikelihood' + selectionPostfix + ',as_char'
            if self.recomputeID:
                # Rerun the likelihood ID
                config.addPrivateTool( 'selectionTool', 'AsgElectronLikelihoodTool' )
                alg.selectionTool.primaryVertexContainer = 'PrimaryVertices'
                # Here we have to match the naming convention of EGSelectorConfigurationMapping.h
                # which differ from the one used for scale factors
                if config.geometry() >= LHCPeriod.Run3:
                    if 'HI' not in self.identificationWP:
                        alg.selectionTool.WorkingPoint = self.identificationWP.replace("BLayer","BL") + 'Electron'
                    else:
                        alg.selectionTool.WorkingPoint = self.identificationWP.replace('_HI', 'Electron_HI')
                elif config.geometry() is LHCPeriod.Run2:
                    alg.selectionTool.WorkingPoint = self.identificationWP.replace("BLayer","BL") + 'Electron_Run2'
            else:
                # Select from Derivation Framework flags
                config.addPrivateTool( 'selectionTool', 'CP::AsgFlagSelectionTool' )
                dfFlag = "DFCommonElectronsLH" + self.identificationWP.split('LH')[0]
                dfFlag = dfFlag.replace("BLayer","BL")
                alg.selectionTool.selectionFlags = [dfFlag]
        elif 'SiHit' in self.identificationWP:
            # Only want SiHit electrons, so veto loose LH electrons
            algVeto = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronLikelihoodAlgVeto')
            algVeto.selectionDecoration = 'selectLikelihoodVeto' + postfix + ',as_char'
            config.addPrivateTool( 'selectionTool', 'CP::AsgFlagSelectionTool' )
            algVeto.selectionTool.selectionFlags = ["DFCommonElectronsLHLoose"]
            algVeto.selectionTool.invertFlags    = [True]
            algVeto.particles = config.readName (self.containerName)
            algVeto.preselection = config.getPreselection (self.containerName, self.selectionName)
            # add the veto as a selection
            config.addSelection (self.containerName, self.selectionName, algVeto.selectionDecoration,
                                 preselection=self.addSelectionToPreselection)

            # Select SiHit electrons using IsEM bits
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronLikelihoodAlg' )
            alg.selectionDecoration = 'selectSiHit' + selectionPostfix + ',as_char'
            # Select from Derivation Framework IsEM bits
            config.addPrivateTool( 'selectionTool', 'CP::AsgMaskSelectionTool' )
            dfVar = "DFCommonElectronsLHLooseBLIsEMValue"
            alg.selectionTool.selectionVars = [dfVar]
            mask = int( 0 | 0x1 << 1 | 0x1 << 2)
            alg.selectionTool.selectionMasks = [mask]
        elif 'DNN' in self.identificationWP:
            if self.chargeIDSelectionRun2:
                raise ValueError('DNN is not intended to be used with '
                                 '`chargeIDSelectionRun2` option as there are '
                                 'DNN WPs containing charge flip rejection.')
            # Set up the DNN ID selection algorithm
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronDNNAlg' )
            alg.selectionDecoration = 'selectDNN' + selectionPostfix + ',as_char'
            if self.recomputeID:
                # Rerun the DNN ID
                config.addPrivateTool( 'selectionTool', 'AsgElectronSelectorTool' )
                # Here we have to match the naming convention of EGSelectorConfigurationMapping.h
                if config.geometry() is LHCPeriod.Run3:
                    raise ValueError ( "DNN working points are not available for Run 3 yet.")
                else:
                    alg.selectionTool.WorkingPoint = self.identificationWP + 'Electron'
            else:
                # Select from Derivation Framework flags
                config.addPrivateTool( 'selectionTool', 'CP::AsgFlagSelectionTool' )
                dfFlag = "DFCommonElectronsDNN" + self.identificationWP.split('DNN')[0]
                alg.selectionTool.selectionFlags = [dfFlag]
        elif self.identificationWP == 'NoID':
            alg = None
        else:
            raise ValueError (f"Electron ID working point '{self.identificationWP}' is not recognised!")

        if alg is not None:
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                                 preselection=self.addSelectionToPreselection)

        # maintain order of selections
        if 'SiHit' in self.identificationWP:
            # Set up the ElectronSiHitDecAlg algorithm to decorate SiHit electrons with a minimal amount of information:
            algDec = config.createAlgorithm( 'CP::ElectronSiHitDecAlg', 'ElectronSiHitDecAlg' )
            selDec = 'siHitEvtHasLeptonPair' + selectionPostfix + ',as_char'
            algDec.selectionName     = selDec.split(",")[0]
            algDec.ElectronContainer = config.readName (self.containerName)
            # Set flag to only collect SiHit electrons for events with an electron or muon pair to minimize size increase from SiHit electrons
            algDec.RequireTwoLeptons = True
            config.addSelection (self.containerName, self.selectionName, selDec,
                                 preselection=self.addSelectionToPreselection)

        # Additional selection for conversions and gamma*
        if self.convSelection is not None:
            # skip if not applied together with TightLH
            if self.identificationWP != 'TightLH':
                raise ValueError(f"convSelection can only be used with TightLH ID, "
                                 f"whereas {self.identificationWP} has been selected. convSelection option will be ignored.")
            # check if allowed value 
            allowedValues = ["Veto", "GammaStar", "MatConv"]
            if self.convSelection not in allowedValues:  
                raise ValueError(f"convSelection has been set to {self.convSelection}, which is not a valid option. "
                                 f"convSelection option must be one of {allowedValues}.")

            # ambiguityType == 0
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronAmbiguityTypeAlg' )
            alg.selectionDecoration = 'selectAmbiguityType' + selectionPostfix + ',as_char'
            config.addPrivateTool( 'selectionTool', 'CP::AsgNumDecorationSelectionToolUInt8' )
            alg.selectionTool.decorationName = "ambiguityType"
            alg.selectionTool.doEqual = True
            alg.selectionTool.equal = 0
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                                 preselection=self.addSelectionToPreselection)

            # DFCommonAddAmbiguity selection
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronDFCommonAddAmbiguityAlg' )
            alg.selectionDecoration = 'selectDFCommonAddAmbiguity' + selectionPostfix + ',as_char'
            config.addPrivateTool( 'selectionTool', 'CP::AsgNumDecorationSelectionToolInt' )
            alg.selectionTool.decorationName = "DFCommonAddAmbiguity"
            if self.convSelection == "Veto":
                alg.selectionTool.doMax = True
                alg.selectionTool.max = 1
            elif self.convSelection == "GammaStar":
                alg.selectionTool.doEqual = True
                alg.selectionTool.equal = 1
            elif self.convSelection == "MatConv":
                alg.selectionTool.doEqual = True
                alg.selectionTool.equal = 2
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                                 preselection=self.addSelectionToPreselection)

        # Set up the FSR selection
        if self.doFSRSelection :
            # save the flag set for the WP
            wpFlag = alg.selectionDecoration.split(",")[0]
            alg = config.createAlgorithm( 'CP::EgammaFSRForMuonsCollectorAlg', 'EgammaFSRForMuonsCollectorAlg' )
            alg.selectionDecoration = wpFlag
            alg.ElectronOrPhotonContKey = config.readName (self.containerName)
            # For SiHit electrons, set flag to remove FSR electrons.
            # For standard electrons, FSR electrons need to be added as they may be missed by the standard selection.
            # For SiHit electrons FSR electrons are generally always selected, so they should be removed since they will be in the standard electron container.
            if 'SiHit' in self.identificationWP:
                alg.vetoFSR = True

        # Set up the isolation selection algorithm:
        if self.isolationWP != 'NonIso' :
            alg = config.createAlgorithm( 'CP::EgammaIsolationSelectionAlg',
                                          'ElectronIsolationSelectionAlg' )
            alg.selectionDecoration = 'isolated' + selectionPostfix + ',as_char'
            config.addPrivateTool( 'selectionTool', 'CP::IsolationSelectionTool' )
            alg.selectionTool.ElectronWP = self.isolationWP
            if self.closeByCorrection:
              alg.selectionTool.IsoDecSuffix = "CloseByCorr"
            alg.egammas = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                                 preselection=self.addSelectionToPreselection)

        if self.chargeIDSelectionRun2 and config.geometry() >= LHCPeriod.Run3:
            log.warning("ECIDS is only available for Run 2 and will not have any effect in Run 3.")

        # Select electrons only if they don't appear to have flipped their charge.
        if self.chargeIDSelectionRun2 and config.geometry() < LHCPeriod.Run3:
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg',
                                          'ElectronChargeIDSelectionAlg' )
            alg.selectionDecoration = 'chargeID' + selectionPostfix + ',as_char'
            if self.recomputeChargeID:
                # Rerun the ECIDS BDT
                config.addPrivateTool( 'selectionTool',
                                       'AsgElectronChargeIDSelectorTool' )
                alg.selectionTool.TrainingFile = \
                    'ElectronPhotonSelectorTools/ChargeID/ECIDS_20180731rel21Summer2018.root'
                alg.selectionTool.WorkingPoint = 'Loose'
                alg.selectionTool.CutOnBDT = -0.337671 # Loose 97%
            else:
                # Select from Derivation Framework flags
                config.addPrivateTool( 'selectionTool', 'CP::AsgFlagSelectionTool' )
                alg.selectionTool.selectionFlags = ["DFCommonElectronsECIDS"]

            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration,
                                 preselection=self.addSelectionToPreselection)

        correlationModels = ["SIMPLIFIED", "FULL", "TOTAL", "TOYS"]
        map_file = 'ElectronEfficiencyCorrection/2015_2025/rel22.2/2025_Run2Rel22_Recommendation_v3/map0.txt' \
                   if config.geometry() is LHCPeriod.Run2 else \
                   'ElectronEfficiencyCorrection/2015_2025/rel22.2/2025_Run3_Consolidated_Prerecom_v3/map1.txt'
        sfList = []
        # Set up the RECO electron efficiency correction algorithm:
        if config.dataType() is not DataType.Data and not self.noEffSF:
            if 'DNN' in self.identificationWP:
                raise ValueError('DNN does not yet have efficiency correction, '
                                 'please disable it by setting `noEffSF` to True.')

            alg = config.createAlgorithm( 'CP::ElectronEfficiencyCorrectionAlg',
                                          'ElectronEfficiencyCorrectionAlgReco' )
            config.addPrivateTool( 'efficiencyCorrectionTool',
                                   'AsgElectronEfficiencyCorrectionTool' )
            alg.scaleFactorDecoration = 'el_reco_effSF' + selectionPostfix + '_%SYS%'
            alg.efficiencyCorrectionTool.MapFilePath = map_file
            alg.efficiencyCorrectionTool.RecoKey = "Reconstruction"
            if self.correlationModelReco not in correlationModels:
                raise ValueError('Invalid correlation model for reconstruction efficiency, '
                                 f'has to be one of: {", ".join(correlationModels)}')
            if config.geometry() >= LHCPeriod.Run3 and self.correlationModelReco != "TOTAL":
                log.warning("Only TOTAL correlation model is currently supported "
                            "for reconstruction efficiency correction in Run 3.")
                alg.efficiencyCorrectionTool.CorrelationModel = "TOTAL"
            else:
                alg.efficiencyCorrectionTool.CorrelationModel = self.correlationModelReco
            if config.dataType() is DataType.FastSim:
                alg.efficiencyCorrectionTool.ForceDataType = (
                    PATCore.ParticleDataType.Full if self.forceFullSimConfig
                    else PATCore.ParticleDataType.Fast)
            elif config.dataType() is DataType.FullSim:
                alg.efficiencyCorrectionTool.ForceDataType = \
                    PATCore.ParticleDataType.Full
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'el_reco_bad_eff' + selectionPostfix
            alg.electrons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            if self.saveDetailedSF:
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration,
                                     'reco_effSF' + postfix)
            sfList += [alg.scaleFactorDecoration]

        # Set up the ID electron efficiency correction algorithm:
        if config.dataType() is not DataType.Data and not self.noEffSF and self.identificationWP != 'NoID':

            alg = config.createAlgorithm( 'CP::ElectronEfficiencyCorrectionAlg',
                                          'ElectronEfficiencyCorrectionAlgID' )
            config.addPrivateTool( 'efficiencyCorrectionTool',
                                   'AsgElectronEfficiencyCorrectionTool' )
            alg.scaleFactorDecoration = 'el_id_effSF' + selectionPostfix + '_%SYS%'
            alg.efficiencyCorrectionTool.MapFilePath = map_file
            alg.efficiencyCorrectionTool.IdKey = self.identificationWP.replace("LH","")
            if self.correlationModelId not in correlationModels:
                raise ValueError('Invalid correlation model for identification efficiency, '
                                 f'has to be one of: {", ".join(correlationModels)}')
            alg.efficiencyCorrectionTool.CorrelationModel = self.correlationModelId
            if config.dataType() is DataType.FastSim:
                alg.efficiencyCorrectionTool.ForceDataType = (
                    PATCore.ParticleDataType.Full if self.forceFullSimConfig
                    else PATCore.ParticleDataType.Fast)
            elif config.dataType() is DataType.FullSim:
                alg.efficiencyCorrectionTool.ForceDataType = \
                    PATCore.ParticleDataType.Full
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'el_id_bad_eff' + selectionPostfix
            alg.electrons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            if self.saveDetailedSF:
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration,
                                     'id_effSF' + postfix)
            sfList += [alg.scaleFactorDecoration]

        # Set up the ISO electron efficiency correction algorithm:
        if config.dataType() is not DataType.Data and self.isolationWP != 'NonIso' and not self.noEffSF:
            alg = config.createAlgorithm( 'CP::ElectronEfficiencyCorrectionAlg',
                                          'ElectronEfficiencyCorrectionAlgIsol' )
            config.addPrivateTool( 'efficiencyCorrectionTool',
                                   'AsgElectronEfficiencyCorrectionTool' )
            alg.scaleFactorDecoration = 'el_isol_effSF' + selectionPostfix + '_%SYS%'
            alg.efficiencyCorrectionTool.MapFilePath = map_file
            alg.efficiencyCorrectionTool.IdKey = self.identificationWP.replace("LH","")
            alg.efficiencyCorrectionTool.IsoKey = self.isolationWP
            if self.correlationModelIso not in correlationModels:
                raise ValueError('Invalid correlation model for isolation efficiency, '
                                 f'has to be one of: {", ".join(correlationModels)}')
            if self.correlationModelIso != 'TOTAL':
                log.warning("Only TOTAL correlation model is currently supported "
                      "for isolation efficiency correction in Run 3.")
            alg.efficiencyCorrectionTool.CorrelationModel = "TOTAL"
            if config.dataType() is DataType.FastSim:
                alg.efficiencyCorrectionTool.ForceDataType = (
                    PATCore.ParticleDataType.Full if self.forceFullSimConfig
                    else PATCore.ParticleDataType.Fast)
            elif config.dataType() is DataType.FullSim:
                alg.efficiencyCorrectionTool.ForceDataType = \
                    PATCore.ParticleDataType.Full
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'el_isol_bad_eff' + selectionPostfix
            alg.electrons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            if self.saveDetailedSF:
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration,
                                     'isol_effSF' + postfix)
            sfList += [alg.scaleFactorDecoration]

        if (self.chargeIDSelectionRun2 and config.geometry() < LHCPeriod.Run3 and 
            config.dataType() is not DataType.Data and not self.noEffSF):
            alg = config.createAlgorithm( 'CP::ElectronEfficiencyCorrectionAlg',
                                          'ElectronEfficiencyCorrectionAlgEcids' )
            config.addPrivateTool( 'efficiencyCorrectionTool',
                                   'AsgElectronEfficiencyCorrectionTool' )
            alg.scaleFactorDecoration = 'el_ecids_effSF' + selectionPostfix + '_%SYS%'
            if self.isolationWP != 'Tight_VarRad':
                raise ValueError('ECIDS SFs are supported only for Tight_VarRad isolation.')
            if self.identificationWP == 'LooseBLayerLH':
                ecids_lh = 'loose'
            elif self.identificationWP == 'MediumLH':
                ecids_lh = 'medium'
            elif self.identificationWP == 'TightLH':
                ecids_lh = 'tight'
            else:  
                raise ValueError('ECIDS SFs are supported only for ID LooseBLayerLH, MediumLH, or TightLH')

            alg.efficiencyCorrectionTool.CorrelationModel = "TOTAL"
            alg.efficiencyCorrectionTool.CorrectionFileNameList = \
                [f'ElectronEfficiencyCorrection/2015_2025/rel22.2/2025_Run2Rel22_Recommendation_v2/ecids/efficiencySF.ChargeID.{ecids_lh}_ECIDS_Tight_VarRad.root']
            if config.dataType() is DataType.FastSim:
                alg.efficiencyCorrectionTool.ForceDataType = (
                    PATCore.ParticleDataType.Full if self.forceFullSimConfig
                    else PATCore.ParticleDataType.Fast)
            elif config.dataType() is DataType.FullSim:
                alg.efficiencyCorrectionTool.ForceDataType = \
                    PATCore.ParticleDataType.Full
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'el_ecids_bad_eff' + selectionPostfix
            alg.electrons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            if self.saveDetailedSF:
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration,
                                     'ecids_effSF' + postfix)
            sfList += [alg.scaleFactorDecoration]
        
        if self.addChargeMisIDSF and config.dataType() is not DataType.Data and not self.noEffSF:
            if config.geometry() >= LHCPeriod.Run3:
                raise ValueError('Run 3 does not yet have charge mis-ID correction, '
                                 'please disable it by setting `noEffSF` to False.')

            alg = config.createAlgorithm( 'CP::ElectronEfficiencyCorrectionAlg',
                                          'ElectronEfficiencyCorrectionAlgMisid' )
            config.addPrivateTool( 'efficiencyCorrectionTool',
                                   'CP::ElectronChargeEfficiencyCorrectionTool' )
            alg.scaleFactorDecoration = 'el_charge_misid_effSF' + selectionPostfix + '_%SYS%'
            if self.isolationWP != 'Tight_VarRad':
                raise ValueError('Charge mis-ID SFs are supported only for Tight_VarRad isolation.')
            if self.identificationWP == 'LooseBLayerLH':
                misid_lh = 'LooseAndBLayerLLH'
            elif self.identificationWP == 'MediumLH':
                misid_lh = 'MediumLLH'
            elif self.identificationWP == 'TightLH':
                misid_lh = 'TightLLH'
            else:  
                raise ValueError('Charge mis-ID SFs are supported only for ID LooseBLayerLH, MediumLH, or TightLH')
            misid_suffix = '_ECIDSloose' if self.chargeIDSelectionRun2 else ''

            alg.efficiencyCorrectionTool.CorrectionFileName = \
                f'ElectronEfficiencyCorrection/2015_2025/rel22.2/2025_Run2Rel22_Recommendation_v2/charge_misID/chargeEfficiencySF.{misid_lh}_d0z0_TightVarRad{misid_suffix}.root'
            if config.dataType() is DataType.FastSim:
                alg.efficiencyCorrectionTool.ForceDataType = (
                    PATCore.ParticleDataType.Full if self.forceFullSimConfig
                    else PATCore.ParticleDataType.Fast)
            elif config.dataType() is DataType.FullSim:
                alg.efficiencyCorrectionTool.ForceDataType = \
                    PATCore.ParticleDataType.Full
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'el_misid_bad_eff' + selectionPostfix
            alg.electrons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            if self.saveDetailedSF:
                config.addOutputVar (self.containerName, alg.scaleFactorDecoration,
                                     'charge_misid_effSF' + postfix)
            sfList += [alg.scaleFactorDecoration]

        if config.dataType() is not DataType.Data and not self.noEffSF and self.saveCombinedSF:
            alg = config.createAlgorithm( 'CP::AsgObjectScaleFactorAlg',
                                          'ElectronCombinedEfficiencyScaleFactorAlg' )
            alg.particles = config.readName (self.containerName)
            alg.inScaleFactors = sfList
            alg.outScaleFactor = 'effSF' + postfix + '_%SYS%'
            config.addOutputVar (self.containerName, alg.outScaleFactor, 'effSF' + postfix)
        


class ElectronTriggerAnalysisSFBlock (ConfigBlock):

    def __init__ (self) :
        super (ElectronTriggerAnalysisSFBlock, self).__init__ ()

        self.addOption ('triggerChainsPerYear', {}, type=None,
                        info="a dictionary with key (string) the year and value (list of "
                        "strings) the trigger chains. The default is {} (empty dictionary).")
        self.addOption ('electronID', '', type=str,
                        info="the electron ID WP (string) to use.")
        self.addOption ('electronIsol', '', type=str,
                        info="the electron isolation WP (string) to use.")
        self.addOption ('saveEff', False, type=bool,
                        info="define whether we decorate also the trigger scale efficiency "
                        "The default is false.")
        self.addOption ('prefixSF', 'trigEffSF', type=str,
                        info="the decoration prefix for trigger scale factors, "
                        "the default is 'trigEffSF'")
        self.addOption ('prefixEff', 'trigEff', type=str,
                        info="the decoration prefix for MC trigger efficiencies, "
                        "the default is 'trigEff'")
        self.addOption ('includeAllYearsPerRun', False, type=bool,
                        info="if True, all configured years in the LHC run will "
                        "be included in all jobs. The default is False.")
        self.addOption ('removeHLTPrefix', True, type=bool,
                        info="remove the HLT prefix from trigger chain names, "
                        "The default is True.")
        self.addOption ('useToolKeyAsOutput', False, type=bool,
                        info="use tool trigger key as output, "
                        "The default is False.")
        self.addOption ('containerName', '', type=str,
                        info="the input electron container, with a possible selection, in "
                        "the format container or container.selection.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs (self, config) :

        if config.dataType() is not DataType.Data:
            log = logging.getLogger('ElectronTriggerSFConfig')

            if self.includeAllYearsPerRun and not self.useToolKeyAsOutput:
                log.warning('`includeAllYearsPerRun` is set to True, but `useToolKeyAsOutput` is set to False. '
                            'This will cause multiple branches to be written out with the same content.')

            # Dictionary from TrigGlobalEfficiencyCorrection/Triggers.cfg
            # Key is trigger chain (w/o HLT prefix)
            # Value is empty for single leg trigger or list of legs
            triggerDict = TriggerDict()

            # currently recommended versions
            version_Run2 = "2015_2018/rel21.2/Precision_Summer2020_v1"
            map_Run2 = f"{version_Run2}/map4.txt"
            version_Run3 = "2015_2025/rel22.2/2025_Run3_Consolidated_Recommendation_v4"
            map_Run3 = "2015_2025/rel22.2/2025_Run3_Consolidated_Recommendation_v4/map2.txt"

            version = version_Run2 if config.geometry() is LHCPeriod.Run2 else version_Run3
            # Dictionary from TrigGlobalEfficiencyCorrection/MapKeys.cfg
            # Key is year_leg
            # Value is list of configs available, first one will be used
            mapKeysDict = MapKeysDict(version)

            # helper function for leg filtering, very hardcoded but allows autoconfiguration
            def filterConfFromMap(conf, electronMapKeys):
                if not conf:
                    raise ValueError("No configuration found for trigger chain.")
                if len(conf) == 1:
                    return conf[0]

                for c in conf:
                    if c in electronMapKeys:
                        return c

                return conf[0]

            if self.includeAllYearsPerRun:
                years = [int(year) for year in self.triggerChainsPerYear.keys()]
            else:
                from TriggerAnalysisAlgorithms.TriggerAnalysisSFConfig import (
                    get_input_years)
                years = get_input_years(config)

            # prepare keys
            import ROOT
            triggerChainsPerYear_Run2 = {}
            triggerChainsPerYear_Run3 = {}
            for year, chains in self.triggerChainsPerYear.items():
                if not chains:
                    log.warning("No trigger chains configured for year %s. "
                                "Assuming this is intended, no Electron trigger SF will be computed.", year)
                    continue

                chains_split = [chain.replace("HLT_", "").replace(" || ", "_OR_") for chain in chains]
                if int(year) >= 2022:
                    triggerChainsPerYear_Run3[str(year)] = ' || '.join(chains_split)
                else:
                    triggerChainsPerYear_Run2[str(year)] = ' || '.join(chains_split)
            electronMapKeys_Run2 = ROOT.std.map("string", "string")()
            electronMapKeys_Run3 = ROOT.std.map("string", "string")()

            sc_Run2 = ROOT.TrigGlobalEfficiencyCorrectionTool.suggestElectronMapKeys(triggerChainsPerYear_Run2, version_Run2, electronMapKeys_Run2)
            sc_Run3 = ROOT.TrigGlobalEfficiencyCorrectionTool.suggestElectronMapKeys(triggerChainsPerYear_Run3, version_Run3, electronMapKeys_Run3)
            if sc_Run2.code() != 2 or sc_Run3.code() != 2:
                raise RuntimeError("Failed to suggest electron map keys")
            electronMapKeys = dict(electronMapKeys_Run2) | dict(electronMapKeys_Run3)

            # collect configurations
            from TriggerAnalysisAlgorithms.TriggerAnalysisConfig import is_year_in_current_period
            triggerConfigs = {}
            for year in years:
                if not is_year_in_current_period(config, year):
                    continue

                triggerChains = self.triggerChainsPerYear.get(int(year), self.triggerChainsPerYear.get(str(year), []))
                for chain in triggerChains:
                    chain = chain.replace(" || ", "_OR_")
                    chain_noHLT = chain.replace("HLT_", "")
                    chain_out = chain_noHLT if self.removeHLTPrefix else chain
                    legs = triggerDict[chain_noHLT]
                    if not legs:
                        if chain_noHLT[0] == 'e' and chain_noHLT[1].isdigit:
                            chain_key = f"{year}_{chain_noHLT}"
                            chain_conf = mapKeysDict[chain_key][0]
                            triggerConfigs[chain_conf if self.useToolKeyAsOutput else chain_out] = chain_conf
                    else:
                        for leg in legs:
                            if leg[0] == 'e' and leg[1].isdigit:
                                leg_out = leg if self.removeHLTPrefix else f"HLT_{leg}"
                                leg_key = f"{year}_{leg}"
                                leg_conf = filterConfFromMap(mapKeysDict[leg_key], electronMapKeys)
                                triggerConfigs[leg_conf if self.useToolKeyAsOutput else leg_out] = leg_conf

            decorations = [self.prefixSF]
            if self.saveEff:
                decorations += [self.prefixEff]

            for label, conf in triggerConfigs.items():
                for deco in decorations:
                    alg = config.createAlgorithm('CP::ElectronEfficiencyCorrectionAlg',
                                                 'EleTrigEfficiencyCorrectionsAlg' + deco +
                                                 '_' + label)
                    config.addPrivateTool( 'efficiencyCorrectionTool',
                                           'AsgElectronEfficiencyCorrectionTool' )

                    # Reproduce config from TrigGlobalEfficiencyAlg
                    alg.efficiencyCorrectionTool.MapFilePath = "ElectronEfficiencyCorrection/" + (map_Run3 if config.geometry() is LHCPeriod.Run3 else map_Run2)
                    alg.efficiencyCorrectionTool.IdKey = self.electronID.replace("LH","")
                    alg.efficiencyCorrectionTool.IsoKey = self.electronIsol
                    alg.efficiencyCorrectionTool.TriggerKey = (
                        ("Eff_" if deco == self.prefixEff else "") + conf)
                    alg.efficiencyCorrectionTool.CorrelationModel = "TOTAL"
                    alg.efficiencyCorrectionTool.ForceDataType = \
                        PATCore.ParticleDataType.Full

                    alg.scaleFactorDecoration = f"el_{deco}_{label}_%SYS%"

                    alg.outOfValidity = 2 #silent
                    alg.outOfValidityDeco = f"bad_eff_ele{deco}_{label}"
                    alg.electrons = config.readName (self.containerName)
                    alg.preselection = config.getPreselection (self.containerName, "")
                    config.addOutputVar (self.containerName, alg.scaleFactorDecoration, f"{deco}_{label}")


class ElectronLRTMergedConfig (ConfigBlock) :
    def __init__ (self) :  
        super (ElectronLRTMergedConfig, self).__init__ ()
        self.addOption (
            'inputElectrons', 'Electrons', type=str,
            noneAction='error',
            info="the name of the input electron container."
        )
        self.addOption (
            'inputLRTElectrons', 'LRTElectrons', type=str,
            noneAction='error',
            info="the name of the input LRT electron container."
        )
        self.addOption (
            'containerName', 'Electrons_LRTMerged', type=str,
            noneAction='error',
            info="the name of the output container after LRT merging."
        )

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs (self, config) :

        if config.isPhyslite() :
            raise(RuntimeError("Electron LRT merging is not available in Physlite mode"))

        alg = config.createAlgorithm( "CP::ElectronLRTMergingAlg", "ElectronLRTMergingAlg" )
        alg.PromptElectronLocation = self.inputElectrons
        alg.LRTElectronLocation = self.inputLRTElectrons
        alg.OutputCollectionName = self.containerName
        alg.CreateViewCollection = False
