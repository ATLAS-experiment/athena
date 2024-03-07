# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# ComponentAccumulator based configuration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
GlobalFieldManagerTool, DetectorFieldManagerTool=CompFactory.getComps("GlobalFieldManagerTool","DetectorFieldManagerTool",)
from G4AtlasServices.G4AtlasFieldServices import StandardFieldSvcCfg, Q1FwdG4FieldSvcCfg, Q2FwdG4FieldSvcCfg, Q3FwdG4FieldSvcCfg, D1FwdG4FieldSvcCfg, D2FwdG4FieldSvcCfg, Q4FwdG4FieldSvcCfg, Q5FwdG4FieldSvcCfg, Q6FwdG4FieldSvcCfg, Q7FwdG4FieldSvcCfg, Q1HKickFwdG4FieldSvcCfg, Q1VKickFwdG4FieldSvcCfg, Q2HKickFwdG4FieldSvcCfg, Q2VKickFwdG4FieldSvcCfg, Q3HKickFwdG4FieldSvcCfg, Q3VKickFwdG4FieldSvcCfg, Q4VKickAFwdG4FieldSvcCfg, Q4HKickFwdG4FieldSvcCfg, Q4VKickBFwdG4FieldSvcCfg, Q5HKickFwdG4FieldSvcCfg, Q6VKickFwdG4FieldSvcCfg


def EquationOfMotionCfg(flags, **kwargs):
    """Return the TruthService config flagged by Sim.TruthStrategy"""
    from Monopole.MonopoleConfig import G4mplEqMagElectricFieldToolCfg
    stratmap = {
       "G4mplEqMagElectricField" : G4mplEqMagElectricFieldToolCfg,
    }
    xCfg = stratmap[flags.Sim.G4EquationOfMotion]
    return xCfg(flags, **kwargs)


# Field Managers
def ATLASFieldManagerToolCfg(flags, name='ATLASFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("IntegratorStepper", flags.Sim.G4Stepper)
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(StandardFieldSvcCfg(flags)).name)
    kwargs.setdefault("UseTightMuonStepping", False)
    if flags.Sim.G4EquationOfMotion:
        kwargs.setdefault("EquationOfMotion", result.popToolsAndMerge(EquationOfMotionCfg(flags)))
    result.setPrivateTools(GlobalFieldManagerTool(name, **kwargs))
    return result


def TightMuonsATLASFieldManagerToolCfg(flags, name='TightMuonsATLASFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("IntegratorStepper", flags.Sim.G4Stepper)
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(StandardFieldSvcCfg(flags)).name)
    kwargs.setdefault("UseTightMuonStepping",True)
    if flags.Sim.G4EquationOfMotion:
        kwargs.setdefault("EquationOfMotion", result.popToolsAndMerge(EquationOfMotionCfg(flags)))
    result.setPrivateTools(GlobalFieldManagerTool(name, **kwargs))
    return result


#not used in G4AtlasServicesConfig?
def ClassicFieldManagerToolCfg(flags, name='ClassicFieldManager', **kwargs):
    kwargs.setdefault("IntegratorStepper", "ClassicalRK4")
    return ATLASFieldManagerToolCfg(flags, name, **kwargs)


def BasicDetectorFieldManagerToolCfg(flags, name='BasicDetectorFieldManager', **kwargs):
    result = ComponentAccumulator()
    if 'FieldSvc' not in kwargs: # don't create the StandardFieldSvc if it is not required by this tool.
        kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(StandardFieldSvcCfg(flags)).name)
    kwargs.setdefault("IntegratorStepper", flags.Sim.G4Stepper)
    kwargs.setdefault('MuonOnlyField',     False)
    if flags.Sim.G4EquationOfMotion:
        kwargs.setdefault("EquationOfMotion", result.popToolsAndMerge(EquationOfMotionCfg(flags)))
    result.setPrivateTools(DetectorFieldManagerTool(name, **kwargs))
    return result


def BeamPipeFieldManagerToolCfg(flags, name='BeamPipeFieldManager', **kwargs):
    kwargs.setdefault("LogicalVolumes", ['BeamPipe::BeamPipe'])
    #kwargs.setdefault('DeltaChord',         0.00001)
    kwargs.setdefault('DeltaIntersection',  0.00001)
    kwargs.setdefault('DeltaOneStep',       0.0001)
    kwargs.setdefault('MaximumEpsilonStep', 0.001)
    kwargs.setdefault('MinimumEpsilonStep', 0.00001)
    return BasicDetectorFieldManagerToolCfg(flags, name, **kwargs)


def InDetFieldManagerToolCfg(flags, name='InDetFieldManager', **kwargs):
    kwargs.setdefault("LogicalVolumes", ['IDET::IDET'])
    #kwargs.setdefault('DeltaChord',         0.00001)
    kwargs.setdefault('DeltaIntersection',  0.00001)
    kwargs.setdefault('DeltaOneStep',       0.0001)
    kwargs.setdefault('MaximumEpsilonStep', 0.001)
    kwargs.setdefault('MinimumEpsilonStep', 0.00001)
    return BasicDetectorFieldManagerToolCfg(flags, name, **kwargs)


def ITkFieldManagerToolCfg(flags, name='ITkFieldManager', **kwargs):
    kwargs.setdefault("LogicalVolumes", ['ITK::ITK'])
    #kwargs.setdefault('DeltaChord',         0.00001)
    kwargs.setdefault('DeltaIntersection',  0.00001)
    kwargs.setdefault('DeltaOneStep',       0.0001)
    kwargs.setdefault('MaximumEpsilonStep', 0.001)
    kwargs.setdefault('MinimumEpsilonStep', 0.00001)
    return BasicDetectorFieldManagerToolCfg(flags, name, **kwargs)


def MuonsOnlyInCaloFieldManagerToolCfg(flags, name='MuonsOnlyInCaloFieldManager', **kwargs):
    kwargs.setdefault("PhysicalVolumes", ['LArBarrel'])
    #kwargs.setdefault('DeltaChord',         0.00000002)
    kwargs.setdefault('DeltaIntersection',  0.00000002)
    kwargs.setdefault('DeltaOneStep',       0.000001)
    kwargs.setdefault('MaximumEpsilonStep', 0.0000009)
    kwargs.setdefault('MinimumEpsilonStep', 0.000001)
    kwargs.setdefault('MuonOnlyField',      True)
    return BasicDetectorFieldManagerToolCfg(flags, name, **kwargs)

def MuonFieldManagerToolCfg(flags, name='MuonFieldManager', **kwargs):
    kwargs.setdefault("LogicalVolumes", ['MUONQ02::MUONQ02'])
    #kwargs.setdefault('DeltaChord',         0.00000002)
    kwargs.setdefault('DeltaIntersection',  0.00000002)
    kwargs.setdefault('DeltaOneStep',       0.000001)
    kwargs.setdefault('MaximumEpsilonStep', 0.0000009)
    kwargs.setdefault('MinimumEpsilonStep', 0.000001)
    return BasicDetectorFieldManagerToolCfg(flags, name, **kwargs)


#not used in G4AtlasServicesConfig?
def BasicFwdFieldManagerToolCfg(flags, name='FwdFieldManagerTool', **kwargs):
    #kwargs.setdefault('DeltaChord',         0.00000002)
    kwargs.setdefault('DeltaIntersection',  1e-9)
    kwargs.setdefault('DeltaOneStep',       1e-8)
    kwargs.setdefault('MaximumEpsilonStep', 1e-8)
    kwargs.setdefault('MinimumEpsilonStep', 1e-9)
    # Deliberately left commented out for now
    #if flags.Sim.FwdStepLimitation > 0:
    #    kwargs.setdefault("MaximumStep", flags.Sim.FwdStepLimitation)
    if False:
        kwargs.setdefault("MaximumStep", 1000.)
    return BasicDetectorFieldManagerToolCfg(flags, name, **kwargs)


def Q1FwdFieldManagerToolCfg(flags, name='Q1FwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q1FwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQXAA.1R1MagQ1'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q2FwdFieldManagerToolCfg(flags, name='Q2FwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q2FwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQXBA.2R1MagQ2a', 'FwdRegion::LQXBA.2R1MagQ2b'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q3FwdFieldManagerToolCfg(flags, name='Q3FwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q3FwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQXAG.3R1MagQ3'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def D1FwdFieldManagerToolCfg(flags, name='D1FwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(D1FwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::MBXW.A4R1MagD1a', 'FwdRegion::MBXW.B4R1MagD1b',
                                         'FwdRegion::MBXW.C4R1MagD1c', 'FwdRegion::MBXW.D4R1MagD1d',
                                         'FwdRegion::MBXW.E4R1MagD1e', 'FwdRegion::MBXW.F4R1MagD1f'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def D2FwdFieldManagerToolCfg(flags, name='D2FwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(D2FwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LBRCD.4R1MagD2'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q4FwdFieldManagerToolCfg(flags, name='Q4FwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q4FwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQYCH.4R1MagQ4'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q5FwdFieldManagerToolCfg(flags, name='Q5FwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q5FwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQNDC.5R1MagQ5'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q6FwdFieldManagerToolCfg(flags, name='Q6FwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q6FwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQNDD.6R1MagQ6'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q7FwdFieldManagerToolCfg(flags, name='Q7FwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q7FwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQNFD.7R1MagQ7a', 'FwdRegion::LQNFD.7R1MagQ7b'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q1HKickFwdFieldManagerToolCfg(flags, name='Q1HKickFwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q1HKickFwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQXAA.1R1MagQ1HKick'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q1VKickFwdFieldManagerToolCfg(flags, name='Q1VKickFwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q1VKickFwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQXAA.1R1MagQ1VKick'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q2HKickFwdFieldManagerToolCfg(flags, name='Q2HKickFwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q2HKickFwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQXBA.2R1MagQ2HKick'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q2VKickFwdFieldManagerToolCfg(flags, name='Q2VKickFwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q2VKickFwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQXBA.2R1MagQ2VKick'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q3HKickFwdFieldManagerToolCfg(flags, name='Q3HKickFwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q3HKickFwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQXAG.3R1MagQ3HKick'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q3VKickFwdFieldManagerToolCfg(flags, name='Q3VKickFwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q3VKickFwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQXAG.3R1MagQ3VKick'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q4VKickAFwdFieldManagerToolCfg(flags, name='Q4VKickAFwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q4VKickAFwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQYCH.4R1MagQ4VKickA'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q4HKickFwdFieldManagerToolCfg(flags, name='Q4HKickFwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q4HKickFwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQYCH.4R1MagQ4HKick'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q4VKickBFwdFieldManagerToolCfg(flags, name='Q4VKickBFwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q4VKickBFwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQYCH.4R1MagQ4VKickB'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q5HKickFwdFieldManagerToolCfg(flags, name='Q5HKickFwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q5HKickFwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQNDC.5R1MagQ5HKick'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def Q6VKickFwdFieldManagerToolCfg(flags, name='Q6VKickFwdFieldManager', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FieldSvc", result.getPrimaryAndMerge(Q6VKickFwdG4FieldSvcCfg(flags)).name)
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::LQNDD.6R1MagQ6VKick'])
    acc=BasicFwdFieldManagerToolCfg(flags, name, **kwargs)
    tool = result.popToolsAndMerge(acc)
    result.setPrivateTools(tool)
    return result


def FwdRegionFieldManagerToolCfg(flags, name='FwdRegionFieldManager', **kwargs):
    kwargs.setdefault("LogicalVolumes", ['FwdRegion::ForwardRegionGeoModel'])
    # Deliberately left commented out for now
    #if flags.Sim.FwdStepLimitation > 0:
    #    kwargs.setdefault("MaximumStep", flags.Sim.FwdStepLimitation)
    if False:
        kwargs.setdefault("MaximumStep", 1000.)
    return BasicDetectorFieldManagerToolCfg(flags, name, **kwargs)
