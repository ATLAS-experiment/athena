# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Low pT di-taus
def AddDiTauLowPtCfg(flags, **kwargs):
    """Configure the low-pt di-tau building"""

    acc = ComponentAccumulator()

    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.StandardLargeRJets import AntiKt10LCTopo_tau
    acc.merge(JetRecCfg(flags,AntiKt10LCTopo_tau))

    from DiTauRec.DiTauBuilderConfig import DiTauBuilderCfg
    acc.merge(DiTauBuilderCfg(flags, name="DiTauLowPtBuilder", doLowPt=True))

    return acc


def AddDiTauIDDecorationCfg(flags, **kwargs):
    """Decorate ditau ID scores """

    acc = ComponentAccumulator()

    import DiTauRec.DiTauToolsConfig as DiTauTools

    diTauOnnxScoreCalculator = acc.popToolsAndMerge(DiTauTools.DiTauOnnxScoreCalculatorCfg(
            flags,
            onnxModelPath = "TrigTauRec/00-11-02/dev/boosted_ditau_omni_model.onnx",
        ))

    diTauWPDecorator = acc.popToolsAndMerge(DiTauTools.DiTauWPDecoratorCfg(
            flags,
            ))

    kwargs.setdefault("DiTauContainerName", "DiTauJets")
    wpDecorationKeys = diTauWPDecorator.DecorWPNames
    decorWPCuts = diTauWPDecorator.DecorWPCuts

    acc.addEventAlgo(CompFactory.DerivationFramework.DiTauIDDecoratorWrapper(name = "DiTauIDDecorKernel",
                                                      DiTauContainerName = kwargs['DiTauContainerName'],
                                                      DiTauOnnxDiscriminantTool = diTauOnnxScoreCalculator,
                                                      DiTauWPDecorator = diTauWPDecorator,
                                                      WPDecorationKeys = wpDecorationKeys,
                                                      DecorWPCuts = decorWPCuts))

    return acc


def AddDiTauChargeDecoratorCfg(flags, **kwargs):
    """Decorate DiTau charge"""

    acc = ComponentAccumulator()
    prefix = kwargs.setdefault("DiTauContainerName", "DiTauJets")
    acc.addEventAlgo(CompFactory.DerivationFramework.DiTauChargeDecorator(name = f"{prefix}_DiTauChargeDecorKernel",
                                                DiTauContainerName = kwargs['DiTauContainerName']))

    return acc
