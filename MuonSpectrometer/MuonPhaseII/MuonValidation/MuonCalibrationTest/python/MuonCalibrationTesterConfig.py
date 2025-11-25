# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MdtCalibDbAlgTestCfg(flags, name = "MdtCalibDbAlgTest", **kwargs):
    result = ComponentAccumulator()
    theAlg = CompFactory.MuonValR4.MdtCalibDbAlgTest(name, **kwargs)    
    result.addEventAlgo(theAlg, primary=True)
    return result

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, MuonPhaseIITestDefaults
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(outRootFile="MdtCalibDbAlgTest.root")
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R4)
    parser.set_defaults(defaultGeoFile="RUN4")
    parser.set_defaults(noMM=True)
    parser.set_defaults(noSTGC=True)
    parser.set_defaults(noRpc=True)    
    parser.set_defaults(noTgc=True)

    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = True
    flags, cfg = setupGeoR4TestCfg(args,flags)

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))

    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MdtCalibDbAlgTest"))


    cfg.merge(MdtCalibDbAlgTestCfg(flags))
    executeTest(cfg)


