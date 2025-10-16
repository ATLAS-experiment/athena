# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def muonSPTesterCfg(flags, **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    from AthenaConfiguration.ComponentFactory import CompFactory

    from MuonSpacePointCalibrator.CalibrationConfig import MuonSpacePointCalibratorCfg
    kwargs.setdefault("Calibrator", result.popToolsAndMerge(MuonSpacePointCalibratorCfg(flags)))

    # from MuonConfig.MuonRecToolsConfig import SimpleMMClusterBuilderToolCfg
    # kwargs.setdefault("MMClusterBuilderTool", result.popToolsAndMerge(SimpleMMClusterBuilderToolCfg(flags)))

    result.addEventAlgo(CompFactory.MuonValR4.MuonSPCalibrationTest(name="MuonSPCalibrationTest", **kwargs)) 
    return result



if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest, MuonPhaseIITestDefaults
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(noSTGC=False)
    parser.set_defaults(noMM=False)
 
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)
   
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = True
    flags.Muon.doFastMMDigitization = True
    flags, cfg = setupGeoR4TestCfg(args,flags)

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))


    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))
    cfg.dropEventAlgo("MuonSpacePointMakerAlg")


    cfg.merge(muonSPTesterCfg(flags))

    executeTest(cfg) 