# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Python helper functions for SUSY event generation
#    written by Zach Marshall <zach.marshall@cern.ch>

# Helper functions we need in MadGraphControl
from MadGraphControl.MadGraphUtils import (generate, generate_from_gridpack, modify_run_card,
                                           add_lifetimes, arrange_output, MADGRAPH_GRIDPACK_LOCATION,
                                           is_gen_from_gridpack, new_process, modify_param_card)

# For moving files around
import shutil

# For checking for files on the disk
import os

# For the type hint on runArgs
from PyJobTransformsCore.runargs import RunArguments

# For logging, we'll build our own logger
from AthenaCommon import Logging
susylog = Logging.logging.getLogger('MadGraphUtilsSUSY')

def helpful_SUSY_definitions() -> str:
    """ Returns a static MadGraph particle definition-style list
    of multi-particle definitions relevant for SUSY event generation
    """

    return """
# Define multiparticle labels
define p = g u c d s u~ c~ d~ s~
define j = g u c d s u~ c~ d~ s~
define pb = g u c d s b u~ c~ d~ s~ b~
define jb = g u c d s b u~ c~ d~ s~ b~
define l+ = e+ mu+
define l- = e- mu-
define vl = ve vm vt
define vl~ = ve~ vm~ vt~
define fu = u c e+ mu+ ta+
define fu~ = u~ c~ e- mu- ta-
define fd = d s ve~ vm~ vt~
define fd~ = d~ s~ ve vm vt
define susystrong = go ul ur dl dr cl cr sl sr t1 t2 b1 b2 ul~ ur~ dl~ dr~ cl~ cr~ sl~ sr~ t1~ t2~ b1~ b2~
define susyweak = el- el+ er- er+ mul- mul+ mur- mur+ ta1- ta1+ ta2- ta2+ n1 n2 n3 n4 x1- x1+ x2- x2+ sve sve~ svm svm~ svt svt~
define susylq = ul ur dl dr cl cr sl sr
define susylq~ = ul~ ur~ dl~ dr~ cl~ cr~ sl~ sr~
define susysq = ul ur dl dr cl cr sl sr t1 t2 b1 b2
define susysq~ = ul~ ur~ dl~ dr~ cl~ cr~ sl~ sr~ t1~ t2~ b1~ b2~
define susysl = el- el+ er- er+ mul- mul+ mur- mur+ ta1- ta1+ ta2- ta2+
define susyv = sve svm svt
define susyv~ = sve~ svm~ svt~
"""


def get_SUSY_variations( process: str, masses: dict[str, str | float], syst_mod: str | None = None, ktdurham: str | None = None ) -> float:
    """
    Provides varied matching scales for SUSY event generation based on inputs
    Params:
        process: the process to be generated (e.g. p p > go go). Used to figure
                 out which particles are relevant for the matching scale
        masses: the dictionary of PDGID to mass mapping, used to figure out the
                scale setting in GeV
        syst_mod: the requested systematic variation (if any); can be msup,
                  msdw, or None
        ktdurham: the matching scale in case the user provides it by hand
    Returns:
        the matching scale
    """
    # Don't override an explicit setting from the run card!
    if ktdurham is None:
        prod_particles = []
        if process is not None:
            id_map = {'go':'1000021','dl':'1000001','ul':'1000002','sl':'1000003','cl':'1000004','b1':'1000005','t1':'1000006',
                      'dr':'2000001','ur':'2000002','sr':'2000003','cr':'2000004','b2':'2000005','t2':'2000006',
                      'n1':'1000022','n2':'1000023','x1':'1000024','x2':'1000037','n3':'1000025','n4':'1000035',
                      'el':'1000011','mul':'1000013','ta1':'1000015','sve':'1000012','svm':'1000014','svt':'1000016',
                      'er':'2000011','mur':'2000013','ta2':'2000015'}
            for l in process:
                if 'generate' in l or 'add process' in l:
                    clean_proc = l.replace('generate','').replace('+','').replace('-','').replace('~','').replace('add process','').split('>')[1].split(',')[0]
                    for particle in clean_proc.split():
                        if particle not in id_map:
                            susylog.info(f'Particle {particle} not found in PDG ID map - skipping')
                        else:
                            prod_particles += id_map[particle]
        # If we don't specify a process, then all we can do is guess based on available masses
        # Same if we failed to identify the right particles
        my_mass = 10000.
        if len(prod_particles)>0:
            for x in prod_particles:
                if x in masses:
                    my_mass = min(my_mass,abs(float(masses[x])))
                else:
                    susylog.info(f'Seem to ask for production of PDG ID {x}, but {x} not in mass dictionary?')
        if my_mass>9999.:
            strong_ids = ['1000001','1000002','1000003','1000004','1000005','1000006','2000001','2000002','2000003','2000004','2000005','2000006','1000021']
            weak_ids = ['1000023','1000024','1000025','1000011','1000013','1000015','2000011','2000013','2000015','1000012','1000014','1000016']
            # First check the lightest of the heavy sparticles - all squarks and gluino
            my_mass = min([abs(float(masses[x])) for x in strong_ids if x in masses])
            # Now check if strong production was not the key mode
            if my_mass>10000.:
                # This is a little tricky, but: we want the heaviest non-decoupled mass
                my_mass = max([abs(float(masses[x])) for x in weak_ids if x in masses and float(masses[x])<10000.])
            # Final check for N1N1 with everything else decoupled
            if my_mass>10000. and '1000022' in masses:
                my_mass = masses['1000022']
            if my_mass>10000.:
                raise RuntimeError('Could not understand which mass to use for matching cut in '+str(masses))

        # Now set the matching scale accordingly
        ktdurham = min(my_mass*0.25,500)
        # Should not be weirdly low - can't imagine a situation where you'd really want the scale below 15 GeV
        ktdurham = max(ktdurham,15)
        if syst_mod == 'msup':
            susylog.info('Applying upward variation (by 2x) of matching scale')
            ktdurham = ktdurham*2.
        elif syst_mod == 'msdw':
            susylog.info('Applying downward variation (by 2x) of matching scale')
            ktdurham = ktdurham*0.5

    susylog.info('For matching, will use ktdurham of '+str(ktdurham))

    return abs(ktdurham)



def SUSY_process(process: str = '') -> str:
    '''
    Helper function to turn a simplified model process definition into
    a full, MG5_aMC-compliant process definition
    '''

    # Generate the new process!
    if 'import model' in process:
        susylog.info('Assuming that you have specified the model in your process string already')
        full_proc = ''
        for l in process.split('\n'):
            if 'import model' in l:
                full_proc += l+'\n'
                break
        # Only magically add helpful definitions if we are in the right model
        if 'MSSM_SLHA2' in full_proc:
            full_proc+=helpful_SUSY_definitions()
        for l in process.split('\n'):
            if 'import model' not in l:
                full_proc += l+'\n'
        full_proc+="""
# Output processes to MadEvent directory
output -f
"""
    else:
        full_proc = "import model MSSM_SLHA2\n"+helpful_SUSY_definitions()+"""
# Specify process(es) to run

"""+process+"""
# Output processes to MadEvent directory
output -f
"""
    return full_proc


def SUSY_Generation(runArgs: RunArguments | None = None, process: str | None = None, plugin: str | None = None,\
                    syst_mod: str | None = None, keepOutput: bool = False, param_card: str | None = None,\
                    writeGridpack: bool = False, madspin_card: str | None = None, run_settings: dict = {},
                    params: dict = {}, fixEventWeightsForBridgeMode: bool = False,\
                    add_lifetimes_lhe: bool = False, usePMGSettings: bool = True) -> float:

    """
    Helper function for SUSY event generation, used in simplified model setups. Attempts to
    simplify and harmonize a bunch of the things that normally have to be done by hand.
    Keyword Arguments:
        runArgs: passed from the job transform, includes random number seed, ecm energy, and setting for output files
        process: simplified process definition for event generation
        plugin: in case a MG5_aMC plugin is requested for event generation
        syst_mod: requested systematic variation of parameter settings
        keepOutput: retains the process directory created by MG5_aMC. Should always be false in production
        param_card: In case an explicit parameter card is needed (e.g. for pMSSM), can be set this way
        writeGridpack: used to construct gridpacks
        madspin_card: used for MadSpin definitions
        run_settings: any additional run card settings required (e.g. cuts on particle momenta). Dictionary to be
                      passed into modify_run_card(...)
        params: any additional param card settings. Dictionary to be passed into modify_param_card(...)
        fixEventWeightsForBridgeMode: In case Bridge mode is being used, this applies a fix to the event weights
        add_lifetimes_lhe: if fundamental particles written to the LHE have a lifetime (e.g. sleptons or charginos), this
                           allows the inclusion of their time of flight lifetime in the LHE file directly
        usePMGSettings (bool): See :py:func:`new_process`. Will set SM parameters to the appropriate values. Default: True.
    returns:
        the setting of the matching scale to be provided to Pythia8
    """
    ktdurham = run_settings['ktdurham'] if 'ktdurham' in run_settings else None
    ktdurham = get_SUSY_variations( process, params['MASS'] , syst_mod , ktdurham=ktdurham )

    process_dir = MADGRAPH_GRIDPACK_LOCATION
    if not is_gen_from_gridpack():
        full_proc = SUSY_process(process)
        process_dir = new_process(full_proc, plugin=plugin, usePMGSettings=usePMGSettings)
    susylog.info('Using process directory '+str(process_dir))

    # Grab the param card and move the new masses into place
    modify_param_card(param_card_input=param_card,process_dir=process_dir,params=params)

    # Set up the extras dictionary
    settings = {'ktdurham':ktdurham}
    settings.update(run_settings) # This allows explicit settings in the input to override these settings

    # Set up the run card
    modify_run_card(process_dir=process_dir,runArgs=runArgs,settings=settings)

    # Set up madspin if needed
    if madspin_card is not None:
        if not os.access(madspin_card,os.R_OK):
            raise RuntimeError('Could not locate madspin card at '+str(madspin_card))
        shutil.copy(madspin_card,process_dir+'/Cards/madspin_card.dat')
    
    # Generate events!
    if is_gen_from_gridpack():
        generate_from_gridpack(runArgs=runArgs)
    else:
        # Grab the run card and move it into place
        generate(runArgs=runArgs,process_dir=process_dir,grid_pack=writeGridpack)

    # Add lifetimes to LHE before arranging output if requested
    if add_lifetimes_lhe :
        susylog.info('Requested addition of lifetimes to LHE files: doing so now.')
        if is_gen_from_gridpack():
            add_lifetimes()
        else:
            add_lifetimes(process_dir=process_dir)

    # Move output files into the appropriate place, with the appropriate name
    arrange_output(process_dir=process_dir,saveProcDir=keepOutput,runArgs=runArgs,fixEventWeightsForBridgeMode=fixEventWeightsForBridgeMode)

    susylog.info('All done generating events!!')
    return settings['ktdurham']

