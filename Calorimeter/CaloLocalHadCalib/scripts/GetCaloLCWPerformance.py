#!/usr/bin/env python
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

if __name__=='__main__':

   import os,sys
   import argparse
   import subprocess
   from AthenaCommon import Logging
   log = Logging.logging.getLogger( 'GetCaloLCWPerformanc' )


   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags = initConfigFlags()
   from CaloLocalHadCalib.LCWConfigFlags import addLCWFlags
   addLCWFlags(flags)

   parser = flags.getArgumentParser()
   # here we extend the parser
   parser.add_argument('--doLocalCalib', default=False, help="Run LCW during reclustering",action="store_true")
   parser.add_argument('--doCalibHitMoments', default=False, help="Run calib moments during reclustering",action="store_true")
   parser.add_argument('--doCellWeight', default=False, help="Run cell weighting during reclustering",action="store_true")

   parser.add_argument('--noiseLumi', default=60*0.1724*50/25, help="Lumi used for noise computation (mu*0.1724*50/bunch_spacing)",type=float)

   args = flags.fillFromArgs(parser=parser)

   if help in args and args.help is not None and args.help:
      parser.print_help()
      sys.exit(0)
   
   for _, value in args._get_kwargs():
      if value is not None:
          log.debug(value)
   

   # now set flags according parsed options

   # for debugging
   from AthenaCommon.Constants import DEBUG
   flags.Exec.OutputLevel=DEBUG

   # redo topo clusters on LC scale
   flags.Calo.TopoCluster.doTopoClusterLocalCalib = args.doLocalCalib
   flags.Calo.TopoCluster.doCellWeightCalib = args.doCellWeight
   flags.Calo.TopoCluster.doCalibHitMoments = args.doCalibHitMoments

   flags.Digitization.HighGainEMECIW = False
   flags.Calo.Noise.fixedLumiForNoise = args.noiseLumi
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

   from CaloLocalHadCalib.CaloLCWPerformanceConfig import GetLCWPerfCfg
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
