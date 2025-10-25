# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Pythonized version of MadGraph steering executables
#    written by Zach Marshall <zach.marshall@cern.ch>
#    updates for aMC@NLO by Josh McFayden <mcfayden@cern.ch>
#    updates to LHE handling and SUSY functionality by Emma Kuwertz <ekuwertz@cern.ch>
#  Attempts to remove path-dependence of MadGraph
#  Class-based version of MadGraph Control
#    written by Kael Kemp <kael.kemp@cern.ch>

import os,time,subprocess,glob,re,sys # noqa: F401 
from AthenaCommon import Logging
from MadGraphControl.MadGraphUtilsHelpers import getDictFromCard,is_version_or_newer,error_check,setup_path_protection,is_NLO_run
from MadGraphControl.MadGraphParamHelpers import do_PMG_updates
mglog = Logging.logging.getLogger('MadGraphUtils')

# Name of python executable
python='python'
# Magic name of gridpack directory
MADGRAPH_GRIDPACK_LOCATION='madevent'
# Name for the run (since we only have 1, just needs consistency)
MADGRAPH_RUN_NAME='run_01'
# For error handling
MADGRAPH_CATCH_ERRORS=True
# PDF setting (global setting)
MADGRAPH_PDFSETTING=None

## Options:
# 'madevent_simd' for SIMD (vector) instructions
# 'madevent_gpu' for GPU-based execution
# 'max' to try to auto-detect the best we can do
MADGRAPH_DEVICES=None

class MGControl:
    def __init__(self, process='generate p p > t t~\noutput -f', plugin=None, keepJpegs=False, usePMGSettings=False):
        """ Generate a new process in madgraph.
        Pass a process string.
        Optionally request JPEGs to be kept and request for PMG settings to be used in the param card
        Return the name of the process directory.
        """
        self.mglog = Logging.logging.getLogger('MadGraphUtils')        
        self.process = process
        self.plugin = plugin
        self.keepJpegs = keepJpegs
        self.usePMGSettings = usePMGSettings
        self.run_card_params = []
        #is_gen_from gridpack
        self.is_gen_from_gridpack = os.access(MADGRAPH_GRIDPACK_LOCATION,os.R_OK)
        # Don't run if generating events from gridpack
        if self.is_gen_from_gridpack:
            self.process_dir = MADGRAPH_GRIDPACK_LOCATION      
            return
        # Actually just sent the process card contents - let's make a card
        card_loc='proc_card_mg5.dat'
        mglog.info('Writing process card to '+card_loc)
        a_card = open( card_loc , 'w' )
        for l in process.split('\n'):
            if 'output' not in l:
                a_card.write(l+'\n')
            else:
                # Special handling for output line
                outline = l.strip()
                if '-nojpeg' not in l and not keepJpegs:
                    # We need to add -nojpeg somehow
                    if '#' in l:
                        outline = outline.split('#')[0]+' -nojpeg #'+outline.split('#')[1]
                    else:
                        outline = outline + ' -nojpeg'
                # Special handling for devises
                if MADGRAPH_DEVICES is not None:
                    if MADGRAPH_DEVICES.lower() in ['madevent_simd','madevent_gpu']:
                        outline = 'output '+MADGRAPH_DEVICES.lower()+' '+outline.split('output')[1]
                    elif MADGRAPH_DEVICES.lower() == 'max':
                        self.mglog.warning('Not fully implemented yet; setting avx')
                        outline = 'output madevent_simd '+outline.split('output')[1]
                a_card.write(outline+'\n')
        a_card.close()

        madpath=os.environ['MADPATH']
        # Just in case
        setup_path_protection()

        # Check if we have a special output directory
        process_dir = ''
        for l in process.split('\n'):
            # Look for an output line
            if 'output' not in l.split('#')[0].split():
                continue
            # Check how many things before the options start
            tmplist = l.split('#')[0].split(' -')[0]
            # if two things, second is the directory
            if len(tmplist.split())==2:
                process_dir = tmplist.split()[1]
            # if three things, third is the directory (second is the format)
            elif len(tmplist.split())==3:
                process_dir = tmplist.split()[2]
            # See if we got a directory
            if ''!=process_dir:
                mglog.info('Saw that you asked for a special output directory: '+str(process_dir))
            break

        mglog.info('Started process generation at '+str(time.asctime()))

        plugin_cmd = '--mode='+plugin if plugin is not None else ''

        # Note special handling here to explicitly print the process
        self.MADGRAPH_COMMAND_STACK = []           #want to change to variable
        self.MADGRAPH_COMMAND_STACK += ['# All jobs should start in a clean directory']
        self.MADGRAPH_COMMAND_STACK += ['mkdir standalone_test; cd standalone_test']
        self.MADGRAPH_COMMAND_STACK += [' '.join([python,madpath+'/bin/mg5_aMC '+plugin_cmd+' << EOF\n'+process+'\nEOF\n'])]
        global MADGRAPH_CATCH_ERRORS
        generate = subprocess.Popen([python,madpath+'/bin/mg5_aMC',plugin_cmd,card_loc],stdin=subprocess.PIPE,stderr=subprocess.PIPE if MADGRAPH_CATCH_ERRORS else None)
        (out,err) = generate.communicate()
        error_check(err,generate.returncode)

        mglog.info('Finished process generation at '+str(time.asctime()))

        # at this point process_dir is for sure defined - it's equal to '' in the worst case
        if process_dir == '': # no user-defined value, need to find the directory created by MadGraph5
            for adir in sorted(glob.glob( os.getcwd()+'/*PROC*' ),reverse=True):
                if os.access('%s/SubProcesses/subproc.mg'%adir,os.R_OK):
                    if process_dir=='':
                        process_dir=adir
                    else:
                        mglog.warning('Additional possible process directory, '+adir+' found. Had '+process_dir)
                        mglog.warning('Likely this is because you did not run from a clean directory, and this may cause errors later.')
        else: # user-defined directory
            if not os.access('%s/SubProcesses/subproc.mg'%process_dir,os.R_OK):
                raise RuntimeError('No diagrams for this process in user-define dir='+str(process_dir))
        if process_dir=='':
            raise RuntimeError('No diagrams for this process from list: '+str(sorted(glob.glob(os.getcwd()+'/*PROC*'),reverse=True)))

        # Special catch related to path setting and using afs
        needed_options = ['ninja','collier','fastjet','lhapdf','syscalc_path']
        in_config = open(os.environ['MADPATH']+'/input/mg5_configuration.txt','r')
        option_paths = {}
        for l in in_config.readlines():
            for o in needed_options:
                if o+' =' in l.split('#')[0] and 'MCGenerators' in l.split('#')[0]:
                    old_path = l.split('#')[0].split('=')[1].strip().split('MCGenerators')[1]
                    old_path = old_path[ old_path.find('/') : ]
                    if o =='lhapdf' and 'LHAPATH' in os.environ:
                        # Patch for LHAPDF version
                        version = os.environ['LHAPATH'].split('lhapdf/')[1].split('/')[0]
                        old_version = old_path.split('lhapdf/')[1].split('/')[0]
                        old_path = old_path.replace(old_version,version)
                    if o=='ninja':
                        # Patch for stupid naming problem
                        old_path.replace('gosam_contrib','gosam-contrib')
                    option_paths[o] = os.environ['MADPATH'].split('madgraph5amc')[0]+old_path
                # Check to see if the option has been commented out
                if o+' =' in l and o+' =' not in l.split('#')[0]:
                    mglog.info('Option '+o+' appears commented out in the config file')

        in_config.close()
        for o in needed_options:
            if o not in option_paths:
                mglog.info('Path for option '+o+' not found in original config')

        mglog.info('Modifying config paths to avoid use of afs:')
        mglog.info(option_paths)

        # Load up the run card dictionary
        self.runCardDict = getDictFromCard(process_dir+'/Cards/run_card.dat')

        # Set the paths appropriately
        self.change_config_card(process_dir=process_dir,settings=option_paths,set_commented=False)
        # Done modifying paths

        # If requested, apply PMG default settings
        if usePMGSettings:
            do_PMG_updates(process_dir)
            
        # After 2.9.3, enforce the standard default sde_strategy, so that this won't randomly change on the user
        if is_version_or_newer([2,9,3]) and not is_NLO_run(process_dir=process_dir):
            mglog.info('Setting default sde_strategy to old default (1)')
            self.runCardDict['sde_strategy']=1

        #tell MadGraph not to bother trying to create popup windows since this is running in a CLI, this will save ~50 seconds every time MadGraph is called.    
        self.change_config_card(process_dir=process_dir,settings={'notification_center':'False'})

        # Add some custom settings based on the device requests
        if MADGRAPH_DEVICES is not None:
            if MADGRAPH_DEVICES.lower()=='madevent_simd':
                self.runCardDict['cudacpp_backend'] = 'cppauto'
            elif MADGRAPH_DEVICES.lower()=='madevent_gpu':
                self.runCardDict['cudacpp_backend'] = 'cuda'
                # In case we have "too new" a gcc version for the nvcc version on the node, which should be ok
                # This patch should be temporary, but is fine while we are validating things at least
                os.environ['ALLOW_UNSUPPORTED_COMPILER_IN_CUDA'] = 'Y'
            elif MADGRAPH_DEVICES.lower() == 'max':
                self.mglog.warning('Not fully implemented yet; setting avx')
                self.runCardDict['cudacpp_backend'] = 'cppauto'

        # Make sure we store the resultant directory
        self.MADGRAPH_COMMAND_STACK += ['export MGaMC_PROCESS_DIR='+os.path.basename(process_dir)]
        self.process_dir = process_dir

    def change_run_card(self, run_card_input=None,run_card_backup=None,process_dir=MADGRAPH_GRIDPACK_LOCATION,runArgs=None,settings={},skipBaseFragment=False ):
        self.mglog.warning('Do not call this function, just update self.run_card_params')
        self.run_card_params += [run_card_input,run_card_backup,process_dir,runArgs,settings,skipBaseFragment ]

    def change_config_card(self, config_card_backup=None,process_dir=MADGRAPH_GRIDPACK_LOCATION,settings={},set_commented=True ):
        self.mglog.warning('Do not call this function, just update self.run_card_params')
        self.run_card_params += [ config_card_backup,process_dir,settings,set_commented ]
