# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def MdtCalibTestAlgCfg(flags, name="MdtCalibDbTestAlg", **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    result = ComponentAccumulator()
    result.addEventAlgo(CompFactory.Muon.MdtCalibTestAlg(name, **kwargs), primary = True)
    return result

if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    parser = SetupArgParser()
    parser.add_argument("--rtJSON", help="Location of the RT json file", default="")
    parser.add_argument("--t0JSON", help="Location of the T0 json file", default="")
    parser.set_defaults(nEvents = 1)
    parser.set_defaults(noMM=True)
    parser.set_defaults(noSTGC=True)
    parser.set_defaults(noRpc=True)
    parser.set_defaults(noTgc=True)
    #parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/R4SimHits.pool.root"])
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Muon.Calib.readMdtJSON = True

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args, flags)
    from MuonConfig.MuonCalibrationConfig import MdtCalibDbAlgCfg
    cfg.merge(MdtCalibDbAlgCfg(flags,RtJSON = args.rtJSON,  TubeT0JSON = args.t0JSON))
    cfg.merge(MdtCalibTestAlgCfg(flags))
    executeTest(cfg)
