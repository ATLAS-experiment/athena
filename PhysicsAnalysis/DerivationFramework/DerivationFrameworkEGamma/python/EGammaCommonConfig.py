# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ********************************************************************
# EGammaCommonConfig.py
# Configures  all tools needed for e-gamma object selection and sets
# up the kernel algorithms so the results can be accessed/written to
# the DAODs.
# Component accumulator version.
# ********************************************************************

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod


def EGammaCommonCfg(flags):
    """Main config method for e-gamma decorations"""

    acc = ComponentAccumulator()

    includeFwdElectrons = "ForwardElectrons" in flags.Input.Collections
    
    # ====================================================================
    # PHOTON ETA (=ETA2), ET (=E/COSH(ETA2))
    # ====================================================================
    from DerivationFrameworkEGamma.EGammaToolsConfig import PhotonsDirectionToolCfg

    DFCommonPhotonsDirection = acc.addPublicTool(acc.popToolsAndMerge(
        PhotonsDirectionToolCfg(
            flags,
            name="DFCommonPhotonsDirection",
            EtaSGEntry="DFCommonPhotons_eta",
            PhiSGEntry="DFCommonPhotons_phi",
            EtSGEntry="DFCommonPhotons_et",
        )
    ))

    isMC = flags.Input.isMC
    isFullSim = False
    if isMC:
        isFullSim = flags.Sim.ISF.Simulator.isFullSim()
    isRun2orRun3 = flags.GeoModel.Run in (LHCPeriod.Run2, LHCPeriod.Run3)
    
    print("EGammaCommon: isMC = ", isMC)
    if isMC:
        print("EGammaCommon: isFullSim = ", isFullSim)

    # ====================================================================
    # ELECTRON LH SELECTORS
    # see Reconstruction/egamma/egammaTools/python/EMPIDBuilderBase.py
    # on how to configure the selectors
    # ====================================================================
    from ROOT import LikeEnum

    from ElectronPhotonSelectorTools.AsgElectronLikelihoodToolsConfig import (
        AsgElectronLikelihoodToolCfg,
    )
    from ElectronPhotonSelectorTools.ElectronLikelihoodToolMapping import electronLHmenu

    lhMenu = electronLHmenu.offlineMC21
    if flags.GeoModel.Run is LHCPeriod.Run2:
        lhMenu = electronLHmenu.offlineMC20

    # Very Loose
    ElectronLHSelectorVeryLoose = acc.popToolsAndMerge(
        AsgElectronLikelihoodToolCfg(
            flags,
            name="ElectronLHSelectorVeryLoose",
            quality=LikeEnum.VeryLoose,
            menu=lhMenu,
        )
    )
    ElectronLHSelectorVeryLoose.primaryVertexContainer = "PrimaryVertices"
    acc.addPublicTool(ElectronLHSelectorVeryLoose)

    # Loose
    ElectronLHSelectorLoose = acc.popToolsAndMerge(
        AsgElectronLikelihoodToolCfg(
            flags,
            name="ElectronLHSelectorLoose",
            quality=LikeEnum.Loose,
            menu=lhMenu,
        )
    )
    ElectronLHSelectorLoose.primaryVertexContainer = "PrimaryVertices"
    acc.addPublicTool(ElectronLHSelectorLoose)

    # LooseBL
    ElectronLHSelectorLooseBL = acc.popToolsAndMerge(
        AsgElectronLikelihoodToolCfg(
            flags,
            name="ElectronLHSelectorLooseBL",
            quality=LikeEnum.LooseBL,
            menu=lhMenu,
        )
    )
    ElectronLHSelectorLooseBL.primaryVertexContainer = "PrimaryVertices"
    acc.addPublicTool(ElectronLHSelectorLooseBL)

    # Medium
    ElectronLHSelectorMedium = acc.popToolsAndMerge(
        AsgElectronLikelihoodToolCfg(
            flags,
            name="ElectronLHSelectorMedium",
            quality=LikeEnum.Medium,
            menu=lhMenu,
        )
    )
    ElectronLHSelectorMedium.primaryVertexContainer = "PrimaryVertices"
    acc.addPublicTool(ElectronLHSelectorMedium)

    # Tight
    ElectronLHSelectorTight = acc.popToolsAndMerge(
        AsgElectronLikelihoodToolCfg(
            flags,
            name="ElectronLHSelectorTight",
            quality=LikeEnum.Tight,
            menu=lhMenu,
        )
    )
    ElectronLHSelectorTight.primaryVertexContainer = "PrimaryVertices"
    acc.addPublicTool(ElectronLHSelectorTight)

    # ====================================================================
    # ELECTRON DNN SELECTORS
    # ====================================================================
    from ElectronPhotonSelectorTools.AsgElectronSelectorToolConfig import (
        AsgElectronSelectorToolCfg,
    )

    # Loose
    ElectronDNNSelectorLoose = acc.popToolsAndMerge(
        AsgElectronSelectorToolCfg(
            flags,
            name="ElectronDNNSelectorLoose",
            WorkingPoint="LooseDNNElectron",
        )
    )
    acc.addPublicTool(ElectronDNNSelectorLoose)

    # Medium
    ElectronDNNSelectorMedium = acc.popToolsAndMerge(
        AsgElectronSelectorToolCfg(
            flags,
            name="ElectronDNNSelectorMedium",
            WorkingPoint="MediumDNNElectron",
        )
    )
    acc.addPublicTool(ElectronDNNSelectorMedium)

    # Tight
    ElectronDNNSelectorTight = acc.popToolsAndMerge(
        AsgElectronSelectorToolCfg(
            flags,
            name="ElectronDNNSelectorTight",
            WorkingPoint="TightDNNElectron",
        )
    )
    acc.addPublicTool(ElectronDNNSelectorTight)

    # ====================================================================
    # ELECTRON DNN SELECTORS WITHOUT CF REJECTION
    # ====================================================================
    # Very-Loose 97%
    ElectronDNNSelectorVeryLooseNoCF97 = acc.popToolsAndMerge(
        AsgElectronSelectorToolCfg(
            flags,
            name="ElectronDNNSelectorVeryLooseNoCF97",
            WorkingPoint="VeryLooseNoCF97DNNElectron",
        )
    )
    acc.addPublicTool(ElectronDNNSelectorVeryLooseNoCF97)

    # Loose
    ElectronDNNSelectorLooseNoCF = acc.popToolsAndMerge(
        AsgElectronSelectorToolCfg(
            flags,
            name="ElectronDNNSelectorLooseNoCF",
            WorkingPoint="LooseNoCFDNNElectron",
        )
    )
    acc.addPublicTool(ElectronDNNSelectorLooseNoCF)

    # Medium
    ElectronDNNSelectorMediumNoCF = acc.popToolsAndMerge(
        AsgElectronSelectorToolCfg(
            flags,
            name="ElectronDNNSelectorMediumNoCF",
            WorkingPoint="MediumNoCFDNNElectron",
        )
    )
    acc.addPublicTool(ElectronDNNSelectorMediumNoCF)

    # Tight
    ElectronDNNSelectorTightNoCF = acc.popToolsAndMerge(
        AsgElectronSelectorToolCfg(
            flags,
            name="ElectronDNNSelectorTightNoCF",
            WorkingPoint="TightNoCFDNNElectron",
        )
    )
    acc.addPublicTool(ElectronDNNSelectorTightNoCF)

    # ====================================================================
    # ELECTRON CHARGE SELECTION
    # ====================================================================
    if flags.Derivation.Egamma.addECIDS:
        from ElectronPhotonSelectorTools.AsgElectronChargeIDSelectorToolConfig import (
            AsgElectronChargeIDSelectorToolCfg,
        )

        ElectronChargeIDSelector = acc.popToolsAndMerge(
            AsgElectronChargeIDSelectorToolCfg(
                flags, name="ElectronChargeIDSelectorLoose"
            )
        )
        ElectronChargeIDSelector.primaryVertexContainer = "PrimaryVertices"
        ElectronChargeIDSelector.TrainingFile = (
            "ElectronPhotonSelectorTools/ChargeID/ECIDS_20180731rel21Summer2018.root"
        )
        acc.addPublicTool(ElectronChargeIDSelector)

    # ====================================================================
    # FWD ELECTRON LH SELECTORS
    # ====================================================================
    if includeFwdElectrons:

        from IsolationAlgs.IsolationSteeringDerivConfig import FwdElectronIsolationSteeringDerivCfg        
        acc.merge(FwdElectronIsolationSteeringDerivCfg(flags))
        
        from ElectronPhotonSelectorTools.AsgForwardElectronLikelihoodToolConfig import (
            AsgForwardElectronLikelihoodToolCfg,
        )
        
        ForwardElectronLHSelectorLoose = acc.popToolsAndMerge(
            AsgForwardElectronLikelihoodToolCfg(
                flags,
                name="ForwardElectronLHSelectorLoose",
                WorkingPoint="LooseLHForwardElectron",
            )
        )
        acc.addPublicTool(ForwardElectronLHSelectorLoose)

        ForwardElectronLHSelectorMedium = acc.popToolsAndMerge(
            AsgForwardElectronLikelihoodToolCfg(
                flags,
                name="ForwardElectronLHSelectorMedium",
                WorkingPoint="MediumLHForwardElectron",
            )
        )
        acc.addPublicTool(ForwardElectronLHSelectorMedium)

        ForwardElectronLHSelectorTight = acc.popToolsAndMerge(
            AsgForwardElectronLikelihoodToolCfg(
                flags,
                name="ForwardElectronLHSelectorTight",
                WorkingPoint="TightLHForwardElectron",
            )
        )
        acc.addPublicTool(ForwardElectronLHSelectorTight)

    # ====================================================================
    # PHOTON SELECTION (loose and tight cut-based)
    # ====================================================================
    from ROOT import egammaPID

    from ElectronPhotonSelectorTools.AsgPhotonIsEMSelectorsConfig import (
        AsgPhotonIsEMSelectorCfg,
    )
    from ElectronPhotonSelectorTools.PhotonIsEMSelectorMapping import photonPIDmenu
    pidMenu = photonPIDmenu.offlineMC21
    if flags.GeoModel.Run is LHCPeriod.Run2:
        pidMenu = photonPIDmenu.offlineMC20

    # Loose
    PhotonIsEMSelectorLoose = acc.popToolsAndMerge(
        AsgPhotonIsEMSelectorCfg(
            flags,
            name="PhotonIsEMSelectorLoose",
            quality=egammaPID.PhotonIDLoose,
            menu=pidMenu
        )
    )
    acc.addPublicTool(PhotonIsEMSelectorLoose)

    # Medium
    PhotonIsEMSelectorMedium = acc.popToolsAndMerge(
        AsgPhotonIsEMSelectorCfg(
            flags,
            name="PhotonIsEMSelectorMedium",
            quality=egammaPID.PhotonIDMedium,
            menu=pidMenu
        )
    )
    acc.addPublicTool(PhotonIsEMSelectorMedium)

    # Tight
    PhotonIsEMSelectorTight = acc.popToolsAndMerge(
        AsgPhotonIsEMSelectorCfg(
            flags,
            name="PhotonIsEMSelectorTight",
            quality=egammaPID.PhotonIDTight,
            menu=pidMenu
        )
    )
    acc.addPublicTool(PhotonIsEMSelectorTight)

    # ====================================================================
    # PHOTON BDT SELECTION
    # ====================================================================
    photonIDBDTWP = "TightBDTPhoton_Run3"
    if flags.GeoModel.Run is LHCPeriod.Run2:
        photonIDBDTWP = "TightBDTPhoton_Run2"
    from ElectronPhotonSelectorTools.AsgPhotonBDTSelectorConfig import (
        PhotonBDTCalculatorCfg,
        AsgPhotonBDTSelectorCfg,
    )
    PhotonBDTCalculator = acc.popToolsAndMerge(
        PhotonBDTCalculatorCfg(
            flags, 
            name="PhotonBDTCalculator",
            useNFs=False,
        )
    )   
    PhotonBDTSelectorTight = acc.popToolsAndMerge(
        AsgPhotonBDTSelectorCfg(
            flags,
            name="PhotonBDTSelectorTight",
            ScoreDecoration="DFCommonPhotonsBDTScore",
            WorkingPoint=photonIDBDTWP,
            useNFs=False,
            SuppressInputDependence=True
        )
    )
    PhotonBDTCalculatorNF = acc.popToolsAndMerge(
        PhotonBDTCalculatorCfg(
            flags, 
            name="PhotonBDTCalculatorNF",
            useNFs=True,
        )
    )   
    PhotonBDTSelectorTightNF = acc.popToolsAndMerge(
        AsgPhotonBDTSelectorCfg(
            flags,
            name="PhotonBDTSelectorTightNF",
            ScoreDecoration="DFCommonPhotonsNFBDTScore",
            WorkingPoint=photonIDBDTWP+"_NFs",
            useNFs=True,
            SuppressInputDependence=True
        )
    )
    # ====================================================================
    # RECTANGULAR CLUSTER TOOLS
    # ====================================================================

    from egammaCaloTools.egammaCaloToolsConfig import CaloFillRectangularClusterCfg

    EGAMCOM_caloFillRect55 = acc.popToolsAndMerge(
        CaloFillRectangularClusterCfg(
            flags,
            name="EGAMCOMCaloFillRectangularCluster55",
            cells_name="AllCalo",
            eta_size=5,
            phi_size=5,
            fill_cluster=True,
        )
    )
    acc.addPublicTool(EGAMCOM_caloFillRect55)

    EGAMCOM_caloFillRect35 = acc.popToolsAndMerge(
        CaloFillRectangularClusterCfg(
            flags,
            name="EGAMCOMCaloFillRectangularCluster35",
            cells_name="AllCalo",
            eta_size=3,
            phi_size=5,
            fill_cluster=True,
        )
    )
    acc.addPublicTool(EGAMCOM_caloFillRect35)

    EGAMCOM_caloFillRect37 = acc.popToolsAndMerge(
        CaloFillRectangularClusterCfg(
            flags,
            name="EGAMCOMCaloFillRectangularCluster37",
            cells_name="AllCalo",
            eta_size=3,
            phi_size=7,
            fill_cluster=True,
        )
    )
    acc.addPublicTool(EGAMCOM_caloFillRect37)

    EGAMCOM_caloFillRect711 = acc.popToolsAndMerge(
        CaloFillRectangularClusterCfg(
            flags,
            name="EGAMCOMCaloFillRectangularCluster711",
            cells_name="AllCalo",
            eta_size=7,
            phi_size=11,
            fill_cluster=True,
        )
    )
    acc.addPublicTool(EGAMCOM_caloFillRect711)

    # ====================================================================
    # AUGMENTATION TOOLS
    # ====================================================================
    from DerivationFrameworkEGamma.EGammaToolsConfig import EGSelectionToolWrapperCfg
    from DerivationFrameworkEGamma.EGammaToolsConfig import (
        EGElectronLikelihoodToolWrapperCfg,
        EGPhotonBDTToolWrapperCfg,
        EGPhotonBDTToolDecoratorCfg
    )

    # Note: LH selectors don't need fudging since the LH is tuned to data

    # decorate electrons with the output of LH very loose
    ElectronPassLHVeryLoose = acc.addPublicTool(acc.popToolsAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            flags,
            name="ElectronPassLHVeryLoose",
            EGammaElectronLikelihoodTool=ElectronLHSelectorVeryLoose,
            CutType="",
            StoreGateEntryName="DFCommonElectronsLHVeryLoose",
            ContainerName="Electrons",
            StoreTResult=False,
        )
    ))

    # decorate electrons with the output of LH loose
    ElectronPassLHLoose = acc.addPublicTool(acc.popToolsAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            flags,
            name="ElectronPassLHLoose",
            EGammaElectronLikelihoodTool=ElectronLHSelectorLoose,
            CutType="",
            StoreGateEntryName="DFCommonElectronsLHLoose",
            ContainerName="Electrons",
            StoreTResult=False,
        )
    ))

    # decorate electrons with the output of LH loose+BL
    ElectronPassLHLooseBL = acc.addPublicTool(acc.popToolsAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            flags,
            name="ElectronPassLHLooseBL",
            EGammaElectronLikelihoodTool=ElectronLHSelectorLooseBL,
            CutType="",
            StoreGateEntryName="DFCommonElectronsLHLooseBL",
            ContainerName="Electrons",
            StoreTResult=False,
        )
    ))

    # decorate electrons with the output of LH medium
    ElectronPassLHMedium = acc.addPublicTool(acc.popToolsAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            flags,
            name="ElectronPassLHMedium",
            EGammaElectronLikelihoodTool=ElectronLHSelectorMedium,
            CutType="",
            StoreGateEntryName="DFCommonElectronsLHMedium",
            ContainerName="Electrons",
            StoreTResult=False,
        )
    ))

    # decorate electrons with the output of LH tight
    ElectronPassLHTight = acc.addPublicTool(acc.popToolsAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            flags,
            name="ElectronPassLHTight",
            EGammaElectronLikelihoodTool=ElectronLHSelectorTight,
            CutType="",
            StoreGateEntryName="DFCommonElectronsLHTight",
            ContainerName="Electrons",
            StoreTResult=False,
        )
    ))

    # decorate electrons with the output of DNN Loose
    ElectronPassDNNLoose = acc.addPublicTool(acc.popToolsAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            flags,
            name="ElectronPassDNNLoose",
            EGammaElectronLikelihoodTool=ElectronDNNSelectorLoose,
            CutType="",
            StoreGateEntryName="DFCommonElectronsDNNLoose",
            ContainerName="Electrons",
            FudgedContainerName="FudgedElectrons" if isMC else "",
            StoreTResult=False,
            StoreGateEntryMultipleNames=[
                "DFCommonElectronsDNN_pel",
                "DFCommonElectronsDNN_pcf",
                "DFCommonElectronsDNN_ppc",
                "DFCommonElectronsDNN_phf",
                "DFCommonElectronsDNN_ple",
                "DFCommonElectronsDNN_plh",
            ],
            StoreMultipleOutputs=True,
        )
    ))

    # decorate electrons with the output of DNN Medium
    ElectronPassDNNMedium = acc.addPublicTool(acc.popToolsAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            flags,
            name="ElectronPassDNNMedium",
            EGammaElectronLikelihoodTool=ElectronDNNSelectorMedium,
            CutType="",
            StoreGateEntryName="DFCommonElectronsDNNMedium",
            ContainerName="Electrons",
            FudgedContainerName="FudgedElectrons" if isMC else "",
            StoreTResult=False,
        )
    ))

    # decorate electrons with the output of DNN Tight
    ElectronPassDNNTight = acc.addPublicTool(acc.popToolsAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            flags,
            name="ElectronPassDNNTight",
            EGammaElectronLikelihoodTool=ElectronDNNSelectorTight,
            CutType="",
            StoreGateEntryName="DFCommonElectronsDNNTight",
            ContainerName="Electrons",
            FudgedContainerName="FudgedElectrons" if isMC else "",
            StoreTResult=False,
        )
    ))

    # decorate electrons with the output of DNN VeryLoose97 without CF
    ElectronPassDNNVeryLooseNoCF97 = acc.addPublicTool(acc.popToolsAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            flags,
            name="ElectronPassDNNVeryLooseNoCF97",
            EGammaElectronLikelihoodTool=ElectronDNNSelectorVeryLooseNoCF97,
            CutType="",
            StoreGateEntryName="DFCommonElectronsDNNVeryLooseNoCF97",
            ContainerName="Electrons",
            FudgedContainerName="FudgedElectrons" if isMC else "",
            StoreTResult=False,
        )
    ))
    # decorate electrons with the output of DNN Loose without CF
    ElectronPassDNNLooseNoCF = acc.addPublicTool(acc.popToolsAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            flags,
            name="ElectronPassDNNLooseNoCF",
            EGammaElectronLikelihoodTool=ElectronDNNSelectorLooseNoCF,
            CutType="",
            StoreGateEntryName="DFCommonElectronsDNNLooseNoCF",
            ContainerName="Electrons",
            FudgedContainerName="FudgedElectrons" if isMC else "",
            StoreTResult=False,
        )
    ))

    # decorate electrons with the output of DNN Medium without CF
    ElectronPassDNNMediumNoCF = acc.addPublicTool(acc.popToolsAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            flags,
            name="ElectronPassDNNMediumNoCF",
            EGammaElectronLikelihoodTool=ElectronDNNSelectorMediumNoCF,
            CutType="",
            StoreGateEntryName="DFCommonElectronsDNNMediumNoCF",
            ContainerName="Electrons",
            FudgedContainerName="FudgedElectrons" if isMC else "",
            StoreTResult=False,
        )
    ))

    # decorate electrons with the output of DNN Tight without CF
    ElectronPassDNNTightNoCF = acc.addPublicTool(acc.popToolsAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            flags,
            name="ElectronPassDNNTightNoCF",
            EGammaElectronLikelihoodTool=ElectronDNNSelectorTightNoCF,
            CutType="",
            StoreGateEntryName="DFCommonElectronsDNNTightNoCF",
            ContainerName="Electrons",
            FudgedContainerName="FudgedElectrons" if isMC else "",
            StoreTResult=False,
        )
    ))

    # decorate electrons with the output of ECIDS
    if flags.Derivation.Egamma.addECIDS:
        ElectronPassECIDS = acc.addPublicTool(acc.popToolsAndMerge(
            EGElectronLikelihoodToolWrapperCfg(
                flags,
                name="ElectronPassECIDS",
                EGammaElectronLikelihoodTool=ElectronChargeIDSelector,
                CutType="",
                StoreGateEntryName="DFCommonElectronsECIDS",
                ContainerName="Electrons",
                StoreTResult=True,
            )
        ))

    if includeFwdElectrons:
        # decorate forward electrons with the output of LH loose
        ForwardElectronPassLHLoose = acc.addPublicTool(acc.popToolsAndMerge(
            EGElectronLikelihoodToolWrapperCfg(
                flags,
                name="ForwardElectronPassLHLoose",
                EGammaElectronLikelihoodTool=ForwardElectronLHSelectorLoose,
                CutType="",
                StoreGateEntryName="DFCommonForwardElectronsLHLoose",
                ContainerName="ForwardElectrons",
            )
        ))

        # decorate forward electrons with the output of LH medium
        ForwardElectronPassLHMedium = acc.addPublicTool(acc.popToolsAndMerge(
            EGElectronLikelihoodToolWrapperCfg(
                flags,
                name="ForwardElectronPassLHMedium",
                EGammaElectronLikelihoodTool=ForwardElectronLHSelectorMedium,
                CutType="",
                StoreGateEntryName="DFCommonForwardElectronsLHMedium",
                ContainerName="ForwardElectrons",
            )
        ))

        # decorate forward electrons with the output of LH tight
        ForwardElectronPassLHTight = acc.addPublicTool(acc.popToolsAndMerge(
            EGElectronLikelihoodToolWrapperCfg(
                flags,
                name="ForwardElectronPassLHTight",
                EGammaElectronLikelihoodTool=ForwardElectronLHSelectorTight,
                CutType="",
                StoreGateEntryName="DFCommonForwardElectronsLHTight",
                ContainerName="ForwardElectrons",
            )
        ))

    # decorate photons with the output of IsEM loose
    # on MC, use fudged shower shapes to compute the ID (but the
    # original shower shapes are not overridden)
    PhotonPassIsEMLoose = acc.addPublicTool(acc.popToolsAndMerge(
        EGSelectionToolWrapperCfg(
            flags,
            name="PhotonPassIsEMLoose",
            EGammaSelectionTool=PhotonIsEMSelectorLoose,
            CutType="",
            StoreGateEntryName="DFCommonPhotonsIsEMLoose",
            ContainerName="Photons",
            FudgedContainerName="FudgedPhotons" if isFullSim else "",
        )
    ))

    # decorate photons with the output of IsEM medium
    # on MC, use fudged shower shapes to compute the ID (but the
    # original shower shapes are not overridden)
    PhotonPassIsEMMedium = acc.addPublicTool(acc.popToolsAndMerge(
        EGSelectionToolWrapperCfg(
            flags,
            name="PhotonPassIsEMMedium",
            EGammaSelectionTool=PhotonIsEMSelectorMedium,
            CutType="",
            StoreGateEntryName="DFCommonPhotonsIsEMMedium",
            ContainerName="Photons",
            FudgedContainerName="FudgedPhotons" if isFullSim else "",
        )
    ))

    # decorate photons with the output of IsEM tight
    # on full-sim MC, use fudged shower shapes to compute the ID
    # (but the original shower shapes are not overridden)
    PhotonPassIsEMTight = acc.addPublicTool(acc.popToolsAndMerge(
        EGSelectionToolWrapperCfg(
            flags,
            name="PhotonPassIsEMTight",
            EGammaSelectionTool=PhotonIsEMSelectorTight,
            CutType="",
            StoreGateEntryName="DFCommonPhotonsIsEMTight",
            ContainerName="Photons",
            FudgedContainerName="FudgedPhotons" if isFullSim else "",
        )
    ))

    # decorate photons with the output of BDT tight
    # on full-sim MC, use fudged shower shapes to compute the ID
    # (but the original shower shapes are not overridden)
    PhotonBDTDecorator = acc.addPublicTool(acc.popToolsAndMerge(
        EGPhotonBDTToolDecoratorCfg(
            flags,
            name="PhotonBDTDecorator",
            PhotonObservableTool=PhotonBDTCalculator,
            StoreGateEntryName="DFCommonPhotonsBDT",
            ContainerName="Photons",
            FudgedContainerName= "FudgedPhotons" if isFullSim else ""
        )
    ))

    
    PhotonPassBDTTight = acc.addPublicTool(acc.popToolsAndMerge(
        EGPhotonBDTToolWrapperCfg(
            flags,
            name="PhotonPassBDTTight",
            PhotonBDTSelectionTool=PhotonBDTSelectorTight,
            CutType="",
            StoreGateEntryName="DFCommonPhotonsBDT",
            WorkingPointName="Tight",
            ContainerName="Photons",
            FudgedContainerName= "FudgedPhotons" if isFullSim else ""
        )
    ))

    # decorate photons with the output of IsEM tight
    # on MC, normalizing flows-based correction used to compute the ID
    # (but the original shower shapes are not overridden)
    PhotonPassIsEMTightNF = acc.addPublicTool(acc.popToolsAndMerge(
        EGSelectionToolWrapperCfg(
            flags,
            name="PhotonPassIsEMTightNF",
            EGammaSelectionTool=PhotonIsEMSelectorTight,
            CutType="",
            StoreGateEntryName="DFCommonPhotonsIsEMTightNF",
            ContainerName="Photons",
            FudgedContainerName= "NFFudgedPhotons" if (isMC and isRun2orRun3) else ""
        )
    ))

    # decorate photons with the output of BDT tight
    # on MC, normalizing flows-based correction used to compute the ID
    # (but the original shower shapes are not overridden)
    PhotonBDTDecoratorNF = acc.addPublicTool(acc.popToolsAndMerge(
        EGPhotonBDTToolDecoratorCfg(
            flags,
            name="PhotonBDTDecoratorNF",
            PhotonObservableTool=PhotonBDTCalculatorNF,
            StoreGateEntryName="DFCommonPhotonsNFBDT",
            ContainerName="Photons",
            FudgedContainerName= "NFFudgedPhotons" if (isMC and isRun2orRun3) else ""
        )
    ))

    PhotonPassBDTTightNF = acc.addPublicTool(acc.popToolsAndMerge(
        EGPhotonBDTToolWrapperCfg(
            flags,
            name="PhotonPassBDTTightNF",
            PhotonBDTSelectionTool=PhotonBDTSelectorTightNF,
            CutType="",
            StoreGateEntryName="DFCommonPhotonsNFBDT",
            WorkingPointName="Tight",
            ContainerName="Photons",
            FudgedContainerName= "NFFudgedPhotons" if (isMC and isRun2orRun3) else ""
        )
    ))


    # decorate photons with the photon cleaning flags
    # on MC, fudge the shower shapes used to compute the flags
    from DerivationFrameworkEGamma.EGammaToolsConfig import EGPhotonCleaningWrapperCfg

    PhotonPassCleaning = acc.addPublicTool(acc.popToolsAndMerge(
        EGPhotonCleaningWrapperCfg(
            flags,
            name="PhotonPassCleaning",
            StoreGateEntryName="DFCommonPhotonsCleaning",
            ContainerName="Photons",
            FudgedContainerName= "FudgedPhotons" if isFullSim else ""
        )
    ))

    # decorate some electrons with an additional ambiguity flag
    # against internal and early material conversion
    from DerivationFrameworkEGamma.EGammaToolsConfig import EGElectronAmbiguityToolCfg

    ElectronAmbiguity = acc.addPublicTool(acc.popToolsAndMerge(
        EGElectronAmbiguityToolCfg(
            flags,
            name="ElectronAdditionnalAmbiguity",
            isMC=flags.Input.isMC,
        )
    ))

    # list of all the decorators so far
    EGAugmentationTools = [
        DFCommonPhotonsDirection,
        ElectronPassLHVeryLoose,
        ElectronPassLHLoose,
        ElectronPassLHLooseBL,
        ElectronPassLHMedium,
        ElectronPassLHTight,
        ElectronPassDNNLoose,
        ElectronPassDNNMedium,
        ElectronPassDNNTight,
        ElectronPassDNNVeryLooseNoCF97,
        ElectronPassDNNLooseNoCF,
        ElectronPassDNNMediumNoCF,
        ElectronPassDNNTightNoCF,
        PhotonPassIsEMLoose,
        PhotonPassIsEMMedium,
        PhotonPassIsEMTight,
        PhotonPassIsEMTightNF,
        PhotonBDTDecorator,
        PhotonPassBDTTight,
        PhotonBDTDecoratorNF,
        PhotonPassBDTTightNF,
        PhotonPassCleaning,
        ElectronAmbiguity,
    ]

    if flags.Derivation.Egamma.addECIDS:
        EGAugmentationTools.extend([ElectronPassECIDS])

    if includeFwdElectrons:
        EGAugmentationTools.extend(
            [
                ForwardElectronPassLHLoose,
                ForwardElectronPassLHMedium,
                ForwardElectronPassLHTight,
            ]
        )

    from egammaAlgs.egammaAODFixesConfig import runAODFix
    _, fixes = runAODFix(flags)
    # the topoIso fix already provides the decorations that this tool creates
    if not('egammatopoIsoFix' in fixes):
        if flags.Derivation.Egamma.addMissingCellInfo:
            from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
                EgammaCoreCellRecoveryCfg,
            )

            CoreCellRecoveryTool = acc.popToolsAndMerge(
                EgammaCoreCellRecoveryCfg(flags)
            )
            acc.addPublicTool(CoreCellRecoveryTool)
            EGAugmentationTools.append(CoreCellRecoveryTool)

    if flags.Derivation.Egamma.addMissingCellInfo:
        from DerivationFrameworkEGamma.EGammaToolsConfig import EGammaEnergyCalibrationWrapperCfg
        TransformerEnergyCalibration = acc.addPublicTool(acc.popToolsAndMerge(
            EGammaEnergyCalibrationWrapperCfg(
                flags,
                name="TransformerEnergyCalibration",
            )
        ))
        EGAugmentationTools.append(TransformerEnergyCalibration)

    # ==================================================
    # Truth Related tools
    if flags.Input.isMC:
        # Decorate Electron with bkg electron type/origin
        from DerivationFrameworkEGamma.EGammaToolsConfig import (
            BkgElectronClassificationCfg,
        )

        BkgElectronClassificationTool = acc.addPublicTool(acc.popToolsAndMerge(
            BkgElectronClassificationCfg(
                flags,
                name="BkgElectronClassificationTool"
            )
        ))
        EGAugmentationTools.append(BkgElectronClassificationTool)

        # Decorate egammaTruthParticles with truth-particle-level etcone20,30,40
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import (
            TruthIsolationToolCfg,
        )

        TruthEgetIsolationTool = acc.getPrimaryAndMerge(
            TruthIsolationToolCfg(
                flags,
                name="TruthEgetIsolationTool",
                isoParticlesKey="egammaTruthParticles",
                allParticlesKey="TruthParticles",
                particleIDsToCalculate=[-11, 11, 22],
                IsolationConeSizes=[0.2, 0.3, 0.4],
                excludeIDsFromCone=[-16, -14, -13, -12, 12, 13, 14, 16],
                IsolationVarNamePrefix="etcone",
                ChargedParticlesOnly=False,
            )
        )
        EGAugmentationTools.append(TruthEgetIsolationTool)

        # Decorate egammaTruthParticles with truth-particle-level ptcone20,30,40
        TruthEgptIsolationTool = acc.getPrimaryAndMerge(
            TruthIsolationToolCfg(
                flags,
                name="TruthEgptIsolationTool",
                isoParticlesKey="egammaTruthParticles",
                allParticlesKey="TruthParticles",
                particleIDsToCalculate=[-11, 11, 22],
                IsolationConeSizes=[0.2, 0.3, 0.4],
                IsolationVarNamePrefix="ptcone",
                ChargedParticlesOnly=True,
            )
        )
        EGAugmentationTools.append(TruthEgptIsolationTool)

        # Compute the truth-particle-level energy density in the central eta region
        from EventShapeTools.EventDensityConfig import configEventDensityTool
        from JetRecConfig.JetRecConfig import (
            getInputAlgs,
            getConstitPJGAlg,
            reOrderAlgs,
        )
        from JetRecConfig.StandardJetConstits import stdConstitDic as cst

        # Schedule PseudoJetTruth
        constit_algs = getInputAlgs(cst.Truth, flags=flags)
        constit_algs, ca = reOrderAlgs([a for a in constit_algs if a is not None])
        acc.merge(ca)
        for a in constit_algs:
            acc.addEventAlgo(a)
        constitPJAlg = getConstitPJGAlg(cst.Truth, suffix=None)
        acc.addEventAlgo(constitPJAlg)

        tc = configEventDensityTool(
            "EDTruthCentralTool",
            cst.Truth,
            0.5,
            AbsRapidityMin=0.0,
            AbsRapidityMax=1.5,
            OutputContainer="TruthIsoCentralEventShape",
            OutputLevel=3,
        )
        acc.addPublicTool(tc)

        # Compute the truth-particle-level energy density in the forward eta region
        tf = configEventDensityTool(
            "EDTruthForwardTool",
            cst.Truth,
            0.5,
            AbsRapidityMin=1.5,
            AbsRapidityMax=3.0,
            OutputContainer="TruthIsoForwardEventShape",
            OutputLevel=3,
        )
        acc.addPublicTool(tf)

        acc.addEventAlgo(
            CompFactory.EventDensityAthAlg("EDTruthCentralAlg", EventDensityTool=tc)
        )
        acc.addEventAlgo(
            CompFactory.EventDensityAthAlg("EDTruthForwardAlg", EventDensityTool=tf)
        )

    # Create Fudged egamma collections
    if flags.Input.isMC:
        from DerivationFrameworkEGamma.EGammaToolsConfig import (
            ElectronFudgeAlgorithmCfg, PhotonFudgeAlgorithmCfg, PhotonNFFudgeAlgorithmCfg)
        acc.merge(ElectronFudgeAlgorithmCfg(flags))
        if isRun2orRun3:
            acc.merge(PhotonNFFudgeAlgorithmCfg(flags))
        if isFullSim:
            acc.merge(PhotonFudgeAlgorithmCfg(flags))
        
    # =======================================
    # CREATE THE DERIVATION KERNEL ALGORITHM
    # =======================================

    for i, tool in enumerate(EGAugmentationTools):
        acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(f"EGammaCommonKernel{i}", AugmentationTools = [tool]))

    # =======================================
    # ADD TOOLS : custom electron, photon and muon track isolation
    # =======================================
    from IsolationAlgs.DerivationTrackIsoConfig import DerivationTrackIsoCfg

    if includeFwdElectrons:
        acc.merge(DerivationTrackIsoCfg(flags, object_types=("Electrons", "Muons", "ForwardElectrons")))
    else:
        acc.merge(DerivationTrackIsoCfg(flags, object_types=("Electrons", "Muons")))

    hasFlowObject = (
        "JetETMissChargedParticleFlowObjects" in flags.Input.Collections
        and "JetETMissNeutralParticleFlowObjects" in flags.Input.Collections
    )
    if hasFlowObject:
        from IsolationAlgs.IsolationSteeringDerivConfig import IsolationSteeringDerivCfg

        acc.merge(IsolationSteeringDerivCfg(flags))

    return acc
