# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# ********************************************************************
# EGammaLRTConfig.py
# Configures  all tools needed for LRT e-gamma object selection and sets
# up the kernel algorithms so the results can be accessed/written to
# the DAODs. Copied and modified from EGammaCommonConfig.py.
# Component accumulator version.
# ********************************************************************

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def EGammaLRTCfg(ConfigFlags):
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
    if ConfigFlags.Derivation.Egamma.addECIDS:
        LRTElectronPassECIDS = acc.getPrimaryAndMerge(
            EGElectronLikelihoodToolWrapperCfg(
                ConfigFlags,
                name="LRTElectronPassECIDS",
                EGammaElectronLikelihoodTool=ElectronChargeIDSelector,
                EGammaFudgeMCTool="",
                CutType="",
                StoreGateEntryName="DFCommonElectronsECIDS",
                ContainerName="LRTElectrons",
                StoreTResult=True,
            )
        )

    # decorate some electrons with an additional ambiguity flag
    # against internal and early material conversion
    from DerivationFrameworkEGamma.EGammaToolsConfig import EGElectronAmbiguityToolCfg

    LRTElectronAmbiguity = acc.getPrimaryAndMerge(
        EGElectronAmbiguityToolCfg(
            ConfigFlags,
            name="LRTElectronAdditionnalAmbiguity",
            idCut="DFCommonElectronsLHLooseNoPix",
            ContainerName="LRTElectrons",
            isMC=ConfigFlags.Input.isMC,
        )
    )

    # list of all the decorators so far
    LRTEGAugmentationTools = [LRTElectronAmbiguity]
    if ConfigFlags.Derivation.Egamma.addECIDS:
        LRTEGAugmentationTools.extend([LRTElectronPassECIDS])

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

        BkgLRTElectronClassificationTool = acc.getPrimaryAndMerge(
            BkgElectronClassificationCfg(
                ConfigFlags,
                name="BkgLRTElectronClassificationTool",
                MCTruthClassifierTool=BkgElectronMCTruthClassifier,
                ElectronContainerName="LRTElectrons",
            )
        )
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
            ConfigFlags, object_types=("Electrons", "Muons"), postfix="LRT"
        )
    )

    if not hasattr(acc, "LRTElectronCaloIsolationBuilder"):
        from IsolationAlgs.IsolationSteeringDerivConfig import (
            LRTElectronIsolationSteeringDerivCfg,
        )

        acc.merge(LRTElectronIsolationSteeringDerivCfg(ConfigFlags))

    from IsolationAlgs.IsolationBuilderConfig import egIsolationCfg

    acc.merge(
        egIsolationCfg(
            ConfigFlags,
            name="electronIsolationLRT",
            # Avoid overlap with the previously-configured IsolationBuilder.
            noCalo=True,
            ElectronCollectionContainerName="LRTElectrons",
        )
    )

    return acc
