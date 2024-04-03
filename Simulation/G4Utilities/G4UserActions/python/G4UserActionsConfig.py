# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from SimulationConfig.SimEnums import CalibrationRun

# this is a bit cumbersome, but it seems ike it is a lot easier to separate
# the getter functionality from all the rest (i.e. adding the action).
# This way, e.g., after the getter is called the tool is automatically added
# to the ToolSvc and can be assigned to a ToolHandle by the add function.
# Also, passing arguments to the getter (like "this is a system action") is not straightforward

def AthenaStackingActionToolCfg(flags, name='G4UA::AthenaStackingActionTool', **kwargs):

    result = ComponentAccumulator()
    ## Killing neutrinos
    if "ATLAS" in flags.GeoModel.AtlasVersion:
        kwargs.setdefault('KillAllNeutrinos', True)
    ## Neutron Russian Roulette
    if flags.Sim.NRRThreshold and flags.Sim.NRRWeight:
        if flags.Sim.CalibrationRun is not CalibrationRun.Off:
            raise NotImplementedError("Neutron Russian Roulette should not be used in Calibration Runs.")
        kwargs.setdefault('ApplyNRR', True)
        kwargs.setdefault('NRRThreshold', flags.Sim.NRRThreshold)
        kwargs.setdefault('NRRWeight', flags.Sim.NRRWeight)
    ## Photon Russian Roulette
    if flags.Sim.PRRThreshold and flags.Sim.PRRWeight:
        if flags.Sim.CalibrationRun is not CalibrationRun.Off:
            raise NotImplementedError("Photon Russian Roulette should not be used in Calibration Runs.")
        kwargs.setdefault('ApplyPRR', True)
        kwargs.setdefault('PRRThreshold', flags.Sim.PRRThreshold)
        kwargs.setdefault('PRRWeight', flags.Sim.PRRWeight)
    kwargs.setdefault('IsISFJob', flags.Sim.ISFRun)

    result.setPrivateTools( CompFactory.G4UA.AthenaStackingActionTool(name,**kwargs) )
    return result


def AthenaTrackingActionToolCfg(flags, name='G4UA::AthenaTrackingActionTool', **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault('SecondarySavingLevel', 2)
    
    subDetLevel=1
    if "ATLAS" in flags.GeoModel.AtlasVersion and flags.Detector.GeometryCavern:
        subDetLevel=2

    kwargs.setdefault('SubDetVolumeLevel', subDetLevel)
    result.setPrivateTools( CompFactory.G4UA.AthenaTrackingActionTool(name,**kwargs) )
    return result

def LooperKillerToolCfg(flags, name='G4UA::LooperKillerTool', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.G4UA.LooperKillerTool(name, **kwargs))
    return result

def G4SimTimerToolCfg(flags, name='G4UA::G4SimTimerTool', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.G4UA.G4SimTimerTool(name,**kwargs))
    return result


def G4TrackCounterToolCfg(flags, name='G4UA::G4TrackCounterTool', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.G4UA.G4TrackCounterTool(name,**kwargs))
    return result


def StoppedParticleActionToolCfg(flags, name="G4UA::StoppedParticleActionTool", **kwargs):
    # Just have to set the stopping condition
    result = ComponentAccumulator()
    # FIXME UserActionConfig not yet migrated
    # example custom configuration 
    # if name in simFlags.UserActionConfig.get_Value().keys():
    #     for prop,value in simFlags.UserActionConfig.get_Value()[name].iteritems():
    #         kwargs.setdefault(prop,value)
    result.setPrivateTools(CompFactory.G4UA.StoppedParticleActionTool(name, **kwargs))
    return result


def FixG4CreatorProcessToolCfg(flags, name="G4UA::FixG4CreatorProcessTool", **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.G4UA.FixG4CreatorProcessTool(name, **kwargs))
    return result


def HitWrapperToolCfg(flags, name="G4UA::HitWrapperTool", **kwargs):
    result = ComponentAccumulator()
    # FIXME UserActionConfig not yet migrated
    # example custom configuration
    # from G4AtlasApps.SimFlags import simFlags
    # if name in simFlags.UserActionConfig.get_Value().keys():
    #     for prop,value in simFlags.UserActionConfig.get_Value()[name].iteritems():
    #         kwargs.setdefault(prop,value)
    result.setPrivateTools(CompFactory.G4UA.HitWrapperTool(name, **kwargs))
    return result


def LengthIntegratorToolCfg(flags, name="G4UA::UserActionSvc.LengthIntegratorTool", **kwargs):
    THistSvc= CompFactory.THistSvc
    result = ComponentAccumulator()
    histsvc = THistSvc(name="THistSvc")
    histsvc.Output = ["lengths DATAFILE='LengthIntegrator.root' OPT='RECREATE'"]
    result.addService(histsvc)
    kwargs.setdefault("HistoSvc", "THistSvc")
    result.setPrivateTools(CompFactory.G4UA.LengthIntegratorTool(name, **kwargs))
    return result

def RadiationMapsMakerToolCfg(flags, name="G4UA::UserActionSvc.RadiationMapsMakerTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ActivationFileName","Activations.txt")
    kwargs.setdefault("NBinsDPhi",1) 
    kwargs.setdefault("NBinsTheta",1) 
    kwargs.setdefault("NBinsR3D",120) 
    kwargs.setdefault("NBinsZ3D",240) 
    kwargs.setdefault("NBinsPhi3D",28) 
    kwargs.setdefault("NBinsLogTimeCut",26) 
    kwargs.setdefault("PhiMinZoom",0.0) 
    kwargs.setdefault("PhiMaxZoom",22.5) 
    kwargs.setdefault("LogTMin",-3.0) 
    kwargs.setdefault("LogTMax",10.0) 
    kwargs.setdefault("ElemZMin",1) 
    kwargs.setdefault("ElemZMax",1) 
    result.setPrivateTools(CompFactory.G4UA.RadiationMapsMakerTool(name, **kwargs))
    return result


def HIPKillerToolCfg(flags, name="G4UA::HIPKillerTool", **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools(CompFactory.G4UA.HIPKillerTool(name, **kwargs))
    return result


def MonopoleLooperKillerToolCfg(flags, name="G4UA::MonopoleLooperKillerTool", **kwargs):
    kwargs.setdefault("MaxSteps",2000000)
    kwargs.setdefault("PrintSteps",2)
    kwargs.setdefault("VerboseLevel",0)
    return LooperKillerToolCfg(flags, name, **kwargs)


def FastIDKillerToolCfg(flags, name="G4UA::FastIDKillerTool", **kwargs):
    """
    isDalek = True : kill all particles with kinetic energy below
    'energyCut' leaving the ID envelope (R,Z).

    isDalek = False : kill all particles leaving the ID envelope
    (R,Z), except e+-/gamma with total energy >= 'energyCut'
    """
    result = ComponentAccumulator()
    # FIXME UserActionConfig not yet migrated
    # example custom configuration
    # if name in flags.Sim.UserActionConfig.keys():
    #     for prop,value in flags.Sim.UserActionConfig[name].iteritems():
    #         kwargs.setdefault(prop,value)
    from AthenaCommon.SystemOfUnits import MeV, mm
    kwargs.setdefault("energyCut", 100.*MeV) # Energy cut in MeV
    kwargs.setdefault("isDalek", False)
    kwargs.setdefault("R", 1150.*mm) # ID outer radius
    kwargs.setdefault("Z", 3490.*mm) # ID maximum Z coordiate
    result.setPrivateTools(CompFactory.G4UA.FastIDKillerTool(name, **kwargs))
    return result
