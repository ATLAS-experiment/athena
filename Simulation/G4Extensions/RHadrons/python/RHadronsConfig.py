# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
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


def create_rhadron_pdgtable(input_param_card='SLHA_INPUT.DAT',spectrum=1):
    """Add lines to the PDG table"""

    from ExtraParticles.PDGHelpers import getPDGTABLE
    if getPDGTABLE('PDGTABLE.MeV'): # FIXME make configurable
        # Update the PDG table using our helper function
        from RHadrons.RHadronMasses import update_PDG_table
        update_PDG_table(input_param_card,'PDGTABLE.MeV',spectrum)


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

    # Standard list of commands stolen from the Pythia8 base fragment
    """pythia_commands += [
        "6:m0 = 172.5",
        "23:m0 = 91.1876",
        "23:mWidth = 2.4952",
        "24:m0 = 80.399",
        "24:mWidth = 2.085",
        "StandardModel:sin2thetaW = 0.23113",
        "StandardModel:sin2thetaWbar = 0.23146",
        "ParticleDecays:limitTau0 = on",
        "ParticleDecays:tau0Max = 10.0"]

    pythia_commands += [
        "Tune:ee = 7",
        "Tune:pp = 14",
        "SpaceShower:rapidityOrder = on",
        "SigmaProcess:alphaSvalue = 0.140",
        "SpaceShower:pT0Ref = 1.56",
        "SpaceShower:pTmaxFudge = 0.91",
        "SpaceShower:pTdampFudge = 1.05",
        "SpaceShower:alphaSvalue = 0.127",
        "TimeShower:alphaSvalue = 0.127",
        "BeamRemnants:primordialKThard = 1.88",
        "MultipartonInteractions:pT0Ref = 2.09",
        "MultipartonInteractions:alphaSvalue = 0.126",
        "PDF:pSet=LHAPDF6:NNPDF23_lo_as_0130_qed",
        "ColourReconnection:range = 1.71"
    ]"""
    ## This block makes sure that the MPI rapidity order is set
    ## consistently for Pythia versions 8.219 and later.  Since it
    ## depends on the tune settings it must be included *after* the
    ## main tune fragment in the JO.
    """addRapidityOrderMPI = True
    rapidityOrderMPICommand = []
    for cmd in pythia_commands:
        if "SpaceShower:rapidityOrderMPI = " in cmd:
            addRapidityOrderMPI = False
        if "SpaceShower:rapidityOrder" in cmd and "SpaceShower:rapidityOrderMPI" not in cmd and addRapidityOrderMPI:
            val = cmd.split("=")[-1]
            rapidityOrderMPICommand = ["SpaceShower:rapidityOrderMPI = " + val]
    if addRapidityOrderMPI and len(rapidityOrderMPICommand) != 0:
        pythia_commands += rapidityOrderMPICommand"""
      

    def unescape_newlines(text: str) -> str:
        return text.replace("\\n", "\n")

    process=restore_equals(process)
    process = unescape_newlines(process)

    SLHA_INPUT_content=flags.Input.SpecialConfiguration["SLHA_INPUT"]
    SLHA_INPUT_content=unescape_newlines(SLHA_INPUT_content)
    SLHA_INPUT_content=restore_equals(SLHA_INPUT_content)
    with open("SLHA_INPUT.DAT",'w') as f:
        f.write(SLHA_INPUT_content)

    import ast
    run_card = ast.literal_eval(run_card)
    param_card = ast.literal_eval(param_card)

    spectrum = 1 if 'SPECTRUM' not in simdict else simdict['SPECTRUM']

    load_files_for_rhadrons_scenario("SLHA_INPUT.DAT",spectrum)

    lifetime = float(simdict['LIFETIME']) if "LIFETIME" in simdict else -1.
    if lifetime>0.:
       addLineToPhysicsConfiguration("DoDecays","1")
       addLineToPhysicsConfiguration("HadronLifeTime", str(lifetime))
    else:
       # Stable case. Can be unset lifetime or lifetime=0 or lifetime=-1
       addLineToPhysicsConfiguration("DoDecays","0")
       addLineToPhysicsConfiguration("HadronLifeTime", -1)

    # Set up R-hadron masses in Pythia8
    from RHadrons.RHadronMasses import get_Pythia8_commands

    pythia_commands += get_Pythia8_commands("SLHA_INPUT.DAT",spectrum)
    pythia_commands += ["Init:showChangedSettings = on"]
    pythia_commands += ["SLHA:verbose = 3"]
    pythia_commands += ["Next:numberShowEvent = 999999"]
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