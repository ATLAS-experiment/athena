# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory 
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg

from IOVDbSvc.IOVDbSvcConfig import addFolders

def CaloAddPedShiftCfg(flags,fileName="",output="ped.root"):

    #msg = logging.getLogger("CaloAddPedShiftCfg")
    result=ComponentAccumulator()

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(TileGMCfg(flags))

    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
    result.merge(LArOnOffIdMappingCfg(flags))

    
    folder= '/CALO/Ofl/Pedestal/CellPedestal'
    result.merge(addFolders(flags,folder,'CALO_OFL',className="CondAttrListCollection"))

    result.addEventAlgo(CompFactory.CaloAddCellPedShift(FolderName=folder,inputFile=fileName))
    

    import os
    rootfile="ped_data.root"
    if os.path.exists(rootfile):
        os.remove(rootfile)
    result.addService(CompFactory.THistSvc(Output = ["file1 DATAFILE='"+output+"' OPT='RECREATE'"]))
    result.setAppProperty("HistogramPersistency","ROOT")

    return result


if __name__=="__main__":
    import argparse
    parser= argparse.ArgumentParser(description="CaloCell Pedestal shift")
    parser.add_argument('-t', '--globaltag', type=str, help="Global conditions tag ")
    parser.add_argument('-i','--input', type=str, default="",  help="Input text file")
    parser.add_argument('-o', '--output',type=str,default="pedestalshift_data.root",help="name for root output files")
    parser.add_argument('-r', '--run', type=int, default=0xFFFFFFE, help="Run number to use to query Conddb")
    parser.add_argument('-l', '--lb', type=int, default=0, help="LB to use to query Conddb")
    args = parser.parse_args()
    print(args)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.RunNumbers=[args.run,]
    flags.Input.LumiBlockNumbers=[args.lb,]
    flags.Input.Files=[]
    flags.IOVDb.DatabaseInstance="CONDBR2"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
  
    if args.globaltag:
        flags.IOVDb.GlobalTag=args.globaltag

    flags.lock()
    cfg=MainEvgenServicesCfg(flags)
    cfg.merge(CaloAddPedShiftCfg(flags,args.input,args.output))

    print("Start running...")
    cfg.run(1)
