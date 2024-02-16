# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import FlagEnum
from AthenaKernel.EventIdOverrideConfig import getMinMaxRunNumbers

folderName = "/Digitization/Parameters"


def writeDigitizationMetadata(flags):
    from IOVDbMetaDataTools import ParameterDbFiller
    dbFiller = ParameterDbFiller.ParameterDbFiller()
    myRunNumber, myEndRunNumber = getMinMaxRunNumbers(flags)
    logDigitizationWriteMetadata = logging.getLogger( 'DigitizationParametersConfig' )
    logDigitizationWriteMetadata.debug('ParameterDbFiller BeginRun = %s', str(myRunNumber) )
    dbFiller.setBeginRun(myRunNumber)
    logDigitizationWriteMetadata.debug('ParameterDbFiller EndRun   = %s', str(myEndRunNumber) )
    dbFiller.setEndRun(myEndRunNumber)
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
            dbFiller.addDigitParam(testKey, testValue)
            logDigitizationWriteMetadata.info('DigitizationMetaData: setting "%s" to be %s', testKey, testValue)
        else :
            logDigitizationWriteMetadata.debug('DigitizationMetaData: ConfigFlags.%s is not available.', testFlag)
    del digitMetaDataKeys

    # doMuonNoise no actual flag in new-style
    testKey = "doMuonNoise"
    testValue = str(not flags.Common.isOverlay) # Hardcoded for now
    dbFiller.addDigitParam(testKey, testValue)
    logDigitizationWriteMetadata.info('DigitizationMetaData: setting "%s" to be %s', testKey, testValue)

    # Bunch Structure - hardcoded for now
    testKey = "BeamIntensityPattern"
    if flags.Digitization.PileUp:
        testValue = str(flags.Digitization.PU.BeamIntensityPattern)
    else:
        testValue = "None"
    logDigitizationWriteMetadata.info('DigitizationMetaData: setting "%s" to be %s', testKey, testValue)
    dbFiller.addDigitParam64(testKey, testValue)

    # intraTrainBunchSpacing - hardcoded for now
    testKey = "intraTrainBunchSpacing"
    testValue = str(25) # This should be either be determined from the BeamIntensityPattern or set as flags.Beam.BunchSpacing
    dbFiller.addDigitParam(testKey, testValue)
    logDigitizationWriteMetadata.info('DigitizationMetaData: setting "%s" to be %s', testKey, testValue)

    ## Digitized detector flags: add each enabled detector to the DigitizedDetectors list - might be better to determine this from the OutputStream or CA-itself? Possibly redundant info though?
    from AthenaConfiguration.DetectorConfigFlags import getEnabledDetectors
    digiDets = ['Truth'] + getEnabledDetectors(flags)
    logDigitizationWriteMetadata.info("Setting 'DigitizedDetectors' = %s" , repr(digiDets))
    dbFiller.addDigitParam('DigitizedDetectors', repr(digiDets))

    #-------------------------------------------------
    # Make the MetaData Db
    #-------------------------------------------------
    dbFiller.genDigitDb()

    return writeDigitizationParameters(flags)


def readDigitizationParameters(flags):
    """Read digitization parameters metadata"""
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    if flags.Digitization.ReadParametersFromDB:
        # Here we are in a job which runs digitization, so the
        # /Digitization/Parameters metadata is not present in the
        # input file and will be created during the job
        return addFolders(flags, folderName, detDb="DigitParams.db", db="DIGPARAM", className="AthenaAttributeList")
    return addFolders(flags, folderName, className="AthenaAttributeList", tag="HEAD")


def writeDigitizationParameters(flags):
    """Write digitization parameters metadata"""
    if flags.Overlay.DataOverlay:
        return ComponentAccumulator()

    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg, addFolders
    acc = IOVDbSvcCfg(flags, FoldersToMetaData=[folderName])
    acc.merge(addFolders(flags, folderName, detDb="DigitParams.db", db="DIGPARAM"))
    return acc
