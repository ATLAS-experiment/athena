# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#==============================================================
#
#
#		This job option runs the G4 simulation
#		of the ATLAS detector and the GeantFollower in ID (and MS)
#
#==============================================================

from AthenaCommon.Constants import INFO

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


# Argument parsing
def setupArgParser():
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument("--inputevntfile",
                        default="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/EVGEN_ParticleGun_FourMuon_Pt10to500.root",
                        type=str,
                        help="The input EVNT file to use")
    parser.add_argument("--outputhitsfile",
                        default="myHITS.pool.root", 
                        type=str,
                        help="The output HITS filename")
    parser.add_argument("--geometrytag",
                        default="ATLAS-R3S-2021-03-00-00", 
                        type=str,
                        help="The geometry tag to use")
    parser.add_argument("--globaltag",
                        default="OFLCOND-MC21-SDR-RUN3-09", 
                        type=str,
                        help="The global tag to use")
    parser.add_argument("--myPDG",
                        default=998, 
                        type=int,
                        help="PDG for particle gun sim. 998 = Charged Geantino 999 = neutral Geantino, 13 = Muon")
    parser.add_argument("--maxEvents",
                        default=10000,
                        type = int,
                        help="Maximum number of events to run on.")
    return parser
  

def MSCfg(flags):
    # Setup main services
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    acc.getService("MessageSvc").debugLimit = 1000000
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))
    
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))

    from BeamEffects.BeamEffectsAlgConfig import BeamEffectsAlgCfg
    acc.merge(BeamEffectsAlgCfg(flags))
    
    from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
    acc.merge(BeamPipeGeometryCfg(flags))
    
    from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import TrackingGeometryCondAlgCfg
    acc.merge(TrackingGeometryCondAlgCfg(flags))

    from TrkConfig.AtlasTrackingGeometrySvcConfig import TrackingGeometrySvcCfg
    acc.merge(TrackingGeometrySvcCfg(flags))
  
    return acc

def ExtrapolationToolCfg(flags, args):

    acc = ComponentAccumulator()

    # the layer material inspector
    LayerMaterialInspector = CompFactory.Trk.LayerMaterialInspector("LayerMaterialInspector", 
                                                                    OutputLevel=INFO)
    acc.addPublicTool(LayerMaterialInspector)

    # the tracking volume displayer
    from TrkDetDescrSvc.TrkDetDescrJobProperties import TrkDetFlags
    TrackingVolumeDisplayer = CompFactory.Trk.TrackingVolumeDisplayer("TrackingVolumeDisplayer",
                                                                        TrackingVolumeOutputFile='TrackingVolumes-'+TrkDetFlags.MaterialMagicTag()+'.C',
                                                                        LayerOutputFile='Layers-'+TrkDetFlags.MaterialMagicTag()+'.C',
                                                                        SurfaceOutputFile='Surfaces-'+TrkDetFlags.MaterialMagicTag()+'.C')
    acc.addPublicTool(TrackingVolumeDisplayer)

    # PROPAGATOR DEFAULTS 
    TestPropagators = []
    TestPropagator = CompFactory.Trk.RungeKuttaPropagator("TestPropagator")
    acc.addPublicTool(TestPropagator)
    TestPropagators += [TestPropagator]

    from TrkConfig.TrkExSTEP_PropagatorConfig import AtlasSTEP_PropagatorCfg
    TestSTEP_Propagator = acc.popToolsAndMerge(AtlasSTEP_PropagatorCfg(flags, name="TestSTEP_Propagator", DetailedEloss=True))
    TestPropagators += [TestSTEP_Propagator]

    TestSTEP_Propagator.Straggling = False

    if args.myPDG == 998 :
        TestSTEP_Propagator.MultipleScattering = False
        TestSTEP_Propagator.EnergyLoss = False

    # UPDATOR DEFAULTS
    TestUpdators = []

    TestMaterialEffectsUpdator = CompFactory.Trk.MaterialEffectsUpdator("TestMaterialEffectsUpdator")
    acc.addPublicTool(TestMaterialEffectsUpdator)
    if args.myPDG == 998 :
        TestMaterialEffectsUpdator.EnergyLoss           = False
        TestMaterialEffectsUpdator.MultipleScattering   = False

    TestUpdators += [TestMaterialEffectsUpdator]

    TestMaterialEffectsUpdatorLandau = CompFactory.Trk.MaterialEffectsUpdator("TestMaterialEffectsUpdatorLandau",
                                                                                LandauMode=True)
    acc.addPublicTool(TestMaterialEffectsUpdatorLandau)

    if args.myPDG == 998 :
        TestMaterialEffectsUpdatorLandau.EnergyLoss           = False
        TestMaterialEffectsUpdatorLandau.MultipleScattering   = False

    ##testUpdators    += [ testMaterialEffectsUpdatorLandau ]

    # the UNIQUE NAVIGATOR ( === UNIQUE GEOMETRY) 
    TestNavigator = CompFactory.Trk.Navigator("TestNavigator",
                                                TrackingGeometrySvc="Trk::TrackingGeometrySvc/AtlasTrackingGeometrySvc")
    acc.addPublicTool(TestNavigator)

    # CONFIGURE PROPAGATORS/UPDATORS ACCORDING TO GEOMETRY SIGNATURE

    TestSubPropagators = []
    TestSubUpdators = []

    # -------------------- set it depending on the geometry ----------------------------------------------------
    # default for Global is (Rk,Mat)
    TestSubPropagators += [ TestPropagator.name ]
    TestSubUpdators    += [ TestMaterialEffectsUpdator.name ]

    # default for ID is (Rk,Mat)
    TestSubPropagators += [ TestPropagator.name ]
    TestSubUpdators    += [ TestMaterialEffectsUpdator.name ]

    # default for Beam pipe is (Rk,Mat) // sometimes the STEP is used
    TestSubPropagators += [ TestPropagator.name ]
    TestSubUpdators    += [ TestMaterialEffectsUpdator.name ]

    # default for Calo is (STEP,Mat)  // sometimes MatLandau is used
    TestSubPropagators += [ TestSTEP_Propagator.name ]
    #TestSubPropagators += [ TestPropagator.name ] #Switch to RK
    TestSubUpdators    += [ TestMaterialEffectsUpdator.name ]

    # default for MS is (STEP,Mat)
    TestSubPropagators += [ TestSTEP_Propagator.name ]
    TestSubUpdators    += [ TestMaterialEffectsUpdator.name ]

    # default for Cavern is (Rk,Mat)
    TestSubPropagators += [ TestPropagator.name ]
    TestSubUpdators    += [ TestMaterialEffectsUpdator.name ]
    # ----------------------------------------------------------------------------------------------------------

    AtlasEnergyLossUpdator = CompFactory.Trk.EnergyLossUpdator("AtlasEnergyLossUpdator",
                                                                DetailedEloss=True)
    acc.addPublicTool(AtlasEnergyLossUpdator)
    
    # AtlasELossUpdater = acc.popToolsAndMerge(TC.AtlasEnergyLossUpdatorCfg(flags))
    # AtlasEnergyLossUpdater = AtlasELossUpdater
   
    # from TrkConfig.AtlasExtrapolatorToolsConfig as TC
    # AtlasEnergyLossUpdator = acc.popToolsAndMerge(AtlasEnergyLossUpdatorCfg(flags, name="AtlasEnergyLossUpdator", DetailedEloss=True) )

    # call the base class constructor
    TestExtrapolator = CompFactory.Trk.Extrapolator("TestExtrapolator",
                                                    Navigator = TestNavigator,
                                                    MaterialEffectsUpdators = TestUpdators,
                                                    STEP_Propagator = TestSTEP_Propagator.name,
                                                    Propagators = TestPropagators,
                                                    SubPropagators = TestSubPropagators,
                                                    SubMEUpdators = TestSubUpdators,
                                                    EnergyLossUpdater = AtlasEnergyLossUpdator
                                                    )
    acc.addPublicTool(TestExtrapolator, primary=True)
    # acc.setPrivateTools(TestExtrapolator)

    return acc


def GeantFollowerMSCfg(flags, args, name="GeantFollowerMSSvc", **kwargs):

    result = ComponentAccumulator()
#    from TrkConfig.AtlasExtrapolatorConfig import MuonExtrapolatorCfg    
#    extrapolator = result.getPrimaryAndMerge(MuonExtrapolatorCfg(flags))    
    extrapolator = result.getPrimaryAndMerge(ExtrapolationToolCfg(flags,args))
    result.addPublicTool(extrapolator)    
   
    #Setup Helper
    followingHelper = CompFactory.Trk.GeantFollowerMSHelper("GeantFollowerMSHelper",
                                                          Extrapolator= extrapolator,
                                                          ExtrapolateDirectly=False,
                                                          ExtrapolateIncrementally=False,
                                                          SpeedUp=True,
                                                          UseCovMatrix=True,
                                                          OutputLevel=INFO)
    result.addPublicTool(followingHelper, primary = True)

    #Setting up the CA for the GeantFollowerMS
    from G4AtlasServices.G4AtlasUserActionConfig import getDefaultActions
    kwargs.setdefault("UserActionTools", result.popToolsAndMerge(getDefaultActions(flags)))
    result.addService(CompFactory.G4UA.UserActionSvc(name, **kwargs))
    return result


if __name__ =="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    args = setupArgParser().parse_args()
    # Configure
    flags = initConfigFlags()
    flags.Input.Files = [args.inputevntfile] # default: ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/EVGEN_ParticleGun_FourMuon_Pt10to500.root']
    flags.Output.HITSFileName = args.outputhitsfile # default: "myHITS.pool.root"
    flags.GeoModel.AtlasVersion = args.geometrytag # default: "ATLAS-R3S-2021-03-00-00"
    flags.IOVDb.GlobalTag = args.globaltag # default: "OFLCOND-MC21-SDR-RUN3-09"
    flags.Input.isMC = True
    flags.GeoModel.Align.Dynamic = False
    flags.Concurrency.NumThreads =1
    flags.Concurrency.NumConcurrentEvents = 1
    from AthenaConfiguration.Enums import  ProductionStep
    flags.Common.ProductionStep = ProductionStep.FastChain 
    flags.LAr.doAlign = True
    flags.Sim.TightMuonStepping = True

    # Setup detector   
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorsFromList
    detectors = ["Muon","ID","Bpipe", "Calo"]  
    setupDetectorsFromList(flags, detectors, toggle_geometry=True)

    flags.lock()
    flags.dump(evaluate = True)

    # Construct component accumulator
    acc = MSCfg(flags)    
    acc.merge(GeantFollowerMSCfg(flags,args))
    
    #DEV#
    #Setting up the CA for the GeantFollowerMS
    from TrkG4UserActions.TrkG4UserActionsConfig import GeantFollowerMSToolCfg
    GeantFollowerMSTool = acc.getPrimaryAndMerge(GeantFollowerMSToolCfg(flags))
    #DEV#

    from G4AtlasAlg.G4AtlasAlgConfig import G4AtlasAlgCfg
    acc.merge(G4AtlasAlgCfg(flags,UserActionTools=[GeantFollowerMSTool],
                                        ExtraInputs={( 'Trk::TrackingGeometry' , 'ConditionStore+AtlasTrackingGeometry'),
                                                     ( 'AtlasFieldCacheCondObj' , 'ConditionStore+fieldCondObj' )}))

    #from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    #from SimuJobTransforms.SimOutputConfig import getStreamHITS_ItemList
    #acc.merge(OutputStreamCfg(flags,"HITS", ItemList=getStreamHITS_ItemList(flags), disableEventTag=True, AcceptAlgs=['MSG4AtlasAlg']))

    # Execute
    acc.printConfig(withDetails=True)
    if not acc.run(maxEvents=args.maxEvents).isSuccess():
        exit(1)
