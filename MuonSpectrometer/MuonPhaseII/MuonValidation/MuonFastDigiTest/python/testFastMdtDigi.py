# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Small CA snippet to run the MDT fast-digi test on an xAOD file containing sim hits.
# Will schedule fast digitisation and subsequently dump the validation NTuple. 
if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest,setupHistSvcCfg
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/R4SimHits.pool.root"])

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    
    from xAODMuonSimHitCnv.MuonSimHitCnvCfg import xAODSimHitToMdtMeasCnvAlgCfg
    from MuonFastDigiTest.MuonFastDigiTesterConfig import MDTFastDigiTesterCfg

    cfg.merge(setupHistSvcCfg(flags,out_file=args.outRootFile,out_stream="MuonFastDigiTest"))
    cfg.merge(xAODSimHitToMdtMeasCnvAlgCfg(flags))
    cfg.merge(MDTFastDigiTesterCfg(flags))

    # output spam reduction
    cfg.getService("AthenaHiveEventLoopMgr").EventPrintoutInterval=500


    executeTest(cfg, args.nEvents)
    
