# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType
from SimulationConfig.SimEnums import BeamPipeSimMode, CalibrationRun, CavernBackground, InDetParameterization, LArParameterization
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator



@AccumulatorCache
def FastSimulationToolListCfg(flags):
    result = ComponentAccumulator()
    tools = []
    if flags.Sim.InDetParameterization is InDetParameterization.FatrasG4:
        from G4FastSimulation.G4FastSimulationConfig import FatrasG4Cfg
        tools += [ result.popToolsAndMerge(FatrasG4Cfg(flags)) ]

    if flags.Sim.InDetParameterization is InDetParameterization.AFatrasG4:
        from G4FastSimulation.G4FastSimulationConfig import AFatrasG4Cfg
        tools += [ result.popToolsAndMerge(AFatrasG4Cfg(flags)) ]    

    if flags.Detector.GeometryBpipe:
        if  not flags.Detector.GeometryFwdRegion and (flags.Detector.GeometryAFP or flags.Detector.GeometryALFA or flags.Detector.GeometryZDC):
            # equivalent of simFlags.ForwardDetectors() == 2:
            from ForwardTransport.ForwardTransportConfig import ForwardTransportModelCfg
            tools += [ result.popToolsAndMerge(ForwardTransportModelCfg(flags)) ]
        if flags.Sim.BeamPipeSimMode is not BeamPipeSimMode.Normal:
            from G4FastSimulation.G4FastSimulationConfig import SimpleFastKillerCfg
            tools += [ result.popToolsAndMerge(SimpleFastKillerCfg(flags)) ]
    if flags.Detector.GeometryLAr:
        if flags.Sim.LArParameterization is LArParameterization.NoFrozenShowers:
            from AthenaCommon.Logging import logging
            msg = logging.getLogger("FastSimulationToolListCfg")
            msg.info( "Not using Frozen Showers" )
        else:
            from LArG4FastSimulation.LArG4FastSimulationConfig import EMBFastShowerCfg, EMECFastShowerCfg, FCALFastShowerCfg, FCAL2FastShowerCfg
            # We run production with LArParameterization.FrozenShowersFCalOnly, so the EMB and EMEC tools are not required
            if flags.Sim.LArParameterization is LArParameterization.FrozenShowers:
                tools += [ result.popToolsAndMerge(EMBFastShowerCfg(flags)) ]
                tools += [ result.popToolsAndMerge(EMECFastShowerCfg(flags)) ]
            tools += [ result.popToolsAndMerge(FCALFastShowerCfg(flags)) ]
            tools += [ result.popToolsAndMerge(FCAL2FastShowerCfg(flags)) ]
            if flags.Sim.LArParameterization in [LArParameterization.DeadMaterialFrozenShowers, LArParameterization.FrozenShowersFCalOnly, LArParameterization.FastCaloSim]: # TODO Check this makes sense.
                from G4FastSimulation.G4FastSimulationConfig import DeadMaterialShowerCfg
                tools += [ result.popToolsAndMerge(DeadMaterialShowerCfg(flags)) ]
            # Enable fast simulation of the calorimeter with FastCaloSim
            if flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
                from G4FastSimulation.G4FastSimulationConfig import FastCaloSimCfg
                tools += [ result.popToolsAndMerge(FastCaloSimCfg(flags)) ]
    if flags.Detector.GeometryMuon:
        if flags.Sim.CavernBackground not in [CavernBackground.Off, CavernBackground.Read] and not flags.Sim.RecordFlux:
            from TrackWriteFastSim.TrackWriteFastSimConfig import NeutronFastSimCfg
            tools += [ result.popToolsAndMerge(NeutronFastSimCfg(flags)) ]

    from G4AtlasServices.G4AtlasServicesConfig import PhysicsListSvcCfg
    result.merge(PhysicsListSvcCfg(flags))
    result.getService("PhysicsListSvc").FastSimConstructor.InitializeFastSimulation = len(tools) > 0

    result.setPrivateTools(tools)
    return result

def G4ThreadPoolSvcCfg(flags):
    acc = ComponentAccumulator()

    svc = CompFactory.ThreadPoolSvc(name="ThreadPoolSvc")
    svc.ThreadInitTools += [CompFactory.G4ThreadInitTool()]

    acc.addService(svc)
    return acc

def FastSimulationMasterToolCfg(flags, **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FastSimulations", result.popToolsAndMerge(FastSimulationToolListCfg(flags)))
    FastSimulationMasterTool = CompFactory.FastSimulationMasterTool
    result.setPrivateTools(FastSimulationMasterTool(name="FastSimulationMasterTool", **kwargs))
    return result


def FastSimulationConstructorToolCfg(flags, **kwargs):
    result = ComponentAccumulator()
    FastSimulationConstructorTool = CompFactory.FastSimulationConstructorTool
    result.setPrivateTools(FastSimulationConstructorTool(name="FastSimulationConstructorTool", **kwargs))
    return result


def EmptyFastSimulationMasterToolCfg(flags, **kwargs):
    result = ComponentAccumulator()
    FastSimulationMasterTool = CompFactory.FastSimulationMasterTool
    tool = result.popToolsAndMerge(FastSimulationMasterTool(name="EmptyFastSimulationMasterTool", **kwargs))
    result.setPrivateTools(tool)
    return result

def FastCaloSimParametrizationToolCfg(flags, name="FastCaloSimParametrizationTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ParamsInputFilename", flags.Sim.FastCalo.ParamsInputFilename)
    kwargs.setdefault("ParamsInputObject", "SelPDGID")
    kwargs.setdefault("CaloGeoInputFolder", flags.Sim.FastCalo.CaloGeoInputFolder)
    # Geometry tag is the one passed to the job (--geometryVersion), keeping the
    # FastCaloSim geometry consistent with the rest of the simulation.
    kwargs.setdefault("CaloGeoTag", flags.GeoModel.AtlasVersion)
    # Simplified transport geometry: honour an explicitly configured path (also
    # used by G4AtlasDetectorConstructionTool, which imports the GDML on the
    # Geant4 master thread); otherwise the tool falls back to its built-in
    # calib-area default. Kept as a fallback for setups where the detector
    # construction tool does not load the transport GDML itself.
    if flags.Sim.SimplifiedGeoPath:
        kwargs.setdefault("SimplifiedGeoPath", flags.Sim.SimplifiedGeoPath)
    result.setPrivateTools(CompFactory.FastCaloSimParametrizationTool(name, **kwargs))
    return result

def ActsFatrasG4ToolCfg(flags, name="ActsFatrasG4Tool", **kwargs):
    # Use rules below to decide how to retrieve dependencies for this tool:
    # Private tool	            popToolsAndMerge()
    # Primary component	        getPrimaryAndMerge()
    # Plain alg/service/cond alg	merge()

    # declare component accumulator
    result = ComponentAccumulator()
    from AthenaCommon.Logging import logging
    mlog = logging.getLogger(name)
    mlog.info('Start configuration of ActsFatrasG4Tool')

    # other arguments
    # see: ActsExtrapolationToolCfg in ActsConfig/python/ActsGeometryConfig.py for example 
    # --- Schedule magnetic field conditions alg properly ---
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    result.merge(AtlasFieldCacheCondAlgCfg(flags))
    
    # Tracking geometry
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    result.merge(ActsTrackingGeometrySvcCfg(flags))

    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags)) 

    kwargs.setdefault("MaxSteps", 2000)
    # always set to false, only for debugging purpose
    kwargs.setdefault("DebugInjectParticle", flags.Sim.ActsFatrasG4.DebugInjectParticle)

    # RNG service
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault(
        "RNGService",
        result.getPrimaryAndMerge(AthRNGSvcCfg(flags))
    )

    # create the tool 
    tool = CompFactory.ActsFatrasG4Tool(name, **kwargs)
    # set as private tool
    result.setPrivateTools(tool)

    return result

def PunchThroughG4ClassifierCfg(flags, name="PunchThroughG4Classifier", **kwargs):
    # declare component accumulator
    result = ComponentAccumulator()
    # other arguments
    kwargs.setdefault("ScalerConfigFileName"        , "FastCaloSim/MC23/TFCSparam_mpt_classScaler_v04.xml" )
    kwargs.setdefault("NetworkConfigFileName"       , "FastCaloSim/MC23/TFCSparam_mpt_classNet_v04.json" )
    kwargs.setdefault("CalibratorConfigFileName"    , "FastCaloSim/MC23/TFCSparam_mpt_classCalib_v04.xml")
    # set as private tool
    result.setPrivateTools(CompFactory.PunchThroughG4Classifier(name, **kwargs))
    return result

def PunchThroughG4ToolCfg(flags, name='PunchThroughG4Tool', **kwargs):
    # get the envelope config
    from SubDetectorEnvelopes.SubDetectorEnvelopesConfig import EnvelopeDefSvcCfg
    # declare component accumulator
    result = ComponentAccumulator()
    # other arguments
    kwargs.setdefault("FilenameLookupTable"     , "FastCaloSim/MC23/TFCSparam_mpt_v07.root")
    kwargs.setdefault("FilenameInverseCdf"      , "FastCaloSim/MC23/TFCSparam_mpt_inverseCdf_v07.xml")
    kwargs.setdefault("FilenameInversePca"      , "FastCaloSim/MC23/TFCSparam_mpt_inversePca_v07.xml")
    kwargs.setdefault("EnergyFactor"            , [ 0.98,  0.831, 0.896, 0.652, 0.717, 1., 0.877, 0.858, 0.919 ]    )
    kwargs.setdefault("DoAntiParticles"         , [ 0,   1,    0,     1,     1,     0,   0,    0,    0 ]    )
    kwargs.setdefault("PunchThroughInitiators"  , [ 211, 321, 311, 310, 130, 2212, 2112]        )
    kwargs.setdefault("InitiatorsMinEnergy"     , [ 65536, 65536, 65536, 65536, 65536, 65536, 65536]                                         )
    kwargs.setdefault("InitiatorsEtaRange"      , [ -3.2,   3.2 ]                               )
    kwargs.setdefault("PunchThroughParticles"   , [ 2212,   211,    22,     11,     13,     2112,   321,    310,    130 ]    )
    kwargs.setdefault("CorrelatedParticle"      , []    )
    kwargs.setdefault("FullCorrelationEnergy"   , [ 100000., 100000., 100000., 100000.,      0., 100000., 100000., 100000., 100000.]    )
    kwargs.setdefault("MinEnergy"               , [   938.3,   135.6,     50.,     50.,   105.7,   939.6, 493.7,   497.6,   497.6 ]    )
    kwargs.setdefault("MaxNumParticles"         , [      -1,      -1,      -1,      -1,      -1,    -1,     -1,     -1,     -1 ]    )
    kwargs.setdefault("EnvelopeDefSvc",         result.getPrimaryAndMerge(EnvelopeDefSvcCfg(flags)))
    kwargs.setdefault("BeamPipeRadius", 500.)
    # set as private tool
    result.setPrivateTools(CompFactory.PunchThroughG4Tool(name, **kwargs))
    return result
    
def PunchThroughSimWrapperCfg(flags, name='PunchThroughSimWrapper', **kwargs):
    result = ComponentAccumulator()

    # Set the PunchThroughG4Classifier
    if "PunchThroughG4Classifier" not in kwargs:
        kwargs.setdefault("PunchThroughG4Classifier", result.addPublicTool(result.popToolsAndMerge(PunchThroughG4ClassifierCfg(flags))))
    
    # Set the PunchThroughG4Tool
    if "PunchThroughG4Tool" not in kwargs:
        kwargs.setdefault("PunchThroughG4Tool", result.addPublicTool(result.popToolsAndMerge(PunchThroughG4ToolCfg(flags))))

    result.setPrivateTools(CompFactory.PunchThroughSimWrapper(name, **kwargs))
    return result

def FwdSensitiveDetectorListCfg(flags):
    # TODO: migrate to CA
    result = ComponentAccumulator()
    tools = []
    if flags.Detector.EnableLucid:
        from LUCID_G4_SD.LUCID_G4_SDConfig import LUCID_SensitiveDetectorCfg
        tools += [ result.popToolsAndMerge(LUCID_SensitiveDetectorCfg(flags)) ]
    if flags.Detector.EnableForward:
        if flags.Detector.EnableZDC:
            from ZDC_SD.ZDC_SDConfig import ZDC_FiberSDCfg
            tools += [ result.popToolsAndMerge(ZDC_FiberSDCfg(flags)) ]
            if flags.Sim.CalibrationRun in [CalibrationRun.ZDC, CalibrationRun.LArTileZDC]:
                from ZDC_SD.ZDC_SDConfig import ZDC_G4CalibSDCfg
                tools += [ result.popToolsAndMerge(ZDC_G4CalibSDCfg(flags)) ]
        if flags.Detector.EnableALFA:
            from ALFA_G4_SD.ALFA_G4_SDConfig import ALFA_SensitiveDetectorCfg
            tools += [ result.popToolsAndMerge(ALFA_SensitiveDetectorCfg(flags)) ]
        if flags.Detector.EnableAFP:
            from AFP_G4_SD.AFP_G4_SDConfig import AFP_SensitiveDetectorCfg
            tools += [ result.popToolsAndMerge(AFP_SensitiveDetectorCfg(flags)) ]
            # Alternative implementations
            # from AFP_G4_SD.AFP_G4_SDConfig import AFP_SiDSensitiveDetectorCfg, AFP_TDSensitiveDetectorCfg
            # tools += [ result.popToolsAndMerge(AFP_SiDSensitiveDetectorCfg(flags)) ]
            # tools += [ result.popToolsAndMerge(AFP_TDSensitiveDetectorCfg(flags)) ]
    result.setPrivateTools(tools)
    return result


def TrackFastSimSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []
    if (flags.Detector.EnableMuon and flags.Sim.CavernBackground in [CavernBackground.Write, CavernBackground.WriteWorld]) or flags.Sim.StoppedParticleFile:
        from TrackWriteFastSim.TrackWriteFastSimConfig import TrackFastSimSDCfg
        tools += [ result.popToolsAndMerge(TrackFastSimSDCfg(flags)) ]
    result.setPrivateTools(tools)
    return result

def FastHitConvertToolCfg(flags, name="ISF_FastHitConvertTool", **kwargs):
    """Configure conversion of FastCaloSim cells into LAr and Tile hits."""
    from ISF_Algorithms.CollectionMergerConfig import CollectionMergerCfg

    acc = ComponentAccumulator()
    mergeable_collection_suffix = "_FastCaloSim"
    region = "CALO"

    EMB_hits_bare_collection_name = "LArHitEMB"
    EMB_hits_merger_input_property = "LArEMBHits"
    acc1, EMB_hits_collection_name = CollectionMergerCfg(
        flags,
        EMB_hits_bare_collection_name,
        mergeable_collection_suffix,
        EMB_hits_merger_input_property,
        region)
    acc.merge(acc1)

    EMEC_hits_bare_collection_name = "LArHitEMEC"
    EMEC_hits_merger_input_property = "LArEMECHits"
    acc2, EMEC_hits_collection_name = CollectionMergerCfg(
        flags,
        EMEC_hits_bare_collection_name,
        mergeable_collection_suffix,
        EMEC_hits_merger_input_property,
        region)
    acc.merge(acc2)

    FCAL_hits_bare_collection_name = "LArHitFCAL"
    FCAL_hits_merger_input_property = "LArFCALHits"
    acc3, FCAL_hits_collection_name = CollectionMergerCfg(
        flags,
        FCAL_hits_bare_collection_name,
        mergeable_collection_suffix,
        FCAL_hits_merger_input_property,
        region)
    acc.merge(acc3)

    HEC_hits_bare_collection_name = "LArHitHEC"
    HEC_hits_merger_input_property = "LArHECHits"
    acc4, HEC_hits_collection_name = CollectionMergerCfg(
        flags,
        HEC_hits_bare_collection_name,
        mergeable_collection_suffix,
        HEC_hits_merger_input_property,
        region)
    acc.merge(acc4)

    tile_hits_bare_collection_name = "TileHitVec"
    tile_hits_merger_input_property = "TileHits"
    acc5, tile_hits_collection_name = CollectionMergerCfg(
        flags,
        tile_hits_bare_collection_name,
        mergeable_collection_suffix,
        tile_hits_merger_input_property,
        region)
    acc.merge(acc5)

    kwargs.setdefault("embHitContainername", EMB_hits_collection_name)
    kwargs.setdefault("emecHitContainername", EMEC_hits_collection_name)
    kwargs.setdefault("fcalHitContainername", FCAL_hits_collection_name)
    kwargs.setdefault("hecHitContainername", HEC_hits_collection_name)

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge(TileCablingSvcCfg(flags))

    from TileConditions.TileSamplingFractionConfig import TileSamplingFractionCondAlgCfg
    acc.merge( TileSamplingFractionCondAlgCfg(flags) )

    # FastHitConvertTool needs LAr sampling fractions to create LAr hits.
    from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
    acc.merge( LArElecCalibDBCfg(flags, ["fSampl"]) )

    kwargs.setdefault("tileHitContainername", tile_hits_collection_name)

    acc.setPrivateTools(CompFactory.FastHitConvertTool(name, **kwargs))
    return acc


def CaloCellContainerSDCfg(flags, name='CaloCellContainerSD', **kwargs):
    """Configure the FastCaloSim calorimeter-cell sensitive detector."""
    result = ComponentAccumulator()
    if flags.GeoModel.Align.LegacyConditionsAccess:
        # EmptyCellBuilderTool accesses CaloDetDescrManager through a
        # conditions handle.  In simulation, legacy alignment is already
        # applied to the GeoModel tree, so build the corresponding static
        # conditions object without additional alignment inputs.
        result.addCondAlgo(CompFactory.CaloAlignCondAlg(
            LArAlignmentStore="",
            CaloCellPositionShiftFolder=""))
    kwargs.setdefault ('NoVolumes', True)
    kwargs.setdefault("OutputCollectionNames", ["DefaultCaloCellContainer"])
    # The conversion tool also creates mergeable FastCaloSim hit collections.
    kwargs.setdefault("FastHitConvertTool",  result.addPublicTool(result.popToolsAndMerge(FastHitConvertToolCfg(flags))))
    result.setPrivateTools(CompFactory.CaloCellContainerSDTool(name, **kwargs))
    return result


def CaloCellContainerSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []
    if flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
        tools += [ result.popToolsAndMerge(CaloCellContainerSDCfg(flags)) ]
    result.setPrivateTools(tools)
    return result

def ITkSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []

    if flags.Detector.EnableITkPixel:
        from PixelG4_SD.PixelG4_SDToolConfig import ITkPixelSensorSDCfg
        tools += [ result.popToolsAndMerge(ITkPixelSensorSDCfg(flags)) ]
        pass
    if flags.Detector.EnableITkStrip:
        from SCT_G4_SD.SCT_G4_SDToolConfig import ITkStripSensorSDCfg
        tools += [ result.popToolsAndMerge(ITkStripSensorSDCfg(flags)) ]
    if flags.Detector.EnablePLR:
        from PixelG4_SD.PixelG4_SDToolConfig import PLRSensorSDCfg
        tools += [ result.popToolsAndMerge(PLRSensorSDCfg(flags)) ]
    
    result.setPrivateTools(tools)
    return result


def HGTDSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []

    if flags.Detector.EnableHGTD:
        from HGTD_G4_SD.HGTD_G4_SDToolConfig import HgtdSensorSDCfg
        tools += [ result.popToolsAndMerge(HgtdSensorSDCfg(flags)) ]
        pass

    result.setPrivateTools(tools)
    return result


def InDetSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []

    if flags.Detector.EnablePixel:
        from PixelG4_SD.PixelG4_SDToolConfig import PixelSensorSDCfg
        tools += [ result.popToolsAndMerge(PixelSensorSDCfg(flags)) ]
    if flags.Detector.EnableSCT:
        from SCT_G4_SD.SCT_G4_SDToolConfig import SctSensorSDCfg
        tools += [ result.popToolsAndMerge(SctSensorSDCfg(flags)) ]
    if flags.Detector.EnableTRT:
        from TRT_G4_SD.TRT_G4_SDToolConfig import TRTSensitiveDetectorCfg
        tools += [ result.popToolsAndMerge(TRTSensitiveDetectorCfg(flags)) ]
    if flags.Detector.EnableBCM:
        from BCM_G4_SD.BCM_G4_SDToolConfig import BCMSensorSDCfg
        tools += [ result.popToolsAndMerge(BCMSensorSDCfg(flags)) ]
        from BLM_G4_SD.BLM_G4_SDToolConfig import BLMSensorSDCfg
        tools += [ result.popToolsAndMerge(BLMSensorSDCfg(flags)) ]

    result.setPrivateTools(tools)
    return result


def CaloSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []

    if flags.Detector.EnableLAr:
        from LArG4SD.LArG4SDToolConfig import LArEMBSensitiveDetectorCfg, LArEMECSensitiveDetectorCfg, LArFCALSensitiveDetectorCfg, LArHECSensitiveDetectorCfg
        tools += [ result.popToolsAndMerge(LArEMBSensitiveDetectorCfg(flags)) ]
        tools += [ result.popToolsAndMerge(LArEMECSensitiveDetectorCfg(flags)) ]
        tools += [ result.popToolsAndMerge(LArFCALSensitiveDetectorCfg(flags)) ]
        tools += [ result.popToolsAndMerge(LArHECSensitiveDetectorCfg(flags)) ]
        
        if flags.Detector.EnableMBTS:
            from MinBiasScintillator.MinBiasScintillatorToolConfig import MinBiasScintillatorSDCfg
            tools += [ result.popToolsAndMerge(MinBiasScintillatorSDCfg(flags)) ]

        if flags.Sim.CalibrationRun in [CalibrationRun.LAr, CalibrationRun.LArTile, CalibrationRun.LArTileZDC]:
            from LArG4SD.LArG4SDToolConfig import LArDeadSensitiveDetectorToolCfg, LArActiveSensitiveDetectorToolCfg, LArInactiveSensitiveDetectorToolCfg
            tools += [ result.popToolsAndMerge(LArDeadSensitiveDetectorToolCfg(flags)) ]
            tools += [ result.popToolsAndMerge(LArInactiveSensitiveDetectorToolCfg(flags)) ]
            tools += [ result.popToolsAndMerge(LArActiveSensitiveDetectorToolCfg(flags)) ]
        elif flags.Sim.CalibrationRun is CalibrationRun.DeadLAr:
            from LArG4SD.LArG4SDToolConfig import LArDeadSensitiveDetectorToolCfg
            tools += [ result.popToolsAndMerge(LArDeadSensitiveDetectorToolCfg(flags)) ]

    if flags.Detector.EnableTile:
        if flags.Sim.CalibrationRun in [CalibrationRun.Tile, CalibrationRun.LArTile, CalibrationRun.LArTileZDC]:
            from TileGeoG4Calib.TileGeoG4CalibConfig import TileGeoG4CalibSDCfg
            tools += [ result.popToolsAndMerge(TileGeoG4CalibSDCfg(flags)) ]  # mode 1 : With CaloCalibrationHits
        else:
            from TileGeoG4SD.TileGeoG4SDToolConfig import TileGeoG4SDCfg
            tools += [ result.popToolsAndMerge(TileGeoG4SDCfg(flags)) ]       # mode 0 : No CaloCalibrationHits
    if flags.Sim.RecordStepInfo:
        from ISF_FastCaloSimSD.ISF_FastCaloSimSDToolConfig import FCS_StepInfoSDToolCfg
        tools += [ result.popToolsAndMerge(FCS_StepInfoSDToolCfg(flags)) ]

    result.setPrivateTools(tools)
    return result


def MuonSensitiveDetectorListCfg(flags):
    if flags.Muon.usePhaseIIGeoSetup:
        from MuonSensitiveDetectorsR4.SensitiveDetectorsCfg import SetupSensitiveDetectorsCfg
        return SetupSensitiveDetectorsCfg(flags)
    from MuonG4SD.MuonG4SDConfig import SetupSensitiveDetectorsCfg
    return SetupSensitiveDetectorsCfg(flags)   

def EnvelopeSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []
    if flags.Beam.Type is BeamType.Cosmics and not flags.Sim.ReadTR:
        from TrackWriteFastSim.TrackWriteFastSimConfig import CosmicTRSDCfg
        tools += [ result.popToolsAndMerge(CosmicTRSDCfg(flags)) ]
    result.setPrivateTools(tools)
    return result


def SimHitContainerListCfg(flags):
    from SimulationConfig.SimEnums import LArParameterization
    writtenContainers =[]
    if flags.Detector.GeometryMuon:
        if flags.Muon.usePhaseIIGeoSetup:
            from MuonSensitiveDetectorsR4.SensitiveDetectorsCfg import SimHitContainerListCfg
            writtenContainers+= SimHitContainerListCfg(flags)
        else:
            from MuonG4SD.MuonG4SDConfig import SimHitContainerListCfg
            writtenContainers += SimHitContainerListCfg(flags)
    if flags.Detector.GeometryLAr:
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('CALO', True)) or flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
            writtenContainers += [("LArHitContainer", "LArHitEMB_G4")]
            writtenContainers += [("LArHitContainer", "LArHitEMEC_G4")]
            writtenContainers += [("LArHitContainer", "LArHitFCAL_G4")]
            writtenContainers += [("LArHitContainer", "LArHitHEC_G4")]
            if flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
                writtenContainers += [("LArHitContainer" , "LArHitHEC_FastCaloSim")]
                writtenContainers += [("LArHitContainer" , "LArHitFCAL_FastCaloSim")]
                writtenContainers += [("LArHitContainer" , "LArHitEMEC_FastCaloSim")]
                writtenContainers += [("LArHitContainer" , "LArHitEMB_FastCaloSim")]
        else:
            writtenContainers += [("LArHitContainer", "LArHitEMB")]
            writtenContainers += [("LArHitContainer", "LArHitEMEC")]
            writtenContainers += [("LArHitContainer", "LArHitFCAL")]
            writtenContainers += [("LArHitContainer", "LArHitHEC")]
    if flags.Detector.GeometryTile:
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('CALO', True)) or flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
            writtenContainers += [("TileHitVector", "MBTSHits_G4")]
            writtenContainers += [("TileHitVector", "TileHitVec_G4")]
            if flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
                writtenContainers += [("TileHitVector" , "TileHitVec_FastCaloSim")]
                writtenContainers += [("TileHitVector" , "MBTSHits_FastCaloSim")]
        else:
            writtenContainers += [("TileHitVector", "MBTSHits")]
            writtenContainers += [("TileHitVector", "TileHitVec")]
    if flags.Detector.GeometryTRT:
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('ID', True)):
            writtenContainers += [("TRTUncompressedHitCollection", "TRTUncompressedHits_G4")]
        else:
            writtenContainers += [("TRTUncompressedHitCollection", "TRTUncompressedHits")]
    if flags.Detector.EnableBCM:
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('ID', True)):
            writtenContainers += [("SiHitCollection", "BCMHits_G4")]
            writtenContainers += [("SiHitCollection", "BLMHits_G4")]
        else:
            writtenContainers += [("SiHitCollection", "BCMHits")]
            writtenContainers += [("SiHitCollection", "BLMHits")]
    if flags.Detector.EnablePixel:
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('ID', True)):
            writtenContainers += [("SiHitCollection", "PixelHits_G4")]
            if flags.Sim.InDetParameterization in (
               InDetParameterization.FatrasG4,
               InDetParameterization.AFatrasG4,
            ):
             writtenContainers += [("SiHitCollection", "PixelHits_ActsFatrasG4")]
        else:
            writtenContainers += [("SiHitCollection", "PixelHits")]
    if flags.Detector.EnableSCT:
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('ID', True)):
            writtenContainers += [("SiHitCollection", "SCT_Hits_G4")]
            if flags.Sim.InDetParameterization in (
               InDetParameterization.FatrasG4,
               InDetParameterization.AFatrasG4,
            ):
              writtenContainers += [("SiHitCollection" , "SCT_Hits_ActsFatrasG4")]
        else:
            writtenContainers += [("SiHitCollection", "SCT_Hits")]
    if flags.Detector.EnableITkPixel:
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('ITk', True)):
            writtenContainers += [("SiHitCollection", "ITkPixelHits_G4")]
        else:
            writtenContainers += [("SiHitCollection", "ITkPixelHits")]
    if flags.Detector.EnableITkStrip:
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('ITk', True)):
           writtenContainers += [("SiHitCollection", "ITkStripHits_G4")]
        else:
            writtenContainers += [("SiHitCollection", "ITkStripHits")]
    if flags.Detector.EnablePLR:
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('ITk', True)):
            writtenContainers += [("SiHitCollection", "PLR_Hits_G4")]
        else:
            writtenContainers += [("SiHitCollection", "PLR_Hits")]
    if flags.Detector.EnableHGTD:
        if (flags.Sim.ISFRun and flags.Sim.ISF.HITSMergingRequired.get('ITk', True)):
            writtenContainers += [("SiHitCollection", "HGTDHits_G4")]
        else:
            writtenContainers += [("SiHitCollection", "HGTDHits")]
    from SimulationConfig.SimEnums import CalibrationRun
    if flags.Sim.CalibrationRun in [CalibrationRun.LAr, CalibrationRun.LArTile, CalibrationRun.LArTileZDC]:
        # Needed to ensure that DeadMaterialCalibrationHitsMerger is scheduled correctly.
        writtenContainers += [
            ( 'CaloCalibrationHitContainer' , 'StoreGateSvc+LArCalibrationHitActive_DEAD' ),
            ( 'CaloCalibrationHitContainer' , 'StoreGateSvc+LArCalibrationHitDeadMaterial_DEAD' ),
            ( 'CaloCalibrationHitContainer' , 'StoreGateSvc+LArCalibrationHitInactive_DEAD' )
        ]

    return writtenContainers


def InputContainerListCfg(flags):
    dependencies = []
    from SimulationConfig.SimEnums import LArParameterization
    if flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
        dependencies+=[('CaloDetDescrManager', 'ConditionStore+CaloDetDescrManager'),
                       ('LArfSamplSym', 'ConditionStore+LArfSamplSym'),
                       ('TileSamplingFraction', 'ConditionStore+TileSamplingFraction')]
    from MuonSensitiveDetectorsR4.SensitiveDetectorsCfg import MuonDependenciesCfg
    dependencies += MuonDependenciesCfg(flags)
    return dependencies

def SensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []
    tools += result.popToolsAndMerge(EnvelopeSensitiveDetectorListCfg(flags))
    tools += result.popToolsAndMerge(InDetSensitiveDetectorListCfg(flags))
    tools += result.popToolsAndMerge(ITkSensitiveDetectorListCfg(flags))
    tools += result.popToolsAndMerge(HGTDSensitiveDetectorListCfg(flags))
    tools += result.popToolsAndMerge(CaloSensitiveDetectorListCfg(flags))
    tools += result.popToolsAndMerge(MuonSensitiveDetectorListCfg(flags))
    tools += result.popToolsAndMerge(TrackFastSimSensitiveDetectorListCfg(flags))
    tools += result.popToolsAndMerge(FwdSensitiveDetectorListCfg(flags))
    tools += result.popToolsAndMerge(CaloCellContainerSensitiveDetectorListCfg(flags))

    result.setPrivateTools(tools)
    return result


def TileTestBeamSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []

    if flags.Detector.EnableTile:
        if flags.Sim.CalibrationRun in [CalibrationRun.Tile, CalibrationRun.LArTile, CalibrationRun.LArTileZDC]:
            from TileGeoG4Calib.TileGeoG4CalibConfig import TileCTBGeoG4CalibSDCfg
            tools += [ result.popToolsAndMerge(TileCTBGeoG4CalibSDCfg(flags)) ] # mode 1 : With CaloCalibrationHits
        else:
            from TileGeoG4SD.TileGeoG4SDToolConfig import TileCTBGeoG4SDCfg
            tools += [ result.popToolsAndMerge(TileCTBGeoG4SDCfg(flags)) ]      # mode 0 : No CaloCalibrationHits
            from MuonWall.MuonWallConfig import MuonWallSDCfg
            tools += [ result.popToolsAndMerge(MuonWallSDCfg(flags)) ]
    result.setPrivateTools(tools)
    return result


def CombinedTestBeamSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []
    if flags.Detector.EnablePixel:
        from PixelG4_SD.PixelG4_SDToolConfig import PixelSensor_CTBCfg
        tools += [ result.popToolsAndMerge(PixelSensor_CTBCfg(flags)) ]
    if flags.Detector.EnableSCT:
        from SCT_G4_SD.SCT_G4_SDToolConfig import SctSensor_CTBCfg
        tools += [ result.popToolsAndMerge(SctSensor_CTBCfg(flags)) ]
    if flags.Detector.EnableTRT:
        from TRT_G4_SD.TRT_G4_SDToolConfig import TRTSensitiveDetector_CTBCfg
        tools += [ result.popToolsAndMerge(TRTSensitiveDetector_CTBCfg(flags)) ]
    if flags.Detector.EnableLAr:
        from LArG4SD.LArG4SDToolConfig import LArEMBSensitiveDetectorCfg
        tools += [ result.popToolsAndMerge(LArEMBSensitiveDetectorCfg(flags)) ]
        if flags.Sim.CalibrationRun in [CalibrationRun.LAr, CalibrationRun.LArTile, CalibrationRun.LArTileZDC, CalibrationRun.DeadLAr]:
            tools += [ 'LArH8CalibSensitiveDetector' ] # mode 1 : With CaloCalibrationHits
    if flags.Detector.EnableTile:
        if flags.Sim.CalibrationRun in [CalibrationRun.Tile, CalibrationRun.LArTile, CalibrationRun.LArTileZDC]:
            from TileGeoG4Calib.TileGeoG4CalibConfig import TileCTBGeoG4CalibSDCfg
            tools += [ result.popToolsAndMerge(TileCTBGeoG4CalibSDCfg(flags)) ] # mode 1 : With CaloCalibrationHits
        else:
            from TileGeoG4SD.TileGeoG4SDToolConfig import TileCTBGeoG4SDCfg
            tools += [ result.popToolsAndMerge(TileCTBGeoG4SDCfg(flags)) ]      # mode 0 : No CaloCalibrationHits
            tools += [ 'MuonWallSD' ]
    if flags.Detector.EnableMuon:
        tools += [ 'MuonEntryRecord' ]
    tools += result.popToolsAndMerge(MuonSensitiveDetectorListCfg(flags))

    result.setPrivateTools(tools)
    return result


def SensitiveDetectorMasterToolCfg(flags, name="SensitiveDetectorMasterTool", **kwargs):
    result = ComponentAccumulator()
    # NB Currently only supporting the standard ATLAS dector and the Tile Test Beam
    if flags.Beam.Type is BeamType.TestBeam:
        kwargs.setdefault("SensitiveDetectors", result.popToolsAndMerge(TileTestBeamSensitiveDetectorListCfg(flags)))
    elif "tb_LArH6" in flags.GeoModel.AtlasVersion:
        pass
    elif "ctbh8" in flags.GeoModel.AtlasVersion:
        kwargs.setdefault("SensitiveDetectors", result.popToolsAndMerge(CombinedTestBeamSensitiveDetectorListCfg(flags)))
    else:
        kwargs.setdefault("SensitiveDetectors", result.popToolsAndMerge(SensitiveDetectorListCfg(flags)))

    result.setPrivateTools(CompFactory.SensitiveDetectorMasterTool(name, **kwargs))
    return result


def EmptySensitiveDetectorMasterToolCfg(name="EmptySensitiveDetectorMasterTool", **kwargs):
    result = ComponentAccumulator()
    tool = result.popToolsAndMerge(CompFactory.SensitiveDetectorMasterTool(name, **kwargs))
    result.setPrivateTools(tool)
    return result
