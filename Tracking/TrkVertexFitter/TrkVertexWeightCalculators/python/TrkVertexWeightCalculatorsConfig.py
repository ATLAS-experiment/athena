# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def DecorateVertexScoreAlgCfg(flags, name="DecorateVertexScoreAlg", **kwargs):
    """
    This algorithm decorates all the vertices with the score computed by a tool.
    """
    acc = ComponentAccumulator()
    if not kwargs.get("VertexWeightCalculator", None):
        from TrkConfig.TrkVertexWeightCalculatorsConfig import BDTVertexWeightCalculatorSeqCfg
        toolBDTAcc = BDTVertexWeightCalculatorSeqCfg(flags)
        tool = toolBDTAcc.popPrivateTools()
        acc.merge(toolBDTAcc)
        kwargs["VertexWeightCalculator"] = tool

    kwargs.setdefault("VertexScoreDecor", "score")
    alg = CompFactory.DecorateVertexScoreAlg(name, **kwargs)
    acc.addEventAlgo(alg)
    return acc


def TrkVertexWeightCalculatorDebugCfg(flags, **kwargs):
    """
    This is a test configuration for the TrkVertexWeightCalculator. It is not meant to be run in production.
    It produces a ROOT file with a tree containing relevant information to check the performance of the tool.
    """
    from AthenaCommon.Constants import DEBUG
    from AthenaCommon.Logging import logging

    mlog = logging.getLogger("TrkVertexWeightCalculatorBDTDebugCfg")
    mlog.warning(
        "This is a test algorithm, it is not meant to be run in production."
    )

    acc = ComponentAccumulator()
    acc.merge(DecorateVertexScoreAlgCfg(flags, **kwargs))
    mlog.info(
        "Setting the output level of BuildVertexPointingAlg, DecorateVertexScoreAlg, DecorateVertexScoreAlg/VertexSelectionTool to DEBUG"
    )
    acc.getEventAlgo("BuildVertexPointingAlg").OutputLevel = DEBUG
    acc.getEventAlgo("DecorateVertexScoreAlg").OutputLevel = DEBUG
    acc.getEventAlgo(
        "DecorateVertexScoreAlg"
    ).VertexWeightCalculator.OutputLevel = DEBUG

    from TrkConfig.TrkVertexWeightCalculatorsConfig import (
        SumPt2VertexWeightCalculatorCfg,
    )

    tool_pt2 = acc.popToolsAndMerge(SumPt2VertexWeightCalculatorCfg(flags))
    acc.merge(
        DecorateVertexScoreAlgCfg(
            flags,
            "DecorateVertexScoreAlgSumPt2",
            VertexWeightCalculator=tool_pt2,
            VertexScoreDecor="score_sumpt2",
        )
    )
    acc.getEventAlgo("DecorateVertexScoreAlgSumPt2").OutputLevel = DEBUG

    tool = CompFactory.Trk.TrueVertexDistanceWeightCalculator()
    acc.merge(
        DecorateVertexScoreAlgCfg(
            flags,
            "DecorateVertexScoreAlgTrueVertexDistance",
            VertexWeightCalculator=tool,
            VertexScoreDecor="score_true_vertex_distance",
        )
    )
    acc.getEventAlgo("DecorateVertexScoreAlgTrueVertexDistance").OutputLevel = DEBUG

    sysService = CompFactory.CP.SystematicsSvc("SystematicsSvc", sigmaRecommended=0)
    acc.addService(sysService)

    histoSvc = CompFactory.THistSvc(
        Output=[
            f"ANALYSIS DATAFILE='{flags.Output.HISTFileName}' TYPE='ROOT' OPT='RECREATE'"
        ]
    )
    acc.addService(histoSvc)
    acc.setAppProperty("HistogramPersistency", "ROOT")

    acc.addEventAlgo(
        CompFactory.CP.TreeMakerAlg("TreeMaker", TreeName=flags.Output.TreeName)
    )
    branches = [
        "EventInfo.runNumber -> runNumber",
        "EventInfo.eventNumber -> eventNumber",
        "EventInfo.actualInteractionsPerCrossing -> actualInteractionsPerCrossing",
        "EventInfo.averageInteractionsPerCrossing -> averageInteractionsPerCrossing",
        "PrimaryVertices.x -> vtx_x",
        "PrimaryVertices.y -> vtx_y",
        "PrimaryVertices.z -> vtx_z",
        "PrimaryVertices.score -> vtx_score",
        "PhotonPointingVertices.z -> z_common",
        "PhotonPointingVertices.nphotons_good -> nphotons_good",
        "PrimaryVertices.score_sumpt2 -> vtx_score_sumpt2",
        "PrimaryVertices.score_true_vertex_distance -> vtx_score_true_vertex_distance",
    ]
    acc.addEventAlgo(
        CompFactory.CP.AsgxAODNTupleMakerAlg(
            "NTupleMaker", TreeName=flags.Output.TreeName, Branches=branches
        )
    )
    acc.addEventAlgo(
        CompFactory.CP.TreeFillerAlg("TreeFiller", TreeName=flags.Output.TreeName)
    )

    return acc


def TrkVertexWeightCalculatorBDTDebugRunCfg():
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Exec.MaxEvents = 100
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags.Input.Files = defaultTestFiles.AOD_RUN3_MC
    flags.Output.HISTFileName = "test_tree.root"
    flags.addFlag("Output.TreeName", "tree")

    flags.fillFromArgs()
    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    acc.merge(PoolReadCfg(flags))

    acc.merge(TrkVertexWeightCalculatorDebugCfg(flags))
    acc.printConfig(withDetails=True, summariseProps=True)
    acc.store(open("TrkVertexWeightCalculatorBDTConfig.pkl", "wb"))
    return acc


if __name__ == "__main__":
    acc = TrkVertexWeightCalculatorBDTDebugRunCfg()
    status = acc.run()

    import sys

    sys.exit(not status.isSuccess())
