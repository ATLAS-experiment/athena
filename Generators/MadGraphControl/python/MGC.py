# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Pythonized version of MadGraph steering executables
#    written by Zach Marshall <zach.marshall@cern.ch>
#    updates for aMC@NLO by Josh McFayden <mcfayden@cern.ch>
#    updates to LHE handling and SUSY functionality by Emma Kuwertz <ekuwertz@cern.ch>
#  Attempts to remove path-dependence of MadGraph
#  Class-based version of MadGraph Control
#    written by Kael Kemp <kael.kemp@cern.ch>

import os,time,subprocess,glob,re,sys # noqa: F401 
from AthenaCommon import Logging
from MadGraphControl.MadGraphUtilsHelpers import error_check,setup_path_protection,get_runArgs_info,modify_param_card,checkSettingExists,checkSetting,checkSettingIsTrue # noqa: F401
from MadGraphControl.MadGraphParamHelpers import do_PMG_updates # noqa: F401
from MadGraphControl.MadGraphSystematicsUtils import convertSysCalcArguments,base_fragment_setup_check,get_pdf_and_systematic_settings,parse_systematics_arguments,SYSTEMATICS_WEIGHT_INFO_ALTDYNSCALES,SYSTEMATICS_WEIGHT_INFO,write_systematics_arguments # noqa: F401

mglog = Logging.logging.getLogger('MadGraphUtils')

# Name of python executable
python = 'python'
# Magic name of gridpack directory
MADGRAPH_GRIDPACK_LOCATION = 'madevent'
# Name for the run (since we only have 1, just needs consistency)
MADGRAPH_RUN_NAME = 'run_01'
# For error handling
MADGRAPH_CATCH_ERRORS = True
# PDF setting (global setting)
MADGRAPH_PDFSETTING = None

## Options:
# 'madevent_simd' for SIMD (vector) instructions
# 'madevent_gpu' for GPU-based execution
# 'max' to try to auto-detect the best we can do
MADGRAPH_DEVICES = None

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
        card_loc = 'proc_card_mg5.dat'
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

        madpath = os.environ['MADPATH']
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
        generate = subprocess.Popen([python,madpath+'/bin/mg5_aMC',plugin_cmd,card_loc],stdin=subprocess.PIPE,stderr=subprocess.PIPE if MADGRAPH_CATCH_ERRORS else None)
        (out,err) = generate.communicate()
        error_check(err,generate.returncode)

        mglog.info('Finished process generation at '+str(time.asctime()))

        # at this point process_dir is for sure defined - it's equal to '' in the worst case
        if process_dir == '': # no user-defined value, need to find the directory created by MadGraph5
            for adir in sorted(glob.glob( os.getcwd()+'/*PROC*' ),reverse=True):
                if os.access('%s/SubProcesses/subproc.mg'%adir,os.R_OK):
                    if process_dir=='':
                        process_dir = adir
                    else:
                        mglog.warning('Additional possible process directory, '+adir+' found. Had '+process_dir)
                        mglog.warning('Likely this is because you did not run from a clean directory, and this may cause errors later.')
        else: # user-defined directory
            if not os.access('%s/SubProcesses/subproc.mg'%process_dir,os.R_OK):
                raise RuntimeError('No diagrams for this process in user-define dir='+str(process_dir))
        if process_dir=='':
            raise RuntimeError('No diagrams for this process from list: '+str(sorted(glob.glob(os.getcwd()+'/*PROC*'),reverse=True)))

        

        self.process_dir = process_dir
        self.get_config_cardloc()
        self.getConfigFromPath(self.config_path)
        
        #load up the run card dictionary
        self.getRunCardDict()
        
        # If requested, apply PMG default settings
        if usePMGSettings:
            do_PMG_updates(self.process_dir)
            
        if not self.isNLO: 
            mglog.info('Setting default sde_strategy to old default (1)')
            self.runCardDict['sde_strategy'] = 1

        #tell MadGraph not to bother trying to create popup windows since this is running in a CLI, this will save ~50 seconds every time MadGraph is called.
        self.configCardDict.update({'notification_center':'False'})

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
        self.MADGRAPH_COMMAND_STACK += ['export MGaMC_PROCESS_DIR='+os.path.basename(self.process_dir)]


    def getRunCardDict(self,lowercase=False):
        ''' Builds a dictionary from the run card.
        This function takes in the card location and saves the contents as a dictionary object in the MGControl class.
        '''
        run_card = self.process_dir + '/Cards/run_card.dat'
        
        if os.access(run_card,os.R_OK):
            mglog.info('Copying default run_card.dat from '+str(run_card))
        else:
            run_card = self.process_dir+'/Cards/run_card_default.dat'
            if os.access(run_card,os.R_OK):
                 mglog.info('Copying default run_card.dat from '+str(run_card))
            else:
                raise RuntimeError('Cannot find default run_card.dat or run_card_default.dat! I was looking here: %s'%run_card)
                       
        card = open(run_card)
        self.runCardDict = {} # Define the dictionary object
        for line in iter(card):
            if not line.strip().startswith('#'): # Ignores line commented out
                command = line.split('!', 1)[0]
                if '=' in command:
                    setting = command.split('=')[-1].strip() #saves the setting
                    value = '='.join(command.split('=')[:-1]).strip() #saves the value associated with the setting
                    if lowercase:
                        value = value.lower()
                        setting = setting.lower()
                    self.runCardDict[setting] = value #adds setting and value to the dictionary
        card.close()


    def get_config_cardloc(self):
        '''Gets the config card location and determines if the process is LO or NLO
        This function takes in the process diectory as an input and uses it to find the configuration.
        Using the path to the config path, we can determine if the process will require a LO or NLO configuration.
        '''
        self.isNLO = None
        #Defining the possible config paths 
        lo_config_card = self.process_dir+'/Cards/me5_configuration.txt'
        nlo_config_card = self.process_dir+'/Cards/amcatnlo_configuration.txt'

        if os.access(lo_config_card,os.R_OK) and not os.access(nlo_config_card,os.R_OK): #Process is LO
            self.config_path = lo_config_card 
            self.isNLO = False
        elif os.access(nlo_config_card,os.R_OK) and not os.access(lo_config_card,os.R_OK): #Process is NLO 
            self.config_path = nlo_config_card 
            self.isNLO = True
        elif os.access(nlo_config_card,os.R_OK) and os.access(lo_config_card,os.R_OK): #Process has two config cards
            mglog.error('Found both types of config card in '+str(self.process_dir))
            raise RuntimeError('Unable to locate configuration card')
        else: # No config Card
            mglog.error('No config card in '+str(self.process_dir))
            raise RuntimeError('Unable to locate configuration card')

        
    def getConfigFromPath(self, card_loc, lowercase=False):
        '''Builds a dictionary from the config card.
        This function creates a dictionary object configCardDict from the config card.
        Using the config card location, we copy over th settings to the dictionary.
        Note: This function is works in the same way as self.getRunCardDict() however with small changes based on how the card is written.
        '''
        card = open(card_loc)
        #define the configCardDict object
        self.configCardDict = {}
        for line in iter(card):
            if not line.strip().startswith('#'): # Ignore lines that are commented out
                command = line.split('!', 1)[0]
                if '=' in command:
                    # Here is where we differ from self.getRunCardDict(), the config card has the setting to the left of the '=' and value to the right
                    value = command.split('=')[-1].strip()
                    setting = '='.join(command.split('=')[:-1]).strip()
                    
                    if lowercase:
                        value = value.lower()
                        setting = setting.lower()
                    self.configCardDict[setting] = value # adds setting to the configCardDict
        card.close()

    def add_runArgs(self, runArgs=None):
        '''This function adds run arguments to the self.runCardDict.
        If the runArgs argument is left blank, the function will get the runArgs information before adding to the dictionary
        '''
        if runArgs is not None:
            beamEnergy,rand_seed = get_runArgs_info(runArgs) # Use get_runArgs_info function to retrieve runArgs

        # Check if the runArgs are already implemented.
        if 'iseed' not in self.runCardDict: #if there is no setting in self.runCardDict for iseed
            self.runCardDict['iseed'] = rand_seed
        if not self.isNLO and 'python_seed' not in self.runCardDict: #If the process is LO and there is no 'python_seed' setting in self.runCardDict
            self.runCardDict['python_seed'] = rand_seed
        if 'beamenergy' in self.runCardDict: #if the beam energy is defined in self.runCardDict
            mglog.warning('Do not set beam energy in MG settings. The variables are ebeam1 and ebeam2. Will use your setting of '+str(self.runCardDict['beamenergy']))
            beamEnergy = self.runCardDict['beamenergy'] #ensure consistency of beam energy
            self.runCardDict.pop('beamenergy') # remove beam energy setting as we only want ebeam1 and ebeam2
        if 'ebeam1' not in self.runCardDict or beamEnergy != self.runCardDict['ebeam1']: # if there is no setting 'ebeam1' in self.runCardDict 
            self.runCardDict['ebeam1'] = beamEnergy
        if 'ebeam2' not in self.runCardDict or beamEnergy != self.runCardDict['ebeam2']: #if there is no setting 'ebeam2' in self.runCardDict
            self.runCardDict['ebeam2'] = beamEnergy


    def write_runCard(self, runArgs=None):
        """Build a new run_card.dat from a run card dictionary.
        This function can get a fresh run card from the runCardDict object.
        Before writing the dictionary to the run card, we require to check a few things first
        """


        # Get info from runArgs
        self.add_runArgs(runArgs)

        # Make sure that nevents is integer
        if 'nevents' in self.runCardDict:
            self.runCardDict['nevents'] = int(self.runCardDict['nevents'])

        # Normalise custom_fcts early so the rewritten run_card uses the full path
        if 'custom_fcts' in self.runCardDict and self.runCardDict['custom_fcts']:
            raw_name = str(self.runCardDict['custom_fcts']).split()[0]
            # Determine jobConfig directory
            if runArgs is not None and hasattr(runArgs, 'jobConfig'):
                cfgdir = runArgs.jobConfig[0] if isinstance(runArgs.jobConfig, (list, tuple)) else runArgs.jobConfig
                # Build full path and make absolute
                full_path = os.path.join(cfgdir, raw_name)
                self.runCardDict['custom_fcts'] = os.path.abspath(full_path)
                print(f"Using custom function(s), specified in custom_fcts with path: {self.runCardDict['custom_fcts']}")
            else:
                # For internal tests, where jobConfig is not set
                self.runCardDict['custom_fcts'] = os.path.abspath(raw_name)
                
        # to avoid writing over the old run card, we rename the old card
        runCard_old = self.process_dir+'/Cards/run_card.dat.old_to_be_deleted'
        os.rename(self.process_dir+'/Cards/run_card.dat', runCard_old)

        #create anew run card in the same location as the old card
        newCard = open(self.process_dir+'/Cards/run_card.dat', 'w')
        
        #write out each line of self.runCardDict to the newCard
        for setting in self.runCardDict:
            newCard.write( ' '+str(self.runCardDict[setting])+'   = '+str(setting)+'\n')
            
        # Check whether mcatnlo_delta is applied to setup pythia8 path
        if 'mcatnlo_delta' in self.runCardDict:	    
            if self.runCardDict['mcatnlo_delta'] == 'True':
                self.configCardDict['pythia8_path'] = os.getenv("PY8PATH")
                    
        # close files
        newCard.close()
        mglog.info('Finished writing to run card.')
        os.unlink(runCard_old) # delete old backup
            
    def write_configCard(self):
        """Build a new configuration from a config card dictionary.
        This function can get a fresh runcard from the configCardDict object.
        This function behaves similaraly to self.write_runCard()
        """
        mglog.info('Writing config card in '+self.process_dir)

        #change name of old config card to avoid writing over
        config_pathOLD = self.config_path+'.old_to_be_deleted'
        os.rename(self.config_path, config_pathOLD) # change name of original card

        # create new config card
        newCard = open(self.config_path, 'w')
        for setting in self.configCardDict:
            if self.configCardDict[setting] is None: # ignore empty settings
                continue
            mglog.info('Writing option '+setting+' to the config card.  Adding a setting to '+str(self.configCardDict[setting]))
            newCard.write(' '+str(setting)+' = '+str(self.configCardDict[setting])+'\n') #writing config card in the format setting = value

        # close file
        newCard.close()

        mglog.info('Finished writing to config card.')

        os.unlink(config_pathOLD) # delete old file



    def compare_runCardCasing(self):
        """This function checks that the casing in the run card dictionary is the same as the default run card.
        It first performs a case-independent check to determine if the setting appears in the default run card.
        It then performs a case-dependent check. If the casing does not match the default run card, the function will raise a run time error. 
        """
        # Put the run card aside for the moment
        temp_run_card = self.runCardDict
        
        # Get the default run card to compare to
        self.getRunCardDict()
        lower_card = {key.lower() for key in self.runCardDict}
        for setting in temp_run_card:
            # If the setting appears in the run card (case-independent)
            if setting.lower() in lower_card:
                # If the setting appears in the run card (case-dependent)
                if setting in self.runCardDict:
                    self.runCardDict = temp_run_card
                else:
                    self.runCardDict = temp_run_card
                    raise RuntimeError(f'Setting collision because of capitalisation isssues! Check the casing of {setting} in your run card')
        mglog.info('Run card casing looks good!')
                
    def run_card_consistency_check(self):
        """Checks the consistency of runCardDict.
        This function should be called before writing runCardDict to disk to ensure that the run card is consistent and has appropriate settings.
        """
        self.compare_runCardCasing()
        
        # We should always use event_norm = average [AGENE-1725] otherwise Pythia cross sections are wrong
        # Modification: average or bias is ok; sum is incorrect. Change the test to set sum to average
        if checkSetting('event_norm','sum',self.runCardDict):
            self.runCardDict['event_norm'] = 'average'
            mglog.warning("setting event_norm to average, there is basically no use case where event_norm=sum is a good idea")

        if not self.isNLO:
            #Check CKKW-L setting
            if 'ktdurham' in self.runCardDict and float(self.runCardDict['ktdurham']) > 0 and int(self.runCardDict['ickkw']) != 0:
                log='Bad combination of settings for CKKW-L merging! ktdurham=%s and ickkw=%s.'%(self.runCardDict['ktdurham'],self.runCardDict['ickkw'])
                mglog.error(log)
                raise RuntimeError(log)

            # Check if user is trying to use deprecated syscalc arguments with the other systematics script
            if 'systematics_program' not in self.runCardDict or self.runCardDict['systematics_program']=='systematics': #if systematics are not set
                syscalc_settings = ['sys_pdf', 'sys_scalefact', 'sys_alpsfact', 'sys_matchscale']
                found_syscalc_setting = False
                for s in syscalc_settings:
                    if s in self.runCardDict: #searches for the systematic setting in the runCard
                        mglog.warning('Using syscalc setting '+s+' with new systematics script. Systematics script is default from 2.6.2 and steered differently (https://cp3.irmp.ucl.ac.be/projects/madgraph/wiki/Systematics#Systematicspythonmodule)')
                        found_syscalc_setting = True
                if found_syscalc_setting: #if a systematic setting was found
                    syst_arguments = convertSysCalcArguments(self.runCardDict)
                    mglog.info('Converted syscalc arguments to systematics arguments: '+syst_arguments)
                    syst_settings_update = {'systematics_arguments':syst_arguments} # save the system arguments to a dictionary
                    for s in syscalc_settings:
                        syst_settings_update[s] = None
                        self.runCardDict.update(syst_settings_update) #update the systematic settings

        # Check pdf and systematics
        mglog.info('Checking PDF and systematics settings')
        if not base_fragment_setup_check(MADGRAPH_PDFSETTING,self.runCardDict,self.isNLO): #if the base fragment has not been setup
            # still need to set pdf and systematics
            syst_settings = get_pdf_and_systematic_settings(MADGRAPH_PDFSETTING,self.isNLO) # get the pdf and systemetatic settings as a dictionary
            self.runCardDict.update(syst_settings) # update the settings in self.runCardDict

        if 'systematics_arguments' in self.runCardDict:# if there are systematics set in the dictionary
            systematics_arguments = parse_systematics_arguments(self.runCardDict['systematics_arguments'])
            if 'weight_info' not in systematics_arguments: #if there is no event weighting information in the system arguments
                mglog.info('Enforcing systematic weight name convention')
                dyn = None
                if '--dyn' in systematics_arguments or ' dyn' in systematics_arguments: #check if dynamics are set in the system arguments and sets dyn to that value.
                    if '--dyn' in systematics_arguments:
                        dyn = systematics_arguments.split('--dyn')[1]
                    if ' dyn' in systematics_arguments:
                        dyn = systematics_arguments.split(' dyn')[1]
                    dyn = dyn.replace('\'',' ').replace('=',' ').split()[0]
                if dyn is not None and len(dyn.split(','))>1: #if there are dynamics defined, set event weights to acordingly
                    systematics_arguments['weight_info'] = SYSTEMATICS_WEIGHT_INFO_ALTDYNSCALES
                else:
                    systematics_arguments['weight_info'] = SYSTEMATICS_WEIGHT_INFO
                self.runCardDict['systematics_arguments'] = write_systematics_arguments(systematics_arguments)
        # If the rocess is LO, we want to set a 'python_seed' in self.runCarDict
        if not self.isNLO:
            if 'python_seed' not in self.runCardDict:
                mglog.warning('No python seed set in run_card -- adding one with same value as iseed')
                self.runCardDict['python_seed'] = self.runCardDict['iseed'] # if there is no python_seed defined, set it to the same value as 'iseed'


        # consistency check of 4/5 flavour shceme settings
        FS_updates={}
        proton_5flav = False
        jet_5flav = False
        with open(self.process_dir+'/Cards/proc_card_mg5.dat', 'r') as file: # This will be updated at a later point when we have added a proc_card_mg5.dat dictionary
            content = file.readlines()
            #we want to read int he proc_card to determine if it is a 4 or 5 flavour scheme
            for rawline in content:
                line = rawline.split('#')[0] #ignore commented lines
                if line.startswith("define p"): # if we define the quarks in a proton
                    if ('b' in line.split() and 'b~' in line.split()) or ('5' in line.split() and '-5' in line.split()):
                        #if there a b and anti b-quarks defined with p we set proton 5flavour scheme to be true
                        proton_5flav = True
                    if 'j' in line.split() and jet_5flav: #if jet is defined in the proton, set proton 5 flavour to be true
                        proton_5flav = True
                if line.startswith("define j"): # if we are defining jets
                    if ('b' in line.split() and 'b~' in line.split()) or ('5' in line.split() and '-5' in line.split()):
                        # if b and anti b-quarks are defined in jets, set jet 5 flavour scheme to be true.
                        jet_5flav = True
                    if 'p' in line.split() and proton_5flav: # if p is defined in jets and proton has been set to 5 flavour scheme then jet_5flav = True
                        jet_5flav = True
            if proton_5flav or jet_5flav: #If either of the proton or jet have been set to the 5 flavour scheme, set asrwgtflavour dictionary entry to 5
                FS_updates['asrwgtflavor'] = 5
                # Before continuing, we must ensure that both proton and jet have the same colour scheme. If they are inconsistent, we assume 5 flavour scheme. 
                if not proton_5flav:
                    mglog.warning('Found 5-flavour jets but 4-flavour proton. This is inconsistent - please pick one.')
                    mglog.warning('Will proceed assuming 5-flavour scheme.')
                if not jet_5flav:
                    mglog.warning('Found 5-flavour protons but 4-flavour jets. This is inconsistent - please pick one.')
                    mglog.warning('Will proceed assuming 5-flavour scheme.')
            else: # otherwise set to 4 flavour scheme
                FS_updates['asrwgtflavor'] = 4

            if len(FS_updates)==0: #if we cannot determine the flavour scheme
                mglog.warning(f'Could not identify 4- or 5-flavor scheme from process card {self.process_dir}/Cards/proc_card_mg5.dat')

            #check if there is a setting in self.runCardDict for flavour scheme
            if 'asrwgtflavor' in self.runCardDict or 'maxjetflavor' in self.runCardDict or 'pdgs_for_merging_cut' in self.runCardDict:
                if FS_updates['asrwgtflavor'] == 5:
                    # Process card says we are in the five-flavor scheme
                    if ('asrwgtflavor' in self.runCardDict and int(self.runCardDict['asrwgtflavor']) != 5) or ('maxjetflavor' in self.runCardDict and int(self.runCardDict['maxjetflavor']) != 5) or ('pdgs_for_merging_cut' in self.runCardDict and '5' not in self.runCardDict['pdgs_for_merging_cut']):
                        # Inconsistent setting detected; warn the users and correct the settings
                        mglog.warning('b and b~ included in p and j for 5-flavor scheme but run card settings are inconsistent; adjusting run card')
                        run_card_updates = {'asrwgtflavor': 5, 'maxjetflavor': 5, 'pdgs_for_merging_cut': '1, 2, 3, 4, 5, 21'}
                        #If there is an inconsistency, update to be consistent with Process card
                        self.runCardDict.update( run_card_updates )
                        modify_param_card(process_dir=self.process_dir, params={'MASS': {'5': '0.000000e+00'}})
                    else:
                        mglog.debug('Consistent 5-flavor scheme setup detected.')

                if FS_updates['asrwgtflavor'] == 4:
                    # Process card says we are in the four-flavor scheme
                    if ('asrwgtflavor' in self.runCardDict and int(self.runCardDict['asrwgtflavor']) != 4) or ('maxjetflavor' in self.runCardDict and int(self.runCardDict['maxjetflavor']) != 4) or ('pdgs_for_merging_cut' in self.runCardDict and '5' in self.runCardDict['pdgs_for_merging_cut']):
                        # Inconsistent setting detected; warn the users and correct the settings
                        mglog.warning('b and b~ not included in p and j (4-flavor scheme) but run card settings are inconsistent; adjusting run card')
                        run_card_updates = {'asrwgtflavor': 4, 'maxjetflavor': 4, 'pdgs_for_merging_cut': '1, 2, 3, 4, 21'}
                        #update cards to be consistent with Process Card
                        self.runCardDict.update( run_card_updates )
                        modify_param_card(process_dir=self.process_dir, params={'MASS': {'5': '4.700000e+00'}})
                    else:
                        mglog.debug('Consistent 4-flavor scheme setup detected.')
            else:
                # Flavor scheme setup is missing, adding by hand
                if FS_updates['asrwgtflavor'] == 4:
                    # Warn the users and add the settings according to process card
                    mglog.warning('Flavor scheme setup is missing, adding by hand according to process card - b and b~ not included in p and j, 4-flavor scheme setup will be used; adjusting run card.')
                    run_card_updates = {'asrwgtflavor': 4, 'maxjetflavor': 4, 'pdgs_for_merging_cut': '1, 2, 3, 4, 21'}
                    self.runCardDict.update( run_card_updates )
                    modify_param_card(process_dir=self.process_dir, params={'MASS': {'5': '4.700000e+00'}})
                elif FS_updates['asrwgtflavor'] == 5:
                    mglog.warning('Flavor scheme setup is missing, adding by hand according to process card - b and b~ included in p and j, 5-flavor scheme setup will be used; adjusting run card.')
                    run_card_updates = {'asrwgtflavor': 5, 'maxjetflavor': 5, 'pdgs_for_merging_cut': '1, 2, 3, 4, 5, 21'}
                    self.runCardDict.update( run_card_updates )
                    modify_param_card(process_dir=self.process_dir, params={'MASS': {'5': '0.000000e+00'}})

            mglog.info('Finished checking run card - All OK!')
