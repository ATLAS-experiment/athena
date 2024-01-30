# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":

   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags = initConfigFlags()

   from AthenaCommon.Logging import log
   from AthenaCommon.Constants import DEBUG
   log.setLevel(DEBUG)

   from LArMonitoring.LArMonConfigFlags import addLArMonFlags
   flags.addFlagsCategory("LArMon", addLArMonFlags)

   from AthenaConfiguration.TestDefaults import defaultTestFiles
   flags.Input.Files = defaultTestFiles.RAW_RUN2

   flags.Output.HISTFileName = 'LArMonitoringOutput.root'
   flags.DQ.enableLumiAccess = False
   flags.DQ.useTrigger = False
   flags.lock()

   ## Cell building
   from CaloRec.CaloRecoConfig import CaloRecoCfg
   cfg=CaloRecoCfg(flags)

  #larCoverage monitoring
   from LArMonitoring.LArCoverageAlg import LArCoverageConfig
   cov_acc = LArCoverageConfig(flags)
   cfg.merge(cov_acc)

   #affectedRegions monitoring
   from LArMonitoring.LArAffectedRegionsAlg import LArAffectedRegionsConfig
   aff_acc = LArAffectedRegionsConfig(flags)
   cfg.merge(aff_acc)

   #collision time algo 
   from LArCellRec.LArCollisionTimeConfig import LArCollisionTimeCfg
   cfg.merge(LArCollisionTimeCfg(flags, cutIteration=False))

   # and collision time monitoring algo
   from LArMonitoring.LArCollisionTimeMonAlg import LArCollisionTimeMonConfig
   collmon=LArCollisionTimeMonConfig(flags)
   cfg.merge(collmon) 

   #ROD monitoring
   from LArMonitoring.LArRODMonAlg import LArRODMonConfig
   rodmon = LArRODMonConfig(flags)
   cfg.merge(rodmon)

   #Digit monitoring

   from LArCellRec.LArNoisyROSummaryConfig import LArNoisyROSummaryCfg
   cfg.merge(LArNoisyROSummaryCfg(flags))

   from LArMonitoring.LArDigitMonAlg import LArDigitMonConfig
   digimon = LArDigitMonConfig(flags)
   cfg.merge(digimon)


   flags.dump()
   f=open("LArMonMaker.pkl","w")
   cfg.store(f)
   f.close()

   #cfg.run(10)

