#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def LArCaliWaveFromTupleCfg(flags,InputRootFile='PhysWave.root',
                                      OutputTag='-calib-00', **kwargs):

     cfg=ComponentAccumulator()
     from AthenaCommon.Logging import logging 
     mlog = logging.getLogger( 'LArCaliWaveFromTuple' )
     if not flags.hasCategory('LArCalib'):
        mlog.error("We need the LArCalib flags")
        return cfg
     
     if flags.LArCalib.isSC:
        mlog.info("Running for SC")

     from LArCalibProcessing.LArCalibBaseConfig import LArCalibBaseCfg
     cfg.merge(LArCalibBaseCfg(flags))

     from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
     cfg.merge(LArOnOffIdMappingCfg(flags))
        
     if 'NtupleName' not in kwargs:
        kwargs.setdefault('NtupleName', 'CALIWAVE')
     if 'SkipPoints' not in kwargs:
        kwargs.setdefault('SkipPoints', 0)
     if 'PrefixPoints' not in kwargs:
        kwargs.setdefault('PrefixPoints', 0)
     if 'StoreKey' not in kwargs:
        kwargs.setdefault('StoreKey', 'FromTuple')

     algo = CompFactory.LArCaliWaveFromTuple("LArCaliWaveFromTuple", **kwargs) 
     algo.FileName = InputRootFile
     algo.GroupingType = flags.LArCalib.GroupingType
     algo.isSC = flags.LArCalib.isSC

     cfg.addEventAlgo(algo)

     if flags.LArCalib.Output.ROOTFile != "":

        ntdump = CompFactory.LArCaliWaves2Ntuple( "LArCaliWaves2Ntuple" ) 
        ntdump.NtupleName   = "CALIWAVE" 
        ntdump.KeyList      = [ kwargs['StoreKey'] ]
        ntdump.SaveDerivedInfo = True
        if flags.LArCalib.isSC:
           ntdump.isSC = flags.LArCalib.isSC
           ntdump.BadChanKey = "LArBadChannelSC"

        cfg.addEventAlgo(ntdump)

        cfg.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+flags.LArCalib.Output.ROOTFile+"' OPT='NEW'" ]))
        cfg.setAppProperty("HistogramPersistency","ROOT")
        
     if ( flags.LArCalib.Output.POOLFile != "" ):

        OutputObjectSpecCaliWave   = "LArCaliWaveContainer#"+kwargs['StoreKey']+"#"+ flags.LArCalib.CaliWave.Folder
        OutputObjectSpecTagCaliWave    = ''.join(flags.LArCalib.CaliWave.Folder.split('/')) + OutputTag
    
        from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
        cfg.merge(OutputConditionsAlgCfg(flags,
                      outputFile=flags.LArCalib.Output.POOLFile,
                      ObjectList=[OutputObjectSpecCaliWave],
                      IOVTagList=[OutputObjectSpecTagCaliWave],
                      Run1=flags.LArCalib.IOVStart,
                      Run2=flags.LArCalib.IOVEnd
                  ))
    
        cfg.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = True))


     cfg.getService("IOVDbSvc").DBInstance=""

     return cfg


if __name__ == "__main__":

    import argparse

    # now process the CL options and assign defaults
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-r','--run', dest='run', default=508000, help='Run number ', type=int)
    parser.add_argument('-p','--npoints', dest='npoints', default=32, help='Number of samples to output', type=int)
    parser.add_argument('-o','--outsqlite', dest='outsql', default="output.db", help='Output sqlite file', type=str)
    parser.add_argument('-f','--folder', dest='folder', default="/LAR/ElecCalibOflSC/CaliWaves/CaliWave", help='Folder to fill ', type=str)
    parser.add_argument('--infile', dest='infile', default="", help='Input root file file', type=str)
    parser.add_argument('--poolfile', dest='poolfile', default="LArCaliWave.pool.root", help='Output pool file', type=str)
    parser.add_argument('--rootfile', dest='rootfile', default="LArCaliWave.root", help='Output ROOT file', type=str)
    parser.add_argument('--isSC', dest='supercells', default=False, help='is SC data ?', action='store_true')
    parser.add_argument('-n','--ntuple', dest='ntuple', default="Data", help='Input ntuple name', type=str)
    parser.add_argument('-t','--tag', dest='tag', default="-RUN4-EMF-00", help='Output tag suffix', type=str)

    args = parser.parse_args()

    for key, value in args._get_kwargs():
      if value is not None:
        print(key,":",value)


    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    flags=initConfigFlags()
    addLArCalibFlags(flags,isSC=args.supercells)

    flags.Input.Files=[]
    flags.Input.RunNumbers=[args.run,]
    flags.Input.ConditionsRunNumber=args.run
    flags.Input.OverrideRunNumber=True
    
    flags.LArCalib.CaliWave.Folder=args.folder
    flags.LArCalib.Output.ROOTFile=args.rootfile
    flags.LArCalib.Output.POOLFile=args.poolfile
    flags.LArCalib.GroupingType="ExtendedFeedThrough"
    flags.IOVDb.GlobalTag="LARCALIB-RUN2-00"
    flags.IOVDb.DatabaseInstance="CONDBR2"
    flags.IOVDb.DBConnection="sqlite://;schema="+args.outsql+";dbname=CONDBR2"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3


    flags.lock()


    cfg=MainServicesCfg(flags)
    cfg.merge(LArCaliWaveFromTupleCfg(flags,InputRootFile=args.infile,NtupleName=args.ntuple, OutputTag=args.tag, NPoints=args.npoints))

    cfg.run(1)

