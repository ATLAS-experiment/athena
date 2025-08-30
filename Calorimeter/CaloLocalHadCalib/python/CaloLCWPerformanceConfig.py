# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# CA config of job to get performance plots for local hadronic
# calibration on single pions


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory



def GetLCWPerfCfg(flags):

   cfg=ComponentAccumulator()

   lcPerf = CompFactory.GetLCSinglePionsPerf("LocalHadPerformance")

   # collection name to study cluster moments
   lcPerf.ClusterBasicCollName = "CaloTopoClusters"

   # collections names to study engReco wrt Truth after different correction steps
   lcPerf.ClusterCollectionNames = ["CaloTopoClusters", "CaloWTopoCluster", "CaloOOCTopoCluster", "CaloCalTopoClusters"]

   lcPerf.CalibrationHitContainerNames= ["LArCalibrationHitInactive","LArCalibrationHitActive","TileCalibHitActiveCell","TileCalibHitInactiveCell"]
   lcPerf.DMCalibrationHitContainerNames= ["LArCalibrationHitDeadMaterial_DEAD","LArCalibrationHitInactive_DEAD","LArCalibrationHitActive_DEAD","TileCalibHitDeadMaterial"]

   from math import pi
   lcPerf.DistanceCut = 1.5
   lcPerf.doEngRecOverTruth = True
   lcPerf.doEngTag = True
   lcPerf.doEngRecSpect = True
   lcPerf.doEngNoiseClus = True
   lcPerf.doClusMoments = True
   lcPerf.doRecoEfficiency = True
   lcPerf.etamin = 0.0
   lcPerf.etamax = 5.0
   lcPerf.netabin = 25
   lcPerf.phimin = -pi
   lcPerf.phimax = pi
   lcPerf.nphibin = 1
   lcPerf.logenermin = 2.0
   lcPerf.logenermax = 6.4
   lcPerf.nlogenerbin = 22
   lcPerf.useGoodClus = False
   lcPerf.usePionClustersOnly = False
   lcPerf.useRecoEfficiency = False

   lcPerf.OutputFileName = flags.LCW.outFileNamePerf

   cfg.addEventAlgo(lcPerf)

   return cfg


if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    from CaloLocalHadCalib.LCWConfigFlags import addLCWFlags
    addLCWFlags(flags)

    #flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc20e_13TeV/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.ESD.e4993_s3227_r12689/myESD.pool.root"]  
    #flags.Input.Files = ["/home/pavol/mc10_7TeV/ESD.371532._001990.pool.root.1"]  
    flags.Input.Files = ["/home/pavol/mc16_13TeV/ESD.29275544._000648.pool.root.1"]  
    flags.Output.ESDFileName="esdOut.pool.root"
    from AthenaConfiguration.Enums import LHCPeriod
    flags.GeoModel.Run = LHCPeriod.Run1
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN1_2010


    flags.fillFromArgs()

    # for debugging
    from AthenaCommon.Constants import DEBUG
    flags.Exec.OutputLevel=DEBUG

    # redo topo clusters on LC scale
    flags.Calo.TopoCluster.doTopoClusterLocalCalib = True
    flags.Calo.TopoCluster.doCellWeightCalib = False
    flags.Calo.TopoCluster.doCalibHitMoments = True

    flags.Digitization.HighGainEMECIW = False
    flags.Calo.Noise.fixedLumiForNoise=60*0.1724*50/25 # mu=60; dt=25ns
    flags.Calo.Noise.useCaloNoiseLumi=False

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg
    cfg.merge(CaloTopoClusterCfg(flags))

    topoAlg = cfg.getEventAlgo("CaloCalTopoClustersMaker")
    topoAlg.ClusterCorrectionTools.__getitem__("TopoCalibMoments").MatchDmType = 1 # 1=loose, 2=medium (default), 3=tight
    topoAlg.ClusterCorrectionTools.__getitem__("TopoCalibMoments").MomentsNames += ["ENG_CALIB_OUT_L"]
    topoAlg.ClusterCorrectionTools.__getitem__("TopoCalibMoments").CalibrationHitContainerNames = ["LArCalibrationHitInactive","LArCalibrationHitActive","TileCalibHitActiveCell","TileCalibHitInactiveCell"]
    topoAlg.ClusterCorrectionTools.__getitem__("TopoCalibMoments").DMCalibrationHitContainerNames = ["LArCalibrationHitDeadMaterial_DEAD","LArCalibrationHitInactive_DEAD","LArCalibrationHitActive_DEAD","TileCalibHitDeadMaterial"]

    # here add snapshots
    from CaloRec.CaloTopoClusterConfig import addSnapshot
    addSnapshot(topoAlg,"OOCPi0Calib","CaloOOCTopoCluster")
    addSnapshot(topoAlg,"LocalCalib","CaloWTopoCluster")

    cfg.merge(GetLCWPerfCfg(flags))

    # configure output
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    myItemList =  ['EventInfo#*', "McEventCollection#TruthEvent",
                 'xAOD::CaloClusterContainer#CaloCalTopoClusters',
                 'xAOD::CaloClusterAuxContainer#CaloCalTopoClustersAux.']
    
    cfg.merge( OutputStreamCfg(flags,"StreamDPD", ItemList=myItemList,
                               disableEventTag=True, takeItemsFromInput=False))

    # remove the default folders
    from IOVDbSvc.IOVDbSvcConfig import blockFolder
    blockFolder(cfg,"/CALO/Ofl/HadCalibration2/CaloEMFrac")
    blockFolder(cfg,"/CALO/Ofl/HadCalibration2/H1ClusterCellWeights")
    blockFolder(cfg,"/CALO/Ofl/HadCalibration2/CaloOutOfCluster")
    blockFolder(cfg,"/CALO/Ofl/HadCalibration2/CaloOutOfClusterPi0")
    blockFolder(cfg,"/CALO/Ofl/HadCalibration2/CaloDMCorr2")



    # LCW folders from sqlite files
    clFolders=( ("/CALO/HadCalibration2/CaloEMFrac<tag>CaloEMFrac2-R3S-2021-02-00-00-FTFP-BERT-DT25-EPOS-A3-OFC25-MU60</tag>", "myclwooc.db","CaloLocalHadCoeff"),
                ("/CALO/HadCalibration2/H1ClusterCellWeights<tag>CaloH1CellWeights2-R3S-2021-02-00-00-FTFP-BERT-DT25-EPOS-A3-OFC25-MU60</tag>", "myclwooc.db","CaloLocalHadCoeff"),
                ("/CALO/HadCalibration2/CaloOutOfCluster<tag>CaloHadOOCCorr2-R3S-2021-02-00-00-FTFP-BERT-DT25-EPOS-A3-OFC25-MU60</tag>", "myclwooc.db","CaloLocalHadCoeff"),
                ("/CALO/HadCalibration2/CaloOutOfClusterPi0<tag>CaloHadOOCCorrPi02-R3S-2021-02-00-00-FTFP-BERT-DT25-EPOS-A3-OFC25-MU60</tag>", "myclwooc.db","CaloLocalHadCoeff"),
                ("/CALO/HadCalibration2/CaloDMCorr2<tag>CaloHadDMCorr2-R3S-2021-02-00-00-FTFP-BERT-DT25-EPOS-A3-OFC25-MU60</tag>", "myDm.db","CaloLocalHadCoeff") )
    from IOVDbSvc.IOVDbSvcConfig import addFolderList
    cfg.merge(addFolderList(flags, clFolders, db="OFLP200"))

    # for debugging purposes:
    cfg.getService("StoreGateSvc").Dump=True
    cfg.getService("MessageSvc").defaultLimit=999999

    cfg.run(10)
