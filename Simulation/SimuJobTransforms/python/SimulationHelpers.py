# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from SimulationConfig.SimEnums import BeamPipeSimMode, CalibrationRun, CavernBackground, LArParameterization, SimulationFlavour


def getDetectorsFromRunArgs(flags, runArgs):
    """Generate detector list based on runtime arguments."""
    if hasattr(runArgs, 'detectors'):
        detectors = runArgs.detectors
    else:
        from AthenaConfiguration.AutoConfigFlags import getDefaultDetectors
        detectors = getDefaultDetectors(flags.GeoModel.AtlasVersion, flags.GeoModel.SQLiteDB, flags.GeoModel.SQLiteDBFullPath, includeForward=False)

    # Support switching on Forward Detectors
    if hasattr(runArgs, 'LucidOn'):
        detectors = detectors.add('Lucid')
    if hasattr(runArgs, 'ZDCOn'):
        detectors = detectors.add('ZDC')
    if hasattr(runArgs, 'AFPOn'):
        detectors = detectors.add('AFP')
    if hasattr(runArgs, 'ALFAOn'):
        detectors = detectors.add('ALFA')
    if hasattr(runArgs, 'FwdRegionOn'):
        detectors = detectors.add('FwdRegion')
    # TODO here support switching on Cavern geometry
    # if hasattr(runArgs, 'CavernOn'):
    #     detectors = detectors.add('Cavern')

    # Fatras does not support simulating the BCM, so have to switch that off
    if flags.Sim.ISF.Simulator.usesFatras():
        try:
            detectors.remove('BCM')
        except ValueError:
            pass

    return detectors


def enableFrozenShowersFCalOnly(flags):
    """Turns on GFlash shower parametrization for FCAL"""
    flags.Sim.LArParameterization = LArParameterization.FrozenShowersFCalOnly
    flags.Sim.CalibrationRun = CalibrationRun.Off


def enableBeamPipeKill(flags):
    flags.Sim.BeamPipeCut = 0.
    flags.Sim.BeamPipeSimMode = BeamPipeSimMode.FastSim


def enableTightMuonStepping(flags):
    flags.Sim.TightMuonStepping = True


def enableG4SignalCavern(flags):
    """Set flags to take care of Neutron BG"""
    flags.Sim.CavernBackground = CavernBackground.Signal


def enableCalHits(flags):
    """Turns on calibration hits for LAr and Tile"""
    flags.Sim.CalibrationRun = CalibrationRun.LArTile
    # deactivate incompatible optimizations
    flags.Sim.LArParameterization = LArParameterization.NoFrozenShowers
    flags.Sim.NRRThreshold = False
    flags.Sim.NRRWeight = False
    flags.Sim.PRRThreshold = False
    flags.Sim.PRRWeight = False


def enableCalHitsZDC(flags):
    """Turns on calibration hits for ZDC only"""
    flags.Sim.CalibrationRun = CalibrationRun.ZDC
    # deactivate incompatible optimizations
    flags.Sim.LArParameterization = LArParameterization.NoFrozenShowers
    flags.Sim.NRRThreshold = False
    flags.Sim.NRRWeight = False
    flags.Sim.PRRThreshold = False
    flags.Sim.PRRWeight = False

def enableCalHitsAll(flags):
    """Turns on calibration hits for LAr, Tile and ZDC"""
    flags.Sim.CalibrationRun = CalibrationRun.LArTileZDC
    # deactivate incompatible optimizations
    flags.Sim.LArParameterization = LArParameterization.NoFrozenShowers
    flags.Sim.NRRThreshold = False
    flags.Sim.NRRWeight = False
    flags.Sim.PRRThreshold = False
    flags.Sim.PRRWeight = False


def enableParticleID(flags):
    """Mods to have primary particle barcode signature on for calorimeter calibration hits."""
    flags.Sim.ParticleID=True


def enableVerboseSelector(flags):
    """ """
    flags.Sim.OptionalUserActionList += ['G4DebuggingTools.G4DebuggingToolsConfig.VerboseSelectorToolCfg']

def enableFastCaloSim(flags):
    # Set LArParametrization to FastCaloSim
    flags.Sim.LArParameterization = LArParameterization.FastCaloSim
    # Deacticate dead material hits for calibration run
    flags.Sim.CalibrationRun = CalibrationRun.Off
    # Set simulator name as metadata
    flags.Sim.ISF.Simulator = SimulationFlavour.ATLFAST3MT