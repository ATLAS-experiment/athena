# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# ********************************************************************
# EGammaLRTConfig.py
# Configures  all tools needed for LRT e-gamma object selection and sets
# up the kernel algorithms so the results can be accessed/written to
# the DAODs. Copied and modified from EGammaCommonConfig.py.
# Component accumulator version.
# ********************************************************************

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def EGammaLRTCfg(flags):
    """Main config method for LRT e-gamma decorations"""

    acc = ComponentAccumulator()

    # ====================================================================
    # DISPLACED ELECTRON LH SELECTORS
    # see Reconstruction/egamma/egammaTools/python/EMPIDBuilderBase.py
    # on how to configure the selectors
    # ====================================================================
    # Setting conf file not supported.  These are currently setup in the
    # LLP1.py config TODO: implement common ID in egamma tools

    # ====================================================================
    # ELECTRON CHARGE SELECTION
    # ====================================================================
    if not hasattr(acc, "ElectronChargeIDSelectorLoose"):
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
                "ElectronPhotonSelectorTools/ChargeID/"
                + "ECIDS_20180731rel21Summer2018.root"
            )
            acc.addPublicTool(ElectronChargeIDSelector)

    # ====================================================================
    # AUGMENTATION TOOLS
    # ====================================================================
    from DerivationFrameworkEGamma.EGammaToolsConfig import (
        EGElectronLikelihoodToolWrapperCfg,
    )

    # decorate electrons with the output of LH very loose
    # TODO same as above, update with central ID

    # decorate electrons with the output of ECIDS
    if flags.Derivation.Egamma.addECIDS:
        LRTElectronPassECIDS = acc.addPublicTool(acc.popToolsAndMerge(
            EGElectronLikelihoodToolWrapperCfg(
                flags,
                name="LRTElectronPassECIDS",
                EGammaElectronLikelihoodTool=ElectronChargeIDSelector,
                EGammaFudgeMCTool="",
                CutType="",
                StoreGateEntryName="DFCommonElectronsECIDS",
                ContainerName="LRTElectrons",
                StoreTResult=True,
            )
        ))

    # decorate some electrons with an additional ambiguity flag
    # against internal and early material conversion
    from DerivationFrameworkEGamma.EGammaToolsConfig import EGElectronAmbiguityToolCfg

    LRTElectronAmbiguity = acc.addPublicTool(acc.popToolsAndMerge(
        EGElectronAmbiguityToolCfg(
            flags,
            name="LRTElectronAdditionnalAmbiguity",
            idCut="DFCommonElectronsLHLooseNoPix",
            ContainerName="LRTElectrons",
            isMC=flags.Input.isMC,
        )
    ))

    # list of all the decorators so far
    LRTEGAugmentationTools = [LRTElectronAmbiguity]
    if flags.Derivation.Egamma.addECIDS:
        LRTEGAugmentationTools.extend([LRTElectronPassECIDS])

    # ==================================================
    # Calo cell recovery tool
    if flags.Derivation.Egamma.addMissingCellInfo:
        from DerivationFrameworkCalo.DerivationFrameworkCaloConfig import EgammaCoreCellRecoveryCfg

        CoreCellRecoveryTool = acc.addPublicTool(acc.popToolsAndMerge(
            EgammaCoreCellRecoveryCfg(flags,
                                      name            = "LRTCoreCellRecoveryTool",
                                      SGKey_photons   = "",
                                      SGKey_electrons = "LRTElectrons")
        ))
        LRTEGAugmentationTools.append(CoreCellRecoveryTool)

    # ==================================================
    # Truth Related tools
    if flags.Input.isMC:
        # Decorate Electron with bkg electron type/origin
        from DerivationFrameworkEGamma.EGammaToolsConfig import (
            BkgElectronClassificationCfg,
        )

        BkgLRTElectronClassificationTool = acc.addPublicTool(acc.popToolsAndMerge(
            BkgElectronClassificationCfg(
                flags,
                name="BkgLRTElectronClassificationTool",
                ElectronContainerName="LRTElectrons"
            )
        ))
        LRTEGAugmentationTools.append(BkgLRTElectronClassificationTool)

    # =======================================
    # CREATE THE DERIVATION KERNEL ALGORITHM
    # =======================================

    acc.addEventAlgo(
        CompFactory.DerivationFramework.CommonAugmentation(
            "EGammaLRTKernel", AugmentationTools=LRTEGAugmentationTools
        )
    )

    # =======================================
    # ADD TOOLS : custom electron, photon and muon track isolation
    # =======================================
    from IsolationAlgs.DerivationTrackIsoConfig import DerivationTrackIsoCfg

    acc.merge(
        DerivationTrackIsoCfg(
            flags, object_types=("Electrons", "Muons"), postfix="LRT"
        )
    )

    if not hasattr(acc, "LRTElectronCaloIsolationBuilder"):
        from IsolationAlgs.IsolationSteeringDerivConfig import (
            LRTElectronIsolationSteeringDerivCfg,
        )

        acc.merge(LRTElectronIsolationSteeringDerivCfg(flags))

    from IsolationAlgs.IsolationBuilderConfig import egIsolationCfg

    acc.merge(
        egIsolationCfg(
            flags,
            name="electronIsolationLRT",
            # Avoid overlap with the previously-configured IsolationBuilder.
            noCalo=True,
            ElectronCollectionContainerName="LRTElectrons",
        )
    )

    return acc
