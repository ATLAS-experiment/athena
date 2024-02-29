# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AthenaConfiguration.Enums import LHCPeriod
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType

# E/gamma import(s).
from xAODEgamma.xAODEgammaParameters import xAOD

import PATCore.ParticleDataType


class ElectronCalibrationConfig (ConfigBlock) :
    """the ConfigBlock for the electron four-momentum correction"""

    def __init__ (self, containerName) :
        super (ElectronCalibrationConfig, self).__init__ (containerName)
        self.containerName = containerName
        self.addOption ('postfix', '', type=str)
        self.addOption ('crackVeto', False, type=bool)
        self.addOption ('ptSelectionOutput', False, type=bool)
        self.addOption ('isolationCorrection', False, type=bool)
        self.addOption ('trackSelection', True, type=bool)
        self.addOption ('recalibratePhyslite', True, type=bool)
        self.addOption ('minPt', 4.5e3, type=float)
        self.addOption ('forceFullSimConfig', False, type=bool)

    def makeAlgs (self, config) :

        if self.forceFullSimConfig:
            print("WARNING! You are running ElectronCalibrationConfig forcing full sim config")
            print("WARNING! This is only intended to be used for testing purposes")

        if config.isPhyslite() :
            config.setSourceName (self.containerName, "AnalysisElectrons")
        else :
            config.setSourceName (self.containerName, "Electrons")

        # Set up a shallow copy to decorate
        if config.wantCopy (self.containerName) :
            alg = config.createAlgorithm( 'CP::AsgShallowCopyAlg', 'ElectronShallowCopyAlg' + self.postfix )
            alg.input = config.readName (self.containerName)
            alg.output = config.copyName (self.containerName)


        # Set up the eta-cut on all electrons prior to everything else
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronEtaCutAlg' + self.postfix )
        alg.selectionDecoration = 'selectEta' + self.postfix + ',as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        alg.selectionTool.maxEta = 2.47
        if self.crackVeto:
            alg.selectionTool.etaGapLow = 1.37
            alg.selectionTool.etaGapHigh = 1.52
        alg.selectionTool.useClusterEta = True
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration)

        # Set up the track selection algorithm:
        if self.trackSelection :
            alg = config.createAlgorithm( 'CP::AsgLeptonTrackSelectionAlg',
                                        'ElectronTrackSelectionAlg' + self.postfix )
            alg.selectionDecoration = 'trackSelection' + self.postfix + ',as_bits'
            alg.maxD0Significance = 5
            alg.maxDeltaZ0SinTheta = 0.5
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', alg.selectionDecoration)

        # Select electrons only with good object quality.
        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronObjectQualityAlg' + self.postfix )
        alg.selectionDecoration = 'goodOQ' + self.postfix + ',as_bits'
        config.addPrivateTool( 'selectionTool', 'CP::EgammaIsGoodOQSelectionTool' )
        alg.selectionTool.Mask = xAOD.EgammaParameters.BADCLUSELECTRON
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration)

        # Set up the calibration and smearing algorithm:
        alg = config.createAlgorithm( 'CP::EgammaCalibrationAndSmearingAlg',
                                      'ElectronCalibrationAndSmearingAlg' + self.postfix )
        config.addPrivateTool( 'calibrationAndSmearingTool',
                               'CP::EgammaCalibrationAndSmearingTool' )
        alg.calibrationAndSmearingTool.ESModel = 'es2022_R22_PRE'
        alg.calibrationAndSmearingTool.decorrelationModel = '1NP_v1'
        alg.calibrationAndSmearingTool.useFastSim = (
            0 if self.forceFullSimConfig
            else int( config.dataType() is DataType.FastSim ))
        alg.egammas = config.readName (self.containerName)
        alg.egammasOut = config.copyName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        if config.isPhyslite() and not self.recalibratePhyslite :
            alg.skipNominal = True

        if self.minPt > 0 :
            # Set up the the pt selection
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronPtCutAlg' + self.postfix )
            alg.selectionDecoration = 'selectPt' + self.postfix + ',as_bits'
            config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
            alg.selectionTool.minPt = self.minPt
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            config.addSelection (self.containerName, '', alg.selectionDecoration,
                                preselection=self.ptSelectionOutput)

        # Set up the isolation correction algorithm:
        if self.isolationCorrection:
            alg = config.createAlgorithm( 'CP::EgammaIsolationCorrectionAlg',
                                          'ElectronIsolationCorrectionAlg' + self.postfix )
            config.addPrivateTool( 'isolationCorrectionTool',
                                   'CP::IsolationCorrectionTool' )
            alg.isolationCorrectionTool.IsMC = config.dataType() is not DataType.Data
            alg.isolationCorrectionTool.AFII_corr = (
                0 if self.forceFullSimConfig
                else config.dataType() is DataType.FastSim)
            alg.egammas = config.readName (self.containerName)
            alg.egammasOut = config.copyName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')

        config.addOutputVar (self.containerName, 'pt', 'pt')
        config.addOutputVar (self.containerName, 'eta', 'eta', noSys=True)
        config.addOutputVar (self.containerName, 'phi', 'phi', noSys=True)
        config.addOutputVar (self.containerName, 'charge', 'charge', noSys=True)



class ElectronWorkingPointConfig (ConfigBlock) :
    """the ConfigBlock for the electron working point

    This may at some point be split into multiple blocks (29 Aug 22)."""

    def __init__ (self, containerName, selectionName) :
        super (ElectronWorkingPointConfig, self).__init__ (containerName + '.' + selectionName)
        self.containerName = containerName
        self.selectionName = selectionName
        self.addOption ('postfix', None, type=str)
        self.addOption ('likelihoodWP', None, type=str)
        self.addOption ('isolationWP', None, type=str)
        self.addOption ('recomputeLikelihood', False, type=bool)
        self.addOption ('chargeIDSelection', False, type=bool)
        self.addOption ('doFSRSelection', False, type=bool)
        self.addOption ('noEffSF', False, type=bool, info='disable all scale factors')
        self.addOption ('forceFullSimConfig', False, type=bool)


    def makeAlgs (self, config) :

        if self.forceFullSimConfig:
            print("WARNING! You are running ElectronWorkingPointConfig forcing full sim config")
            print("WARNING! This is only intended to be used for testing purposes")

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

        if 'LH' in self.likelihoodWP:
            # Set up the likelihood ID selection algorithm
            # It is safe to do this before calibration, as the cluster E is used
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronLikelihoodAlg' + postfix )
            alg.selectionDecoration = 'selectLikelihood' + selectionPostfix + ',as_bits'
            if self.recomputeLikelihood:
                # Rerun the likelihood ID
                config.addPrivateTool( 'selectionTool', 'AsgElectronLikelihoodTool' )
                alg.selectionTool.primaryVertexContainer = 'PrimaryVertices'
                # Here we have to match the naming convention of EGSelectorConfigurationMapping.h
                if config.geometry() >= LHCPeriod.Run3:
                    alg.selectionTool.WorkingPoint = self.likelihoodWP + 'Electron'
                elif config.geometry() == LHCPeriod.Run2:
                    alg.selectionTool.WorkingPoint = self.likelihoodWP + 'Electron_Run2'
            else:
                # Select from Derivation Framework flags
                config.addPrivateTool( 'selectionTool', 'CP::AsgFlagSelectionTool' )
                dfFlag = "DFCommonElectronsLH" + self.likelihoodWP.split('LH')[0]
                dfFlag = dfFlag.replace("BLayer","BL")
                alg.selectionTool.selectionFlags = [dfFlag]
        elif 'SiHit' in self.likelihoodWP:
            # Only want SiHit electrons, so veto loose LH electrons
            algVeto = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronLikelihoodAlgVeto' + postfix + 'Veto')
            algVeto.selectionDecoration = 'selectLikelihoodVeto' + postfix + ',as_bits'  
            config.addPrivateTool( 'selectionTool', 'CP::AsgFlagSelectionTool' )
            algVeto.selectionTool.selectionFlags = ["DFCommonElectronsLHLoose"]
            algVeto.selectionTool.invertFlags    = [True]
            algVeto.particles = config.readName (self.containerName)
            algVeto.preselection = config.getPreselection (self.containerName, self.selectionName)
            # add in as preselection a veto
            config.addSelection (self.containerName, self.selectionName, algVeto.selectionDecoration)

            # Select SiHit electrons using IsEM bits
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronLikelihoodAlg' + postfix )
            alg.selectionDecoration = 'selectSiHit' + selectionPostfix + ',as_bits'
            # Select from Derivation Framework IsEM bits
            config.addPrivateTool( 'selectionTool', 'CP::AsgMaskSelectionTool' )
            dfVar = "DFCommonElectronsLHLooseBLIsEMValue"
            alg.selectionTool.selectionVars = [dfVar]
            mask = int( 0 | 0x1 << 1 | 0x1 << 2)
            alg.selectionTool.selectionMasks = [mask]

            # Set up the ElectronSiHitDecAlg algorithm to decorate SiHit electrons with a minimal amount of information:
            algDec = config.createAlgorithm( 'CP::ElectronSiHitDecAlg', 'ElectronSiHitDecAlg' + postfix )
            selDec = 'siHitEvtHasLeptonPair' + selectionPostfix + ',as_bits'
            algDec.selectionName     = selDec.split(",")[0]
            algDec.ElectronContainer = config.readName (self.containerName)
            # Set flag to only collect SiHit electrons for events with an electron or muon pair to minimize size increase from SiHit electrons
            algDec.RequireTwoLeptons = True
            config.addSelection (self.containerName, self.selectionName, selDec)
        elif 'DNN' in self.likelihoodWP:
            # Set up the DNN ID selection algorithm
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'ElectronDNNAlg' + postfix )
            alg.selectionDecoration = 'selectDNN' + selectionPostfix + ',as_bits'
            if self.recomputeLikelihood:
                # Rerun the DNN ID
                config.addPrivateTool( 'selectionTool', 'AsgElectronSelectorTool' )
                # Here we have to match the naming convention of EGSelectorConfigurationMapping.h
                if config.geometry() == LHCPeriod.Run3:
                    raise ValueError ( "DNN working points are not available for Run 3 yet.")
                else:
                    alg.selectionTool.WorkingPoint = self.likelihoodWP + 'Electron'
            else:
                # Select from Derivation Framework flags
                raise ValueError ( "DNN working points are not available in derivations yet.")

        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, self.selectionName)
        config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration)

        # Set up the FSR selection
        if self.doFSRSelection :
            # save the flag set for the WP
            wpFlag = alg.selectionDecoration.split(",")[0]
            alg = config.createAlgorithm( 'CP::EgammaFSRForMuonsCollectorAlg', 'EgammaFSRForMuonsCollectorAlg' + postfix )
            alg.selectionDecoration = wpFlag
            alg.ElectronOrPhotonContKey = config.readName (self.containerName)
            # For SiHit electrons, set flag to remove FSR electrons. 
            # For standard electrons, FSR electrons need to be added as they may be missed by the standard selection. 
            # For SiHit electrons FSR electrons are generally always selected, so they should be removed since they will be in the standard electron container.
            if 'SiHit' in self.likelihoodWP:
                alg.vetoFSR = True

        # Set up the isolation selection algorithm:
        if self.isolationWP != 'NonIso' :
            alg = config.createAlgorithm( 'CP::EgammaIsolationSelectionAlg',
                                          'ElectronIsolationSelectionAlg' + postfix )
            alg.selectionDecoration = 'isolated' + selectionPostfix + ',as_bits'
            config.addPrivateTool( 'selectionTool', 'CP::IsolationSelectionTool' )
            alg.selectionTool.ElectronWP = self.isolationWP
            alg.egammas = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration)

        # Select electrons only if they don't appear to have flipped their charge.
        if self.chargeIDSelection:
            alg = config.createAlgorithm( 'CP::AsgSelectionAlg',
                                          'ElectronChargeIDSelectionAlg' + postfix )
            alg.selectionDecoration = 'chargeID' + selectionPostfix + ',as_bits'
            config.addPrivateTool( 'selectionTool',
                                   'AsgElectronChargeIDSelectorTool' )
            alg.selectionTool.TrainingFile = \
                'ElectronPhotonSelectorTools/ChargeID/ECIDS_20180731rel21Summer2018.root'
            alg.selectionTool.WorkingPoint = 'Loose'
            alg.selectionTool.CutOnBDT = -0.337671 # Loose 97%
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addSelection (self.containerName, self.selectionName, alg.selectionDecoration)

        # Set up the RECO electron efficiency correction algorithm:
        if config.dataType() is not DataType.Data and not self.noEffSF:
            alg = config.createAlgorithm( 'CP::ElectronEfficiencyCorrectionAlg',
                                          'ElectronEfficiencyCorrectionAlgReco' + postfix )
            config.addPrivateTool( 'efficiencyCorrectionTool',
                                   'AsgElectronEfficiencyCorrectionTool' )
            alg.scaleFactorDecoration = 'el_reco_effSF' + selectionPostfix + '_%SYS%'
            alg.efficiencyCorrectionTool.RecoKey = "Reconstruction"
            alg.efficiencyCorrectionTool.CorrelationModel = "TOTAL"
            if config.dataType() is DataType.FastSim:
                alg.efficiencyCorrectionTool.ForceDataType = (
                    PATCore.ParticleDataType.Full if self.forceFullSimConfig
                    else PATCore.ParticleDataType.Fast)
            elif config.dataType() is DataType.FullSim:
                alg.efficiencyCorrectionTool.ForceDataType = \
                    PATCore.ParticleDataType.Full
            if config.geometry() == LHCPeriod.Run2:
                alg.efficiencyCorrectionTool.MapFilePath = "ElectronEfficiencyCorrection/2015_2018/rel21.2/Precision_Summer2020_v1/map4.txt"
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'el_reco_bad_eff' + selectionPostfix
            alg.electrons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'reco_effSF' + postfix)

        # Set up the ID electron efficiency correction algorithm:
        if config.dataType() is not DataType.Data and not self.noEffSF:
            alg = config.createAlgorithm( 'CP::ElectronEfficiencyCorrectionAlg',
                                          'ElectronEfficiencyCorrectionAlgID' + postfix )
            config.addPrivateTool( 'efficiencyCorrectionTool',
                                   'AsgElectronEfficiencyCorrectionTool' )
            alg.scaleFactorDecoration = 'el_id_effSF' + selectionPostfix + '_%SYS%'
            alg.efficiencyCorrectionTool.IdKey = self.likelihoodWP.replace("LH","")
            alg.efficiencyCorrectionTool.CorrelationModel = "TOTAL"
            if config.dataType() is DataType.FastSim:
                alg.efficiencyCorrectionTool.ForceDataType = (
                    PATCore.ParticleDataType.Full if self.forceFullSimConfig
                    else PATCore.ParticleDataType.Fast)
            elif config.dataType() is DataType.FullSim:
                alg.efficiencyCorrectionTool.ForceDataType = \
                    PATCore.ParticleDataType.Full
            if config.geometry() == LHCPeriod.Run2:
                alg.efficiencyCorrectionTool.MapFilePath = "ElectronEfficiencyCorrection/2015_2018/rel21.2/Precision_Summer2020_v1/map4.txt"
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'el_id_bad_eff' + selectionPostfix
            alg.electrons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'id_effSF' + postfix)

        # Set up the ISO electron efficiency correction algorithm:
        if config.dataType() is not DataType.Data and self.isolationWP != 'NonIso' and not self.noEffSF:
            alg = config.createAlgorithm( 'CP::ElectronEfficiencyCorrectionAlg',
                                          'ElectronEfficiencyCorrectionAlgIsol' + postfix )
            config.addPrivateTool( 'efficiencyCorrectionTool',
                                   'AsgElectronEfficiencyCorrectionTool' )
            alg.scaleFactorDecoration = 'el_isol_effSF' + selectionPostfix + '_%SYS%'
            alg.efficiencyCorrectionTool.IdKey = self.likelihoodWP.replace("LH","")
            alg.efficiencyCorrectionTool.IsoKey = self.isolationWP
            alg.efficiencyCorrectionTool.CorrelationModel = "TOTAL"
            if config.dataType() is DataType.FastSim:
                alg.efficiencyCorrectionTool.ForceDataType = (
                    PATCore.ParticleDataType.Full if self.forceFullSimConfig
                    else PATCore.ParticleDataType.Fast)
            elif config.dataType() is DataType.FullSim:
                alg.efficiencyCorrectionTool.ForceDataType = \
                    PATCore.ParticleDataType.Full
            if config.geometry() == LHCPeriod.Run2:
                alg.efficiencyCorrectionTool.MapFilePath = "ElectronEfficiencyCorrection/2015_2018/rel21.2/Precision_Summer2020_v1/map4.txt"
            alg.outOfValidity = 2 #silent
            alg.outOfValidityDeco = 'el_isol_bad_eff' + selectionPostfix
            alg.electrons = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, self.selectionName)
            config.addOutputVar (self.containerName, alg.scaleFactorDecoration, 'isol_effSF' + postfix)

        # TO-DO: add trigger SFs, for which we need ID key + ISO key + Trigger key !

        if self.chargeIDSelection:
            # ECIDS is currently not supported in R22.
            # SFs might become available or it will be part of the DNN ID.
            pass


def makeElectronCalibrationConfig( seq, containerName, postfix = None,
                                   crackVeto = None,
                                   ptSelectionOutput = None,
                                   isolationCorrection = None,
                                   forceFullSimConfig = None):
    """Create electron calibration configuration blocks

    This makes all the algorithms that need to be run first befor
    all working point specific algorithms and that can be shared
    between the working points.

    Keyword arguments:
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      isolationCorrection -- Whether or not to perform isolation correction
      ptSelectionOutput -- Whether or not to apply pt selection when creating
                           output containers.
      forceFullSimConfig -- imposes full-sim config for FastSim for testing
    """

    config = ElectronCalibrationConfig (containerName)
    config.setOptionValue ('crackVeto', crackVeto, noneAction='ignore')
    config.setOptionValue ('ptSelectionOutput', ptSelectionOutput, noneAction='ignore')
    config.setOptionValue ('isolationCorrection', isolationCorrection, noneAction='ignore')
    config.setOptionValue ('forceFullSimConfig', forceFullSimConfig, noneAction='ignore')
    seq.append (config)





def makeElectronWorkingPointConfig( seq, containerName, workingPoint,
                                    selectionName,
                                    recomputeLikelihood = None,
                                    chargeIDSelection = None,
                                    noEffSF = None,
                                    forceFullSimConfig = None):
    """Create electron analysis configuration blocks

    Keyword arguments:
      workingPoint -- The working point to use
      selectionName -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      recomputeLikelihood -- Whether to rerun the LH. If not, use derivation flags
      chargeIDSelection -- Whether or not to perform charge ID/flip selection
      noEffSF -- Disables the calculation of efficiencies and scale factors
      forceFullSimConfig -- imposes full-sim config for FastSim for testing
    """


    config = ElectronWorkingPointConfig (containerName, selectionName)
    if workingPoint is not None :
        splitWP = workingPoint.split ('.')
        if len (splitWP) != 2 :
            raise ValueError ('working point should be of format "likelihood.isolation", not ' + workingPoint)
        config.setOptionValue ('likelihoodWP', splitWP[0])
        config.setOptionValue ('isolationWP', splitWP[1])
    config.setOptionValue ('recomputeLikelihood', recomputeLikelihood, noneAction='ignore')
    config.setOptionValue ('chargeIDSelection', chargeIDSelection, noneAction='ignore')
    config.setOptionValue ('noEffSF', noEffSF, noneAction='ignore')
    config.setOptionValue ('forceFullSimConfig', forceFullSimConfig, noneAction='ignore')
    seq.append (config)
