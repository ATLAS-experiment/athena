#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration


### Simple script to test the conversion of the xAOD -> legacy Mdt sim hit format
if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    parser = SetupArgParser()
    parser.set_defaults(nEvents = 1000)
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/R4SimHits.pool.root"])

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    from xAODMuonSimHitCnv.MuonSimHitCnvCfg import MuonSimHitToMeasurementCfg
    cfg.merge(MuonSimHitToMeasurementCfg(flags))

    executeTest(cfg, num_events = args.nEvents)
  
