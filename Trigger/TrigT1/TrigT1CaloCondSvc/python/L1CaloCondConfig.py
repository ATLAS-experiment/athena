# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys


def L1CaloCondAlgCfg(flags, readTest=False, Physics=True, Calib1=False, Calib2=False):
    # The three flags: Physics, Calib1, Calib2 are the timing regimes that should or should not be supported
    # i.e. should the required necessary folders be loaded for these regimes or not.
    # default is only the Physics timing regime is loaded. If Calib1 or Calib2 are found in the DerivedRunPars
    # then this will lead to error unless Calib1 and/or Calib2 is True.

    L1CaloFolders = {} # will use this dict to populate condalg properties below

    # both data and mc have this folder
    L1CaloFolders['PhysicsKeys'] = ['/TRIGGER/L1Calo/V2/Calibration/Physics/PprChanCalib']

    if not flags.Input.isMC:
        # folders only existing or used if running on data
        if Physics:
            L1CaloFolders['PhysicsKeys'] += ['/TRIGGER/L1Calo/V2/Calibration/Physics/PprChanCommon',
                                            '/TRIGGER/L1Calo/V2/Calibration/Physics/PprChanHighMu',
                                            '/TRIGGER/L1Calo/V2/Calibration/Physics/PprChanLowMu']
        if Calib1:
            L1CaloFolders['Calib1Keys'] = ['/TRIGGER/L1Calo/V2/Calibration/Calib1/PprChanCalib',
                                            '/TRIGGER/L1Calo/V2/Calibration/Calib1/PprChanCommon',
                                            '/TRIGGER/L1Calo/V2/Calibration/Calib1/PprChanHighMu',
                                            '/TRIGGER/L1Calo/V2/Calibration/Calib1/PprChanLowMu']
        if Calib2:
            L1CaloFolders['Calib2Keys'] = ['/TRIGGER/L1Calo/V2/Calibration/Calib2/PprChanCalib',
                                           '/TRIGGER/L1Calo/V2/Calibration/Calib2/PprChanCommon',
                                           '/TRIGGER/L1Calo/V2/Calibration/Calib2/PprChanHighMu',
                                           '/TRIGGER/L1Calo/V2/Calibration/Calib2/PprChanLowMu']

        L1CaloFolders['ReadoutConfig'] =  "/TRIGGER/L1Calo/V2/Configuration/ReadoutConfig"
        L1CaloFolders['ReadoutConfigJSON'] =  "/TRIGGER/L1Calo/V2/Configuration/ReadoutConfigJSON"
        L1CaloFolders['PprChanStrategy'] = "/TRIGGER/L1Calo/V2/Configuration/PprChanStrategy"
        L1CaloFolders['PpmFineTimeRefs'] =  "/TRIGGER/L1Calo/V1/References/FineTimeReferences"
        L1CaloFolders['RunParameters'] = "/TRIGGER/L1Calo/V1/Conditions/RunParameters"
        L1CaloFolders['PprChanStrategy'] = '/TRIGGER/Receivers/Conditions/Strategy'
        L1CaloFolders['DerivedRunPars'] =   '/TRIGGER/L1Calo/V1/Conditions/DerivedRunPars'


    L1CaloFolders['PprChanDefaults'] = '/TRIGGER/L1Calo/V2/Configuration/PprChanDefaults'
    # Different folders for data and MC
    ver = 'V2' if flags.Input.isMC else 'V1'
    L1CaloFolders['DisabledTowers'] = f'/TRIGGER/L1Calo/{ver}/Conditions/DisabledTowers'
    L1CaloFolders['PpmDeadChannels'] = f'/TRIGGER/L1Calo/{ver}/Calibration/PpmDeadChannels'


    from IOVDbSvc.IOVDbSvcConfig import addFolders
    db = 'TRIGGER_ONL' if not flags.Input.isMC else 'TRIGGER_OFL'
    result = addFolders(flags,[xx for x in L1CaloFolders.values() if isinstance(x,list) for xx in x if xx!=""] +
                              [x for x in L1CaloFolders.values() if not isinstance(x,list)], db, className='CondAttrListCollection')

    from AthenaConfiguration.ComponentFactory import CompFactory

    alg = CompFactory.L1CaloCondAlg(UsePhysicsRegime = Physics,
                                    UseCalib1Regime = Calib1,
                                    UseCalib2Regime = Calib2)
    for k,v in L1CaloFolders.items(): setattr(alg,k,v)

    if flags.Input.isMC:
        # some conditions containers are not created for MC
        alg.OutputKeyDerRunsPars = ""
        alg.OutputKeyTimeRefs = ""
        alg.OutputKeyRunParameters = ""
        alg.OutputKeyPprChanStrategy = ""
        alg.OutputKeyPprConditionsRun2 = ""
        alg.OutputKeyReadoutConfig = ""
        alg.OutputKeyReadoutConfigJSON = ""

    result.addCondAlgo(alg)
        
    if readTest:
            # Set True for test L1CaloCondAlg and print condition container parameters
            L1CaloCondReader = CompFactory.L1CaloCondAlgReader()
            result.addEventAlgo(L1CaloCondReader, 'AthAlgSeq')


    return result
        

if __name__=="__main__":
        
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Exec.MaxEvents = 1
    flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2022-02'
    flags.Trigger.enableL1CaloLegacy = True 
    flags.fillFromArgs()    
    flags.lock() 
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamDecoderCfg
    from TrigT1CaloByteStream.LVL1CaloRun2ByteStreamConfig import LVL1CaloRun2ReadBSCfg
    

    acc = MainServicesCfg(flags)
    acc.merge( ByteStreamReadCfg(flags) )
    acc.merge( L1TriggerByteStreamDecoderCfg(flags) )
    acc.merge( LVL1CaloRun2ReadBSCfg(flags))

    #### Default 
    # outputlevel=1 (INFO = 1, DEBUG=2) 
    # timingRegime = "", strategy = "" are empty. Read it from DB directly 
    # readTest False, True execute L1CaloCondAlgReader  
    ####
    acc.merge(L1CaloCondAlgCfg(flags, Physics=True, Calib1=False, Calib2=False))

    #Example ...
    #acc.merge(L1CaloCondAlgCfg(flags,readTest=True, Physics=True, Calib1=False, Calib2=False))
    #acc.getCondAlgo('L1CaloCondAlg').OutputLevel = 2
    #acc.getCondAlgo('L1CaloCondAlg').timingRegime = "Calib2"
    #acc.getCondAlgo('L1CaloCondAlg').strategy = "LowMu"

    
   
    
    
    sys.exit(acc.run().isFailure())
