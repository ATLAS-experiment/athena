# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def L0MuonTGCSimCfg(flags, name="L0Muon.TGCSimulation", **kwargs):

    result = ComponentAccumulator()

    if "CandidateBuilderTool" not in kwargs:
        from L0MuonS1TGCFloatingTools.L0MuonS1TGCFloatingToolsConfig import (
            TgcL0FloatingCandidateBuilderToolCfg,
        )
        kwargs["CandidateBuilderTool"] = result.popToolsAndMerge(
            TgcL0FloatingCandidateBuilderToolCfg(flags)
        )

    if "InnerCoincidenceTool" not in kwargs:
        from L0MuonS1TGCFloatingTools.L0MuonS1TGCFloatingToolsConfig import (
            TgcL0FloatingInnerCoincidenceToolCfg,
        )
        kwargs["InnerCoincidenceTool"] = result.popToolsAndMerge(
            TgcL0FloatingInnerCoincidenceToolCfg(flags)
        )

    if "TrackSelectorTool" not in kwargs:
        from L0MuonS1TGCFloatingTools.L0MuonS1TGCFloatingToolsConfig import (
            TgcL0FloatingTrackSelectorToolCfg,
        )
        kwargs["TrackSelectorTool"] = result.popToolsAndMerge(
            TgcL0FloatingTrackSelectorToolCfg(flags)
        )

    alg = CompFactory.L0Muon.TGCSimulation(name=name, **kwargs)

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

    histSvc = CompFactory.THistSvc(
        Output=["EXPERT DATAFILE='" + name + ".root' OPT='RECREATE'"]
    )
    result.addEventAlgo(alg)
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
    flags.Output.RDOFileName = "L0MuonS1TGC.RDO.pool.root"

    flags, acc = setupGeoR4TestCfg(args, flags)
    from AthenaCommon.Constants import DEBUG
    from L0MuonS1TGCFloatingTools.L0MuonS1TGCFloatingToolsConfig import (
        TgcL0FloatingCandidateBuilderToolCfg,
    )

    candidateBuilderTool = acc.popToolsAndMerge(
        TgcL0FloatingCandidateBuilderToolCfg(
            flags,
            EnableTruthValidation=True,
        )
    )
    acc.merge(
        L0MuonTGCSimCfg(
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
                "xAOD::TGCCandDataContainer#L0MuonTGCCandData",
                "xAOD::TGCCandDataAuxContainer#L0MuonTGCCandDataAux.",
            ],
            takeItemsFromInput=False,
        )
    )

    executeTest(acc)
