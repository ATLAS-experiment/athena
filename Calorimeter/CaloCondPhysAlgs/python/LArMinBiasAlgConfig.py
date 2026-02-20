# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory 
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def LArMinBiasAlgCfg(flags, output='ntuple.root', supercell=False,
                      idlowpt=900311, idhighpt=800831, wlowpt=0.099791, whighpt=0.00209):

    result=ComponentAccumulator()

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(TileGMCfg(flags))

    if(supercell):
       from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
       result.merge(LArOnOffIdMappingSCCfg(flags))
       result.addCondAlgo(CompFactory.CaloSuperCellAlignCondAlg('CaloSuperCellAlignCondAlg'))

       rKey="LArOnOffIdMapSC"
    else:   
       from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
       result.merge(LArOnOffIdMappingCfg(flags))

       rKey="LArOnOffIdMap"

    result.addCondAlgo(CompFactory.LArMCSymCondAlg("LArMCSymCondAlg",SuperCell=supercell,ReadKey=rKey))

    from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
    result.merge(EventInfoCnvAlgCfg(flags, disableBeamSpot=True))

    larMinBiasAlg = CompFactory.LArMinBiasAlg() 
    larMinBiasAlg.datasetID_lowPt=idlowpt
    larMinBiasAlg.datasetID_highPt=idhighpt
    # in mc16 files, computed at the beginning: 
    larMinBiasAlg.weight_lowPt = wlowpt
    larMinBiasAlg.weight_highPt= whighpt
    larMinBiasAlg.EvtInfo="EventInfo"
    larMinBiasAlg.CablingKey=rKey
    larMinBiasAlg.SuperCell=supercell
    #larMinBiasAlg.OutputLevel=2
    
    result.addEventAlgo(larMinBiasAlg)

    import os
    if os.path.exists(output):
        os.remove(output)
    result.addService(CompFactory.THistSvc(Output = ["file1 DATAFILE='"+output+"' OPT='RECREATE'"]))
    result.setAppProperty("HistogramPersistency","ROOT")

    return result

def list_of_strings(arg):
    return arg.split(',')

if __name__=="__main__":
    import argparse
    parser= argparse.ArgumentParser(description="Compute LArMinBias  from hits")
    parser.add_argument('-r', '--run',type=int,default=999999,help="run number")
    parser.add_argument('-o', '--output',type=str,default="ntuple.root",help="name of the root output file")
    parser.add_argument('-i', '--input',type=list_of_strings,default=[],help="name of the input files")
    parser.add_argument('-t', '--globaltag', type=str, help="Global conditions tag ")
    parser.add_argument( '--idlow',type=int,default=900311,help="ID of lowPt sample")
    parser.add_argument( '--idhigh',type=int,default=800831,help="ID of highPt sample")
    parser.add_argument( '--wlow',type=float,default=0.099791,help="weight of lowPt sample")
    parser.add_argument( '--whigh',type=float,default=0.00209,help="weight of highPt sample")
    parser.add_argument('-s', '--isSC',default=False,action='store_true',help="running for SC ?")
    parser.add_argument('-g', '--geometrytag',type=str,default="",help="geometry tag to use")
    args = parser.parse_args()
    print(args)
    print(len(args.input))

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.RunNumbers=[args.run]
    print("set the runnumber: ",flags.Input.RunNumbers)
    flags.Input.Files=args.input
    flags.IOVDb.DatabaseInstance="OFLP200"
    if args.geometrytag:
       flags.GeoModel.AtlasVersion = args.geometrytag
    else:   
       from AthenaConfiguration.TestDefaults import defaultGeometryTags
       flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
  
    if args.globaltag:
        flags.IOVDb.GlobalTag=args.globaltag

    #flags.Debug.DumpEvtStore=True
    #flags.Debug.DumpDetStore=True    
    #flags.Debug.DumpCondStore=True    

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    cfg.merge(LArMinBiasAlgCfg(flags,output=args.output, supercell=args.isSC,
                               idlowpt=args.idlow, idhighpt=args.idhigh, 
                                wlowpt=args.wlow, whighpt=args.whigh))

    print("Start running...")
    cfg.getService("MessageSvc").debugLimit=1000000
    cfg.run()
