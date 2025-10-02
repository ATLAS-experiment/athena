# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

import sys

from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude
from LArCafJobs.LArShapeDumperConfig import LArShapeDumperCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    mlog_SCD = logging.getLogger( 'LArShapeDumperSkeleton' )

    from AthenaConfiguration.AllConfigFlags import initConfigFlags    

    flags=initConfigFlags()
    from LArCafJobs.LArShapeDumperFlags import addShapeDumpFlags
    addShapeDumpFlags(flags)

    commonRunArgsToFlags(runArgs, flags)

    processPreInclude(runArgs, flags)
    processPreExec(runArgs, flags)

    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.LAr.ROD.forceIter=True
    flags.LAr.OFCShapeFolder="4samples3bins17phases"
    flags.Input.Files=runArgs.inputBSFile
    flags.LArShapeDump.outputNtup=runArgs.outputNTUP_SAMPLESMONFile

    flags.LArShapeDump.energySCCut = 500.

    #protection for LArPEB event:
    flags.Trigger.triggerConfig = 'DB'
    flags.Trigger.L1.doCTP = True
    flags.Trigger.L1.doMuon=False
    flags.Trigger.L1.doCalo=False
    flags.Trigger.L1.doTopo=False

    if hasattr(runArgs,"outputNTUP_HECNOISEFile"):
        flags.LArShapeDump.HECNoiseNtup=runArgs.outputNTUP_HECNOISEFile
        
    if runArgs.doSC:
       from LArConditionsCommon.LArRunFormat import getLArDTInfoForRun
       try:
          runinfo=getLArDTInfoForRun(flags.Input.RunNumbers[0], connstring="COOLONL_LAR/CONDBR2")
       except Exception:
          mlog_SCD.warning("Could not get DT run info, using defaults !")   
          flags.LArShapeDump.ndigitsSC=6
          flags.LArShapeDump.nrawSC=1
          flags.LArShapeDump.rawSCKey="SC_ET_ID"    
          flags.LArShapeDump.digitsKeySC="SC_ADC_BAS"
          fw=5
       else:   
          fw=runinfo.FWversion()
          for i in range(0,len(runinfo.streamTypes())):
             if runinfo.streamTypes()[i] ==  "SelectedEnergy":
                   flags.LArShapeDump.rawSCKey = "SC_ET_ID"
                   flags.LArShapeDump.nrawSC = runinfo.streamLengths()[i]
             elif runinfo.streamTypes()[i] ==  "Energy":
                   flags.LArShapeDump.rawSCKey = "SC_ET"
                   flags.LArShapeDump.nrawSC = runinfo.streamLengths()[i]
             elif runinfo.streamTypes()[i] ==  "RawADC":
                   flags.LArShapeDump.digitsKeySC="SC"
                   flags.LArShapeDump.ndigitsSC = runinfo.streamLengths()[i]
             elif runinfo.streamTypes()[i] ==  "ADC":
                   flags.LArShapeDump.digitsKeySC="SC_ADC_BAS"
                   flags.LArShapeDump.ndigitsSC = runinfo.streamLengths()[i]

    # To respect --athenaopts 
    flags.fillFromArgs()

    flags.lock()
    
    cfg=MainServicesCfg(flags)
    cfg.merge(LArShapeDumperCfg(flags))

    processPostInclude(runArgs, flags, cfg)
    processPostExec(runArgs, flags, cfg)

    if runArgs.doSC and fw==6:
       from IOVDbSvc.IOVDbSvcConfig import addOverride
       cfg.merge(addOverride(flags,"/LAR/Identifier/LatomeMapping","LARIdentifierLatomeMapping-fw6"))

    # Run the final accumulator
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
