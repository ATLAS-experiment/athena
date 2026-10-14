# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def L1MuonEndcapAlgCfg(flags, name="L1MuonEndcapAlg", **kwargs):
    """Configure the initial TGC endcap data-flow boundary."""
    result = ComponentAccumulator()
    result.addEventAlgo(CompFactory.L1Muon.L1MuonEndcapAlg(name, **kwargs))
    return result


def L1MuonTGCChainCfg(flags, **kwargs):
    """Configure the minimal S1TGC-to-Endcap data-flow chain."""
    result = ComponentAccumulator()

    from L1MuonS1TGC.L1MuonS1TGCConfig import L1MuonTGCSimCfg

    result.merge(L1MuonTGCSimCfg(flags, **kwargs.pop("S1TGC", {})))
    result.merge(L1MuonEndcapAlgCfg(flags, **kwargs.pop("Endcap", {})))
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
    flags.Output.RDOFileName = "L1MuonEndcap.RDO.pool.root"

    flags, acc = setupGeoR4TestCfg(args, flags)

    from AthenaCommon.Constants import DEBUG
    acc.merge(
        L1MuonTGCChainCfg(
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
                "xAOD::L1TGCCandDataContainer#L1TGCCandData",
                "xAOD::L1TGCCandDataAuxContainer#L1TGCCandDataAux.",
                "xAOD::SectorLogicCandDataContainer#L1EndcapCandData",
                "xAOD::SectorLogicCandDataAuxContainer#L1EndcapCandDataAux.",
            ],
            takeItemsFromInput=False,
        )
    )

    executeTest(acc)
