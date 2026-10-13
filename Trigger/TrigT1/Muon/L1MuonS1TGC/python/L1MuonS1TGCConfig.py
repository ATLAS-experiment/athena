# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def L1MuonTGCSimCfg(
        flags,
        name="L1Muon.TGCSimulation",
        configureHistSvc=True,
        **kwargs):

    result = ComponentAccumulator()

    if "CandidateBuilderTool" not in kwargs:
        from L1MuonS1TGCFloatingTools.L1MuonS1TGCFloatingToolsConfig import (
            TgcSLFloatingCandidateBuilderToolCfg,
        )
        kwargs["CandidateBuilderTool"] = result.popToolsAndMerge(
            TgcSLFloatingCandidateBuilderToolCfg(flags)
        )

    if "InnerCoincidenceTool" not in kwargs:
        from L1MuonS1TGCFloatingTools.L1MuonS1TGCFloatingToolsConfig import (
            TgcSLFloatingInnerCoincidenceToolCfg,
        )
        kwargs["InnerCoincidenceTool"] = result.popToolsAndMerge(
            TgcSLFloatingInnerCoincidenceToolCfg(flags)
        )

    if "TrackSelectorTool" not in kwargs:
        from L1MuonS1TGCFloatingTools.L1MuonS1TGCFloatingToolsConfig import (
            TgcSLFloatingTrackSelectorToolCfg,
        )
        kwargs["TrackSelectorTool"] = result.popToolsAndMerge(
            TgcSLFloatingTrackSelectorToolCfg(flags)
        )

    alg = CompFactory.L1Muon.TGCSimulation(name=name, **kwargs)

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, "MonTool")
    monTool.defineHistogram(
        "nTgcRdoCollections",
        path="EXPERT",
        type="TH1F",
        title=";TGC RDO collections;Events",
        xbins=50,
        xmin=0,
        xmax=100,
    )
    alg.MonTool = monTool

    result.addEventAlgo(alg)
    if configureHistSvc:
        histSvc = CompFactory.THistSvc(
            Output=["EXPERT DATAFILE='" + name + ".root' OPT='RECREATE'"]
        )
        result.addService(histSvc)
    return result


if __name__ == "__main__":

    from MuonGeoModelTestR4.testGeoModel import (
        setupGeoR4TestCfg,
        SetupArgParser,
        MuonPhaseIITestDefaults,
    )
    from MuonConfig.MuonConfigUtils import executeTest
    parser = SetupArgParser()
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.RDO_R3)
    parser.set_defaults(nEvents=1000)

    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Common.MsgSuppression = False
    flags.Output.RDOFileName = "L1MuonS1TGC.RDO.pool.root"

    flags, acc = setupGeoR4TestCfg(args, flags)
    from AthenaCommon.Constants import DEBUG
    from L1MuonS1TGCFloatingTools.L1MuonS1TGCFloatingToolsConfig import (
        TgcSLFloatingCandidateBuilderToolCfg,
    )

    candidateBuilderTool = acc.popToolsAndMerge(
        TgcSLFloatingCandidateBuilderToolCfg(flags)
    )
    acc.merge(
        L1MuonTGCSimCfg(
            flags,
            OutputLevel=DEBUG,
            CandidateBuilderTool=candidateBuilderTool,
        )
    )

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg

    acc.merge(
        OutputStreamCfg(
            flags,
            "RDO",
            ItemList=[
                "xAOD::L1TGCCandDataContainer#L1TGCCandData",
                "xAOD::L1TGCCandDataAuxContainer#L1TGCCandDataAux.",
            ],
            takeItemsFromInput=False,
        )
    )

    executeTest(acc)
