# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags, isGaudiEnv
from AthenaConfiguration.AutoConfigFlags import GetFileMD
from AthenaConfiguration.Enums import BeamType, LHCPeriod
from SimulationConfig.SimEnums import BeamPipeSimMode, CalibrationRun, CavernBackground, \
    LArParameterization, SimulationFlavour, TruthStrategy, VertexSource
from AthenaCommon.SystemOfUnits import m, ns

#todo? add in the explanatory text from previous implementation

def createSimConfigFlags():
    scf = AthConfigFlags()
    scf.addFlag("Sim.ParticleID", False)

    def _checkCalibrationRun(prevFlags):
        if prevFlags.Sim.ISF.Simulator not in [SimulationFlavour.FullG4MT, SimulationFlavour.FullG4MT_QS, SimulationFlavour.PassBackG4MT, SimulationFlavour.AtlasG4] \
            or prevFlags.Sim.LArParameterization is not LArParameterization.NoFrozenShowers:
            return CalibrationRun.Off
        return CalibrationRun.DeadLAr

    scf.addFlag("Sim.CalibrationRun", _checkCalibrationRun, type=CalibrationRun)

    scf.addFlag("Sim.CavernBackground", CavernBackground.Off, type=CavernBackground)
    scf.addFlag("Sim.ReadTR", False)
    scf.addFlag("Sim.WorldRRange", False)  # 12500. / int or float
    scf.addFlag("Sim.WorldZRange", False)  # 22031. / int or float

    def _barcodeOffsetFromTruthStrategy(prevFlags):
        if prevFlags.Sim.TruthStrategy in [TruthStrategy.MC15, TruthStrategy.MC18, TruthStrategy.MC18LLP]:
            return 1000000 # 1M
        return 200000 # 200k - This is the default value - in practice it has been the same for all campaigns

    def _checkSimBarcodeOffsetConf(prevFlags):
        simBarcodeOffset  = 0
        if prevFlags.Input.Files:
            mdstring = GetFileMD(prevFlags.Input.Files).get("SimBarcodeOffset", "0")
            simBarcodeOffset = eval(mdstring)
            if not simBarcodeOffset:
                simBarcodeOffset  = _barcodeOffsetFromTruthStrategy(prevFlags)
        return simBarcodeOffset

    def _regenerationIncrementFromTruthStrategy(prevFlags):
        if prevFlags.Sim.TruthStrategy in [TruthStrategy.MC15, TruthStrategy.MC18, TruthStrategy.MC18LLP]:
            return 10000000 # 10M
        return 1000000 # 1M - This is the default value - in practice it has been the same for all campaigns

    def _checkRegenerationIncrementConf(prevFlags):
        regenInc  = 0
        if prevFlags.Input.Files:
            mdstring = GetFileMD(prevFlags.Input.Files).get("RegenerationIncrement", "0")
            regenInc = eval(mdstring)
            if not regenInc:
                regenInc  = _regenerationIncrementFromTruthStrategy(prevFlags)
        return regenInc

    # the G4 offset.
    scf.addFlag("Sim.SimBarcodeOffset", _checkSimBarcodeOffsetConf)
    # barcode offset when a particle survives an interaction during simulation
    scf.addFlag("Sim.RegenerationIncrement", _checkRegenerationIncrementConf)

    # Forward region
    scf.addFlag("Sim.TwissFileBeam1", False)
    scf.addFlag("Sim.TwissFileBeam2", False)
    scf.addFlag("Sim.TwissEnergy", lambda prevFlags : float(prevFlags.Beam.Energy)) # energy of each beam
    scf.addFlag("Sim.TwissFileBeta", 90.*m)
    scf.addFlag("Sim.TwissFileNomReal", 'nominal')  # "nominal", "real" / default to one of these?!
    scf.addFlag("Sim.TwissFileVersion", "v02")

    # G4AtlasAlg
    scf.addFlag("Sim.ReleaseGeoModel", False)
    scf.addFlag("Sim.RecordFlux", False)
    scf.addFlag("Sim.TruthStrategy", lambda prevFlags : TruthStrategy.Validation if prevFlags.Sim.ISF.ValidationMode else TruthStrategy.MC12,
                type=TruthStrategy)
    scf.addFlag("Sim.UseShadowEvent", False)
    scf.addFlag("Sim.G4Commands", ["/run/verbose 2"])
    scf.addFlag("Sim.FlagAbortedEvents", False)
    scf.addFlag("Sim.KillAbortedEvents", True)
    scf.addFlag("Sim.IncludeParentsInG4Event", False)

    # Do full simulation + digitisation + reconstruction chain
    scf.addFlag("Sim.DoFullChain", False)

    def _check_G4_version(prevFlags):
        # Determine the Geant4 version which will be used by the
        # configuration.  In jobs where we are running simulation,
        # then the G4Version should reflect the version in the
        # release, so the version in environment should take
        # precedence over any input file metadata.  In jobs where
        # simulation is not run, then the G4Version from the input
        # file metadata should take precedence.
        version = ""
        from AthenaConfiguration.Enums import ProductionStep
        if prevFlags.Common.ProductionStep not in [ProductionStep.Simulation, ProductionStep.FastChain]:
            if prevFlags.Input.Files:
                version = GetFileMD(prevFlags.Input.Files).get("G4Version", "")
        if not version:
            from os import environ
            version = str(environ.get("G4VERS", ""))
        if prevFlags.Input.isMC and isGaudiEnv() and not version:
            raise ValueError("Unknown G4 version")
        return version

    scf.addFlag("Sim.G4Version", _check_G4_version)

    def _checkPhysicsListConf(prevFlags):
        physicsList = "FTFP_BERT_ATL"
        if prevFlags.Input.Files:
            physicsList = GetFileMD(prevFlags.Input.Files).get("PhysicsList", "")
            if not physicsList:
                # Currently physicsList is also part of /Digitization/Parameters metadata. TODO migrate away from this.
                physicsList = GetFileMD(prevFlags.Input.Files).get("physicsList", "FTFP_BERT_ATL")
        return physicsList

    scf.addFlag("Sim.PhysicsList", _checkPhysicsListConf)
    scf.addFlag("Sim.NeutronTimeCut", 150.) # Sets the value for the neutron out of time cut in G4
    scf.addFlag("Sim.NeutronEnergyCut", -1.) # Sets the value for the neutron energy cut in G4
    scf.addFlag("Sim.ApplyEMCuts", False) # Turns on the G4 option to apply cuts for EM physics
    scf.addFlag("Sim.MuonFieldOnlyInCalo", False) # Only muons see the B-field in the calo

    # G4AtlasToolsConfig
    scf.addFlag("Sim.RecordStepInfo", False)
    scf.addFlag("Sim.StoppedParticleFile", "")
    scf.addFlag("Sim.BeamPipeSimMode", BeamPipeSimMode.Normal, type=BeamPipeSimMode)
    scf.addFlag("Sim.LArParameterization", LArParameterization.NoFrozenShowers, type=LArParameterization)
    # TRT Range cut used in simulation in mm. Should be 0.05 or 30.
    scf.addFlag("Sim.TRTRangeCut",
                lambda prevFlags: float(GetFileMD(prevFlags.Input.Files).get('TRTRangeCut', 30.0)))

    # BeameffectsAlg
    scf.addFlag("Sim.VertexSource", VertexSource.CondDB, type=VertexSource)
    scf.addFlag("Sim.VertexTimeSmearing", lambda prevFlags:
                prevFlags.Beam.Type == BeamType.Collisions and prevFlags.GeoModel.Run >= LHCPeriod.Run4)

    def _checkVertexTimeWidth(prevFlags):
        default = 0.175*ns
        vertexTimeWidth  = default
        if prevFlags.Input.Files:
            vertexTimeWidth = GetFileMD(prevFlags.Input.Files).get("VertexTimeWidth", default)
        return vertexTimeWidth

    scf.addFlag("Sim.VertexTimeWidth", _checkVertexTimeWidth)

    # G4UserActions
    scf.addFlag("Sim.NRRThreshold", False)
    scf.addFlag("Sim.NRRWeight", False)
    scf.addFlag("Sim.PRRThreshold", False)
    scf.addFlag("Sim.PRRWeight", False)
    scf.addFlag("Sim.OptionalUserActionList", [])

    # G4FieldConfig
    scf.addFlag("Sim.G4Stepper", "AtlasRK4")
    scf.addFlag("Sim.G4EquationOfMotion", "")
    scf.addFlag("Sim.UsingGeant4", True)

    # Cosmics
    #  volume(s) used to do cosmics filtering
    #  G4 volume names from {"Muon", "Calo", "InnerDetector", "TRT_Barrel", "TRT_EC", "SCT_Barrel", "Pixel"}
    scf.addFlag("Sim.CosmicFilterVolumeNames", ["InnerDetector"])
    scf.addFlag("Sim.CosmicFilterID", False) # PDG ID to be filtered ("13")
    scf.addFlag("Sim.CosmicFilterPTmin", False) # min pT filtered in cosmics processing (MeV) ("5000")
    scf.addFlag("Sim.CosmicFilterPTmax", False) # max pT filtered in cosmics processing (MeV) ("6000")
    scf.addFlag("Sim.CosmicPtSlice", "Off") # 'slice1', 'slice2', 'slice3', 'slice4', 'NONE'

    # ISF
    scf.addFlag("Sim.ISFRun", False)

    def _checkSimulationFlavour(prevFlags):
        simulator = SimulationFlavour.Unknown
        if prevFlags.Input.Files:
            simFlavour = GetFileMD(prevFlags.Input.Files).get("Simulator", "")
            if not simFlavour:
                simFlavour = GetFileMD(prevFlags.Input.Files).get("SimulationFlavour", "")
            try:
                simulator = SimulationFlavour(simFlavour)
            except ValueError:
                # Deal with old non-thread-safe simulators
                if simFlavour in ['default']: # This is the case when ISF was not configured in sim
                    simulator = SimulationFlavour.AtlasG4
                elif simFlavour in ['MC12G4', 'FullG4']:
                    simulator = SimulationFlavour.FullG4MT
                elif simFlavour in ['FullG4_QS', 'FullG4_LongLived']:
                    simulator = SimulationFlavour.FullG4MT_QS
                elif simFlavour in ['PassBackG4']:
                    simulator = SimulationFlavour.PassBackG4MT
                elif simFlavour in ['ATLFASTII']:
                    simulator = SimulationFlavour.ATLFASTIIMT
                elif simFlavour in ['ATLFASTIIF']:
                    simulator = SimulationFlavour.ATLFASTIIFMT
                elif simFlavour in ['ATLFAST3']:
                    simulator = SimulationFlavour.ATLFAST3MT
                elif simFlavour in ['ATLFAST3_QS']:
                    simulator = SimulationFlavour.ATLFAST3MT_QS
                else:
                    # Obscure old-style configuration used - do not try to interpret
                    simulator = SimulationFlavour.Unknown
        return simulator

    scf.addFlag("Sim.ISF.Simulator", _checkSimulationFlavour, type=SimulationFlavour)
    scf.addFlag("Sim.ISF.DoTimeMonitoring", True) # bool: run time monitoring
    scf.addFlag("Sim.ISF.DoMemoryMonitoring", True) # bool: run time monitoring
    scf.addFlag("Sim.ISF.ValidationMode", False) # bool: run ISF internal validation checks
    scf.addFlag("Sim.ISF.ReSimulation", False) # Using ReSimulation workflow
    scf.addFlag("Sim.ISF.UseTrackingGeometryCond", False) # Using Condition for tracking Geometry

    def _decideHITSMerging(prevFlags):
        # Further specialization possible in future
        if prevFlags.Sim.ISF.Simulator.isFullSim():
            doID = False
            doITk = False
            doCALO = False
            doMUON = False
        elif prevFlags.Sim.ISF.Simulator.usesFatras() and prevFlags.Sim.ISF.Simulator.usesFastCaloSim():
            doID = True
            doITk = True
            doCALO = True
            doMUON = True
        elif prevFlags.Sim.ISF.Simulator.usesFastCaloSim():
            doID = False
            doITk = False
            doCALO = True
            doMUON = False
        elif prevFlags.Sim.ISF.Simulator in [SimulationFlavour.Unknown]:
            doID = True
            doITk = True
            doCALO = True
            doMUON = True
        else:
            raise ValueError("Invalid simulator")
        return {"ID": doID, "CALO": doCALO, "MUON": doMUON, "ITk": doITk}

    scf.addFlag("Sim.ISF.HITSMergingRequired", _decideHITSMerging)

    scf.addFlag("Sim.FastCalo.ParamsInputFilename", "FastCaloSim/MC23/TFCSparam_AF3_MC23_Sep23.root") # filename of the input parametrizations file
    scf.addFlag("Sim.FastCalo.RunOnGPU", False) # Determines if run the FastCaloSim on GPU or not
    scf.addFlag("Sim.FastCalo.CaloCellsName", "AllCalo") # StoreGate collection name for FastCaloSim hits

    scf.addFlag("Sim.FastShower.InputCollection", "TruthEvent") # StoreGate collection name of modified TruthEvent for legacy FastCaloSim use

    # FastChain
    # Setting the BCID for Out-of-Time PU events, list of int
    scf.addFlag("Sim.FastChain.BCID", [1])
    # weights for Out-of-Time PU events
    scf.addFlag("Sim.FastChain.PUWeights_lar_em", [1.0]) # LAr EM
    scf.addFlag("Sim.FastChain.PUWeights_lar_hec", [1.0]) # LAr HEC
    scf.addFlag("Sim.FastChain.PUWeights_lar_bapre", [1.0]) # LAr Barrel presampler
    scf.addFlag("Sim.FastChain.PUWeights_tile", [1.0]) # Tile

    # Fatras
    scf.addFlag("Sim.Fatras.RandomStreamName", "FatrasRnd")
    scf.addFlag("Sim.Fatras.G4RandomStreamName", "FatrasG4")
    scf.addFlag("Sim.Fatras.TrkExRandomStreamName", "TrkExRnd")
    # Fatras fine tuning
    scf.addFlag("Sim.Fatras.MomCutOffSec", 50.) # common momentum cut-off for secondaries
    scf.addFlag("Sim.Fatras.HadronIntProb", 1.) # hadronic interaction scale factor
    scf.addFlag("Sim.Fatras.GaussianMixtureModel", True) # use Gaussian mixture model for Multiple Scattering
    scf.addFlag("Sim.Fatras.BetheHeitlerScale", 1.) # scale to Bethe-Heitler contribution

    scf.addFlag("Sim.BeamPipeCut", 100.0)
    scf.addFlag("Sim.TightMuonStepping", False)

    scf.addFlag('Sim.GenerationConfiguration', 'NONE') # TODO replace this property with something more central for all Generator configuration

    return scf


def simulationRunArgsToFlags(runArgs, flags):
    """Fill simulation configuration flags from run arguments."""
    if hasattr(runArgs, "DataRunNumber"):
        flags.Input.RunNumbers = [runArgs.DataRunNumber]
        flags.Input.OverrideRunNumber = True
        flags.Input.LumiBlockNumbers = [1] # dummy value

    if hasattr(runArgs, "jobNumber"):
        flags.Input.JobNumber = runArgs.jobNumber

    if hasattr(runArgs, "physicsList"):
        flags.Sim.PhysicsList = runArgs.physicsList

    if hasattr(runArgs, "truthStrategy"):
        flags.Sim.TruthStrategy = TruthStrategy(runArgs.truthStrategy)

    # Not used as deprecated
    # '--enableLooperKiller'
    # '--perfmon'
    # '--randomSeed'
    # '--useISF'
