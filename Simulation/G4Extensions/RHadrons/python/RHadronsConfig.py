# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaCommon import Logging
rhlog = Logging.logging.getLogger('RHadronConfig')

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

def create_rhadron_particles_file(input_param_card='SLHA_INPUT.DAT',spectrum=1):
    """Create a list of particles for custom particle creation"""
    # Just use our helper function
    from RHadrons.RHadronMasses import update_particle_table
    update_particle_table(input_param_card, 'particles.txt', mass_spectrum=spectrum)
    import os
    if not os.path.isfile('particles.txt'):
        raise RuntimeError('Failed to create particles.txt file - will abort')

def create_rhadron_particles_file_fromDict(input_dict={},spectrum=1):
    """Create a list of particles for custom particle creation"""
    # Just use our helper function
    from RHadrons.RHadronMasses import update_particle_table_fromDict
    update_particle_table_fromDict(input_dict, 'particles.txt', mass_spectrum=spectrum)
    import os
    if not os.path.isfile('particles.txt'):
        raise RuntimeError('Failed to create particles.txt file - will abort')

def create_rhadron_particles_file_fromDict(input_param_dict={},spectrum=1):
    """Create a list of particles for custom particle creation"""
    # Just use our helper function
    from RHadrons.RHadronMasses import update_particle_table_fromDict
    update_particle_table_fromDict(input_param_dict, 'particles.txt', mass_spectrum=spectrum)
    import os
    if not os.path.isfile('particles.txt'):
        raise RuntimeError('Failed to create particles.txt file - will abort')


def create_rhadron_pdgtable(input_param_card='SLHA_INPUT.DAT',spectrum=1):
    """Add lines to the PDG table"""

    from ExtraParticles.PDGHelpers import getPDGTABLE
    if getPDGTABLE('PDGTABLE.MeV'): # FIXME make configurable
        # Update the PDG table using our helper function
        from RHadrons.RHadronMasses import update_PDG_table
        update_PDG_table(input_param_card,'PDGTABLE.MeV',spectrum)

def create_rhadron_pdgtable_fromDict(input_param_dict={},spectrum=1):
    """Add lines to the PDG table"""

    from ExtraParticles.PDGHelpers import getPDGTABLE
    if getPDGTABLE('PDGTABLE.MeV'): # FIXME make configurable
        # Update the PDG table using our helper function
        from RHadrons.RHadronMasses import update_PDG_table_fromDict
        update_PDG_table_fromDict(input_param_dict,'PDGTABLE.MeV',spectrum)

def load_files_for_rhadrons_scenario(input_param_card='SLHA_INPUT.DAT',spectrum=1):
    """ Load all the files needed for a given scenario"""
    import os
    if not os.path.isfile(input_param_card):
        raise RuntimeError('input_param_card file is missing - will abort')
    # Create custom PDGTABLE.MeV file
    create_rhadron_pdgtable(input_param_card,spectrum)
    # Create particles.txt file
    create_rhadron_particles_file(input_param_card,spectrum)
    from RHadrons.RHadronMasses import get_interaction_list
    get_interaction_list(input_param_card, interaction_file='ProcessList.txt', mass_spectrum=spectrum)
    # Remove existing physics configuration file ([MDJ]: FIXME: Is this happening earlier, or is it needed?)
    if os.path.isfile('PhysicsConfiguration.txt'):
        rhlog.warning("load_files_for_rhadrons_scenario() Found pre-existing PhysicsConfiguration.txt file - deleting.")
        os.remove('PhysicsConfiguration.txt')

def load_files_for_rhadrons_scenario_fromDict(input_param_dict={},spectrum=1):
    """ Load all the files needed for a given scenario"""
    import os
    # Create custom PDGTABLE.MeV file
    create_rhadron_pdgtable_fromDict(input_param_dict,spectrum)
    # Create particles.txt file
    create_rhadron_particles_file_fromDict(input_param_dict,spectrum)
    from RHadrons.RHadronMasses import get_interaction_list_fromDict
    get_interaction_list_fromDict(input_param_dict, interaction_file='ProcessList.txt', mass_spectrum=spectrum)
    # Remove existing physics configuration file ([MDJ]: FIXME: Is this happening earlier, or is it needed?)
    if os.path.isfile('PhysicsConfiguration.txt'):
        rhlog.warning("load_files_for_rhadrons_scenario() Found pre-existing PhysicsConfiguration.txt file - deleting.")
        os.remove('PhysicsConfiguration.txt')

def addProcessCardsToDATAPATH():
    import os
    cwd_path = os.getcwd()
    from glob import glob
    cwd_sub = glob(cwd_path + "/PROC*/Cards")
    cwd_sub_str = ' '.join(str(e)+":" for e in cwd_sub)
    os.environ["DATAPATH"] = cwd_sub_str +":"+os.environ["DATAPATH"]


def addLineToPhysicsConfiguration(KEY, VALUE):
    """Add lines to the physics configuration"""
    import os
    os.system('touch PhysicsConfiguration.txt')
    newphysconfig = "{key} = {value}".format(key=KEY, value=VALUE)
    os.system('echo "%s" >> PhysicsConfiguration.txt' % newphysconfig)

def RHadronsPreInclude(flags):
    print ("Start of RHadronsPreInclude")
    if 'Py8' not in flags.Input.GeneratorsInfo and 'Pythia8' not in flags.Input.GeneratorsInfo:
        raise RuntimeError('Pythia8 not found in generator metadata - will abort')
    print("AAAAAAAA,  ",flags.Input.SpecialConfiguration)
    simdict = flags.Input.SpecialConfiguration

    pythia_commands_str = flags.Input.SpecialConfiguration["pythia_commands"]
    process = flags.Input.SpecialConfiguration["process"]

    run_card = flags.Input.SpecialConfiguration["run_card"]
    param_card = flags.Input.SpecialConfiguration["param_card"]

    def restore_equals(text: str, replacement: str = "->") -> str:
        """Reverse of replace_equals: turn `replacement` back into '='."""
        return text.replace(replacement, "=")
    pythia_commands_str = restore_equals(pythia_commands_str)
    pythia_commands = pythia_commands_str.split("\\n")
    
    process=restore_equals(process)
    process = process.replace("\\n","\n")
    import ast
    run_card = ast.literal_eval(run_card)
    param_card = ast.literal_eval(param_card)

    print(run_card)
    print(param_card)

    ## Eventually this method should create SLHA_INPUT.DAT,
    ## PhysicsConfiguration.txt and PYTHIA8_COMMANDS.TXT in the run
    ## directory
    from RHadrons.GeneratePythiaCommands_RHadrons import generatePythia8Commands
    #generatePythia8Commands(flags)
    #buildGeneratorConfigurationFiles(flags)
    #from MadGraphControl.MadGraphConfig import MadGraphCfg
    #from MadGraphControl.MadGraphPDFSettings import MadGraphPDFSets
    #sample_config = MadGraphCfg(
    #    flags,
    #    process_definition=process,
    #    run_card_settings=run_card,
    #    param_card_settings=param_card,
    #    pdf_setting=MadGraphPDFSets.NNPDF30NLOnf4,
    #    usePMGSettings=True,
    #    prepare_lhe_for_shower=True,
    #    #lhe_file="events.lhe",
    #    saveProcDir=True,
    #)

    #from Pythia8_i.Pythia8Config import (
    #    Pythia8_A14_NNPDF23LO_EvtGen_Common_Cfg,
    #    Pythia8CommandsCfg,
    #    Pythia8_MadGraph_Cfg,
    #)

    #sample_config.merge(
    #    Pythia8_MadGraph_Cfg(
    #        flags,
    #        ShowerCfg=Pythia8_A14_NNPDF23LO_EvtGen_Common_Cfg,
    #        LHEFile="tmp_LHE_events.events",
    #        computeEfficiency=False,
    #    )
    #)

    #from GeneratorConfig.GeneratorSettingsSemantics import (
    #    GeneratorSettingsPrecedence,
    #)

    #sample_config.merge(
    #    Pythia8CommandsCfg(
    #        flags,
    #        source="long_lived_stop_job_options",
    #        commands=pythia_commands,
    #        precedence=GeneratorSettingsPrecedence.USER,
    #    )
    #)


    #addProcessCardsToDATAPATH()

    #from MadGraphControl.MadGraphUtils import modify_param_card
    #modify_param_card(param_card_input='PROC_MSSM*/Cards/param_card.dat', params=param_card, output_location='./SLHA_INPUT.DAT')
    
    spectrum = 1 if 'SPECTRUM' not in simdict else simdict['SPECTRUM']


    #load_files_for_rhadrons_scenario("PROC_MSSM_SLHA2-full_0/Cards/param_card.dat",spectrum)
    load_files_for_rhadrons_scenario_fromDict(param_card,spectrum)

    lifetime = float(simdict['LIFETIME']) if "LIFETIME" in simdict else -1.
    if lifetime>0.:
       addLineToPhysicsConfiguration("DoDecays","1")
       addLineToPhysicsConfiguration("HadronLifeTime", str(lifetime))
    else:
       # Stable case. Can be unset lifetime or lifetime=0 or lifetime=-1
       addLineToPhysicsConfiguration("DoDecays","0")
       addLineToPhysicsConfiguration("HadronLifeTime", -1)

    # Set up R-hadron masses in Pythia8
    from RHadrons.RHadronMasses import get_Pythia8_commands_fromDict
    pythia_commands += get_Pythia8_commands_fromDict(param_card,spectrum)
    f = open('PYTHIA8_COMMANDS.TXT','w')
    f.write('\n'.join(pythia_commands))
    f.close()

    # Check for the presence of the other files needed at run-time
    import os
    if not os.path.isfile('ProcessList.txt'):
        raise RuntimeError('ProcessList.txt (needed by G4ProcessHelper) is missing - will abort')
    if not os.path.isfile('PhysicsConfiguration.txt'):
        raise RuntimeError('PhysicsConfiguration.txt (needed by G4ProcessHelper) is missing - will abort')
    if not os.path.isfile('PYTHIA8_COMMANDS.TXT'):
        raise RuntimeError('PYTHIA8_COMMANDS.TXT (needed by Pythia8ForDecays) is missing - will abort')
    print ("End of RHadronsPreInclude")

def RHadronsPreInclude_legacy(flags):
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