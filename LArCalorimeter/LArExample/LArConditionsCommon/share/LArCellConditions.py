#!/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

##=======================================================================================
## Name:        LArCellConditions.py
##
## Description: Python script using athena infrastructure to display database information 
##              about LAr Cells:
##                - Converts online <-> offline identifiers
##                - Displays bad channel status
##                - Optionally displays some basic calibration constants
##                - Can search for (bad) channels in a given detector region
##========================================================================================


import os,sys,getopt
from AthenaConfiguration.TestDefaults import defaultGeometryTags
fhistory = os.path.expanduser("~/.LArCellConditionsHist")


def usage():
    print(sys.argv[0]+": Convert and expand LAr Identifiers, print some database content")
    print("Options:")
    print("-c Print also (some) electronic calibration constants")
    print("-s Use SingleVersion folders (default)")
    print("-m Use MultiVersion folders (the opposite of -s)")
    print("-g Include geometrical position (true eta/phi)")
    print("-r <run> Specify a run number")
    print("-t <tag> Specify global tag")
    print("--detdescr <DetDescrVersion>")
    print("--sqlite <sqlitefile>")
    print("-h Print this help text and exit")
        
try:
    opts,args=getopt.getopt(sys.argv[1:],"csmgdhr:t:",["help","detdescr=","sqlite="])
except Exception as e:
    usage()
    print(e)
    sys.exit(-1)

    
printCond=False
run=None
tag=None
sv=True
geo=False
detdescrtag=defaultGeometryTags.RUN2
detdescrset=False
sqlite=""

for o,a in opts:
    if (o=="-c"): printCond=True
    if (o=="-t"): tag=a
    if (o=="-r" and a.isdigit()): run=int(a)
    if (o=="-s"): sv=True
    if (o=="-m"): sv=False
    if (o=="-g"): geo=True
    if (o=="-h" or o=="--help"):
        usage()
        sys.exit(0)
    if (o=="--detdescr"):
        detdescrtag=a
        detdescrset=True
    if (o=="--sqlite"): sqlite=a
    
import readline

try:
    if run is None:
        defRun=0x7fffffff
        prompt= "Enter run number [%i]:" % defRun
        runIn=input(prompt).strip()
        if runIn=="":
            run=defRun
        else:
            if runIn.isdigit():
                run=int(runIn)
            else:
                usage()
                print("Expect numerical parameter for run, got",runIn)
                sys.exit(0)
                pass
            pass
        pass

    if tag is None:
        if (run>222222):
            defTag="CONDBR2-BLKPA-2014-00"
        else:
            defTag="COMCOND-BLKPA-RUN1-06"
            pass
        prompt= "Enter conditions tag [%s]:" % defTag
        tagIn=input(prompt).strip()
        if tagIn=="":
            tag=defTag
        else:
            tag=tagIn


    if geo and not detdescrset:
        prompt="Enter DetectorDescripton tag [%s]:" % detdescrtag
        detdescrtagIn=input(prompt).strip()
        if detdescrtagIn != "":
            detdescrtag=detdescrtagIn
                
except:
    print("Failed to get run number and/or conditions tag")
    sys.exit(0)


#Don't let PyRoot open X-connections
sys.argv = sys.argv[:1] + ['-b'] 

from AthenaConfiguration.AllConfigFlags import initConfigFlags 
flags=initConfigFlags()
flags.Input.Files = []
flags.Input.TimeStamps = [1000]
flags.Input.isMC=False
flags.Input.RunNumbers=[run]
flags.IOVDb.DatabaseInstance="CONDBR2" if run>222222 else "COMP200"
flags.IOVDb.GlobalTag=tag
flags.GeoModel.AtlasVersion = detdescrtag 
flags.LAr.doAlign=False
from AthenaCommon.Constants import FATAL
flags.Exec.OutputLevel=FATAL
flags.lock()

from RootUtils import PyROOTFixes  # noqa F401
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
cfg=MainServicesCfg(flags)

from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
cfg.merge (McEventSelectorCfg (flags))

if geo:
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(flags))
else:
    from DetDescrCnvSvc.DetDescrCnvSvcConfig import DetDescrCnvSvcCfg
    cfg.merge(DetDescrCnvSvcCfg(flags))    

from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg 
cfg.merge(LArOnOffIdMappingCfg(flags))

from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
requiredConditions=["Pedestal","Ramp","DAC2uA","uA2MeV","MphysOverMcal","HVScaleCorr"]
cfg.merge(LArElecCalibDBCfg(flags,requiredConditions))

from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg
cfg.merge(LArBadChannelCfg(flags))

from LArConditionsCommon.LArCellConditionsAlg import LArCellConditionsAlg
theLArCellConditionsAlg=LArCellConditionsAlg("LArCellConditions",
                                             printConditions=printCond,
                                             printLocation=geo)
cfg.addEventAlgo(theLArCellConditionsAlg)

if os.path.exists( fhistory ):
    readline.read_history_file( fhistory )
readline.set_history_length( 128 )
cfg.run(2) #First event is dummy to close DB connections, second has the user-loop

readline.write_history_file(fhistory)
