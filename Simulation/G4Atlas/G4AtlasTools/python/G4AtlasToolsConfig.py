# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType
from SimulationConfig.SimEnums import BeamPipeSimMode, CalibrationRun, CavernBackground, LArParameterization
from AthenaConfiguration.AccumulatorCache import AccumulatorCache


@AccumulatorCache
def FastSimulationToolListCfg(flags):
    result = ComponentAccumulator()
    tools = []
    if flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
        from G4FastSimulation.G4FastSimulationConfig import FastCaloSimCfg
        tools += [ result.popToolsAndMerge(FastCaloSimCfg(flags)) ]

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
    result.setPrivateTools(tools)
    return result


def FastSimulationMasterToolCfg(flags, **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FastSimulations", result.popToolsAndMerge(FastSimulationToolListCfg(flags)))
    FastSimulationMasterTool = CompFactory.FastSimulationMasterTool
    result.setPrivateTools(FastSimulationMasterTool(name="FastSimulationMasterTool", **kwargs))
    return result


def EmptyFastSimulationMasterToolCfg(flags, **kwargs):
    result = ComponentAccumulator()
    FastSimulationMasterTool = CompFactory.FastSimulationMasterTool
    tool = result.popToolsAndMerge(FastSimulationMasterTool(name="EmptyFastSimulationMasterTool", **kwargs))
    result.setPrivateTools(tool)
    return result

def G4CaloTransportToolCfg(flags, name='G4CaloTransportTool', **kwargs):
    result = ComponentAccumulator()
    # Use simplified calorimeter geometry if path to simplified geometry is provided
    # Otherwise, use the full geometry for the transport (Note that this will be very slow) 
    kwargs.setdefault("UseSimplifiedGeo", bool(flags.Sim.SimplifiedGeoPath))
    if flags.Sim.SimplifiedGeoPath:
        # What is the name of the logical world volume of the simplified geometry?
        kwargs.setdefault('SimplifiedWorldLogName', "WorldLog")
        # At what volume will we stop the transport?
        kwargs.setdefault('TransportLimitVolume', "Envelope")
        # What is the maximum number of Geant4 steps taken in the transport?
        kwargs.setdefault('MaxSteps', 100)
    else:
        # At what volume will be stop the transport? 
        kwargs.setdefault('TransportLimitVolume', "MuonSys")
        # What is the maximum number of Geant4 steps taken in the transport?
        kwargs.setdefault('MaxSteps', 5000)
    
    result.setPrivateTools(CompFactory.G4CaloTransportTool(name, **kwargs))
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
    kwargs.setdefault("EnvelopeDefSvc",         result.getPrimaryAndMerge(EnvelopeDefSvcCfg(flags)).name)
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

def CaloCellContainerSensitiveDetectorListCfg(flags):
    result = ComponentAccumulator()
    tools = []
    if flags.Sim.LArParameterization is LArParameterization.FastCaloSim:
        from ISF_FastCaloSimParametrization.ISF_FastCaloSimParametrizationConfig import CaloCellContainerSDCfg
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
         writtenContainers += [("TRTUncompressedHitCollection", "TRTUncompressedHits")]
    if flags.Detector.EnableBCM:
       writtenContainers += [("SiHitCollection", "BCMHits")]
       writtenContainers += [("SiHitCollection", "BLMHits")]
    if flags.Detector.EnablePixel:
       writtenContainers += [("SiHitCollection", "PixelHits")]
    if flags.Detector.EnableSCT:
       writtenContainers += [("SiHitCollection", "SCT_Hits")]
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
