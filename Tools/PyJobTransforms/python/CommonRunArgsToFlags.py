# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Reset preload libs for proper execution of child-processes (ATR-26769).
# We only put this here because this is executed by all CA transform skeletons:
import os
if 'LD_PRELOAD_ORIG' in os.environ:
    os.environ['LD_PRELOAD'] = os.getenv('LD_PRELOAD_ORIG')
    os.unsetenv('LD_PRELOAD_ORIG')


#Translate the commonly used runArgs into configFlags
def commonRunArgsToFlags(runArgs,configFlags):
    ## Max/skip events
    if hasattr(runArgs,"skipEvents"):
        configFlags.Exec.SkipEvents=runArgs.skipEvents

    if hasattr(runArgs,"maxEvents"):
        configFlags.Exec.MaxEvents=runArgs.maxEvents

    if hasattr(runArgs,"conditionsTag"):
        configFlags.IOVDb.GlobalTag=runArgs.conditionsTag

    if hasattr(runArgs,"geometryVersion"): 
        configFlags.GeoModel.AtlasVersion=runArgs.geometryVersion

    if hasattr(runArgs,"geometrySQLite"):
        configFlags.GeoModel.SQLiteDB=runArgs.geometrySQLite

    if hasattr(runArgs,"triggerConfig"): 
        configFlags.Trigger.triggerConfig=runArgs.triggerConfig

    if hasattr(runArgs,"beamType"):
        from AthenaConfiguration.Enums import BeamType
        configFlags.Beam.Type=BeamType(runArgs.beamType)

    # Read the input entries for some common types
    for ftype in ["BS", "EVNT", "HITS", "RDO", "ESD", "AOD"]:
        if hasattr(runArgs, f"input{ftype}FileNentries"):
            configFlags.Input.FileNentries = getattr(runArgs, f"input{ftype}FileNentries")

    ## Threading arguments
    if hasattr(runArgs,"nprocs"):
        configFlags.Concurrency.NumProcs = runArgs.nprocs
        # also parse MP stuff in case of nprocs > 0
        if runArgs.nprocs > 0:
            from AthenaMP.AthenaMPConfig import athenaMPRunArgsToFlags
            athenaMPRunArgsToFlags(runArgs, configFlags)

    if hasattr(runArgs,"threads"):
        configFlags.Concurrency.NumThreads = runArgs.threads

    if hasattr(runArgs,"concurrentEvents"):
        configFlags.Concurrency.NumConcurrentEvents = runArgs.concurrentEvents

    ## Executor steps
    if hasattr(runArgs,"totalExecutorSteps"):
        configFlags.ExecutorSplitting.TotalSteps = runArgs.totalExecutorSteps

    if hasattr(runArgs,"executorStep"):
        configFlags.ExecutorSplitting.Step = runArgs.executorStep

    if hasattr(runArgs,"executorEventCounts"):
        configFlags.ExecutorSplitting.TotalEvents = configFlags.Exec.MaxEvents
        configFlags.Exec.MaxEvents = runArgs.executorEventCounts[configFlags.ExecutorSplitting.Step]

    if hasattr(runArgs,"executorEventSkips"):
        configFlags.Exec.SkipEvents = runArgs.executorEventSkips[configFlags.ExecutorSplitting.Step]
