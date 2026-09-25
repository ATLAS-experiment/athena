# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def L0MuonEndcapAlgCfg(flags, name="L0MuonEndcapAlg", **kwargs):
    """Configure the initial TGC endcap data-flow boundary."""
    result = ComponentAccumulator()
    result.addEventAlgo(CompFactory.L0Muon.L0MuonEndcapAlg(name, **kwargs))
    return result


def L0MuonTGCChainCfg(flags, **kwargs):
    """Configure the minimal S1TGC-to-Endcap data-flow chain."""
    result = ComponentAccumulator()

    from L1MuonS1TGC.L0MuonS1TGCConfig import L0MuonTGCSimCfg

    result.merge(L0MuonTGCSimCfg(flags, **kwargs.pop("S1TGC", {})))
    result.merge(L0MuonEndcapAlgCfg(flags, **kwargs.pop("Endcap", {})))
    if kwargs:
        raise TypeError(f"Unexpected configuration groups: {sorted(kwargs)}")
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
    parser.set_defaults(nEvents=20)

    args = parser.parse_args()

    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Common.MsgSuppression = False
    flags.Output.RDOFileName = "L0MuonEndcap.RDO.pool.root"

    flags, acc = setupGeoR4TestCfg(args, flags)

    from AthenaCommon.Constants import DEBUG
    acc.merge(
        L0MuonTGCChainCfg(
            flags,
            S1TGC={"OutputLevel": DEBUG},
            Endcap={"OutputLevel": DEBUG},
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
                "xAOD::SectorLogicCandDataContainer#L0MuonTGCSectorLogicCandData",
                "xAOD::SectorLogicCandDataAuxContainer#L0MuonTGCSectorLogicCandDataAux.",
            ],
            takeItemsFromInput=False,
        )
    )

    executeTest(acc)
