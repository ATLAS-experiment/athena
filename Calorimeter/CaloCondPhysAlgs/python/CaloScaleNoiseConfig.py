# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory 
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg

from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
from LArCalibUtils.LArHVScaleConfig import LArHVScaleCfg
from IOVDbSvc.IOVDbSvcConfig import addOverride
from AthenaCommon.Logging import logging

def CaloScaleNoiseCfg(flagsIn,absolute=True,mu=60,dt=25,output='cellnoise_data.root'):

    #Clone flags-container and modify it, since this is not a standard reco job
    flags=flagsIn.clone()
    flags.Calo.Noise.fixedLumiForNoise=1 
    flags.LAr.doHVCorr = False #Avoid double-rescaling
    flags.lock()

    msg = logging.getLogger("CaloScaleNoiseCfg")
    #pick noise-tag depending on mu and dt

    if not absolute:
        #relative rescale, use current UPD-online tag 
        noisetag="LARNoiseOflCellNoise-RUN2-UPD1-00"
        msg.info("Noise rescaling using tag %s", noisetag)
    else:
        if (dt!=25):
            raise RuntimeError("At this point (early run 3), only a dt of 25ns is supported")
        
        if mu==0:
            noisetag="LARNoiseOflCellNoisenoise-mc16-EposA3-ofc25mu0-25ns"
        elif mu==60:
            noisetag="LARNoiseOflCellNoisenoise-mc16-EposA3-ofc25mu60-25ns"
        else:
            raise RuntimeError("At this point (early run 3), only mu values of 0 and 60 are supported")


        msg.info("Absolute noise scaling using tag %s for mu=%i and dt=%i" , noisetag,mu,dt)

    result=ComponentAccumulator()

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(TileGMCfg(flags))


    result.merge(CaloNoiseCondAlgCfg(flags,noisetype="totalNoise"))
    result.merge(CaloNoiseCondAlgCfg(flags,noisetype="electronicNoise"))
    result.merge(CaloNoiseCondAlgCfg(flags,noisetype="pileupNoise"))


    if absolute:
        result.merge(addOverride(flags,"/LAR/NoiseOfl/CellNoise",noisetag))
    else:
       result.merge(addOverride(flags,"/LAR/NoiseOfl/CellNoise",noisetag))

    result.merge(LArHVScaleCfg(flags))
    
    result.addEventAlgo(CompFactory.CaloRescaleNoise(absScaling=absolute))

    import os
    rootfile="cellnoise_data.root"
    if os.path.exists(rootfile):
        os.remove(rootfile)
    result.addService(CompFactory.THistSvc(Output = ["file1 DATAFILE='"+output+"' OPT='RECREATE'"]))
    result.setAppProperty("HistogramPersistency","ROOT")

    return result


if __name__=="__main__":
    import sys, argparse
    parser= argparse.ArgumentParser(description="(Re-)scale noise based on HV DCS values")
    parser.add_argument('datestamp',help="time specification like 2007-05-25:14:01:00")
    parser.add_argument('-a', '--absolute', action="store_true", help="Absolute rescaling based on noise derived from MC")
    parser.add_argument('-t', '--globaltag', type=str, help="Global conditions tag ")
    parser.add_argument('-o', '--output',type=str,default="hvcorr",help="name stub for root and sqlite output files")
    args = parser.parse_args()
    print(args)

    from time import strptime
    from calendar import timegm
    try:
        ts=strptime(args.datestamp+'/UTC','%Y-%m-%d:%H:%M:%S/%Z')
        TimeStamp=int(timegm(ts))
        TimeStamp_ns=TimeStamp*1000000000
    except ValueError as e:
        print("ERROR in time specification, use e.g. 2007-05-25:14:01:00")
        print(e)
        sys.exit(-1)


    from LArCalibProcessing.TimeStampToRunLumi import TimeStampToRunLumi
    rlb=TimeStampToRunLumi(TimeStamp_ns)
    if rlb is None:
        rlb=[0xFFFFFFFF-1,0]
        print("WARNING: Failed to convert time",TimeStamp_ns,"into a run/lumi number. Using 'infinite' run-number",rlb[0])


    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.RunNumbers=[rlb[0]]
    flags.Input.TimeStamps=[TimeStamp]
    flags.Input.Files=[]
    flags.IOVDb.DatabaseInstance="CONDBR2"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
  
    if args.globaltag:
        flags.IOVDb.GlobalTag=args.globaltag

    flags.lock()
    cfg=MainEvgenServicesCfg(flags)
    cfg.merge(CaloScaleNoiseCfg(flags,absolute=args.absolute,output=args.output))

    print("Start running...")
    cfg.run(1)
