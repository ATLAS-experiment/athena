# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def SG_StepNtupleTool(flags, name="G4UA::SG_StepNtupleTool", **kwargs):
    result = ComponentAccumulator()
    if flags.Concurrency.NumThreads >1:
        from AthenaCommon import Logging
        log=Logging.logging.getLogger(name)
        log.fatal(' Attempt to run '+name+' with more than one thread, which is not supported')
        return False
    # Get the PDG IDs for RHadrons
    from RHadronMasses import offset_options
    kwargs.setdefault('RHadronPDGIDList',offset_options.keys())
    ## if name in simFlags.UserActionConfig.get_Value().keys(): ## FIXME missing functionality
    ##     for prop,value in simFlags.UserActionConfig.get_Value()[name].items():
    ##         kwargs.setdefault(prop,value)
    result.setPrivateTools( CompFactory.G4UA__SG_StepNtupleTool(name, **kwargs) )
    return result


def RHadronsPhysicsToolCfg(flags, name='RHadronsPhysicsTool', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools( CompFactory.RHadronsPhysicsTool(name,**kwargs) )
    return result


def RHadronsPreInclude(flags):
    print ("Start of RHadronsPreInclude")
    if 'Py8' not in flags.Input.GeneratorsInfo and 'Pythia8' not in flags.Input.GeneratorsInfo:
        raise RuntimeError('Pythia8 not found in generator metadata - will abort')

    ## Eventually this method should create SLHA_INPUT.DAT,
    ## PhysicsConfiguration.txt and PYTHIA8_COMMANDS.TXT in the run
    ## directory
    from RHadrons.GeneratePythiaCommands_RHadrons import generatePythia8Commands
    generatePythia8Commands(flags)
    #buildGeneratorConfigurationFiles(flags)

    # Check for the presence of the other files needed at run-time
    import os
    if not os.path.isfile('ProcessList.txt'):
        raise RuntimeError('ProcessList.txt (needed by G4ProcessHelper) is missing - will abort')
    if not os.path.isfile('PhysicsConfiguration.txt'):
        raise RuntimeError('PhysicsConfiguration.txt (needed by G4ProcessHelper) is missing - will abort')
    if not os.path.isfile('PYTHIA8_COMMANDS.TXT'):
        raise RuntimeError('PYTHIA8_COMMANDS.TXT (needed by Pythia8ForDecays) is missing - will abort')
    print ("End of RHadronsPreInclude")


def RHadronsCfg(flags):
    result = ComponentAccumulator()
    print("Running RHadronsCfg")
    ## simdict = flags.Input.SpecialConfiguration # TODO will need this!
    from AthenaConfiguration.Enums import ProductionStep
    if flags.Common.ProductionStep == ProductionStep.Simulation:
        from G4AtlasServices.G4AtlasServicesConfig import PhysicsListSvcCfg
        result.merge(PhysicsListSvcCfg(flags))
        physicsOptions = [ result.popToolsAndMerge(RHadronsPhysicsToolCfg(flags)) ]
        result.getService("PhysicsListSvc").PhysOption += physicsOptions
    return result

def RHadrons_VerboseSelectorCfg(flags, name="G4UA::VerboseSelectorTool", **kwargs):
    kwargs.setdefault('TargetEvent',1)
    kwargs.setdefault('VerboseLevel',1)
    kwargs.setdefault('TargetPdgIDs',
                                    [
                        1000612,1000622,1000632,1000642,1000652,1006113,1006211,1006213,1006223,1006311,1006313,1006321,1006323,1006333,-1000612,-1000622,-1000632,-1000642,-1000652,-1006113,-1006211,-1006213,-1006223,-1006311,-1006313,-1006321,-1006323,-1006333
                                    ])
    from G4DebuggingTools.G4DebuggingToolsConfig import VerboseSelectorToolCfg
    return VerboseSelectorToolCfg(flags, name, **kwargs)