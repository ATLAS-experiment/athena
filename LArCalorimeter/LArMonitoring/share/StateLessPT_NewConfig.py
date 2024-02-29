#!/usr/bin/env python
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

if __name__=='__main__':
   import os,sys,getopt

   if len(sys.argv)>1 and (sys.argv[1]=="-h" or sys.argv[1]=="--help"):
        print("Usage:")
        print(" ")
        print("StateLessPT_NewConfig.py {--config XXX} {--stream YYY} {--run Z}")
        print("                         default XXX: LArMon")
        print("                         default YYY: ''")
        print("                         default Z: '435946'")

   # #####################################################
   #  Read jobOpt configuration options
   # #####################################################

   #some defaults
   CONFIG = 'LArMon'
   STREAM = "NONE"
   RUNN   = 448285
   if len(sys.argv)>1:
      for ii in range(1,len(sys.argv)):
         print(ii," ",sys.argv[ii])
      try:
         opts,args=getopt.getopt(sys.argv[1:],"t:",["config=","stream=","run="])
      except getopt.GetoptError as e:
         print("Failed to interpret arguments")
         print(e)
         sys.exit(-1)
         pass

      for o,a in opts:
          if o=="--config": 
             print("Got option --config: ",a)
             CONFIG=a
          if o=="--stream": STREAM=a
          if o=="--run": RUNN=int(a)

   from AthenaConfiguration.Enums import Format

   # #####################################################
   # Get environment variables
   # #####################################################

   partition = os.environ.get("TDAQ_PARTITION")
   
   from ispy import IPCPartition, ISObject

   # ################################
   # To read run parameters from IS
   # ################################
   p = IPCPartition(partition)
   if not p.isValid():
      print("Partition:",p.name(),"is not valid")
      sys.exit(1)
   
   ### ATLAS partition: Read Global Run Parameters to configure the jobs
   from AthenaConfiguration.Enums import BeamType
   runnumber = RUNN
   if partition == "ATLAS" or partition[0:3] == "LAr":
       try:
           y = ISObject(p, 'RunParams.SOR_RunParams', 'RunParams')
       except:
           print("Could not find Run Parameters in IS - Set default beam type to 'cosmics'")
           beamType=BeamType.Cosmics
       else:
           y.checkout()
           beamtype = y.beam_type
           beamenergy = y.beam_energy
           runnumber = y.run_number
           project = y.T0_project_tag
           print("RUN CONFIGURATION: beam type %i, beam energy %i, run number %i, project tag %s"%(beamtype,beamenergy,runnumber,project))    
           # define beam type based on project tag name
           if project[7:10]=="cos":
               beamType=BeamType.Cosmics
           else:
               beamType=BeamType.Collisions
               
   ### LAr TEST partition
   else:
       beamType=BeamType.Collisions
   
   ### Read LAr Parameters
   try:
      x = ISObject(p, 'LArParams.LAr.RunLogger.GlobalParams', 'GlobalParamsInfo')
   except:
      print("Could not find IS Parameters - Set default flag")
      ReadDigits = False
      FirstSample = 3
      NSamples = 4
      LArFormat = 1
      RunType = 2
      #for splash test
      #ReadDigits = True
      #FirstSample = 6
      #NSamples = 32
      #LArFormat = 0
      #RunType = 0
   else:
      try:
         x.checkout()
      except:
         print("Couldn not find IS Parameters - Set default flag")
         ReadDigits = False
         FirstSample = 3
         NSamples = 4
         LArFormat = 1
         RunType = 2
         #for splash test
         #ReadDigits = True
         #FirstSample = 6
         #NSamples = 32
         #LArFormat = 0
         #RunType = 0
      else:
         RunType = x.runType
         LArFormat = x.format
         FirstSample = x.firstSample
         NSamples = x.nbOfSamples   
         RunType = x.runType
         print("RUN CONFIGURATION: format %i,run type %i"%(RunType,LArFormat))
         # Decide how to get cell energy: DSP or digits
         if LArFormat==0:
             ReadDigits = True
         else:
             ReadDigits = False
   
   print("RUN CONFIGURATION: ReadDigits =", ReadDigits)

   ## And now CA
   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags = initConfigFlags()
   from AthenaMonitoring.DQConfigFlags import allSteeringFlagsOff
   allSteeringFlagsOff(flags)

   ### Set Beam Type
   flags.Beam.Type=beamType 
   flags.Beam.BunchSpacing=25
   print("RUN CONFIGURATION: Beamtype =",flags.Beam.Type)
   
   flags.Common.isOnline=True
   flags.Input.Format=Format.BS
   flags.Input.isMC=False


   from AthenaCommon.Constants import WARNING
   flags.Exec.MaxEvents=-1

   from AthenaConfiguration.AutoConfigOnlineRecoFlags import autoConfigOnlineRecoFlags
   autoConfigOnlineRecoFlags(flags,partition)

   flags.IOVDb.DatabaseInstance="CONDBR2"
   flags.IOVDb.GlobalTag="CONDBR2-ES1PA-2023-01"

   flags.GeoModel.Layout="atlas"
   from AthenaConfiguration.TestDefaults import defaultGeometryTags
   flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN2

   #overwrite the run number 
   flags.Input.RunNumbers=[runnumber]
   # overwrite LB number for playback partition if needed....
   flags.Input.LumiBlockNumbers=[0]

   from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
   setupDetectorFlags(flags, ['LAr'], toggle_geometry=True)

   flags.Trigger.doID=False
   flags.Trigger.doMuon=False
   flags.Trigger.L1.doMuon=False
   flags.Trigger.L1.doTopo=False
   flags.Trigger.doHLT=False
   flags.Trigger.doLVL1=False
   flags.Trigger.Online.isPartition=True
   flags.Trigger.triggerConfig='DB'
   flags.Trigger.DecisionMakerValidation.Execute=False
   flags.Trigger.enableL1CaloPhase1=False

   flags.DQ.doMonitoring=True
   flags.DQ.disableAtlasReadyFilter=True
   flags.DQ.useTrigger=True
   flags.DQ.FileKey=''
   flags.DQ.Environment='online'
   
   flags.LAr.doAlign=False
   flags.LAr.doHVCorr=False

   # in case to debug FPE:
   #flags.Exec.FPE=3
   #if 'DTMon' in CONFIG:
   #   flags.Exec.FPE=5

   if RunType == 0:
      flags.LAr.ROD.forceIter=True

   flags.Calo.TopoCluster.doTopoClusterLocalCalib=False

   def __monflags():
      from LArMonitoring.LArMonConfigFlags import createLArMonConfigFlags
      return createLArMonConfigFlags()

   flags.addFlagsCategory("LArMon", __monflags)

   if STREAM=="CosmicCalo" or STREAM=="LArCellsEmpty":
      flags.LArMon.doLArRawMonitorSignal=True

   if 'CaloMon' in CONFIG: # needs Lumi access
      flags.DQ.enableLumiAccess=False
   else:
      flags.DQ.enableLumiAccess=True


   flags.lock()

   flags.dump()

   # taken from future EmonByteStreamConfig
   from AthenaConfiguration.ComponentFactory import CompFactory
   from SGComps.SGInputLoaderConfig import SGInputLoaderCfg

   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   acc = MainServicesCfg(flags)
 
   bytestream_conversion = CompFactory.ByteStreamCnvSvc()
   acc.addService(bytestream_conversion, primary=True)

   from ByteStreamEmonSvc.EmonByteStreamConfig import EmonByteStreamCfg
   acc.merge(EmonByteStreamCfg(flags))

   bytestream_input = acc.getService("ByteStreamInputSvc")

   # ############################################################
   # The name of the partition you want to connect to is taken
   # from 'TDAQ_PARTITION' if it exists, otherwise from here.
   # ############################################################

   bytestream_input.Partition = partition
   
   # #########################################
   # The source of events, dcm for full events.
   # #########################################
   
   if partition == "ATLAS" or partition[:3] == "LAr":
      bytestream_input.Key = "dcm"
   else:
      bytestream_input.Key = "CompleteEvent"
   
   bytestream_input.KeyCount = 15
   bytestream_input.KeyCount = 150
   bytestream_input.UpdatePeriod = 200
   #
   # #######################################
   # Set this to the IS server where you want
   # to publish histograms, too. If unset, no histograms 
   # are published.
   # #######################################
   
   bytestream_input.ISServer = 'LArHistogramming'
   
   # ########################################
   # The provider name under which your histograms
   #  appear in OH.
   # ########################################
   
   if STREAM!='NONE':
     bytestream_input.PublishName = '%s-%s'%(CONFIG,STREAM)
   else:
     bytestream_input.PublishName = '%s-%s'%(CONFIG,"NoTrigSel")
   
   # ###################################################
   # Should histograms be cleared at new run ? default: yes
   # Moved to False 27th Jan 2010 - Reset for each event in HLT 15.5.5
   # ###################################################
   
   bytestream_input.ClearHistograms = True
   
   # #########################
   # To get all Beam Splashes this needs uncomment
   # #########################
   
   #bytestream_input.Dispersion=False
   
   # ###############################################
   # Frequency of updates (in number of events, not secs...)
   # ###############################################
   
   bytestream_input.Frequency = 5
   
   # #######################################################
   # TimeOut (in milliseconds) - Prevent job with low rates 
   # to reconnect too often to SFIs
   # ######################################################
   
   bytestream_input.Timeout = 3600000
   
   # ###################################################
   #          Stream/trigger Selection
   # --> For LAr partitions, no stream available
   # --> For ATLAS, can create streams from L1 items
   # ##################################################

   if bytestream_input.Partition[:3] == "LAr":
       bytestream_input.StreamType = ""
   else:
       bytestream_input.LVL1Logic   = "Or"
       bytestream_input.LVL1Origin  = "TAV"
       bytestream_input.StreamType = "physics"
       bytestream_input.StreamLogic = "And"
   if  STREAM!="NONE":
       if  STREAM=="LArCellsEmpty":
           bytestream_input.StreamType = "calibration"
           bytestream_input.StreamNames = ['LArCellsEmpty']
       if STREAM=="Main":
           bytestream_input.StreamNames = ['Main']
       if STREAM=="CosmicCalo":
           bytestream_input.StreamNames = ['CosmicCalo']
       if STREAM=="L1Topo":
           bytestream_input.StreamNames = ['L1Topo']
       if STREAM=="L1Calo":
           bytestream_input.StreamNames = ['L1Calo']
       if STREAM=="express":
           bytestream_input.StreamNames = ['express']
           bytestream_input.StreamType = "express"
       if  STREAM=="LArNoiseBurst":
           bytestream_input.StreamType = "calibration"
           bytestream_input.StreamNames = ['LArNoiseBurst']
       if STREAM=="Standby":
           bytestream_input.StreamNames = ['Standby']
       if STREAM=="ZeroBias":
           bytestream_input.StreamNames = ['ZeroBias']
       if STREAM=="MinBias":
           bytestream_input.StreamNames = ['MinBias']
       if STREAM=="Background":
           bytestream_input.StreamNames = ['Background']
       if  STREAM=="05_Jets":
           bytestream_input.StreamType = "calibration"
           bytestream_input.StreamNames = ['DataScouting_05_Jets']
       if STREAM=="random":
           bytestream_input.StreamNames = ['monitoring_random']
           bytestream_input.StreamType = "monitoring"
       if  STREAM=="calibration":
           bytestream_input.StreamType = "calibration"

           
       print("DEBUG: bytestream_input.StreamNames:",bytestream_input.StreamNames)

   else: # select any trigger, but from physics events only !
       bytestream_input.StreamType = "physics"
       bytestream_input.StreamLogic = "Or"
       # not possible to ignore one stream, so creating a full list
       bytestream_input.StreamNames = ["Main","CosmicCalo","L1Topo","L1Calo","Standby","ZeroBias","Background", "EnhancedBias", "MinBias", "LArCells", "LArCellsEmpty", "LArNoiseBurst", "BphysDelayed", "Late"]
    
            
   # #################################################
   # Shall athena exit if the partition is shutdown ?
   # For offline athena tasks ONLY.
   # Set this parameter to FALSE online.
   # #################################################
   bytestream_input.ExitOnPartitionShutdown = False
   
   # #################################################
   # From Sergei Kolos, Nov. 2009:
   # Parameter default overwritten to drastically
   # decrease the load on the network
   # #################################################
   
   bytestream_input.BufferSize = 10
   
   # #################################################
   # Private Application Configuration options, replace with
   # your configuration.
   # #################################################
   
   event_selector = CompFactory.EventSelectorByteStream(
        name='EventSelector',
        ByteStreamInputSvc=bytestream_input.name,
        FileBased=False)
   acc.addService(event_selector)
   acc.setAppProperty("EvtSel", event_selector.name)

   event_persistency = CompFactory.EvtPersistencySvc(
        name="EventPersistencySvc",
        CnvServices=[bytestream_conversion.name])
   acc.addService(event_persistency)

   address_provider = CompFactory.ByteStreamAddressProviderSvc()
   acc.addService(address_provider)

   proxy = CompFactory.ProxyProviderSvc()
   proxy.ProviderNames += [address_provider.name]
   acc.addService(proxy)

   acc.merge(SGInputLoaderCfg(flags, FailIfNoProxy=True))


   from TriggerJobOpts.TriggerRecoConfig import TriggerRecoCfg
   acc.merge(TriggerRecoCfg(flags))

   ## fix for trigger
   l1bsdec = acc.getEventAlgo("L1TriggerByteStreamDecoder")
   l1bsdec.ByteStreamMetadataRHKey=""
   print("MaybeMissingROBs: ",l1bsdec.MaybeMissingROBs)
   l1bsdec.MaybeMissingROBs += [0x770001, 0x7500ac, 0x7500ad, 0x7300a8, 0x7300a9, 0x7300aa, 0x7300ab]

   print('CONFIG ',CONFIG)
   print('STREAM ',STREAM)
   if CONFIG!="LArDTMon":
      from LArMonitoring.RecoPT_NewConfig import LArMonitoringConfig
      acc.merge(LArMonitoringConfig(flags,CONFIG,STREAM,RunType))
   else:
      from LArMonitoring.RecoPT_Phase1NewConfig import LArDTMonitoringConfig
      acc.merge(LArDTMonitoringConfig(flags,STREAM))
   
   # fixes for splashes
   if RunType == 0 and CONFIG!="LArDTMon":
      acc.getEventAlgo("LArRawDataReadingAlg").LArRawChannelKey="" 

   # example for blocking the folder not filled during cosmics
   cil=acc.getCondAlgo('CondInputLoader')
   iovdbsvc=acc.getService('IOVDbSvc') 
   folder='/TRIGGER/LUMI/LBLB'
   for i in range(0,len(iovdbsvc.Folders)):
      if (iovdbsvc.Folders[i].find(folder)>=0):
         del iovdbsvc.Folders[i]
         break

   remove_folder = False
   for cil_Loadval in cil.Load:
      if folder in cil_Loadval:
         print(f"Removing {cil_Loadval} from cil/Load")
         remove_folder = True
         break
   if remove_folder: cil.Load.remove(cil_Loadval)

   if flags.DQ.enableLumiAccess:
      lbd = acc.getCondAlgo('LBDurationCondAlg')
      lbd.LBLBFolderInputKey=""
      print('lbd ',lbd)

   # somehow needs to add postprocessing
   from DataQualityUtils.DQPostProcessingAlg import DQPostProcessingAlg
   ppa = DQPostProcessingAlg("DQPostProcessingAlg")
   ppa.ExtraInputs = {( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' )}
   ppa.Interval = 200
   ppa.OutputLevel = WARNING
   if flags.Common.isOnline:
      ppa.FileKey = ((flags.DQ.FileKey + '/') if not flags.DQ.FileKey.endswith('/') 
                     else flags.DQ.FileKey) 

   acc.addEventAlgo(ppa, sequenceName='AthEndSeq')

   # example how to dump the store:
   #acc.getService("StoreGateSvc").Dump=True
   #acc.getService("StoreGateSvc").OutputLevel=DEBUG

   acc.printConfig()
   acc.run()
