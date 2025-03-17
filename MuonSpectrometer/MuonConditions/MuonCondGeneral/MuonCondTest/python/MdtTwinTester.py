# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def MdtTwinCablingTestCfg(flags, name="MdtTwinMappingTestAlg", **kwargs):
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    the_alg = CompFactory.Muon.MdtTwinTubeTestAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result


if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    parser = SetupArgParser()
    parser.add_argument("--cablingJSON", help="Location of the twin tube cabling file to test", type=str, default="")
    parser.set_defaults(nEvents = 1)
    parser.set_defaults(noMM=True)
    parser.set_defaults(noSTGC=True)
    parser.set_defaults(noRpc=True)
    parser.set_defaults(noTgc=True)

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    from MuonConfig.MuonCablingConfig import MdtTwinTubeMapCondAlgCfg
    cfg.merge(MdtTwinTubeMapCondAlgCfg(flags, JSONFile = args.cablingJSON))
    cfg.merge(MdtTwinCablingTestCfg(flags))
    executeTest(cfg)
