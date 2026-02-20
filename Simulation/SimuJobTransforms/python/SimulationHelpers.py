# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from SimulationConfig.SimEnums import CalibrationRun, CavernBackground, LArParameterization, SimulationFlavour


def getDetectorsFromRunArgs(flags, runArgs):
    """Generate detector list based on runtime arguments."""
    if hasattr(runArgs, 'detectors'):
        detectors = set(runArgs.detectors)
    else:
        from AthenaConfiguration.AutoConfigFlags import getDefaultDetectors
        detectors = set(getDefaultDetectors(flags.GeoModel.AtlasVersion, flags.GeoModel.SQLiteDB, flags.GeoModel.SQLiteDBFullPath, includeForward=False))

    # Support switching on Forward Detectors
    if hasattr(runArgs, 'LucidOn'):
        detectors.add('Lucid')
    if hasattr(runArgs, 'ZDCOn'):
        detectors.add('ZDC')
    if hasattr(runArgs, 'AFPOn'):
        detectors.add('AFP')
    if hasattr(runArgs, 'ALFAOn'):
        detectors.add('ALFA')
    if hasattr(runArgs, 'FwdRegionOn'):
        detectors.add('FwdRegion')
    # TODO here support switching on Cavern geometry
    # if hasattr(runArgs, 'CavernOn'):
    #     detectors.add('Cavern')

    # Fatras does not support simulating the BCM, so have to switch that off
    if flags.Sim.ISF.Simulator.usesFatras() and not flags.Detector.GeometryITk:
        try:
            detectors.remove('BCM')
        except ValueError:
            pass

    return detectors


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


def useVerboseTracking(flags):
    # Use verbose G4 tracking
    flags.Sim.G4Commands += ['/tracking/verbose 1']


## Change the field stepper
def useSimpleRungeStepper(flags):
    flags.Sim.G4Stepper = 'SimpleRunge'


def useClassicalRK4Stepper(flags):
    flags.Sim.G4Stepper = 'ClassicalRK4'


def useNystromRK4Stepper(flags):
    flags.Sim.G4Stepper = 'NystromRK4'


def enableFastIDKiller(flags):
    """ """
    flags.Sim.OptionalUserActionList += ['G4UserActions.G4UserActionsConfig.FastIDKillerToolCfg']
