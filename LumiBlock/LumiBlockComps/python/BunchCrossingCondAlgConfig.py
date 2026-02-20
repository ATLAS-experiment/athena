# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BunchStructureSource


def BunchCrossingCondAlgCfg(flags):
    result=ComponentAccumulator()

    run1 = flags.IOVDb.DatabaseInstance == 'COMP200'
    cfgsvc = None
    folder = ''
    bgkey = ''

    if flags.Beam.BunchStructureSource == BunchStructureSource.MC:
        # Only read digitization parameters from conditions DB if NOT ByteStream input
        # For ByteStream input, the metadata will be read from BS metadata instead
        from AthenaConfiguration.Enums import Format
        # Special case: if no input files, treat as non-BS even if format defaults to BS
        if flags.Input.Format != Format.BS or not flags.Input.Files:
            folder = '/Digitization/Parameters'
            from DigitizationConfig.DigitizationParametersConfig import readDigitizationParameters
            result.merge(readDigitizationParameters(flags))
        else:
            # For ByteStream input, don't set folder key - will read from BS metadata
            folder = ''
    elif flags.Beam.BunchStructureSource == BunchStructureSource.FILLPARAMS:
        folder = '/TDAQ/OLC/LHC/FILLPARAMS'
        from IOVDbSvc.IOVDbSvcConfig import addFolders
        result.merge(addFolders(flags,folder,'TDAQ',className = 'AthenaAttributeList',tag='HEAD'))
    elif flags.Beam.BunchStructureSource == BunchStructureSource.TrigConf:
        from TrigConfxAOD.TrigConfxAODConfig import getxAODConfigSvc
        cfgsvc = result.getPrimaryAndMerge(getxAODConfigSvc(flags))
        if cfgsvc.UseInFileMetadata:
            if 'TriggerMenuJson_BG' not in flags.Input.MetadataItems:
                # this is for when we need to configure the BunchGroupCondAlg with info extracted from converted JSON
                # in this case avoid using the xAODConfigSvc, because it will be set up incorrectly
                from TrigConfigSvc.TrigConfigSvcCfg import BunchGroupCondAlgCfg
                flagsWithFile = flags.clone()
                flagsWithFile.Trigger.triggerConfig = 'FILE'
                flagsWithFile.lock()
                result.merge(BunchGroupCondAlgCfg(flagsWithFile))
                bgkey = 'L1BunchGroup'
            else:  # trust that we can use the in-file metadata
                bgkey = ''
        else:
            from TrigConfigSvc.TrigConfigSvcCfg import BunchGroupCondAlgCfg
            result.merge(BunchGroupCondAlgCfg(flags))
            bgkey = 'L1BunchGroup'
    elif flags.Beam.BunchStructureSource == BunchStructureSource.Lumi:
        from .LuminosityCondAlgConfig import LuminosityCondAlgCfg
        result.merge(LuminosityCondAlgCfg(flags))

    alg = CompFactory.BunchCrossingCondAlg('BunchCrossingCondAlgDefault',
                                            Run1=run1,
                                            FillParamsFolderKey=folder,
                                            Mode=flags.Beam.BunchStructureSource.value,
                                            TrigConfigSvc=cfgsvc,
                                            L1BunchGroupCondData=bgkey)

    result.addCondAlgo(alg)

    return result



if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Input.isMC=False
    flags.IOVDb.DatabaseInstance="CONDBR2"
    flags.IOVDb.GlobalTag="CONDBR2-BLKPA-2017-05"
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    result=MainServicesCfg(flags)

    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    result.merge(McEventSelectorCfg(flags,
                                    RunNumber=330470,
                                    EventsPerRun=1,
                                    FirstEvent=1183722158,
                                    FirstLB=310,
                                    EventsPerLB=1,
                                    InitialTimeStamp=1500867637,
                                    TimeStampInterval=1))

    result.merge(BunchCrossingCondAlgCfg(flags))

    BunchCrossingCondTest=CompFactory.BunchCrossingCondTest
    result.addEventAlgo(BunchCrossingCondTest(FileName="BCData1.txt"))

    result.run(1)

    #f=open("test.pkl","wb")
    #result.store(f)
    #f.close()
