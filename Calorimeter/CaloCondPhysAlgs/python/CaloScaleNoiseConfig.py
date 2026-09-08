# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory 
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg

from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
from LArCalibUtils.LArHVScaleConfig import LArHVScaleCfg
from IOVDbSvc.IOVDbSvcConfig import addOverride
from AthenaCommon.Logging import logging

def CaloScaleNoiseCfg(flagsIn, absolute=True, mu=60, dt=25, output='cellnoise_data.root', **hvscaleprops):

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
            raise RuntimeError("At this point (late run 3), only a dt of 25ns is supported")
        if mu==0:
            noisetag="LARNoiseOflCellNoisenoise-mc16-EposA3-ofc25mu0-25ns"
        elif mu==60:
            noisetag="LARNoiseOflCellNoisenoise-mc16-EposA3-ofc25mu60-25ns"
        else:
            raise RuntimeError("At this point (late run 3), only mu values of 0 and 60 are supported")


        msg.info("Absolute noise scaling using tag %s for mu=%i and dt=%i" , noisetag,mu,dt)

    result=ComponentAccumulator()
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(TileGMCfg(flags))
    result.merge(CaloNoiseCondAlgCfg(flags,noisetype="totalNoise"))
    result.merge(CaloNoiseCondAlgCfg(flags,noisetype="electronicNoise"))
    result.merge(CaloNoiseCondAlgCfg(flags,noisetype="pileupNoise"))
    if noisetag is not None:
        result.merge(addOverride(flags,"/LAR/NoiseOfl/CellNoise", noisetag))
    result.merge(LArHVScaleCfg(flags, **hvscaleprops))
    result.addEventAlgo(CompFactory.CaloRescaleNoise(absScaling=absolute))

    import os
    if os.path.exists(output):
        os.remove(output)
    result.addService(CompFactory.THistSvc(Output = ["file1 DATAFILE='"+output+"' OPT='RECREATE'"]))
    result.setAppProperty("HistogramPersistency","ROOT")
    return result


if __name__=="__main__":
    import argparse
    from LArCalibUtils.LArHVScaleConfig import addHvScaleParserArgs, buildHvScaleProps
    parser= argparse.ArgumentParser(description="(Re-)scale noise based on HV DCS values")
    addHvScaleParserArgs(parser)
    parser.add_argument('datestamp',help="time specification like 2007-05-25:14:01:00")
    parser.add_argument('-a', '--absolute', action="store_true",help="Absolute rescaling based on noise derived from MC")
    parser.add_argument('-t', '--globaltag', type=str, help="Global conditions tag ")
    parser.add_argument('-s', '--sqlite', type=str,help="sqlite with CellNoise and HVCorr folders to be scaled ")
    parser.add_argument('-o', '--output', type=str, default="cellnoise_data.root",
                        help="name stub for root and sqlite output files (default: %(default)s)")
    addHvScaleParserArgs(parser)
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from LArCalibProcessing.TimeStampToRunLumi import fillInputFlags
    flags = initConfigFlags()
    fillInputFlags(flags, args.datestamp+'/UTC')
    print("set the runnumber:",flags.Input.RunNumbers)
    flags.Input.Files=[]
    flags.IOVDb.DatabaseInstance="CONDBR2"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    if args.globaltag:
        flags.IOVDb.GlobalTag=args.globaltag
    if args.sqlite:
        flags.IOVDb.SqliteInput=args.sqlite
        flags.IOVDb.SqliteFolders=("/LAR/NoiseOfl/CellNoise","/LAR/ElecCalibFlat/HVScaleCorr",)
    flags.lock()
    hvsp=buildHvScaleProps(args)
    cfg=MainEvgenServicesCfg(flags)
    cfg.merge(CaloScaleNoiseCfg(flags,absolute=args.absolute,output=args.output,**hvsp))
    print("Start running...")
    cfg.run(1)
