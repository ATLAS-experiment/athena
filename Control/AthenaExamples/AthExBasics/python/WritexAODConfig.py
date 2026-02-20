# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
# Configure the WritexAOD algorithm
# For guidelines on writing configuration scripts see the following pages:
# https://atlas-software.docs.cern.ch/athena/configuration/

# Imports of the configuration machinery
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.SystemOfUnits import GeV

def WritexAODCfg(flags):
    '''Method to configure the ReadTriggerDecision algorithm'''
    acc = ComponentAccumulator()

    # Track selection tool configuration
    # https://twiki.cern.ch/twiki/bin/view/AtlasProtected/InDetTrackSelectionTool
    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
        InDetTrackSelectionToolCfg)
    trackSelectionTool = acc.popToolsAndMerge(InDetTrackSelectionToolCfg(
        flags,
        name="TestTrackSelectorTool",
        minPt=1.0*GeV)
    )

    # Configure the algorithm.... note that the tool from above is passed
    # Then add the algorithm to the accumulator
    acc.addEventAlgo(CompFactory.WritexAOD(name="WritexAOD",
        TrackSelectionTool=trackSelectionTool,
        TrackParticlesKey="InDetTrackParticles",
        NewTrackParticlesKey="SelectedTrackParticles"))
    return acc

# Lines to allow the script to be run stand-alone via python
if __name__ == "__main__":

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import sys
    # Configuration flags
    flags = initConfigFlags()

    # Obtain default test files (user can provide their own as well)
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    # Set the input file - can also use command line via --files
    flags.Input.Files = defaultTestFiles.AOD_RUN3_MC
    
    # Number of events to process
    flags.Exec.MaxEvents = 1000
    flags.fillFromArgs()
    flags.lock()
    
    # Configure the file reading machinery
    cfg = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    # Configure the output writing
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    cfg.merge(OutputStreamCfg(
        flags,
        streamName="TestStream",
        ItemList=["EventInfo#*",
                  "xAOD::TrackParticleContainer#SelectedTrackParticles"]) 
    )

    # Run the job
    cfg.merge(WritexAODCfg(flags))
    sys.exit(cfg.run().isFailure())
