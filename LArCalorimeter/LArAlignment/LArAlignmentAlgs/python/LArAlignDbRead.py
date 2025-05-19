# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":

    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags=initConfigFlags()

    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

    flags.Input.Files=[]

    #flags.IOVDb.DBConnection = 'sqlite://;schema=LArAlign-2025-v0.db;dbname=CONDBR2'
    flags.IOVDb.DBConnection = "COOLONL_LAR/CONDBR2"

    flags.lock()

    cfg=MainServicesCfg(flags)

    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    cfg.merge(McEventSelectorCfg(flags,
            RunNumber=999999,
            EventsPerRun=1,
            FirstEvent=1,
            EventsPerLB=1,
            InitialTimeStamp=0,
            TimeStampInterval=1))

    from IOVDbSvc.IOVDbSvcConfig import addFolders
    cfg.merge(addFolders(flags,"/LAR/Align<tag>LARAlign-RUN2-UPD1-01</tag>"))
    
    cfg.addEventAlgo(CompFactory.LArAlignDbAlg())
   
    cfg.getService("PoolSvc").ReadCatalog += ["xmlcatalog_file:/afs/cern.ch/atlas/conditions/poolcond/catalogue/poolcond/PoolCat_comcond.xml"]

    cfg.run(1)
