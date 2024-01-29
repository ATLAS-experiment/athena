#!/usr/bin/env python
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

if __name__=='__main__':

   import os,sys,getopt

   if len(sys.argv)>1 and (sys.argv[1]=="-h" or sys.argv[1]=="--help"):
        print("Usage:")
        print(" ")
        print("StateLessPT_TestFile_NewConfig.py {--input=<infile>} {--config=XXX} {--stream=YYY} {--noPost} {--postFreq=ZZ}")
        print("                         default input: /det/dqm/GlobalMonitoring/SMW_test/testpart_sample/data21_900GeV.00405543.physics_MinBias.daq.RAW._lb1977._SFO-6._0001.data")
        print("                         default XXX: LArMon")
        print("                         default YYY: ''")
        print("                         --noPost is switching off postProcessing")
        print("                         default ZZ: 10") 

   #some defaults
   #INPUT = '/det/dqm/GlobalMonitoring/SMW_test/testpart_sample/data21_900GeV.00405543.physics_MinBias.daq.RAW._lb1977._SFO-6._0001.data'
   # At P1 use:     
   #INPUT = '/detwork/dqm/GlobalMonitoring_test_data/data22_13p6TeV.00428353.express_express.merge.RAW._lb0479._SFO-ALL._0001.1'
   INPUT = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/data18_13TeV/data18_13TeV.00357750.physics_Main.daq.RAW/data18_13TeV.00357750.physics_Main.daq.RAW._lb0083._SFO-1._0001.data'
   CONFIG = 'LArMon'
   STREAM = "NONE"
   DOPOSTPROC = True
   POSTFREQ = 10
   if len(sys.argv)>1:
      for ii in range(1,len(sys.argv)):
         print(ii," ",sys.argv[ii])
      try:
         opts,args=getopt.getopt(sys.argv[1:],"t:",["config=","stream=","postFreq=","noPost"])
      except getopt.GetoptError as e:
         print("Failed to interpret arguments")
         print(e)
         sys.exit(-1)
         pass

      for o,a in opts:
          if o=="--config": CONFIG=a
          if o=="--stream": STREAM=a
          if o=="--input":  INPUT=a
          if o=="--noPost": DOPOSTPROC=False
          if o=="--postFreq": POSTFREQ=int(a)

   # #####################################################
   #  Read jobOpt configuration options
   # #####################################################

   ## Select the monitoring tool
   if 'CONFIG' not in dir():
       CONFIG = 'LArMon'
    
   ## Select events based on stream name
   if "STREAM" not in dir():
       STREAM = "NONE"
    
   from AthenaConfiguration.Enums import Format

   # #####################################################
   # Get environment variables
   # #####################################################

   partition = os.environ.get("TDAQ_PARTITION")
   
   from ispy import IPCPartition, ISObject

   # ################################
   # To read run parameters from IS
   # ################################
   
   if partition is not None:
      p = IPCPartition(partition)
      if not p.isValid():
         print("Partition:",p.name(),"is not valid")
         sys.exit(1)
      
      ### ATLAS partition: Read Global Run Parameters to configure the jobs
      if partition == "ATLAS":
          try:
              y = ISObject(p, 'RunParams.SOR_RunParams', 'RunParams')
          except:
              print("Could not find Run Parameters in IS - Set default beam type to 'cosmics'")
              beamType='cosmics'
          else:
              y.checkout()
              beamtype = y.beam_type
              beamenergy = y.beam_energy
              runnumber = y.run_number
              project = y.T0_project_tag
              print("RUN CONFIGURATION: beam type %i, beam energy %i, run number %i, project tag %s"%(beamtype,beamenergy,runnumber,project))    
              # define beam type based on project tag name
              if project[7:10]=="cos":
                  beamType='cosmics'
              else:
                  beamType='collisions'
                  
      ### LAr TEST partition
      else:
          beamType='collisions'
      
      ### Read LAr Parameters
      try:
         x = ISObject(p, 'LArParams.LAr.RunLogger.GlobalParams', 'GlobalParamsInfo')
      except:
         print("Couldn not find IS Parameters - Set default flag")
         ReadDigits = False
         FirstSample = 3
         NSamples = 4
         LArFormat = 1
      else:
         try:
            x.checkout()
         except:
            print("Couldn not find IS Parameters - Set default flag")
            ReadDigits = False
            FirstSample = 3
            NSamples = 4
            LArFormat = 1
         else:
            RunType = x.runType
            LArFormat = x.format
            FirstSample = x.firstSample
            NSamples = x.nbOfSamples   
            print("RUN CONFIGURATION: format %i,run type %i"%(RunType,LArFormat))
            # Decide how to get cell energy: DSP or digits
            if LArFormat==0:
                ReadDigits = True
            else:
                ReadDigits = False
   else: # defaults when reading from file
      ReadDigits = False
      FirstSample = 3
      NSamples = 4
      LArFormat = 1
      ReadDigits = False
      beamType='collisions'

   print("RUN CONFIGURATION: ReadDigits =", ReadDigits)
   
   ## And now CA
   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags = initConfigFlags()
   from AthenaMonitoring.DQConfigFlags import allSteeringFlagsOff
   allSteeringFlagsOff(flags)

   ### Set Beam Type
   from AthenaConfiguration.Enums import BeamType
   if beamType=='collisions':
      flags.Beam.Type=BeamType.Collisions
   elif beamType=='cosmics':
      flags.Beam.Type=BeamType.Cosmics   
   else:   
      print('Setting default collisions beam type')
      flags.Beam.Type=BeamType.Collisions
   flags.Beam.BunchSpacing=25
   print("RUN CONFIGURATION: Beamtype =",flags.Beam.Type)
   
   flags.Common.isOnline=True
   flags.Input.Format=Format.BS
   flags.Input.isMC=False

   flags.IOVDb.DatabaseInstance="CONDBR2"
   flags.IOVDb.GlobalTag="CONDBR2-ES1PA-2016-03"

   flags.GeoModel.Layout="alas"
   from AthenaConfiguration.TestDefaults import defaultGeometryTags
   flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN2

   #Run clustering w/o calibration
   flags.Calo.TopoCluster.doTopoClusterLocalCalib=False

   flags.Exec.MaxEvents=-1

   from AthenaConfiguration.AutoConfigOnlineRecoFlags import setDefaultOnlineRecoFlags
   setDefaultOnlineRecoFlags(flags)

   from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
   setupDetectorFlags(flags, ['LAr'], toggle_geometry=True)

   flags.Trigger.doID=False
   flags.Trigger.doMuon=False
   flags.Trigger.L1.doMuon=False
   flags.Trigger.L1.doTopo=False
   flags.Trigger.triggerConfig='COOL'

   flags.DQ.doMonitoring=True
   flags.DQ.disableAtlasReadyFilter=True
   flags.DQ.enableLumiAccess=False
   flags.DQ.useTrigger=True
   # for P1
   flags.DQ.FileKey=''
   
   flags.LAr.doAlign=False
   flags.LAr.doHVCorr=False

   flags.Calo.TopoCluster.doTopoClusterLocalCalib=False

   flags.Input.Files=[INPUT]

   #test multithreads
   flags.Concurrency.NumThreads=4
   flags.Concurrency.NumConcurrentEvents=4

   def __monflags():
      from LArMonitoring.LArMonConfigFlags import createLArMonConfigFlags
      return createLArMonConfigFlags()

   flags.addFlagsCategory("LArMon", __monflags)

   if 'CaloMon' in CONFIG: # needs Lumi access
      flags.DQ.enableLumiAccess=True

   flags.lock()

   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   acc = MainServicesCfg(flags)
 
   from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
   acc.merge( TileRawDataReadingCfg(flags) )


   #from RecoPT_NewConfig import LArMonitoringConfig
   # include("RecoPT_NewConfig.py")
   from LArMonitoring.RecoPT_NewConfig import LArMonitoringConfig
   acc.merge(LArMonitoringConfig(flags,CONFIG,STREAM))
   
   # somehow needs to add postprocessing
   if DOPOSTPROC:
      from DataQualityUtils.DQPostProcessingAlg import DQPostProcessingAlg
      ppa = DQPostProcessingAlg("DQPostProcessingAlg")
      ppa.ExtraInputs = {( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' )}
      ppa.Interval = POSTFREQ
      if flags.Common.isOnline:
         ppa.FileKey = ((flags.DQ.FileKey + '/') if not flags.DQ.FileKey.endswith('/') 
                        else flags.DQ.FileKey) 

      acc.addEventAlgo(ppa, sequenceName='AthEndSeq')

   acc.run()
