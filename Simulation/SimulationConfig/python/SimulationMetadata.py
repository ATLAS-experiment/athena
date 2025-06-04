# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
### This module contains functions which may need to peek at the input file metadata
from AthenaCommon.Logging import logging
from AthenaConfiguration.Enums import FlagEnum, ProductionStep
from AthenaKernel.EventIdOverrideConfig import getMinMaxRunNumbers

folderName = "/Simulation/Parameters"


def fillAtlasMetadata(flags, dbFiller):
    simMDlog = logging.getLogger('Sim_Metadata')
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
            if "FastCalo.doEMECFCS" in flag and not flags.Sim.FastCalo.doEMECFCS:
                # This flag is only written to metadata in case FCS is used in EMEC region (1.5 < AbsEta < 3.2, Ekin < 8 GeV) is set
                continue
            if "FastCalo.doPunchThrough" in flag and not flags.Sim.FastCalo.doPunchThrough:
                # This flag is only written to metadata in case PunchThroughG4Tool is set
                continue

            key = flag.split(".")[-1] #use final part of flag as the key
            value = flags._get(flag)
            if isinstance(value, FlagEnum):
                value = value.value
            if not isinstance(value, str):
                value = str(value)
            dbFiller.addSimParam(key, value)
            simMDlog.info('SimulationMetaData: setting "%s" to be %s', key, value)

    dbFiller.addSimParam('G4Version', flags.Sim.G4Version)
    dbFiller.addSimParam('RunType', 'atlas')
    dbFiller.addSimParam('beamType', flags.Beam.Type.value)
    dbFiller.addSimParam('SimLayout', flags.GeoModel.AtlasVersion)
    dbFiller.addSimParam('MagneticField', 'AtlasFieldSvc') # TODO hard-coded for now for consistency with old-style configuration.

    #---------
    ## Simulated detector flags: add each enabled detector to the simulatedDetectors list
    from AthenaConfiguration.DetectorConfigFlags import getEnabledDetectors
    simDets = ['Truth'] + getEnabledDetectors(flags)
    simMDlog.info("Setting 'SimulatedDetectors' = %r", simDets)
    dbFiller.addSimParam('SimulatedDetectors', repr(simDets))

    ## Hard-coded simulation hit file magic number (for major changes)
    dbFiller.addSimParam('hitFileMagicNumber', '0') ##FIXME Remove this?

    if flags.Sim.ISFRun:
        dbFiller.addSimParam('Simulator', flags.Sim.ISF.Simulator.value)
        dbFiller.addSimParam('SimulationFlavour', flags.Sim.ISF.Simulator.value.replace('MT', '')) # used by egamma
    else:
        # TODO hard-code for now, but set flag properly later
        dbFiller.addSimParam('Simulator', 'AtlasG4')
        dbFiller.addSimParam('SimulationFlavour', 'AtlasG4')

    ## Data overlay
    if flags.Common.isOverlay and flags.Overlay.DataOverlay:
        dbFiller.addSimParam('IsDataOverlay', 'True')


def writeSimulationParametersMetadata(flags):
    simMDlog = logging.getLogger('Sim_Metadata')
    from IOVDbMetaDataTools import ParameterDbFiller
    dbFiller = ParameterDbFiller.ParameterDbFiller()
    myRunNumber, myEndRunNumber = getMinMaxRunNumbers(flags)
    simMDlog.debug('ParameterDbFiller BeginRun = %s', str(myRunNumber) )
    dbFiller.setBeginRun(myRunNumber)
    simMDlog.debug('ParameterDbFiller EndRun   = %s', str(myEndRunNumber) )
    dbFiller.setEndRun(myEndRunNumber)

    fillAtlasMetadata(flags, dbFiller)

    #-------------------------------------------------
    # Make the MetaData Db
    #-------------------------------------------------
    dbFiller.genSimDb()

    return writeSimulationParameters(flags)


def readSimulationParameters(flags):
    """Read digitization parameters metadata"""
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    if flags.Common.ProductionStep not in [ProductionStep.Simulation, ProductionStep.FastChain]:
        return addFolders(flags, folderName, className="AthenaAttributeList", tag="HEAD")

    # Here we are in a job which runs simulation, so the
    # /Simulation/Parameters metadata is not present in the
    # input file and will be created during the job
    return addFolders(flags, folderName, detDb="SimParams.db", db="SIMPARAM", className="AthenaAttributeList")


def writeSimulationParameters(flags):
    """Write digitization parameters metadata"""
    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg, addFolders
    acc = IOVDbSvcCfg(flags, FoldersToMetaData=[folderName])
    acc.merge(addFolders(flags, folderName, detDb="SimParams.db", db="SIMPARAM"))
    return acc
