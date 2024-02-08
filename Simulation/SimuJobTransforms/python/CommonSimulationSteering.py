# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Possible cases:
# 1) inputEVNTFile (normal)
# 2) inputEVNT_TRFile (TrackRecords from pre-simulated events,
# used with TrackRecordGenerator)
# Three common cases here:
# 2a) Cosmics simulation
# 2b) Stopped particle simulation
# 2c) Cavern background simulation
# 3) no input file (on-the-fly generation)
# Common cases
# 3a) ParticleGun
# 3b) CosmicGenerator
# 4) inputHITSFile (re-simulation)

from PyJobTransforms.TransformUtils import executeFromFragment
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType
from SimulationConfig.SimEnums import CavernBackground


def specialConfigPreInclude(flags):
    fragment = flags.Input.SpecialConfiguration.get("preInclude", None)
    if fragment and fragment != 'NONE':
        executeFromFragment(fragment, flags) #FIXME assumes only one fragment?


def specialConfigPostInclude(flags, cfg):
    fragment = flags.Input.SpecialConfiguration.get("postInclude", None)
    if fragment and fragment != 'NONE':
        executeFromFragment(fragment, flags, cfg) #FIXME assumes only one fragment?


def CommonSimulationCfg(flags, log):
    # Configure main services and input reading (if required)
    if not flags.Input.Files:
        # Cases 3a, 3b
        from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
        cfg = MainEvgenServicesCfg(flags)
        # For Simulation we need to override the RunNumber to pick up
        # the right conditions. These next two lines are required for
        # this to work.
        cfg.getService("EventSelector").FirstLB = flags.Input.LumiBlockNumbers[0]
        cfg.getService("EventSelector").OverrideRunNumber = True
        from AthenaKernel.EventIdOverrideConfig import EvtIdModifierSvcCfg
        cfg.merge(EvtIdModifierSvcCfg(flags))
        if flags.Beam.Type is BeamType.Cosmics:
            # Case 3b: Configure the cosmic Generator
            from CosmicGenerator.CosmicGeneratorConfig import CosmicGeneratorCfg
            cfg.merge(CosmicGeneratorCfg(flags))
        else:
            # Case 3a: Configure ParticleGun
            fragment = flags.Sim.GenerationConfiguration
            if fragment and fragment != 'NONE':
                executeFromFragment(fragment, flags, cfg)
                log.info("On-the-fly generation using ParticleGun!")
            else:
                log.error("No input file or on-the-fly generation configuration provided!")
    else:
        # Cases 1, 2a, 2b, 2c, 4
        from AthenaConfiguration.MainServicesConfig import MainServicesCfg
        cfg = MainServicesCfg(flags)
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(flags))
        if flags.Sim.ReadTR or flags.Sim.CavernBackground is CavernBackground.Read:
            # Cases 2a, 2b, 2c
            from TrackRecordGenerator.TrackRecordGeneratorConfig import Input_TrackRecordGeneratorCfg
            cfg.merge(Input_TrackRecordGeneratorCfg(flags))
        if flags.Sim.ISF.ReSimulation:
            # Case 4
            if "xAOD::EventInfo#EventInfo" not in flags.Input.TypedCollections:
                from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
                cfg.merge(EventInfoCnvAlgCfg(flags))
            else:
                from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoUpdateFromContextAlgCfg
                cfg.merge(EventInfoUpdateFromContextAlgCfg(flags))
            from McEventCollectionFilter.McEventCollectionFilterConfig import TruthResetAlgCfg
            cfg.merge(TruthResetAlgCfg(flags))
            cfg.addSequence(CompFactory.AthSequencer('SimSequence'), parentName='AthAlgSeq')
            cfg.addSequence(CompFactory.AthSequencer('CopyHitSequence'), parentName='AthAlgSeq')

    if flags.Sim.ISF.ReSimulation:
        # Case 4
        from ISF_Algorithms.ISF_AlgorithmsConfig import SimEventFilterCfg, InvertedSimEventFilterCfg, RenameHitCollectionsCfg
        cfg.merge(SimEventFilterCfg(flags, sequenceName='SimSequence'))
        cfg.merge(InvertedSimEventFilterCfg(flags, sequenceName='CopyHitSequence'))
        cfg.merge(RenameHitCollectionsCfg(flags, sequenceName='CopyHitSequence'))
    else:
        #Cases 1, 2, 3
        # add BeamEffectsAlg
        from BeamEffects.BeamEffectsAlgConfig import BeamEffectsAlgCfg
        cfg.merge(BeamEffectsAlgCfg(flags))
        if flags.Input.Files:
            #Cases 1, 2
            if "xAOD::EventInfo#EventInfo" not in flags.Input.TypedCollections:
                from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
                cfg.merge(EventInfoCnvAlgCfg(flags)) ## TODO: update config so that ReSim can use the same xAOD::EventInfo
            else:
                from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoUpdateFromContextAlgCfg
                cfg.merge(EventInfoUpdateFromContextAlgCfg(flags))
        else:
            #Case 3: xAOD::EventInfo#EventInfo will have already been created
            pass

    if flags.Beam.Type is BeamType.TestBeam:
        from TBDetDescrAlg.TBDetDescrAlgConfig import TBDetDescrLoaderCfg
        cfg.merge(TBDetDescrLoaderCfg(flags))
    AcceptAlgNames=[]
    if flags.Sim.ISFRun:
        # add the ISF_MainConfig
        from ISF_Config.ISF_MainConfig import ISF_KernelCfg
        cfg.merge(ISF_KernelCfg(flags))
        AcceptAlgNames = ['ISF_Kernel_' + flags.Sim.ISF.Simulator.value]
        if flags.Sim.ISF.ReSimulation:
            AcceptAlgNames += ['RenameHitCollections']
    else:
        AcceptAlgNames = ['G4AtlasAlg']
        #add the G4AtlasAlg
        from G4AtlasAlg.G4AtlasAlgConfig import G4AtlasAlgCfg
        cfg.merge(G4AtlasAlgCfg(flags))
        from SimulationConfig.SimEnums import LArParameterization
        if flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
            cfg.getEventAlgo("ISF_CollectionMerger").InputLArEMBHits.data.sort() # temporary workaround
            cfg.getEventAlgo("ISF_CollectionMerger").InputLArEMECHits.data.sort() # temporary workaround
            cfg.getEventAlgo("ISF_CollectionMerger").InputLArFCALHits.data.sort() # temporary workaround
            cfg.getEventAlgo("ISF_CollectionMerger").InputLArHECHits.data.sort() # temporary workaround
            cfg.getEventAlgo("ISF_CollectionMerger").InputTileHits.data.sort() # temporary workaround
            cfg.getEventAlgo("ISF_CollectionMerger").InputMBTSHits.data.sort() # temporary workaround

    from SimulationConfig.SimEnums import CalibrationRun
    if flags.Sim.CalibrationRun in [CalibrationRun.LAr, CalibrationRun.LArTile, CalibrationRun.LArTileZDC]:
        from LArG4SD.LArG4SDToolConfig import DeadMaterialCalibrationHitMergerCfg
        cfg.merge(DeadMaterialCalibrationHitMergerCfg(flags))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    if flags.Output.HITSFileName:
        from SimuJobTransforms.SimOutputConfig import getStreamHITS_ItemList
        cfg.merge( OutputStreamCfg(flags,"HITS", ItemList=getStreamHITS_ItemList(flags), disableEventTag=False, AcceptAlgs=AcceptAlgNames) )
        if flags.Sim.ISF.ReSimulation:
            cfg.getEventAlgo("OutputStreamHITS").TakeItemsFromInput=False

    if flags.Output.EVNT_TRFileName:
        from SimuJobTransforms.SimOutputConfig import getStreamEVNT_TR_ItemList
        cfg.merge( OutputStreamCfg(flags,"EVNT_TR", ItemList=getStreamEVNT_TR_ItemList(flags), disableEventTag=True, AcceptAlgs=AcceptAlgNames) )

    # Add MT-safe PerfMon
    if flags.PerfMon.doFastMonMT or flags.PerfMon.doFullMonMT:
        from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
        cfg.merge(PerfMonMTSvcCfg(flags))

    # Add in-file MetaData
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    cfg.merge(SetupMetaDataForStreamCfg(flags, "HITS", AcceptAlgs=AcceptAlgNames))

    return cfg
