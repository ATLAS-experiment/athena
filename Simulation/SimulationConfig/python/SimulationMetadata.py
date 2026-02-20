# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
### This module contains functions which may need to peek at the input file metadata
from AthenaCommon.Logging import logging
from AthenaConfiguration.Enums import FlagEnum, ProductionStep
from AthenaKernel.EventIdOverrideConfig import getMinMaxRunNumbers

folderName = "/Simulation/Parameters"


def collectSimulationMetadata(flags):
    """Collect simulation metadata parameters as a dictionary"""
    simMDlog = logging.getLogger('Sim_Metadata')
    params = {}

    #add all flags to the metadata
    #todo - only add certain ones?
    #in future this should be a ConfigFlags method...?
    for flag in sorted(flags._flagdict): #only sim
        if flag.startswith("Sim."):
            if "GenerationConfiguration" in flag:
                # This flag is only temporarily defined in the SimConfigFlags module
                continue
            if "Twiss" in flag and not flags.Detector.GeometryForward:
                # The various Twiss flags should only be written out when Forward Detector simulation is enabled
                continue
            if "UseShadowEvent" in flag and not flags.Sim.UseShadowEvent:
                # This flag is added temporarily to allow a new approach to quasi-stable particle simulation to be tested.
                continue
            if "VertexTimeWidth" in flag and not flags.Sim.VertexTimeSmearing:
                # This flag is only written to metadata when vertex time smearing is enabled
                continue
            if "RunOnGPU" in flag and not flags.Sim.ISF.Simulator.usesFastCaloSim():
                # This flag is only written to metadata when FastCaloSim/FastCaloGAN is enabled
                continue
            if "FastCalo.ParamsInputFilename" in flag and not flags.Sim.ISF.Simulator.usesFastCaloSim():
                # This flag is only written to metadata when FastCaloSim/FastCaloGAN is enabled
                continue
            if "SimplifiedGeoPath" in flag and not flags.Sim.SimplifiedGeoPath:
                # This flag is only written to metadata in case the FastCaloSim simplified geometry path is set
                continue
            if "FastCalo.doPunchThrough" in flag and not flags.Sim.FastCalo.doPunchThrough:
                # This flag is only written to metadata in case PunchThroughG4Tool is set
                continue
            if "UseG4Workers" in flag:
                # This flag is still experimental, and should not be recorded in metadata yet
                continue

            key = flag.split(".")[-1] #use final part of flag as the key
            value = flags._get(flag)
            if isinstance(value, FlagEnum):
                value = value.value
            if not isinstance(value, str):
                value = str(value)
            params[key] = value
            simMDlog.info('SimulationMetaData: setting "%s" to be %s', key, value)

    params['G4Version'] = flags.Sim.G4Version
    params['RunType'] = 'atlas'
    params['beamType'] = flags.Beam.Type.value
    params['SimLayout'] = flags.GeoModel.AtlasVersion
    params['MagneticField'] = 'AtlasFieldSvc' # TODO hard-coded for now for consistency with old-style configuration.

    #---------
    ## Simulated detector flags: add each enabled detector to the simulatedDetectors list
    from AthenaConfiguration.DetectorConfigFlags import getEnabledDetectors
    simDets = ['Truth'] + getEnabledDetectors(flags)
    simMDlog.info("Setting 'SimulatedDetectors' = %r", simDets)
    params['SimulatedDetectors'] = repr(simDets)

    ## Hard-coded simulation hit file magic number (for major changes)
    params['hitFileMagicNumber'] = '0' ##FIXME Remove this?

    if flags.Sim.ISFRun:
        params['Simulator'] = flags.Sim.ISF.Simulator.value
        params['SimulationFlavour'] = flags.Sim.ISF.Simulator.value.replace('MT', '') # used by egamma
    else:
        # TODO hard-code for now, but set flag properly later
        params['Simulator'] = 'AtlasG4'
        params['SimulationFlavour'] = 'AtlasG4'

    ## Data overlay
    if flags.Common.isOverlay and flags.Overlay.DataOverlay:
        params['IsDataOverlay'] = 'True'

    return params


def fillAtlasMetadata(flags, dbFiller):
    """Fill ParameterDbFiller with simulation metadata (sqlite mode interface)"""
    params = collectSimulationMetadata(flags)
    for key, value in params.items():
        dbFiller.addSimParam(key, value)


def writeSimulationParametersMetadata(flags):
    simMDlog = logging.getLogger('Sim_Metadata')
    myRunNumber, myEndRunNumber = getMinMaxRunNumbers(flags)
    simMDlog.debug('Metadata BeginRun = %s', str(myRunNumber))
    simMDlog.debug('Metadata EndRun   = %s', str(myEndRunNumber))

    if flags.IOVDb.WriteParametersAsMetaData:
        # Direct in-file metadata mode: bypass intermediate sqlite files
        from IOVDbMetaDataTools.ParameterWriterConfig import writeParametersToMetaData
        simMDlog.info('Writing simulation parameters directly to in-file metadata (bypassing SimParams.db)')
        params = collectSimulationMetadata(flags)
        return writeParametersToMetaData(flags, folderName, params, myRunNumber, myEndRunNumber)
    else:
        # Sqlite mode: write to SimParams.db intermediate file
        from IOVDbMetaDataTools import ParameterDbFiller
        simMDlog.info('Writing simulation parameters to intermediate sqlite file (SimParams.db)')
        dbFiller = ParameterDbFiller.ParameterDbFiller()
        dbFiller.setBeginRun(myRunNumber)
        dbFiller.setEndRun(myEndRunNumber)

        fillAtlasMetadata(flags, dbFiller)

        #-------------------------------------------------
        # Make the MetaData Db
        #-------------------------------------------------
        dbFiller.genSimDb()

        return writeSimulationParameters(flags)


def readSimulationParameters(flags):
    """Read simulation parameters metadata"""
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from IOVDbSvc.IOVDbSvcConfig import addFolders

    # Direct in-file metadata mode: IOVDbMetaDataTool populates ConditionStore from file metadata
    # Exception: In overlay mode, always use IOVDbSvc since input files may not have parameters in metadata
    if flags.IOVDb.WriteParametersAsMetaData and not flags.Common.isOverlay:
        return ComponentAccumulator()

    # Sqlite mode: use IOVDbSvc to read and populate ConditionStore
    if flags.Common.ProductionStep in [ProductionStep.Simulation, ProductionStep.FastChain]:
        # Reading from intermediate sqlite file SimParams.db (during simulation job)
        return addFolders(flags, folderName, detDb="SimParams.db", db="SIMPARAM", className="AthenaAttributeList")
    else:
        # Reading from input file metadata via IOVDbSvc
        return addFolders(flags, folderName, className="AthenaAttributeList", tag="HEAD")


def writeSimulationParameters(flags):
    """Write digitization parameters metadata"""
    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg, addFolders
    acc = IOVDbSvcCfg(flags, FoldersToMetaData=[folderName])
    acc.merge(addFolders(flags, folderName, detDb="SimParams.db", db="SIMPARAM"))
    return acc
