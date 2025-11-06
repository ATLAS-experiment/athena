# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# File: LumiBlockComps/python/LuminosityCondAlgConfig.py
# Created: May 2019, sss, from existing LuminosityToolDefault.
# Purpose: Configure LuminosityCondAlg.
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType
from IOVDbSvc.IOVDbSvcConfig import addFolders
from AthenaCommon.Logging import logging
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.Enums import ProductionStep

@AccumulatorCache
def LuminosityCondAlgCfg (flags, useOnlineLumi=None, suffix=None):
    # This function, without the useOnlineLumi and suffix arguments,  will set up a default configuration 
    # appropriate to the job; the conditions object will be called LuminosityCondData
    # Can override whether it sets up the online lumi (but then you are strongly urged to use the suffix
    # argument!)
    # Suffix will allow you to make an object with a different name, e.g. LuminosityCondDataOnline
    log = logging.getLogger ('LuminosityCondAlgCfg')
    name = 'LuminosityCondAlg'
    result = ComponentAccumulator()

    if suffix is None: suffix = ''
    if useOnlineLumi is not None and suffix is None:
        log.warning('useOnlineLumi argument is provided but not a suffix for the lumi object name. Is this really intended?')

    if flags.Input.isMC:
        kwargs = luminosityCondAlgMCCfg (flags, name, result)
    elif ((useOnlineLumi is None and flags.Common.useOnlineLumi)
          or (useOnlineLumi is not None and useOnlineLumi)):
        kwargs = luminosityCondAlgOnlineCfg (flags, name, result)
    elif flags.IOVDb.DatabaseInstance == 'COMP200':
        kwargs = luminosityCondAlgRun1Cfg (flags, name, result)
    elif flags.IOVDb.DatabaseInstance == 'CONDBR2':
        kwargs = luminosityCondAlgRun2Cfg (flags, name, result)

    else:
        log.warning ("LuminosityCondAlgCfg can't resolve database instance = %s, assume Run2!" % flags.IOVDb.DatabaseInstance)
        kwargs = luminosityCondAlgRun2Cfg (flags, name, result)
  
    if flags.Common.ProductionStep is ProductionStep.PileUpPretracking:
         kwargs.setdefault("EventInfoKey", flags.Overlay.BkgPrefix + "EventInfo")

    kwargs.setdefault("LuminosityOutputKey", "LuminosityCondData" + suffix)

    result.addCondAlgo(CompFactory.LuminosityCondAlg(name, **kwargs))
    return result


def luminosityCondAlgMCCfg (flags, name, result):
    from DigitizationConfig.DigitizationParametersConfig import readDigitizationParameters
    result.merge(readDigitizationParameters(flags))
    return { 'LuminosityFolderInputKey' : '',
             'DigitizationFolderInputKey' : '/Digitization/Parameters',
             'OnlineLumiCalibrationInputKey' : '',
             'BunchLumisInputKey' : '',
             'BunchGroupInputKey' : '',
             'FillParamsInputKey' : '',
             'IsMC' : True }


# Configuration for offline default luminosity used in Run2
def luminosityCondAlgRun2Cfg (flags, name, result):
    log = logging.getLogger(name)

    kwargs = {}

    # Check if this is express stream or bulk
    if flags.Common.doExpressProcessing:
        lumiFolder  = "/TRIGGER/LUMI/OnlPrefLumi"
        result.merge (addFolders (flags, lumiFolder, 'TRIGGER_ONL',
                                  className = 'CondAttrListCollection'))

    else:
        lumiFolder = "/TRIGGER/OFLLUMI/OflPrefLumi"
        result.merge (addFolders (flags, lumiFolder, 'TRIGGER_OFL',
                                  className = 'CondAttrListCollection'))

    log.info ("luminosityCondAlgRun2Config requested %s", lumiFolder)
    kwargs['LuminosityFolderInputKey'] = lumiFolder

    log.info ("Created Run2 %s using folder %s" % (name, lumiFolder))

    # Need the calibration just to get the proper MuToLumi value
    from CoolLumiUtilities.OnlineLumiCalibrationCondAlgConfig \
        import OnlineLumiCalibrationCondAlgCfg
    result.merge (OnlineLumiCalibrationCondAlgCfg(flags))
    olalg = result.getCondAlgo ('OnlineLumiCalibrationCondAlg')
    kwargs['OnlineLumiCalibrationInputKey'] = olalg.LumiCalibOutputKey
    
    # Other folder names should be blank.
    kwargs['BunchLumisInputKey'] = ''
    kwargs['BunchGroupInputKey'] = ''
    kwargs['FillParamsInputKey'] = ''

    # if cosmics, suppress warnings.
    if flags.Beam.Type is BeamType.Cosmics:
        kwargs['ExpectInvalid'] = True

    return kwargs


# Configuration for offline default luminosity used in Run1
def luminosityCondAlgRun1Cfg (flags, name, result):
    log = logging.getLogger(name)

    kwargs = {}
       
    # Check if this is express stream or bulk
    if flags.Common.doExpressProcessing:
        lumiFolder  = "/TRIGGER/LUMI/LBLESTONL"
        result.merge (addFolders (flags, lumiFolder, 'TRIGGER_ONL',
                                  className = 'CondAttrListCollection'))

    else:
        lumiFolder = "/TRIGGER/OFLLUMI/LBLESTOFL"
        result.merge (addFolders (flags, lumiFolder, 'TRIGGER_OFL',
                                  className = 'CondAttrListCollection'))

    log.info ("configureLuminosityCondAlgRun1 requested %s", lumiFolder)
    kwargs['LuminosityFolderInputKey'] = lumiFolder

    # Configure input conditions data.
    from CoolLumiUtilities.FillParamsCondAlgConfig \
        import FillParamsCondAlgCfg
    result.merge (FillParamsCondAlgCfg (flags))
    fpalg = result.getCondAlgo ('FillParamsCondAlg')
    kwargs['FillParamsInputKey'] = fpalg.FillParamsOutputKey

    from CoolLumiUtilities.BunchLumisCondAlgConfig \
        import BunchLumisCondAlgCfg
    result.merge (BunchLumisCondAlgCfg (flags))
    blalg = result.getCondAlgo ('BunchLumisCondAlg')
    kwargs['BunchLumisInputKey'] = blalg.BunchLumisOutputKey

    from CoolLumiUtilities.BunchGroupCondAlgConfig \
        import BunchGroupCondAlgCfg
    result.merge (BunchGroupCondAlgCfg (flags))
    bgalg = result.getCondAlgo ('BunchGroupCondAlg')
    kwargs['BunchGroupInputKey'] = bgalg.BunchGroupOutputKey

    from CoolLumiUtilities.OnlineLumiCalibrationCondAlgConfig \
        import OnlineLumiCalibrationCondAlgCfg
    result.merge (OnlineLumiCalibrationCondAlgCfg (flags))
    olalg = result.getCondAlgo ('OnlineLumiCalibrationCondAlg')
    kwargs['OnlineLumiCalibrationInputKey'] = olalg.LumiCalibOutputKey

    return kwargs



# Configuration for online luminosity.
def luminosityCondAlgOnlineCfg (flags, name, result):
    log = logging.getLogger(name)

    kwargs = {}

    # Keep values for invalid data
    kwargs['SkipInvalid'] = False

    if flags.IOVDb.DatabaseInstance == 'COMP200': # Run1
        folder  = "/TRIGGER/LUMI/LBLESTONL"
        result.merge (addFolders (flags, folder, 'TRIGGER_ONL',
                                  className = 'CondAttrListCollection'))
      
    else: # Run 2+
        if flags.IOVDb.DatabaseInstance != 'CONDBR2':
            log.warning("LuminosityCondAlgOnlineCfg can't resolve DatabaseInstance = %s, assuming CONDBR2!", flags.IOVDb.DatabaseInstance)

        folder  = "/TRIGGER/LUMI/HLTPrefLumi"
        result.merge (addFolders (flags, folder, 'TRIGGER_ONL',
                                  className = 'CondAttrListCollection',
                                  extensible = flags.Trigger.doHLT and flags.Trigger.Online.isPartition))

    kwargs['LuminosityFolderInputKey'] = folder
    log.info ("Created online %s using folder %s" % (name, folder))

    # Need the calibration just to get the proper MuToLumi value
    from CoolLumiUtilities.OnlineLumiCalibrationCondAlgConfig \
        import OnlineLumiCalibrationCondAlgCfg
    result.merge (OnlineLumiCalibrationCondAlgCfg(flags))
    olalg = result.getCondAlgo ('OnlineLumiCalibrationCondAlg')
    kwargs['OnlineLumiCalibrationInputKey'] = olalg.LumiCalibOutputKey
    
    # Other folder names should be blank.
    kwargs['BunchLumisInputKey'] = ''
    kwargs['BunchGroupInputKey'] = ''
    kwargs['FillParamsInputKey'] = ''

    return kwargs


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    print ('--- run2')
    flags1 = initConfigFlags()
    flags1.Input.Files = defaultTestFiles.RAW_RUN2
    flags1.lock()
    acc1 = LuminosityCondAlgCfg (flags1)
    acc1.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc1.getService('IOVDbSvc').Folders)
    acc1.wasMerged()

    print ('--- run2/express')
    flags2 = initConfigFlags()
    flags2.Input.Files = defaultTestFiles.RAW_RUN2
    flags2.Common.doExpressProcessing = True
    flags2.lock()
    acc2 = LuminosityCondAlgCfg (flags2)
    acc2.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc2.getService('IOVDbSvc').Folders)
    acc2.wasMerged()

    print ('--- run1')
    flags3 = initConfigFlags()
    flags3.Input.Files = defaultTestFiles.RAW_RUN2
    flags3.Input.ProjectName = 'data12_8TeV'
    flags3.lock()
    acc3 = LuminosityCondAlgCfg (flags3)
    acc3.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc3.getService('IOVDbSvc').Folders)
    acc3.wasMerged()

    print ('--- run1/express')
    flags4 = initConfigFlags()
    flags4.Input.Files = defaultTestFiles.RAW_RUN2
    flags4.Input.ProjectName = 'data12_8TeV'
    flags4.Common.doExpressProcessing = True
    flags4.lock()
    acc4 = LuminosityCondAlgCfg (flags4)
    acc4.printCondAlgs(summariseProps=True)
    print ('IOVDbSvc:', acc4.getService('IOVDbSvc').Folders)
    acc4.wasMerged()

    print ('--- mc')
    flags5 = initConfigFlags()
    flags5.Input.Files = defaultTestFiles.ESD
    flags5.lock()
    acc5 = LuminosityCondAlgCfg (flags5)
    acc5.printCondAlgs(summariseProps=True)
    acc5.wasMerged()

    print ('--- online')
    flags6 = initConfigFlags()
    flags6.Input.Files = defaultTestFiles.RAW_RUN2
    flags6.Common.useOnlineLumi = True
    flags6.lock()
    acc6 = LuminosityCondAlgCfg (flags6)
    acc6.printCondAlgs(summariseProps=True)
    acc6.wasMerged()

    print ('--- forced online')
    flags7 = initConfigFlags()
    flags7.Input.Files = defaultTestFiles.RAW_RUN2
    flags7.lock()
    acc7 = LuminosityCondAlgCfg (flags7, useOnlineLumi=True, suffix='Online')
    acc7.printCondAlgs(summariseProps=True)
    acc7.wasMerged()
