# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Low pT di-taus
def AddDiTauLowPtCfg(flags, **kwargs):
    """Configure the low-pt di-tau building"""

    acc = ComponentAccumulator()

    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.StandardLargeRJets import AntiKt10LCTopo
    acc.merge(JetRecCfg(flags,AntiKt10LCTopo))

    from DiTauRec.DiTauBuilderConfig import DiTauBuilderCfg
    acc.merge(DiTauBuilderCfg(flags, name="DiTauLowPtBuilder", doLowPt=True))

    return acc


def AddDiTauIDDecorationCfg(flags, **kwargs):
    """Decorate ditau ID scores """

    acc = ComponentAccumulator()

    import DiTauRec.DiTauToolsConfig as DiTauTools

    diTauOnnxScoreCalculator = acc.popToolsAndMerge(DiTauTools.DiTauOnnxScoreCalculatorCfg(
            flags,
            onnxModelPath                   = "TrigTauRec/00-11-02/dev/boosted_ditau_omni_model.onnx",
        ))

    diTauWPDecorator = acc.popToolsAndMerge(DiTauTools.DiTauWPDecoratorCfg(
            flags,
            ))

    kwargs.setdefault("DiTauContainerName", "DiTauJets")
    wpDecorationKeys = diTauWPDecorator.DecorWPNames
    decorWPCuts = diTauWPDecorator.DecorWPCuts
    acc.addPublicTool(diTauOnnxScoreCalculator)
    acc.addPublicTool(diTauWPDecorator)

    DiTauIDDecoratorWrapper = CompFactory.DerivationFramework.DiTauIDDecoratorWrapper
    DiTauIDDecoratorKernel = CompFactory.DerivationFramework.CommonAugmentation

    DiTauIDDecoratorWrapper = DiTauIDDecoratorWrapper(name               = "DiTauIDDecoratorWrapper",
                                                      DiTauContainerName = kwargs['DiTauContainerName'],
                                                      DiTauOnnxDiscriminantTool = diTauOnnxScoreCalculator,
                                                      DiTauWPDecorator = diTauWPDecorator,
                                                      WPDecorationKeys = wpDecorationKeys,
                                                      DecorWPCuts = decorWPCuts)

    acc.addPublicTool(DiTauIDDecoratorWrapper)
    acc.addEventAlgo(DiTauIDDecoratorKernel(name              = "DiTauIDDecorKernel",
                                            AugmentationTools = [DiTauIDDecoratorWrapper]))
    return acc


def AddDiTauChargeDecoratorCfg(flags, **kwargs):
    """Decorate DiTau charge"""

    kwargs.setdefault("DiTauContainerName", "DiTauJets")
    kwargs.setdefault("prefix",           kwargs['DiTauContainerName'])

    acc = ComponentAccumulator()

    DiTauChargeDecorator = CompFactory.DerivationFramework.DiTauChargeDecorator
    DiTauChargeDecoratorKernel = CompFactory.DerivationFramework.CommonAugmentation

    prefix = kwargs['prefix']
    diTauChargeDecorator = DiTauChargeDecorator(name               = f"{prefix}_DiTauChargeDecorator",
                                                DiTauContainerName = kwargs['DiTauContainerName'])
    acc.addPublicTool(diTauChargeDecorator)
    acc.addEventAlgo(DiTauChargeDecoratorKernel(name              = f"{prefix}_DiTauIDDecorKernel",
                                                AugmentationTools = [diTauChargeDecorator]))

    return acc
