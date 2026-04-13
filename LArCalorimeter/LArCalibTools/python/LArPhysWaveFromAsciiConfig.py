#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def LArPhysWaveFromAsciiCfg(flags,InputFile='',hasIndex=False,  **kwargs):

     cfg=ComponentAccumulator()
     from AthenaCommon.Logging import logging 
     mlog = logging.getLogger( 'LArPhysWaveFromAscii' )
     if not flags.hasCategory('LArCalib'):
        mlog.error("We need the LArCalib flags")
        return cfg
     
     if flags.LArCalib.isSC:
        mlog.info("Running for SC")

     from LArCalibProcessing.LArCalibBaseConfig import LArCalibBaseCfg
     cfg.merge(LArCalibBaseCfg(flags))

     from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
     cfg.merge(LArOnOffIdMappingCfg(flags))
        
     if 'SkipPoints' not in kwargs:
        kwargs.setdefault('SkipPoints', 0)
     if 'PrefixPoints' not in kwargs:
        kwargs.setdefault('PrefixPoints', 0)
     if 'StoreKey' not in kwargs:
        kwargs.setdefault('StoreKey', 'LArPhysWaveSCMeasured')

     algo = CompFactory.LArPhysWaveFromAscii("LArPhysWaveFromAscii", **kwargs) 
     algo.InputFile = InputFile
     algo.GroupingType = flags.LArCalib.GroupingType
     algo.isSC = flags.LArCalib.isSC
     algo.Index = hasIndex

     cfg.addEventAlgo(algo)

     if flags.LArCalib.Output.ROOTFile != "":

        ntdump = CompFactory.LArPhysWaves2Ntuple( "LArPhysWaves2Ntuple" ) 
        ntdump.NtupleName   = "PHYSWAVE" 
        ntdump.KeyList      = [ kwargs['StoreKey'] ]
        ntdump.SaveDerivedInfo = True
        if flags.LArCalib.isSC:
           ntdump.isSC = flags.LArCalib.isSC
           ntdump.BadChanKey = "LArBadChannelSC"

        cfg.addEventAlgo(ntdump)

        cfg.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+flags.LArCalib.Output.ROOTFile+"' OPT='NEW'" ]))
        cfg.setAppProperty("HistogramPersistency","ROOT")
        
     if ( flags.LArCalib.Output.POOLFile != "" ):

        OutputObjectSpecPhysWave   = "LArPhysWaveContainer#"+kwargs['StoreKey']+"#"+ flags.LArCalib.PhysWave.Folder
        OutputObjectSpecTagPhysWave    = ''.join(flags.LArCalib.PhysWave.Folder.split('/')) + "-test-00"
    
        from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
        cfg.merge(OutputConditionsAlgCfg(flags,
                      outputFile=flags.LArCalib.Output.POOLFile,
                      ObjectList=[OutputObjectSpecPhysWave],
                      IOVTagList=[OutputObjectSpecTagPhysWave],
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
    parser.add_argument('-o','--outsqlite', dest='outsql', default="output.db", help='Output sqlite file', type=str)
    parser.add_argument('-f','--folder', dest='folder', default="/LAR/ElecCalibOflSC/PhysWaves/Measured", help='Folder to fill ', type=str)
    parser.add_argument('--infile', dest='infile', default="", help='Input text file file', type=str)
    parser.add_argument('--poolfile', dest='poolfile', default="LArPhysWave_SC.pool.root", help='Output pool file', type=str)
    parser.add_argument('--rootfile', dest='rootfile', default="LArPhysWave_SC.root", help='Output ROOT file', type=str)
    parser.add_argument('--isSC', dest='supercells', default=False, help='is SC data ?', action='store_true')
    parser.add_argument('--hasIndex', dest='index', default=False, help='has index in input ?', action='store_true')

    args = parser.parse_args()

    for _, value in args._get_kwargs():
      if value is not None:
        print(_,":",value)

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    flags=initConfigFlags()
    addLArCalibFlags(flags,isSC=True)

    flags.Input.Files=[]
    flags.Input.RunNumbers=[args.run,]
    flags.Input.ConditionsRunNumber=args.run
    flags.Input.OverrideRunNumber=True
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    
    flags.LArCalib.PhysWave.Folder=args.folder
    flags.LArCalib.Output.ROOTFile=args.rootfile
    flags.LArCalib.Output.POOLFile=args.poolfile
    if args.supercells:
       flags.LArCalib.GroupingType="SuperCells"
    else:   
       flags.LArCalib.GroupingType="ExtendedSubDetector"
    flags.IOVDb.DBConnection="sqlite://;schema="+args.outsql+";dbname=CONDBR2"
    flags.IOVDb.GlobalTag="LARCALIB-RUN2-00"
    flags.IOVDb.DatabaseInstance="CONDBR2"

    #flags.fillFromArgs()
    flags.lock()


    cfg=MainServicesCfg(flags)
    cfg.merge(LArPhysWaveFromAsciiCfg(flags,InputFile=args.infile,hasIndex=args.index))

    cfg.run(1)

