#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def LArPhysWaveFromTupleCfg(flags,InputRootFile='PhysWave.root',
                                      OutputTag='-calib-00', **kwargs):

     cfg=ComponentAccumulator()
     from AthenaCommon.Logging import logging 
     mlog = logging.getLogger( 'LArPhysWaveFromTuple' )
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
        kwargs.setdefault('NtupleName', 'PHYSWAVE')
     if 'SkipPoints' not in kwargs:
        kwargs.setdefault('SkipPoints', 0)
     if 'PrefixPoints' not in kwargs:
        kwargs.setdefault('PrefixPoints', 0)
     if 'StoreKey' not in kwargs:
        kwargs.setdefault('StoreKey', 'LArPhysWaveIdeal')

     algo = CompFactory.LArPhysWaveFromTuple("LArPhysWaveFromTuple", **kwargs) 
     algo.FileName = InputRootFile
     algo.GroupingType = flags.LArCalib.GroupingType
     algo.isSC = flags.LArCalib.isSC

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
        OutputObjectSpecTagPhysWave    = ''.join(flags.LArCalib.PhysWave.Folder.split('/')) + OutputTag
    
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

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    flags=initConfigFlags()
    addLArCalibFlags(flags,isSC=True)

    flags.Input.Files=[]
    flags.Input.RunNumbers=[440000,]
    flags.Input.ConditionsRunNumber=440000
    flags.Input.OverrideRunNumber=True
    
    flags.LArCalib.PhysWave.Folder="/LAR/ElecCalibOflSC/PhysWaves/FCALmeasured"
    flags.LArCalib.Output.ROOTFile="LArPhysWave_FCAL_SC.root"
    flags.LArCalib.Output.POOLFile="LArPhysWave_FCAL_SC.pool.root"
    flags.LArCalib.GroupingType="SuperCells"
    flags.IOVDb.DBConnection="sqlite://;schema=fcal_measured.sqlite;dbname=CONDBR2"
    flags.IOVDb.GlobalTag="LARCALIB-RUN2-00"
    flags.IOVDb.DatabaseInstance="CONDBR2"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3


    flags.fillFromArgs()
    flags.lock()


    cfg=MainServicesCfg(flags)
    cfg.merge(LArPhysWaveFromTupleCfg(flags,InputRootFile="shape_patched.root",NtupleName="PHYSWAVE", OutputTag="-data-00"))

    cfg.run(1)

