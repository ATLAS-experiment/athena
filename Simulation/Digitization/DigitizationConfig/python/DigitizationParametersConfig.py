# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import FlagEnum
from AthenaKernel.EventIdOverrideConfig import getMinMaxRunNumbers

folderName = "/Digitization/Parameters"


def collectDigitizationMetadata(flags):
    """Collect digitization metadata parameters as a dictionary"""
    logDigitizationWriteMetadata = logging.getLogger('DigitizationParametersConfig')
    params = {}

    #-------------------------------------------------
    # Adding jobproperties to the list of MetaData
    #-------------------------------------------------
    # Here list the digitization jobproperties we want to write out as MetaData.
    digitMetaDataKeys = { 'doInDetNoise' : 'Digitization.DoInnerDetectorNoise',
                          'doCaloNoise' : 'Digitization.DoCaloNoise',
                          'bunchSpacing' : 'Beam.BunchSpacing',
                          'beamType' : 'Beam.Type',
                          'IOVDbGlobalTag' : 'IOVDb.GlobalTag',
                          'DetDescrVersion' : 'GeoModel.AtlasVersion',
                          'finalBunchCrossing' : 'Digitization.PU.FinalBunchCrossing',
                          'initialBunchCrossing' : 'Digitization.PU.InitialBunchCrossing',
                          'physicsList' : 'Sim.PhysicsList', #TODO migrate clients to use /Simulation/Parameters metadata?
                          'digiSteeringConf' : 'Digitization.DigiSteeringConf',
                          'pileUp' : 'Digitization.PileUp',
                      }
    logDigitizationWriteMetadata.info('Filling Digitization MetaData')
    for testKey, testFlag in digitMetaDataKeys.items():
        if flags.hasFlag(testFlag):
            testValue = flags._get(testFlag)
            if isinstance(testValue, FlagEnum):
                testValue = testValue.value
            if not isinstance(testValue, str):
                testValue = str(testValue)
            params[testKey] = testValue
            logDigitizationWriteMetadata.info('DigitizationMetaData: setting "%s" to be %s', testKey, testValue)
        else :
            logDigitizationWriteMetadata.debug('DigitizationMetaData: ConfigFlags.%s is not available.', testFlag)

    # doMuonNoise no actual flag in new-style
    testKey = "doMuonNoise"
    testValue = str(not flags.Common.isOverlay) # Hardcoded for now
    params[testKey] = testValue
    logDigitizationWriteMetadata.info('DigitizationMetaData: setting "%s" to be %s', testKey, testValue)

    # Bunch Structure - hardcoded for now
    testKey = "BeamIntensityPattern"
    if flags.Digitization.PileUp:
        testValue = str(flags.Digitization.PU.BeamIntensityPattern)
    else:
        testValue = "None"
    logDigitizationWriteMetadata.info('DigitizationMetaData: setting "%s" to be %s', testKey, testValue)
    params[testKey] = testValue

    # intraTrainBunchSpacing - hardcoded for now
    testKey = "intraTrainBunchSpacing"
    testValue = str(25) # This should be either be determined from the BeamIntensityPattern or set as flags.Beam.BunchSpacing
    params[testKey] = testValue
    logDigitizationWriteMetadata.info('DigitizationMetaData: setting "%s" to be %s', testKey, testValue)

    ## Digitized detector flags: add each enabled detector to the DigitizedDetectors list - might be better to determine this from the OutputStream or CA-itself? Possibly redundant info though?
    from AthenaConfiguration.DetectorConfigFlags import getEnabledDetectors
    digiDets = ['Truth'] + getEnabledDetectors(flags)
    logDigitizationWriteMetadata.info("Setting 'DigitizedDetectors' = %s" , repr(digiDets))
    params['DigitizedDetectors'] = repr(digiDets)

    return params


def writeDigitizationMetadata(flags):
    myRunNumber, myEndRunNumber = getMinMaxRunNumbers(flags)
    logDigitizationWriteMetadata = logging.getLogger('DigitizationParametersConfig')
    logDigitizationWriteMetadata.debug('Metadata BeginRun = %s', str(myRunNumber))
    logDigitizationWriteMetadata.debug('Metadata EndRun   = %s', str(myEndRunNumber))

    if flags.IOVDb.WriteParametersAsMetaData:
        # Direct in-file metadata mode: bypass intermediate sqlite files
        from IOVDbMetaDataTools.ParameterWriterConfig import writeParametersToMetaData
        logDigitizationWriteMetadata.info('Writing digitization parameters directly to in-file metadata (bypassing DigitParams.db)')
        params = collectDigitizationMetadata(flags)
        return writeParametersToMetaData(flags, folderName, params, myRunNumber, myEndRunNumber)
    else:
        # Sqlite mode: write to DigitParams.db intermediate file
        from IOVDbMetaDataTools import ParameterDbFiller
        logDigitizationWriteMetadata.info('Writing digitization parameters to intermediate sqlite file (DigitParams.db)')
        dbFiller = ParameterDbFiller.ParameterDbFiller()
        dbFiller.setBeginRun(myRunNumber)
        dbFiller.setEndRun(myEndRunNumber)

        # Collect parameters and write to dbFiller
        params = collectDigitizationMetadata(flags)
        for key, value in params.items():
            # Handle the special case of BeamIntensityPattern which uses addDigitParam64
            if key == "BeamIntensityPattern":
                dbFiller.addDigitParam64(key, value)
            else:
                dbFiller.addDigitParam(key, value)

        dbFiller.genDigitDb()
        return writeDigitizationParameters(flags)


def readDigitizationParameters(flags):
    """Read digitization parameters metadata"""
    from IOVDbSvc.IOVDbSvcConfig import addFolders

    # Direct in-file metadata mode: IOVDbMetaDataTool populates ConditionStore from file metadata
    # Exception: In overlay mode, always use IOVDbSvc since background file may not have parameters in metadata
    if flags.IOVDb.WriteParametersAsMetaData and not flags.Common.isOverlay:
        return ComponentAccumulator()

    # Sqlite mode or overlay mode: use IOVDbSvc to read and populate ConditionStore
    if flags.Digitization.ReadParametersFromDB:
        # Reading from intermediate sqlite file DigitParams.db (during digitization job)
        return addFolders(flags, folderName, detDb="DigitParams.db", db="DIGPARAM", className="AthenaAttributeList")
    else:
        # Reading from input file metadata via IOVDbSvc
        return addFolders(flags, folderName, className="AthenaAttributeList", tag="HEAD")


def writeDigitizationParameters(flags):
    """Write digitization parameters metadata (sqlite mode only)"""
    if flags.Overlay.DataOverlay:
        return ComponentAccumulator()

    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg, addFolders
    acc = IOVDbSvcCfg(flags, FoldersToMetaData=[folderName])
    # Note: addFolders only needed in sqlite mode since direct mode handles it in ParameterMetaDataWriterCfg
    acc.merge(addFolders(flags, folderName, detDb="DigitParams.db", db="DIGPARAM"))
    return acc
