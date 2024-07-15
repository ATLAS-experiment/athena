#!/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

##=======================================================================================
## Name:        Merge_ofc_blobs.py
##
## Description: Python script using athena infrastructure to merge LAr OFC blobs from two 
##              different sources to one, patching one part of detector 
##
##              based on example by Walter Lampl
##========================================================================================


import os,sys,getopt
from AthenaConfiguration.TestDefaults import defaultGeometryTags
import argparse

parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('-r', "--run", dest='run', default=0x7fffffff, help="Run number. Default %(default)s.")
parser.add_argument('-i', "--indb", dest='indb', default="COOLONL_LAR/CONDBR2",help="Input DB for base constants. Default %(default)s.")
parser.add_argument('-p', "--patchdb", dest='patchdb', default="freshConstantsOnl.db", help="Patching DB, from which constants are taken only for the specified region(s). Default %(default)s.")
parser.add_argument('-o', "--outdb", dest='outdb', default="new.db", help="Output DB. Default %(default)s.")
parser.add_argument('-f', "--folder", dest='folder', default="/LAR/ElecCalibFlat/OFC", help="COOL folder. Default %(default)s.")
parser.add_argument('-s', "--issc", dest='issc', default=False, action="store_true", help="Is this SuperCell Data? Default %(default)s.")
parser.add_argument('-d', "--det", dest='patchdet', default=[], nargs='+', type=int, help="Detector to patch. Default %(default)s.")
parser.add_argument('-ft', "-FT", dest='patchFT', default=[], nargs='+', type=str, help="Feedthrough to patch. Default %(default)s")
parser.add_argument('-ch', dest='patchChan', default=[], nargs='+', type=str, help="Channel(s) to patch. Default %(default)s")

def is_hexadecimal(s):
    try:
        int(s, 16)
        return True
    except ValueError:
        return False
    
# std::string LArOnlineID_Base::feedthrough_name( const HWIdentifier id )const{

args = parser.parse_args()
if help in args and args.help is not None and args.help:
    parser.print_help()
    sys.exit(0)

filled = [ len(x) > 0 for x in [args.patchdet, args.patchFT, args.patchChan] ]

if filled.count(True) == 0:
    print("No list of HW to patch was provided! Please provide desired detectors/FTs/channels to patch")
    sys.exit(1)
elif filled.count(True) > 1:
    print("Ambiguous options given. Please only provide one unit of HW to patch (detectors/FTs/channels)")
    sys.exit(1)

if os.path.isfile(args.outdb):
    print(f"OUTPUT FILE {args.outdb} ALREADY EXISTS! Please check, and delete it / supply a different output name")
    sys.exit(1)

else:
    # Convert channel list to int (including hex conversion, where needed)
    args.patchChan = [int(ch,16) if is_hexadecimal(ch) else int(ch) for ch in args.patchChan]
    
if ".db" in args.indb:
   indb="sqlite://;schema="+args.indb+";dbname=CONDBR2"
else:   
   indb=args.indb
if ".db" in args.patchdb:
   patchdb="sqlite://;schema="+args.patchdb+";dbname=CONDBR2"
else:   
   patchdb=args.patchdb
if ".db" in args.outdb:
   outdb="sqlite://;schema="+args.outdb+";dbname=CONDBR2"
else:   
   outdb=args.outdb

#Don't let PyRoot open X-connections
sys.argv = sys.argv[:1] + ['-b'] 

detdescrtag=defaultGeometryTags.RUN3
defTag="CONDBR2-BLKPA-2024-01"

from AthenaConfiguration.AllConfigFlags import initConfigFlags 
flags=initConfigFlags()
from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
addLArCalibFlags(flags,isSC=args.issc)
flags.Input.Files = []
flags.Input.TimeStamps = [1000]
flags.Input.isMC=False
flags.Input.RunNumbers=[int(args.run)]
flags.IOVDb.DatabaseInstance="CONDBR2" if int(args.run)>222222 else "COMP200"
flags.IOVDb.GlobalTag=defTag
flags.GeoModel.AtlasVersion = detdescrtag 
flags.LAr.doAlign=False
from AthenaCommon.Constants import INFO
flags.Exec.OutputLevel=INFO
flags.lock()

from RootUtils import PyROOTFixes  # noqa F401
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
cfg=MainServicesCfg(flags)

from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
cfg.merge (McEventSelectorCfg (flags))

#if geo:
#    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
#    cfg.merge(LArGMCfg(flags))
#else:
from DetDescrCnvSvc.DetDescrCnvSvcConfig import DetDescrCnvSvcCfg
cfg.merge(DetDescrCnvSvcCfg(flags))    

if flags.LArCalib.isSC:
    #Setup SuperCell cabling
    from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
    cfg.merge(LArOnOffIdMappingSCCfg(flags))
else:
    #Setup regular cabling
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg 
    cfg.merge(LArOnOffIdMappingCfg(flags))

from LArConditionsCommon.LArOFCMergeAlg import LArOFCMergeAlg
theLArMergeAlg=LArOFCMergeAlg("LAOFCMergeAlg", fld=args.folder,
                              inputdb=indb,
                              patchingdb=patchdb,
                              outputdb=outdb,
                              runnum=int(args.run),
                              patchdet=args.patchdet,
                              patchFT=args.patchFT,
                              patchChan=args.patchChan,
                              isSC=flags.LArCalib.isSC)
cfg.addEventAlgo(theLArMergeAlg)

cfg.run(2) # to make sure all conditions are red-out, 
           #  actual code is in algo stop
