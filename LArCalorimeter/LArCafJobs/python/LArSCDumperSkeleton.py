# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import sys
import os

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from LArCalibTools.LArSC2NtupleConfig import LArSC2NtupleCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

def L1CaloMenuCfg(flags):


    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

    acc=ComponentAccumulator()
    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg, HLTConfigSvcCfg, L1PrescaleCondAlgCfg, HLTPrescaleCondAlgCfg
    from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamDecoderCfg

    acc.merge( L1TriggerByteStreamDecoderCfg(flags) )
    acc.merge( L1ConfigSvcCfg(flags) )
    acc.merge( HLTConfigSvcCfg(flags) )
    acc.merge( L1PrescaleCondAlgCfg(flags) )
    acc.merge( HLTPrescaleCondAlgCfg(flags) )


    from TrigConfigSvc.TrigConfigSvcCfg import BunchGroupCondAlgCfg
    acc.merge( BunchGroupCondAlgCfg( flags ) )

    from AthenaConfiguration.ComponentFactory import CompFactory
    tdm = CompFactory.getComp('TrigDec::TrigDecisionMakerMT')()
    tdm.doL1 = True
    tdm.doHLT = False
    acc.addEventAlgo( tdm, 'AthAlgSeq' )

    return acc


def fromRunArgs(runArgs):

    from AthenaCommon.Logging import logging
    mlog_SCD = logging.getLogger( 'LArSCDumpSkeleton' )

    from AthenaConfiguration.AllConfigFlags import initConfigFlags    
    flags=initConfigFlags()

    from LArCafJobs.LArSCDumperFlags import addSCDumpFlags
    addSCDumpFlags(flags)

    commonRunArgsToFlags(runArgs, flags)

    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)

    flags.Input.Files=runArgs.inputBSFile
    flags.LArSCDump.outputNtup=runArgs.outputNTUP_SCMONFile

    # real geom not working yet
    flags.LArSCDump.doGeom=False

    from LArConditionsCommon.LArRunFormat import getLArDTInfoForRun
    try:
       runinfo=getLArDTInfoForRun(flags.Input.RunNumbers[0], connstring="COOLONL_LAR/CONDBR2")
    except Exception:
       mlog_SCD.warning("Could not get DT run info, using defaults !")   
       flags.LArSCDump.doEt=True
       flags.LArSCDump.nSamples=5
       flags.LArSCDump.nEt=1
       CKeys=["SC_ET"]    
    else:   
       CKeys=[]
       flags.LArSCDump.digitsKey=""
       for i in range(0,len(runinfo.streamTypes())):
          if runinfo.streamTypes()[i] ==  "SelectedEnergy":
                CKeys += ["SC_ET_ID"]
                flags.LArSCDump.doEt=True
                flags.LArSCDump.nEt=runinfo.streamLengths()[i]
          elif runinfo.streamTypes()[i] ==  "Energy":
                CKeys += ["SC_ET"]
                flags.LArSCDump.doEt=True
                flags.LArSCDump.nEt=runinfo.streamLengths()[i]
          elif runinfo.streamTypes()[i] ==  "RawADC":
                flags.LArSCDump.digitsKey="SC"
                flags.LArSCDump.nSamples=runinfo.streamLengths()[i]
          elif runinfo.streamTypes()[i] ==  "ADC":
                CKeys += ["SC_ADC_BAS"]
                flags.LArSCDump.nSamples=runinfo.streamLengths()[i]
                
    finally:
       flags.LArSCDump.doRawChan=True
       flags.LArSCDump.fillNoisyRO=False
       CKeys+=["LArRawChannels"]

    mlog_SCD.debug("CKeys generated %s",str(CKeys))   

    flags.Trigger.triggerConfig = 'DB'
    flags.Trigger.L1.doCTP = True
    flags.Trigger.L1.doMuon = False
    flags.Trigger.L1.doCalo = False
    flags.Trigger.L1.doTopo = False

    flags.Trigger.enableL1CaloLegacy = True
    flags.Trigger.enableL1CaloPhase1 = True

    # To respect --athenaopts 
    flags.fillFromArgs()

    flags.lock()
    
    cfg=MainServicesCfg(flags)
    cfg.merge(L1CaloMenuCfg(flags))

    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    tdt = cfg.getPrimaryAndMerge(TrigDecisionToolCfg(flags))

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(flags))

    if flags.LArSCDump.doBC:
       # FIXME should be SC version
       from LArBadChannelTool.LArBadChannelConfig import  LArBadFebCfg, LArBadChannelCfg
       cfg.merge(LArBadChannelCfg(flags))
       cfg.merge(LArBadFebCfg(flags))

    from LArCellRec.LArTimeVetoAlgConfig import LArTimeVetoAlgCfg
    cfg.merge(LArTimeVetoAlgCfg(flags))

    cfg.merge(LArSC2NtupleCfg(flags, AddBadChannelInfo=flags.LArSCDump.doBC, AddFEBTempInfo=False, isSC=True, isFlat=False,
                            OffId=flags.LArSCDump.doOfflineId, AddHash=flags.LArSCDump.doHash, AddCalib=flags.LArSCDump.doCalib, RealGeometry=flags.LArSCDump.doGeom, ExpandId=flags.LArSCDump.expandId, # from LArCond2NtupleBase 
                            NSamples=flags.LArSCDump.nSamples, FTlist=[], FillBCID=flags.LArSCDump.doBCID, ContainerKey=flags.LArSCDump.digitsKey,  # from LArDigits2Ntuple
                            SCContainerKeys=CKeys, OverwriteEventNumber = flags.LArSCDump.overwriteEvN, Net=flags.LArSCDump.nEt, # from LArSC2Ntuple
                            FillRODEnergy = flags.LArSCDump.doRawChan, FillLB = True, FillTriggerType = True,
                            TrigNames=["L1_EM3","L1_EM7","L1_EM15","L1_EM22VHI","L1_eEM5","L1_eEM15","L1_eEM22M"],
                            TrigDecisionTool=tdt,
                            OutputLevel=3))

    if os.path.exists(flags.LArSCDump.outputNtup):
          os.remove(flags.LArSCDump.outputNtup)
    from AthenaConfiguration.ComponentFactory import CompFactory
    cfg.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+flags.LArSCDump.outputNtup+"' OPT='NEW'" ]))
    cfg.setAppProperty("HistogramPersistency","ROOT")


    processPostInclude(runArgs, flags, cfg)
    processPostExec(runArgs, flags, cfg)

    #example how to dump the stores
    #cfg.getService("StoreGateSvc").Dump=True
    #from AthenaCommon.Constants import DEBUG
    #cfg.getService("MessageSvc").OutputLevel=DEBUG
    # Run the final accumulator
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
