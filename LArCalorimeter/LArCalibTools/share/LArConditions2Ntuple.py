#!/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory


if __name__=='__main__':

  import os,sys
  import argparse

  # now process the CL options and assign defaults
  parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument('-r','--run', dest='run', default=0x7fffffff, help='Run number', type=int)
  parser.add_argument('--sqlite', dest='sqlite', default=None, help='sqlite file to read from (default: oracle)', type=str)
  parser.add_argument('-t','--tag',dest='dbtag',default=None,help="Global conditions tag", type=str)
  parser.add_argument('-o','--out', dest='out', default="LArConditions.root", help='Output root file', type=str)
  parser.add_argument('--ofcfolder',dest='ofcfolder',default="", help="OFC flavor",type=str)
  parser.add_argument('-s','--isSC', action='store_true', default=False, help='is SC?')
  parser.add_argument('-m','--isMC', action='store_true', default=False, help='is MC?')

  parser.add_argument("--objects",dest="objects",default="PEDESTAL,RAMP,OFC,MPHYSOVERMCAL",help="List of conditions types to be dumped",type=str)
   
  args = parser.parse_args()
  if help in args and args.help is not None and args.help:
    parser.print_help()
    sys.exit(0)


  #Translation table ... with a few potential variant spellings
  objTable={"RAMP":"Ramp",
            "DAC2UA":"DAC2uA",
            "DACUA":"DAC2uA",
            "PEDESTAL":"Pedestal",
            "PED":"Pedestal",
            "UA2MEV":"uA2MeV",
            "UAMEV":"uA2MeV",
            "MPHYSOVERMCAL":"MphysOverMcal",
            "MPHYSMCAL":"MphysOverMcal",
            "MPMC":"MphysOverMcal",
            "OFC":"OFC",
            "SHAPE":"Shape",
            "HVSCALECORR":"HVScaleCorr",
            "HVSCALE":"HVScaleCorr",
            "FSAMPL":"fSampl",
            "AUTOCORR":"AutoCorr",
            "AC":"AutoCorr"
          }

  objects=set()
  for obj in args.objects.split(","):
    objU=obj.upper()
    if objU not in objTable:
      print("ERROR: Unknown conditions type",obj)
      sys.exit(0)

    objects.add(objTable[objU])
    
 
  from AthenaConfiguration.AllConfigFlags import initConfigFlags
  flags=initConfigFlags()
  from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
  addLArCalibFlags(flags, args.isSC)
   
  flags.Input.RunNumbers=[args.run]
  from AthenaConfiguration.TestDefaults import defaultGeometryTags
  flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3

  flags.Input.Files=[]

  flags.Input.isMC=args.isMC
  flags.LArCalib.isSC=args.isSC

  flags.LAr.doAlign=False

  flags.LAr.OFCShapeFolder=args.ofcfolder

  from AthenaConfiguration.Enums import LHCPeriod
  if flags.Input.RunNumbers[0] < 222222:
    #Set to run1 for early run-numbers
    flags.GeoModel.Run=LHCPeriod.Run1 
    flags.IOVDb.DatabaseInstance="OFLP200" if flags.Input.isMC else "COMP200" 
  else:
    flags.GeoModel.Run=LHCPeriod.Run2
    flags.IOVDb.DatabaseInstance="OFLP200" if flags.Input.isMC else "CONDBR2"

  if args.dbtag:
    flags.IOVDb.GlobalTag=args.dbtag
  elif flags.Input.isMC:
    flags.IOVDb.GlobalTag="OFLCOND-MC16-SDR-20"
  elif flags.IOVDb.DatabaseInstance == "COMP200":
    flags.IOVDb.GlobalTag="COMCOND-BLKPA-RUN1-09"
  else: 
    flags.IOVDb.GlobalTag="CONDBR2-ES1PA-2022-07"

  #flags.Exec.OutputLevel=1

  if (args.sqlite):
    flags.IOVDb.SqliteInput=args.sqlite

  flags.lock()
  
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  cfg=MainServicesCfg(flags)

  #MC Event selector since we have no input data file
  from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
  cfg.merge(McEventSelectorCfg(flags,
                               EventsPerRun      = 1,
                               FirstEvent	      = 1,
                               InitialTimeStamp  = 0,
                               TimeStampInterval = 1))



  #Get LAr basic services and cond-algos
  from LArGeoAlgsNV.LArGMConfig import LArGMCfg
  cfg.merge(LArGMCfg(flags))

  if flags.LArCalib.isSC:
    #Setup SuperCell cabling
    from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg, LArCalibIdMappingSCCfg, LArLATOMEMappingCfg
    cfg.merge(LArOnOffIdMappingSCCfg(flags))
    cfg.merge(LArCalibIdMappingSCCfg(flags))
    cfg.merge(LArLATOMEMappingCfg(flags))
  else: 
    #Setup regular cabling
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg, LArCalibIdMappingCfg
    cfg.merge(LArOnOffIdMappingCfg(flags))
    cfg.merge(LArCalibIdMappingCfg(flags))
    
  
  from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg
  cfg.merge(LArBadChannelCfg(flags))

  if flags.LArCalib.isSC: 
    bcKey = "LArBadChannelSC" 
  else: 
    bcKey = "LArBadChannel"


  from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
  cfg.merge(LArElecCalibDBCfg(flags,objects))

  if "Pedestal" in objects:
    cfg.addEventAlgo(CompFactory.LArPedestals2Ntuple(ContainerKey = "LArPedestal",
                                                        AddFEBTempInfo = False, 
                                                        isSC = flags.LArCalib.isSC,
                                                        BadChanKey = bcKey
                                                      ))
                      
  if "AutoCorr" in objects:
    cfg.addEventAlgo(CompFactory.LArAutoCorr2Ntuple(ContainerKey = "LArAutoCorrSym" if flags.Input.isMC else "LArAutoCorr",
                                                    AddFEBTempInfo  = False, 
                                                    isSC = flags.LArCalib.isSC,
                                                    BadChanKey = bcKey
                                                  ))
  if "Ramp" in objects:
    cfg.addEventAlgo(CompFactory.LArRamps2Ntuple(RampKey="LArRampSym" if flags.Input.isMC else "LArRamp",
                                                 AddFEBTempInfo = False, 
                                                 isSC = flags.LArCalib.isSC,
                                                 BadChanKey = bcKey
                                               ))
  
  if "OFC" in objects:
    cfg.addEventAlgo(CompFactory.LArOFC2Ntuple(AddFEBTempInfo   = False,   
                                               isSC = flags.LArCalib.isSC,
                                               BadChanKey = bcKey
                                             ))


  if "Shape" in objects:
    cfg.addEventAlgo(CompFactory.LArShape2Ntuple(ContainerKey="LArShapeSym" if flags.Input.isMC else "LArShape",
                                                 AddFEBTempInfo   = False,   
                                                 isSC = flags.LArCalib.isSC,
                                                 BadChanKey = bcKey
               
                                               ))
  if "MphysOverMcal" in objects:
    cfg.addEventAlgo(CompFactory.LArMphysOverMcal2Ntuple(ContainerKey   = "LArMphysOverMcal",
                                                         AddFEBTempInfo   = False,
                                                         isSC = flags.LArCalib.isSC,
                                                         BadChanKey = bcKey
                                                       ))

  #ADC2MeV and DACuA are handled by the same ntuple dumper
  if "DAC2uA" in objects or "uA2MeV" in objects:
    ua2MeVKey="LAruA2MeVSym" if flags.Input.isMC else "LAruA2MeV"
    dac2uAKey="LArDAC2uASym" if flags.Input.isMC else "LArDAC2uA" 

    cfg.addEventAlgo(CompFactory.LAruA2MeV2Ntuple(uA2MeVKey=ua2MeVKey if "uA2MeV" in objects else "",
                                                  DAC2uAKey=dac2uAKey if "DAC2uA" in objects else "",
                                                  isSC = flags.LArCalib.isSC,
                                                  BadChanKey = bcKey
                                                ))
    

  if "HVScaleCorr" in objects:
    cfg.addEventAlgo(CompFactory.LArHVScaleCorr2Ntuple(AddFEBTempInfo = False,
                                                       isSC = flags.LArCalib.isSC,
                                                       BadChanKey = bcKey
                                                     ))

  if "fSampl" in objects:
    cfg.addEventAlgo(CompFactory.LArfSampl2Ntuple(ContainerKey="LArfSamplSym",
                                                  isSC=flags.LArCalib.isSC
                                                ))


  rootfile=args.out
  if os.path.exists(rootfile):
    os.remove(rootfile)
  cfg.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+rootfile+"' OPT='NEW'" ]))
  cfg.setAppProperty("HistogramPersistency","ROOT")

  cfg.run(1)
  sys.exit(0)
  
