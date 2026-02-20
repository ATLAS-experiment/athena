# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def setupTestOutputCfg(flags,**kwargs):

    kwargs.setdefault("AcceptAlgs",[])
  
    result = ComponentAccumulator()
    ### Setup an xAOD Stream to test the size of the Mdt container
    # =============================
    # Define contents of the format
    # =============================
    from MuonSensitiveDetectorsR4.SensitiveDetectorsCfg import OutputSimContainersCfg
    container_items = ["xAOD::TruthParticleContainer#",
                       "xAOD::TruthParticleAuxContainer#",
                       "xAOD::TruthEventContainer#",
                       "xAOD::TruthEventAuxContainer#",
                       "McEventCollection#TruthEvent"] + OutputSimContainersCfg(flags)

    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    result.merge(SetupMetaDataForStreamCfg(flags, kwargs["streamName"]))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    kwargs.setdefault("ItemList", container_items)
    result.merge(OutputStreamCfg(flags, **kwargs))
    return result

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    parser = SetupArgParser()
    parser.add_argument("--saveTestNtuple", help="Schedule the SimHits tester n-tuple", action='store_true',
                         default=False)
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(outRootFile="SimHits.pool.root")

    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Sim.ReleaseGeoModel = True

    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep = ProductionStep.Simulation

    from SimulationConfig.SimEnums import SimulationFlavour
    flags.Sim.ISF.Simulator = SimulationFlavour.AtlasG4
    streamName = "MuonSimTestStream"
    flags.addFlag(f"Output.{streamName}FileName", args.outRootFile)
    flags.addFlag(f"Output.doWrite{streamName}", True)

    flags, cfg = setupGeoR4TestCfg(args, flags)
    
    from BeamEffects.BeamEffectsAlgConfig import BeamEffectsAlgCfg
    cfg.merge(BeamEffectsAlgCfg(flags))

    from G4AtlasAlg.G4AtlasAlgConfig import G4AtlasAlgCfg
    cfg.merge(G4AtlasAlgCfg(flags))
    ### Keep the Volume debugger commented for the moment
    #from G4DebuggingTools.PostIncludes import VolumeDebuggerAtlas
    #cfg.merge(VolumeDebuggerAtlas(flags, name="G4UA::UserActionSvc", 
    #                                     PrintGeometry = True,
    #                                     TargetVolume="BIS7_RPC26_7_0_1_1_1"
    #                                    ))
    
    from xAODTruthCnv.xAODTruthCnvConfig import GEN_EVNT2xAODCfg
    cfg.merge(GEN_EVNT2xAODCfg(flags,name="GEN_EVNT2xAOD",AODContainerName="TruthEvent"))

    cfg.merge(setupTestOutputCfg(flags, streamName=streamName))
    if args.saveTestNtuple:
        from MuonPRDTestR4.MuonHitTestConfig import MuonHitTesterCfg
        cfg.merge(MuonHitTesterCfg(flags))
    executeTest(cfg)
  
