# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from MadGraphControl.MadGraphUtils import * # noqa: F401 F403
from MadGraphControl.MadGraph_NNPDF30NLO_Base_Fragment import * # noqa: F401 F403
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaCommon import Logging
rhlog = Logging.logging.getLogger('RHadronConfig')

def determineLegacyGeneratorJobOptions(mcChannelNumber):
    # Load up the configuration using the MC Channel number (DSID).
    # Not the most beautiful thing, but this works.
    from glob import glob
    # Default position: look in cvmfs for job options
    if(mcChannelNumber ==  449497 ):
        mcChannelNumber =  421442
        rhlog.info('MC channel number changed from 449497 to %d', mcChannelNumber)

    cvmfs_mc16 = '/cvmfs/atlas.cern.ch/repo/sw/Generators/MCJobOptions/'

    JO = glob(cvmfs_mc16+str(int(mcChannelNumber/1000))+'xxx/'+str(mcChannelNumber)+'/mc.'+'*.py')

    JO_path = cvmfs_mc16+str(int(mcChannelNumber/1000))+'xxx/'+str(mcChannelNumber)
    import os
    os.environ["JOBOPTSEARCHPATH"] = JO_path +":"+os.environ["JOBOPTSEARCHPATH"]
    os.environ["DATAPATH"] = JO_path +":"+os.environ["DATAPATH"]
    if len(JO)>0:
        JO = JO[0]
    else:
        # Miss.  Try local in dir=DSID
        JO = glob(str(mcChannelNumber)+'/mc.'+'*.py')
        if len(JO)>0:
            JO=JO[0]
        else:
            # Miss.  Try one directory deeper (any name)
            JO = glob('*/mc.'+str(mcChannelNumber)+'*.py')
            if len(JO)>0: JO=JO[0]
            else:
                # Miss.  Try local
                JO = glob('mc.'+'*.py')
                if len(JO)>0:
                    JO=JO[0]
                else:
                    # Miss.  Fall back to datapath
                    for adir in os.environ['DATAPATH'].split(":"):
                        JO = glob(adir+'/mc.'+'*.py')
                        if len(JO)>0:
                            JO=JO[0]
                            break
    if not JO:
        raise RuntimeError('Could not locate job options for DSID '+str(mcChannelNumber))
    return JO


def addProcessCardsToDATAPATH():
    import os
    cwd_path = os.getcwd()
    from glob import glob
    cwd_sub = glob(cwd_path + "/PROC*/Cards")
    cwd_sub_str = ' '.join(str(e)+":" for e in cwd_sub)
    os.environ["DATAPATH"] = cwd_sub_str +":"+os.environ["DATAPATH"]


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


def addLineToPhysicsConfiguration(KEY, VALUE):
    """Add lines to the physics configuration"""
    import os
    os.system('touch PhysicsConfiguration.txt')
    newphysconfig = "{key} = {value}".format(key=KEY, value=VALUE)
    os.system('echo "%s" >> PhysicsConfiguration.txt' % newphysconfig)


@AccumulatorCache
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


def buildRunArgs(flags):
    from PyJobTransforms.trfJobOptions import RunArguments
    runArgs = RunArguments()
    # add any necessary elements to the runArgs here!
    JO = determineLegacyGeneratorJobOptions(flags.Input.MCChannelNumber) # Everything flows from the MCChannelNumber!
    runArgs.jobConfig = [JO.split('/')[-2] if '/' in JO else JO]
    runArgs.runNumber = flags.Input.MCChannelNumber
    runArgs.ecmEnergy = 13000.
    runArgs.randomSeed = 1234
    runArgs.generatorJobNumber = 0 # Workaround for ATLASSIM-6687 - FIXME is this needed anywhere other than one print statment?
    return runArgs


def configureAndRunMadGraph(flags):
    # Based on Generators/MadGraphControl/share/common/SUSY_SimplifiedModel_PreInclude.py
    runArgs = buildRunArgs(flags)
    #  Gets us ready for on-the-fly SUSY SM generation

    # Simple variable setups
    MadGraph_param_blocks = {} # For general params
    decoupled_mass = '4.5E9'
    masses = {}
    for p in ['1000001','1000002','1000003','1000004','1000005','1000006','2000001','2000002','2000003','2000004','2000005','2000006','1000021',\
              '1000023','1000024','1000025','1000011','1000013','1000015','2000011','2000013','2000015','1000012','1000014','1000016','1000022',\
              '1000035','1000037','35','36','37']: # Note that gravitino is non-standard
        masses[p]=decoupled_mass
    decays = {}

    # Useful definitions
    squarks = []
    squarksl = []
    for anum in [1,2,3,4]:
        squarks += [str(1000000+anum),str(-1000000-anum),str(2000000+anum),str(-2000000-anum)]
        squarksl += [str(1000000+anum),str(-1000000-anum)]
    dict_index_syst = {0:'scalefactup',
                       1:'scalefactdown',
                       2:'alpsfactup',
                       3:'alpsfactdown',
                       4:'moreFSR',
                       5:'lessFSR',
                       6:'qup',
                       7:'qdown'}

    # Basic settings for production and filters
    MadGraph_syst_mod = None
    ktdurham = None # Only set if you want a non-standard setting (1/4 heavy mass)
    madspin_card = None
    MadGraph_param_card = None # Only set if you *can't* just modify the default param card to get your settings (e.g. pMSSM)

    # Default run settings
    MadGraph_run_settings = {'event_norm':'average',
                    'drjj':0.0,
                    'lhe_version':'3.0',
                    'cut_decays':'F',
                    'ickkw':0,
                    'xqcut':0} # use CKKW-L merging (yes, this is a weird setting)

    # Setting for writing out a gridpack
    MadGraph_writeGridpack = False

    # In case someone needs to be able to keep the output directory
    # for testing.  Turn on for debugging param cards. Should only
    # ever be true for testing!
    keepMadGraphOutput = False

    # fixing LHE files after madspin?  do that here.
    fixEventWeightsForBridgeMode=False

    # In case you want to keep lifetimes in the LHE files
    MadGraph_add_lifetimes_lhe = True

    # Do we want to use PDG defaults?
    MadGraph_usePMGSettings = True

    # Do we need to use a custom plugin?
    customMadGraphPlugin = None

    # Do we want 4FS or 5FS? 5 is now default
    # * 5-flavor scheme always should use nQuarksMerge=5 [5FS -> nQuarksMerge=5]
    # * 4-flavor scheme with light-flavor MEs (p p > go go j , with j = g d u s c)
    #       should use nQuarksMerge=4 [4FS -> nQuarksMerge=4]
    # * 4-flavor scheme with HF MEs (p p > go go j, with j = g d u s c b) should
    #       use nQuarksMerge=5 [4FS + final state b -> nQuarksMerge=5]
    flavourScheme = 5 # FIXME NB This is set to 4 again later, before it is used!!!
    define_pj_5FS = True # Defines p and j to include b in MadGraph_process string with 5FS
    force_nobmass_5FS = True # Forces massless b with 5FS
    finalStateB = False # Used with 4FS

    """
    This JO is long-lived stop RHadrons decaying to b+mu
    Migrated from r19 JO: https://gitlab.cern.ch/atlas-physics/pmg/infrastructure/mc15joboptions/-/blob/master/common/MadGraph/MadGraphControl_SimplifiedModel_TT_RPVdirectBL_LongLived_RHadron.py
    JIRA: https://its.cern.ch/jira/browse/ATLMCPROD-5979
    """

    from MCJobOptionUtils.JOsupport import get_physics_short
    phys_short = get_physics_short() # FIXME There must be a more robust way of doing this!?
    infoStrings = phys_short.split("_")
    rhlog.info( "  jobConfig: %s  ", phys_short[0] )
    rhlog.info( "  stop mass: %s  ", infoStrings[4] )
    rhlog.info( "  stop ctau: %s  ", infoStrings[6].replace('p','.') )

    ## Setting masses from filename parsing
    masses['1000006'] = float(infoStrings[4])
    masses['1000005'] = 3.00000000E+05
    masses['1000022'] = 100000.

    ## Converting lifetime from filename to width
    lifetimeString = str(infoStrings[6])
    stopLifetime = lifetimeString.replace("ns","").replace(".py","").replace("p",".")
    hbar = 6.582119514e-16
    stopWidth = hbar/float(stopLifetime)
    rhlog.info( "  stop lifetime, width: %f, %f  ", float(stopLifetime), stopWidth )

    ## Set flavour scheme to 4  (default setup in mc15 JO)
    flavourScheme = 4

    ## Optional custom gluinoball probability in the filename

    if len(infoStrings)>7:
        gluinoBallProbabilityString = str(infoStrings[7])
    else:
        gluinoBallProbabilityString = "gball10"

    gluinoBallProbability = float(gluinoBallProbabilityString.replace("gball",""))/100.
    rhlog.info( "  gluino-ball probability: %f  ", gluinoBallProbability )

    ## Defining the narrow width stop and its decay modes to b+mu
    decays['1000006'] = """DECAY   1000006     %s   # stop1 decays
    #          BR         NDA          ID1       ID2       ID3       ID4
         1.00000000000    2      -13        5         # stop1 to b mu
    """%(stopWidth)


    # Specify the MadGraph_process here, use MSSM_SLHA2 since its used in MC15 JO
    MadGraph_process = '''
    import model MSSM_SLHA2-full
    define susylq = ul ur dl dr cl cr sl sr
    define susylq~ = ul~ ur~ dl~ dr~ cl~ cr~ sl~ sr~
    generate p p > t1 t1~ $ go susylq susylq~ b2 t1 t2 b2~ t1~ t2~ @1
    add process p p > t1 t1~ j $ go susylq susylq~ b2 t1 t2 b2~ t1~ t2~ @2
    add process p p > t1 t1~ j j $ go susylq susylq~ b2 t1 t2 b2~ t1~ t2~ @3
    output -f
    '''

    # Register generation
    rhlog.info('Registered generation of stop pair production to b+mu; grid point '+str(runArgs.generatorJobNumber))

    MadGraph_run_settings.update({'time_of_flight':'1E-2',  'event_norm':'sum'}) ## In MC15 JO 'use_syst': F but code complains here about trusting base fragment (?)

    # This comes after all Simplified Model setup files
    if 'rpv' in phys_short.lower() and 'import ' not in MadGraph_process:
        raise RuntimeError('Please import a model when using an RPV decay; these are not handled by the standard MSSM model in MadGraph')

    # Set maximum number of events if the event multiplier has been modified
    # Sensible default
    nevts = flags.Exec.MaxEvents * 2. # Workaround for ATLASSIM-6687

    MadGraph_run_settings.update({'nevents':int(nevts)})

    # Only needed for something non-standard (not 1/4 heavy mass)
    if ktdurham is not None:
        MadGraph_run_settings.update({'ktdurham':ktdurham})

    if flavourScheme not in [4,5]:
        raise RuntimeError('flavourScheme must be 4 or 5.')

    if flavourScheme == 4:
        MadGraph_run_settings.update({
            'pdgs_for_merging_cut': '1, 2, 3, 4, 21' # Terrible default in MG
        })
        _nQuarksMerge = 5 if finalStateB else 4
    else:
        MadGraph_run_settings.update({
            'pdgs_for_merging_cut': '1, 2, 3, 4, 5, 21',
            'asrwgtflavor': 5,
            'maxjetflavor': 5
        })
        _nQuarksMerge = 5
        if define_pj_5FS:
            # Add the 5FS p and j definition to the beginning of the MadGraph_process string
            MadGraph_process = "define p = g u c d s b u~ c~ d~ s~ b~\ndefine j = g u c d s b u~ c~ d~ s~ b~\n" + MadGraph_process
            # Check that if p and j get redefined that it is consistent with 5FS otherwise raise error
            for l in MadGraph_process.split('\n'):
                l_nocomment = l.split('#')[0]
                if ("define p" in l_nocomment or "define j" in l_nocomment) and l_nocomment.count("=") == 1:
                    l_equals = (l_nocomment.split("=")[-1]).split(" ")
                    if not set(['g', 'u', 'c', 'd', 's', 'b', 'u~', 'c~', 'd~', 's~', 'b~']) <= set(l_equals):
                        raise RuntimeError('Invalid definition found for p or j in MadGraph_process string while using 5FS')
        if force_nobmass_5FS:
            if masses.get('5',0.0) != 0.0:
                raise RuntimeError('Non-zero mass found for b while using 5FS')
            masses['5'] = 0.0

    # systematic variation
    if 'scup' in phys_short:
        MadGraph_syst_mod=dict_index_syst[0]
    elif 'scdw' in phys_short:
        MadGraph_syst_mod=dict_index_syst[1]
    elif 'alup' in phys_short:
        MadGraph_syst_mod=dict_index_syst[2]
    elif 'aldw' in phys_short:
        MadGraph_syst_mod=dict_index_syst[3]
    elif 'qcup' in phys_short:
        MadGraph_syst_mod=dict_index_syst[6]
    elif 'qcdw' in phys_short:
        MadGraph_syst_mod=dict_index_syst[7]

    # Pass arguments as a dictionary: the "decays" argument is not accepted in older versions of MadGraphControl
    if 'mass' in [x.lower() for x in MadGraph_param_blocks]:
        raise RuntimeError('Do not provide masses in MadGraph_param_blocks; use the masses variable instead')
    MadGraph_param_blocks['MASS'] = masses

    # Add decays in if needed
    if len(decays)>0:
        MadGraph_param_blocks['DECAY'] = decays

    argdict = {'runArgs'        : runArgs,
               'process'        : MadGraph_process,
               'params'         : MadGraph_param_blocks,
               'fixEventWeightsForBridgeMode': fixEventWeightsForBridgeMode,
               'madspin_card'   : madspin_card,
               'keepOutput'     : keepMadGraphOutput,
               'run_settings'   : MadGraph_run_settings, # All goes into the run card
               'writeGridpack'  : MadGraph_writeGridpack,
               'syst_mod'       : MadGraph_syst_mod,
               'param_card'     : MadGraph_param_card, # Only set if you *can't* modify the default param card to get your settings
               'add_lifetimes_lhe' : MadGraph_add_lifetimes_lhe,
               'usePMGSettings' : MadGraph_usePMGSettings,
               'plugin'         : customMadGraphPlugin,
               }

    rhlog.info("Calling SUSY_Generation")
    # Note that for gridpack generation (i.e. MadGraph_writeGridpack=True), the job will exit after this command
    from MadGraphControl.SUSY_Helpers import SUSY_Generation
    ktdurham = SUSY_Generation(**argdict)

    addProcessCardsToDATAPATH()

    # Build the param card, aka SLHA file
    from MadGraphControl.MadGraphUtils import modify_param_card
    modify_param_card(param_card_input='param_card.dat', params={'MASS': masses,'DECAY':decays}, output_location='SLHA_INPUT.DAT')

    return ktdurham, MadGraph_process, _nQuarksMerge, gluinoBallProbability


def generatePythia8Commands(flags):

    # Standard list of commands stolen from the Pythia8 base fragment
    Pythia8CommandList = [
        "6:m0 = 172.5",
        "23:m0 = 91.1876",
        "23:mWidth = 2.4952",
        "24:m0 = 80.399",
        "24:mWidth = 2.085",
        "StandardModel:sin2thetaW = 0.23113",
        "StandardModel:sin2thetaWbar = 0.23146",
        "ParticleDecays:limitTau0 = on",
        "ParticleDecays:tau0Max = 10.0"]

    Pythia8CommandList += [
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
    ]

    ## This block makes sure that the MPI rapidity order is set
    ## consistently for Pythia versions 8.219 and later.  Since it
    ## depends on the tune settings it must be included *after* the
    ## main tune fragment in the JO.
    addRapidityOrderMPI = True
    rapidityOrderMPICommand = []
    for cmd in Pythia8CommandList:
        if "SpaceShower:rapidityOrderMPI = " in cmd:
            addRapidityOrderMPI = False
        if "SpaceShower:rapidityOrder" in cmd and "SpaceShower:rapidityOrderMPI" not in cmd and addRapidityOrderMPI:
            val = cmd.split("=")[-1]
            rapidityOrderMPICommand = ["SpaceShower:rapidityOrderMPI = " + val]
    if addRapidityOrderMPI and len(rapidityOrderMPICommand) != 0:
        Pythia8CommandList += rapidityOrderMPICommand

    # Configure and Run MadGraph, then pass back any information needed to consistently configure Pythia8
    ktdurham, MadGraph_process, _nQuarksMerge, gluinoBallProbability = configureAndRunMadGraph(flags)

    njets = 2
    # Pythia8 setup for matching if necessary
    njets = max([l.count('j') for l in MadGraph_process.split('\n')])
    njets_min = min([l.count('j') for l in MadGraph_process.split('\n') if 'generate ' in l or 'add process' in l])
    if njets > 0 and njets != njets_min:
        Pythia8CommandList += ["Merging:mayRemoveDecayProducts = on",
                               "Merging:nJetMax = "+str(njets),
                               "Merging:doKTMerging = on",
                               "Merging:TMS = "+str(ktdurham),
                               "Merging:ktType = 1",
                               "Merging:Dparameter = 0.4",
                               "Merging:nQuarksMerge = {0:d}".format(_nQuarksMerge)]

    # Pythia8 R-Hadron Module Config
    Pythia8CommandList += ["Init:showChangedSettings = on"]
    Pythia8CommandList += ["Rhadrons:allow = on"]
    Pythia8CommandList += ["RHadrons:allowDecay = off"]
    #Pythia8CommandList += ["RHadrons:allowDecay = on"] ## Setting toggled to validate against MC15 JO
    Pythia8CommandList += ["RHadrons:probGluinoball = %f"%gluinoBallProbability]
    Pythia8CommandList += ["Next:showScaleAndVertex = on"]
    Pythia8CommandList += ["Check:nErrList = 2"]

    # Merging configuration in case of extra jets in ME
    if njets > 0:
        Pythia8CommandList += ["Merging:Process = pp>{t1,1000006}{t1~,-1000006}"]

    simdict = flags.Input.SpecialConfiguration
    # Get the spectrum number if it's in the metadata
    spectrum = 1 if 'SPECTRUM' not in simdict else simdict['SPECTRUM']

    # Last step, load up the files
    load_files_for_rhadrons_scenario('SLHA_INPUT.DAT',spectrum)

    # Add any lines that were missing
    # In case we want to use Pythia8 for decays during simulation
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
    Pythia8CommandList += get_Pythia8_commands('SLHA_INPUT.DAT',spectrum)
    f = open('PYTHIA8_COMMANDS.TXT','w')
    f.write('\n'.join(Pythia8CommandList))
    f.close()
