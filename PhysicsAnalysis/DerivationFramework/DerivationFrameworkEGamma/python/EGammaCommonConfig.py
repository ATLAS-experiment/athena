# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# ********************************************************************
# EGammaCommonConfig.py
# Configures  all tools needed for e-gamma object selection and sets
# up the kernel algorithms so the results can be accessed/written to
# the DAODs.
# Component accumulator version.
# ********************************************************************

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def EGammaCommonCfg(ConfigFlags):
    """Main config method for e-gamma decorations"""

    acc = ComponentAccumulator()

    includeFwdElectrons = "ForwardElectrons" in ConfigFlags.Input.Collections

    # ====================================================================
    # PHOTON ETA (=ETA2), ET (=E/COSH(ETA2))
    # ====================================================================
    from DerivationFrameworkEGamma.EGammaToolsConfig import PhotonsDirectionToolCfg

    DFCommonPhotonsDirection = acc.getPrimaryAndMerge(
        PhotonsDirectionToolCfg(
            ConfigFlags,
            name="DFCommonPhotonsDirection",
            EtaSGEntry="DFCommonPhotons_eta",
            PhiSGEntry="DFCommonPhotons_phi",
            EtSGEntry="DFCommonPhotons_et",
        )
    )

    # ====================================================================
    # SHOWER SHAPE CORRECTIONS IN MC
    # TUNE27: e FUDGE FACTORS RUN2 FULL DATA, derived with rel 22.2
    # TUNE25: gamma FUDGE FACTORS RUN2 FULL DATA, derived with or 21.2
    # AF3 is tuned to FullSim, so same FFs can be used for AF3 and FS
    # ====================================================================
    isMC = ConfigFlags.Input.isMC
    isFullSim = False
    if isMC:
        isFullSim = ConfigFlags.Sim.ISF.Simulator.isFullSim()

    print("EGammaCommon: isMC = ", isMC)
    if isMC:
        print("EGammaCommon: isFullSim = ", isFullSim)

    if isMC:
        from EGammaVariableCorrection.EGammaVariableCorrectionConfig import (
            ElectronVariableCorrectionToolCfg,
            PhotonVariableCorrectionToolCfg,
        )

        ElectronVariableCorrectionTool = acc.popToolsAndMerge(
            ElectronVariableCorrectionToolCfg(ConfigFlags)
        )
        acc.addPublicTool(ElectronVariableCorrectionTool)

        PhotonVariableCorrectionTool = acc.popToolsAndMerge(
            PhotonVariableCorrectionToolCfg(ConfigFlags)
        )
        acc.addPublicTool(PhotonVariableCorrectionTool)

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
    from AthenaConfiguration.Enums import LHCPeriod
    if ConfigFlags.GeoModel.Run is LHCPeriod.Run2:
        lhMenu = electronLHmenu.offlineMC20

    # Very Loose
    ElectronLHSelectorVeryLoose = acc.popToolsAndMerge(
        AsgElectronLikelihoodToolCfg(
            ConfigFlags,
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
            ConfigFlags,
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
            ConfigFlags,
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
            ConfigFlags,
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
            ConfigFlags,
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
            ConfigFlags,
            name="ElectronDNNSelectorLoose",
            WorkingPoint="LooseDNNElectron",
        )
    )
    acc.addPublicTool(ElectronDNNSelectorLoose)

    # Medium
    ElectronDNNSelectorMedium = acc.popToolsAndMerge(
        AsgElectronSelectorToolCfg(
            ConfigFlags,
            name="ElectronDNNSelectorMedium",
            WorkingPoint="MediumDNNElectron",
        )
    )
    acc.addPublicTool(ElectronDNNSelectorMedium)

    # Tight
    ElectronDNNSelectorTight = acc.popToolsAndMerge(
        AsgElectronSelectorToolCfg(
            ConfigFlags,
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
            ConfigFlags,
            name="ElectronDNNSelectorVeryLooseNoCF97",
            WorkingPoint="VeryLooseNoCF97DNNElectron",
        )
    )
    acc.addPublicTool(ElectronDNNSelectorVeryLooseNoCF97)

    # Loose
    ElectronDNNSelectorLooseNoCF = acc.popToolsAndMerge(
        AsgElectronSelectorToolCfg(
            ConfigFlags,
            name="ElectronDNNSelectorLooseNoCF",
            WorkingPoint="LooseNoCFDNNElectron",
        )
    )
    acc.addPublicTool(ElectronDNNSelectorLooseNoCF)

    # Medium
    ElectronDNNSelectorMediumNoCF = acc.popToolsAndMerge(
        AsgElectronSelectorToolCfg(
            ConfigFlags,
            name="ElectronDNNSelectorMediumNoCF",
            WorkingPoint="MediumNoCFDNNElectron",
        )
    )
    acc.addPublicTool(ElectronDNNSelectorMediumNoCF)

    # Tight
    ElectronDNNSelectorTightNoCF = acc.popToolsAndMerge(
        AsgElectronSelectorToolCfg(
            ConfigFlags,
            name="ElectronDNNSelectorTightNoCF",
            WorkingPoint="TightNoCFDNNElectron",
        )
    )
    acc.addPublicTool(ElectronDNNSelectorTightNoCF)

    # ====================================================================
    # ELECTRON CHARGE SELECTION
    # ====================================================================
    if ConfigFlags.Derivation.Egamma.addECIDS:
        from ElectronPhotonSelectorTools.AsgElectronChargeIDSelectorToolConfig import (
            AsgElectronChargeIDSelectorToolCfg,
        )

        ElectronChargeIDSelector = acc.popToolsAndMerge(
            AsgElectronChargeIDSelectorToolCfg(
                ConfigFlags, name="ElectronChargeIDSelectorLoose"
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
        from ElectronPhotonSelectorTools.AsgForwardElectronLikelihoodToolConfig import (
            AsgForwardElectronLikelihoodToolCfg,
        )

        ForwardElectronLHSelectorLoose = acc.popToolsAndMerge(
            AsgForwardElectronLikelihoodToolCfg(
                ConfigFlags,
                name="ForwardElectronLHSelectorLoose",
                WorkingPoint="LooseLHForwardElectron",
            )
        )
        acc.addPublicTool(ForwardElectronLHSelectorLoose)

        ForwardElectronLHSelectorMedium = acc.popToolsAndMerge(
            AsgForwardElectronLikelihoodToolCfg(
                ConfigFlags,
                name="ForwardElectronLHSelectorMedium",
                WorkingPoint="MediumLHForwardElectron",
            )
        )
        acc.addPublicTool(ForwardElectronLHSelectorMedium)

        ForwardElectronLHSelectorTight = acc.popToolsAndMerge(
            AsgForwardElectronLikelihoodToolCfg(
                ConfigFlags,
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
    if ConfigFlags.GeoModel.Run is LHCPeriod.Run2:
        pidMenu = photonPIDmenu.offlineMC20

    # Loose
    PhotonIsEMSelectorLoose = acc.popToolsAndMerge(
        AsgPhotonIsEMSelectorCfg(
            ConfigFlags,
            name="PhotonIsEMSelectorLoose",
            quality=egammaPID.PhotonIDLoose,
            menu=pidMenu
        )
    )
    acc.addPublicTool(PhotonIsEMSelectorLoose)

    # Medium
    PhotonIsEMSelectorMedium = acc.popToolsAndMerge(
        AsgPhotonIsEMSelectorCfg(
            ConfigFlags,
            name="PhotonIsEMSelectorMedium",
            quality=egammaPID.PhotonIDMedium,
            menu=pidMenu
        )
    )
    acc.addPublicTool(PhotonIsEMSelectorMedium)

    # Tight
    PhotonIsEMSelectorTight = acc.popToolsAndMerge(
        AsgPhotonIsEMSelectorCfg(
            ConfigFlags,
            name="PhotonIsEMSelectorTight",
            quality=egammaPID.PhotonIDTight,
            menu=pidMenu
        )
    )
    acc.addPublicTool(PhotonIsEMSelectorTight)


    # ====================================================================
    # RECTANGULAR CLUSTER TOOLS
    # ====================================================================

    from egammaCaloTools.egammaCaloToolsConfig import CaloFillRectangularClusterCfg

    EGAMCOM_caloFillRect55 = acc.popToolsAndMerge(
        CaloFillRectangularClusterCfg(
            ConfigFlags,
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
            ConfigFlags,
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
            ConfigFlags,
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
            ConfigFlags,
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
    )

    # Note: LH selectors don't need fudging since the LH is tuned to data

    # decorate electrons with the output of LH very loose
    ElectronPassLHVeryLoose = acc.getPrimaryAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            ConfigFlags,
            name="ElectronPassLHVeryLoose",
            EGammaElectronLikelihoodTool=ElectronLHSelectorVeryLoose,
            EGammaFudgeMCTool=None,
            CutType="",
            StoreGateEntryName="DFCommonElectronsLHVeryLoose",
            ContainerName="Electrons",
            StoreTResult=False,
        )
    )

    # decorate electrons with the output of LH loose
    ElectronPassLHLoose = acc.getPrimaryAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            ConfigFlags,
            name="ElectronPassLHLoose",
            EGammaElectronLikelihoodTool=ElectronLHSelectorLoose,
            EGammaFudgeMCTool=None,
            CutType="",
            StoreGateEntryName="DFCommonElectronsLHLoose",
            ContainerName="Electrons",
            StoreTResult=False,
        )
    )

    # decorate electrons with the output of LH loose+BL
    ElectronPassLHLooseBL = acc.getPrimaryAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            ConfigFlags,
            name="ElectronPassLHLooseBL",
            EGammaElectronLikelihoodTool=ElectronLHSelectorLooseBL,
            EGammaFudgeMCTool=None,
            CutType="",
            StoreGateEntryName="DFCommonElectronsLHLooseBL",
            ContainerName="Electrons",
            StoreTResult=False,
        )
    )

    # decorate electrons with the output of LH medium
    ElectronPassLHMedium = acc.getPrimaryAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            ConfigFlags,
            name="ElectronPassLHMedium",
            EGammaElectronLikelihoodTool=ElectronLHSelectorMedium,
            EGammaFudgeMCTool=None,
            CutType="",
            StoreGateEntryName="DFCommonElectronsLHMedium",
            ContainerName="Electrons",
            StoreTResult=False,
        )
    )

    # decorate electrons with the output of LH tight
    ElectronPassLHTight = acc.getPrimaryAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            ConfigFlags,
            name="ElectronPassLHTight",
            EGammaElectronLikelihoodTool=ElectronLHSelectorTight,
            EGammaFudgeMCTool=None,
            CutType="",
            StoreGateEntryName="DFCommonElectronsLHTight",
            ContainerName="Electrons",
            StoreTResult=False,
        )
    )

    # decorate electrons with the output of DNN Loose
    ElectronPassDNNLoose = acc.getPrimaryAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            ConfigFlags,
            name="ElectronPassDNNLoose",
            EGammaElectronLikelihoodTool=ElectronDNNSelectorLoose,
            EGammaFudgeMCTool=(ElectronVariableCorrectionTool if isMC else None),
            CutType="",
            StoreGateEntryName="DFCommonElectronsDNNLoose",
            ContainerName="Electrons",
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
    )

    # decorate electrons with the output of DNN Medium
    ElectronPassDNNMedium = acc.getPrimaryAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            ConfigFlags,
            name="ElectronPassDNNMedium",
            EGammaElectronLikelihoodTool=ElectronDNNSelectorMedium,
            EGammaFudgeMCTool=(ElectronVariableCorrectionTool if isMC else None),
            CutType="",
            StoreGateEntryName="DFCommonElectronsDNNMedium",
            ContainerName="Electrons",
            StoreTResult=False,
        )
    )

    # decorate electrons with the output of DNN Tight
    ElectronPassDNNTight = acc.getPrimaryAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            ConfigFlags,
            name="ElectronPassDNNTight",
            EGammaElectronLikelihoodTool=ElectronDNNSelectorTight,
            EGammaFudgeMCTool=(ElectronVariableCorrectionTool if isMC else None),
            CutType="",
            StoreGateEntryName="DFCommonElectronsDNNTight",
            ContainerName="Electrons",
            StoreTResult=False,
        )
    )

    # decorate electrons with the output of DNN VeryLoose97 without CF
    ElectronPassDNNVeryLooseNoCF97 = acc.getPrimaryAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            ConfigFlags,
            name="ElectronPassDNNVeryLooseNoCF97",
            EGammaElectronLikelihoodTool=ElectronDNNSelectorVeryLooseNoCF97,
            EGammaFudgeMCTool=(ElectronVariableCorrectionTool if isMC else None),
            CutType="",
            StoreGateEntryName="DFCommonElectronsDNNVeryLooseNoCF97",
            ContainerName="Electrons",
            StoreTResult=False,
        )
    )
    # decorate electrons with the output of DNN Loose without CF
    ElectronPassDNNLooseNoCF = acc.getPrimaryAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            ConfigFlags,
            name="ElectronPassDNNLooseNoCF",
            EGammaElectronLikelihoodTool=ElectronDNNSelectorLooseNoCF,
            EGammaFudgeMCTool=(ElectronVariableCorrectionTool if isMC else None),
            CutType="",
            StoreGateEntryName="DFCommonElectronsDNNLooseNoCF",
            ContainerName="Electrons",
            StoreTResult=False,
        )
    )

    # decorate electrons with the output of DNN Medium without CF
    ElectronPassDNNMediumNoCF = acc.getPrimaryAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            ConfigFlags,
            name="ElectronPassDNNMediumNoCF",
            EGammaElectronLikelihoodTool=ElectronDNNSelectorMediumNoCF,
            EGammaFudgeMCTool=(ElectronVariableCorrectionTool if isMC else None),
            CutType="",
            StoreGateEntryName="DFCommonElectronsDNNMediumNoCF",
            ContainerName="Electrons",
            StoreTResult=False,
        )
    )

    # decorate electrons with the output of DNN Tight without CF
    ElectronPassDNNTightNoCF = acc.getPrimaryAndMerge(
        EGElectronLikelihoodToolWrapperCfg(
            ConfigFlags,
            name="ElectronPassDNNTightNoCF",
            EGammaElectronLikelihoodTool=ElectronDNNSelectorTightNoCF,
            EGammaFudgeMCTool=(ElectronVariableCorrectionTool if isMC else None),
            CutType="",
            StoreGateEntryName="DFCommonElectronsDNNTightNoCF",
            ContainerName="Electrons",
            StoreTResult=False,
        )
    )

    # decorate electrons with the output of ECIDS
    if ConfigFlags.Derivation.Egamma.addECIDS:
        ElectronPassECIDS = acc.getPrimaryAndMerge(
            EGElectronLikelihoodToolWrapperCfg(
                ConfigFlags,
                name="ElectronPassECIDS",
                EGammaElectronLikelihoodTool=ElectronChargeIDSelector,
                EGammaFudgeMCTool=None,
                CutType="",
                StoreGateEntryName="DFCommonElectronsECIDS",
                ContainerName="Electrons",
                StoreTResult=True,
            )
        )

    if includeFwdElectrons:
        # decorate forward electrons with the output of LH loose
        ForwardElectronPassLHLoose = acc.getPrimaryAndMerge(
            EGElectronLikelihoodToolWrapperCfg(
                ConfigFlags,
                name="ForwardElectronPassLHLoose",
                EGammaElectronLikelihoodTool=ForwardElectronLHSelectorLoose,
                EGammaFudgeMCTool=None,
                CutType="",
                StoreGateEntryName="DFCommonForwardElectronsLHLoose",
                ContainerName="ForwardElectrons",
            )
        )

        # decorate forward electrons with the output of LH medium
        ForwardElectronPassLHMedium = acc.getPrimaryAndMerge(
            EGElectronLikelihoodToolWrapperCfg(
                ConfigFlags,
                name="ForwardElectronPassLHMedium",
                EGammaElectronLikelihoodTool=ForwardElectronLHSelectorMedium,
                EGammaFudgeMCTool=None,
                CutType="",
                StoreGateEntryName="DFCommonForwardElectronsLHMedium",
                ContainerName="ForwardElectrons",
            )
        )

        # decorate forward electrons with the output of LH tight
        ForwardElectronPassLHTight = acc.getPrimaryAndMerge(
            EGElectronLikelihoodToolWrapperCfg(
                ConfigFlags,
                name="ForwardElectronPassLHTight",
                EGammaElectronLikelihoodTool=ForwardElectronLHSelectorTight,
                EGammaFudgeMCTool=None,
                CutType="",
                StoreGateEntryName="DFCommonForwardElectronsLHTight",
                ContainerName="ForwardElectrons",
            )
        )

    # decorate photons with the output of IsEM loose
    # on MC, fudge the shower shapes before computing the ID (but the
    # original shower shapes are not overridden)
    PhotonPassIsEMLoose = acc.getPrimaryAndMerge(
        EGSelectionToolWrapperCfg(
            ConfigFlags,
            name="PhotonPassIsEMLoose",
            EGammaSelectionTool=PhotonIsEMSelectorLoose,
            EGammaFudgeMCTool=(PhotonVariableCorrectionTool if isFullSim else None),
            CutType="",
            StoreGateEntryName="DFCommonPhotonsIsEMLoose",
            ContainerName="Photons",
        )
    )

    # decorate photons with the output of IsEM medium
    # on MC, fudge the shower shapes before computing the ID (but the
    # original shower shapes are not overridden)
    PhotonPassIsEMMedium = acc.getPrimaryAndMerge(
        EGSelectionToolWrapperCfg(
            ConfigFlags,
            name="PhotonPassIsEMMedium",
            EGammaSelectionTool=PhotonIsEMSelectorMedium,
            EGammaFudgeMCTool=(PhotonVariableCorrectionTool if isFullSim else None),
            CutType="",
            StoreGateEntryName="DFCommonPhotonsIsEMMedium",
            ContainerName="Photons",
        )
    )

    # decorate photons with the output of IsEM tight
    # on full-sim MC, fudge the shower shapes before computing the ID
    # (but the original shower shapes are not overridden)
    PhotonPassIsEMTight = acc.getPrimaryAndMerge(
        EGSelectionToolWrapperCfg(
            ConfigFlags,
            name="PhotonPassIsEMTight",
            EGammaSelectionTool=PhotonIsEMSelectorTight,
            EGammaFudgeMCTool=(PhotonVariableCorrectionTool if isFullSim else None),
            CutType="",
            StoreGateEntryName="DFCommonPhotonsIsEMTight",
            ContainerName="Photons",
        )
    )


    # decorate photons with the photon cleaning flags
    # on MC, fudge the shower shapes before computing the flags
    from DerivationFrameworkEGamma.EGammaToolsConfig import EGPhotonCleaningWrapperCfg

    PhotonPassCleaning = acc.getPrimaryAndMerge(
        EGPhotonCleaningWrapperCfg(
            ConfigFlags,
            name="PhotonPassCleaning",
            EGammaFudgeMCTool=(PhotonVariableCorrectionTool if isFullSim else None),
            StoreGateEntryName="DFCommonPhotonsCleaning",
            ContainerName="Photons",
        )
    )

    # decorate some electrons with an additional ambiguity flag
    # against internal and early material conversion
    from DerivationFrameworkEGamma.EGammaToolsConfig import EGElectronAmbiguityToolCfg

    ElectronAmbiguity = acc.getPrimaryAndMerge(
        EGElectronAmbiguityToolCfg(
            ConfigFlags,
            name="ElectronAdditionnalAmbiguity",
            isMC=ConfigFlags.Input.isMC,
        )
    )

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
        PhotonPassCleaning,
        ElectronAmbiguity,
    ]

    if ConfigFlags.Derivation.Egamma.addECIDS:
        EGAugmentationTools.extend([ElectronPassECIDS])

    if includeFwdElectrons:
        EGAugmentationTools.extend(
            [
                ForwardElectronPassLHLoose,
                ForwardElectronPassLHMedium,
                ForwardElectronPassLHTight,
            ]
        )

    if ConfigFlags.Derivation.Egamma.addMissingCellInfo:
        from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import (
            EgammaCoreCellRecoveryCfg,
        )

        CoreCellRecoveryTool = acc.popToolsAndMerge(
            EgammaCoreCellRecoveryCfg(ConfigFlags)
        )
        acc.addPublicTool(CoreCellRecoveryTool)
        EGAugmentationTools.append(CoreCellRecoveryTool)

    # ==================================================
    # Truth Related tools
    if ConfigFlags.Input.isMC:
        # Decorate Electron with bkg electron type/origin
        from MCTruthClassifier.MCTruthClassifierConfig import MCTruthClassifierCfg

        BkgElectronMCTruthClassifier = acc.popToolsAndMerge(
            MCTruthClassifierCfg(
                ConfigFlags,
                name="BkgElectronMCTruthClassifier",
                ParticleCaloExtensionTool="",
            )
        )
        acc.addPublicTool(BkgElectronMCTruthClassifier)

        from DerivationFrameworkEGamma.EGammaToolsConfig import (
            BkgElectronClassificationCfg,
        )

        BkgElectronClassificationTool = acc.getPrimaryAndMerge(
            BkgElectronClassificationCfg(
                ConfigFlags,
                name="BkgElectronClassificationTool",
                MCTruthClassifierTool=BkgElectronMCTruthClassifier,
            )
        )
        EGAugmentationTools.append(BkgElectronClassificationTool)

        # Decorate egammaTruthParticles with truth-particle-level etcone20,30,40
        from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import (
            TruthIsolationToolCfg,
        )

        TruthEgetIsolationTool = acc.getPrimaryAndMerge(
            TruthIsolationToolCfg(
                ConfigFlags,
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
                ConfigFlags,
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
        constit_algs = getInputAlgs(cst.Truth, flags=ConfigFlags)
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

    # =======================================
    # CREATE THE DERIVATION KERNEL ALGORITHM
    # =======================================

    acc.addEventAlgo(
        CompFactory.DerivationFramework.CommonAugmentation(
            "EGammaCommonKernel", AugmentationTools=EGAugmentationTools
        )
    )

    # =======================================
    # ADD TOOLS : custom electron, photon and muon track isolation
    # =======================================
    from IsolationAlgs.DerivationTrackIsoConfig import DerivationTrackIsoCfg

    acc.merge(DerivationTrackIsoCfg(ConfigFlags, object_types=("Electrons", "Muons")))

    hasFlowObject = (
        "JetETMissChargedParticleFlowObjects" in ConfigFlags.Input.Collections
        and "JetETMissNeutralParticleFlowObjects" in ConfigFlags.Input.Collections
    )
    if hasFlowObject:
        from IsolationAlgs.IsolationSteeringDerivConfig import IsolationSteeringDerivCfg

        acc.merge(IsolationSteeringDerivCfg(ConfigFlags))

    return acc
