# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    # Required for MT using --threads argument
    flags.fillFromArgs(parser=flags.getArgumentParser())

    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc21_13p6TeV/ESDFiles/mc21_13p6TeV.421450.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep_fct.recon.ESD.e8445_e8447_s3822_r13565/ESD.28877240._000046.pool.root.1"]
    # Use latest MC21 tag to pick up latest muon folders apparently needed
    flags.IOVDb.GlobalTag = "OFLCOND-MC21-SDR-RUN3-10"
    calo_seeds=True

    import os
    if os.environ.get('ATHENA_CORE_NUMBER',None) is not None :
        flags.Concurrency.NumThreads = int(os.environ['ATHENA_CORE_NUMBER'])

    if not calo_seeds :
        flags.Tracking.doCaloSeededBrem=False
        flags.Tracking.doHadCaloSeededSSS=False
        flags.Tracking.doCaloSeededAmbi=False
        flags.Detector.EnableCalo=False

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    if calo_seeds :
        from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg
        acc.merge(CaloTopoClusterCfg(flags))

    if flags.Tracking.doTruth :
        from xAODTruthCnv.RedoTruthLinksConfig import RedoTruthLinksAlgCfg
        acc.merge( RedoTruthLinksAlgCfg(flags) )

    from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
    acc.merge(InDetTrackRecoCfg(flags))

    with open("config.pkl", "wb") as file:
      acc.store(file)

    acc.run(100)
