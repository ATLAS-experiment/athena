# Copyright (C) 2002-20204 CERN for the benefit of the ATLAS collaboration

import os,glob
#The Import line  is  temporary for backwards compatibility of clients.
from AthenaCommon import Logging
mglog = Logging.logging.getLogger('MadGraphUtils')

# Magic name of gridpack directory
MADGRAPH_GRIDPACK_LOCATION='madevent'
# For error handling
MADGRAPH_CATCH_ERRORS=True
MADGRAPH_COMMAND_STACK = []

def getDictFromCard(card_loc,lowercase=False):
    card=open(card_loc)
    mydict={}
    for line in iter(card):
        if not line.strip().startswith('#'): # line commented out
            command = line.split('!', 1)[0]
            if '=' in command:
                setting = command.split('=')[-1].strip()
                value = '='.join(command.split('=')[:-1]).strip()
                if lowercase:
                    value=value.lower()
                    setting=setting.lower()
                mydict[setting]=value
    card.close()
    return mydict

def settingIsTrue(setting):
    if setting.replace("'",'').replace('"','').replace('.','').lower() in ['t','true']:
        return True
    return False

def totallyStripped(x):
    y=str(x).lower().strip()
    # remove leading and trailing "/'
    while len(y)>0 and (y[0]=='"' or y[0]=="'"):
        y=y[1:]
    while len(y)>0 and (y[-1]=='"' or y[-1]=="'"):
        y=y[:-1]
    return y

def checkSetting(key_,value_,mydict_):
    key=totallyStripped(key_)
    value=totallyStripped(value_)
    mydict={}
    for k in mydict_:
        mydict[totallyStripped(k)]=totallyStripped(mydict_[k])
    return key in mydict and mydict[key]==value

def checkSettingIsTrue(key_,mydict_):
    key=totallyStripped(key_)
    mydict={}
    for k in mydict_:
        mydict[totallyStripped(k)]=totallyStripped(mydict_[k])
    return key in mydict and mydict[key] in ['t','true']

def checkSettingExists(key_,mydict_):
    key=totallyStripped(key_)
    keys=[]
    for k in mydict_:
        keys+=[totallyStripped(k)]
    return key in keys

def is_version_or_newer(args):
    # also need to find out the version (copied from generate)
    import os
    version=None
    version_file = open(os.environ['MADPATH']+'/VERSION','r')

    for line in version_file:
        if 'version' in line:
            version=line.split('=')[1].strip()
    version_file.close()

    if not version:
        raise RuntimeError('Failed to find MadGraph/MadGraph5_aMC@NLO version in '+version_file)

    vs=[int(v) for v in version.split('.')]

    # this is lazy, let's hope there wont be a subversion > 100...
    y=int(100**max(len(vs),len(args)))
    testnumber=0
    for x in args:
        testnumber+=x*y
        y/=100

    y=int(100**max(len(vs),len(args)))
    versionnumber=0
    for x in vs:
        versionnumber+=x*y
        y/=100
    return versionnumber>=testnumber

def isNLO_from_run_card(run_card):
    f = open(run_card,'r')
    if "parton_shower" in f.read().lower():
        f.close()
        return True
    else:
        f.close()
        return False

def get_runArgs_info(runArgs):
    if runArgs is None:
        raise RuntimeError('runArgs must be provided!')
    if hasattr(runArgs,'ecmEnergy'):
        beamEnergy = runArgs.ecmEnergy / 2.
    else:
        raise RuntimeError("No center of mass energy found in runArgs.")
    if hasattr(runArgs,'randomSeed'):
        random_seed = runArgs.randomSeed
    else:
        raise RuntimeError("No random seed found in runArgs.")
    return beamEnergy,random_seed


def error_check(errors_a, return_code):
    global MADGRAPH_CATCH_ERRORS
    if not MADGRAPH_CATCH_ERRORS:
        return
    unmasked_error = False
    my_debug_file = None
    bad_variables = []
    # Make sure we are getting a string and not a byte string (python3 ftw)
    errors = errors_a
    if type(errors)==bytes:
        errors = errors.decode('utf-8')
    if len(errors):
        mglog.info('Some errors detected by MadGraphControl - checking for serious errors')
        for err in errors.split('\n'):
            if len(err.strip())==0:
                continue
            # Errors to do with I/O... not clear on their origin yet
            if 'Inappropriate ioctl for device' in err:
                mglog.info(err)
                continue
            if 'stty: standard input: Invalid argument' in err:
                mglog.info(err)
                continue
            # Errors for PDF sets that should be fixed in MG5_aMC 2.7
            if 'PDF already installed' in err:
                mglog.info(err)
                continue
            if 'Read-only file system' in err:
                mglog.info(err)
                continue
            if 'HTML' in err:
                # https://bugs.launchpad.net/mg5amcnlo/+bug/1870217
                mglog.info(err)
                continue
            if 'impossible to set default multiparticles' in err:
                # https://answers.launchpad.net/mg5amcnlo/+question/690004
                mglog.info(err)
                continue
            if 'More information is found in' in err:
                my_debug_file = err.split("'")[1]
            if err.startswith('tar'):
                mglog.info(err)
                continue
            if 'python2 support will be removed' in err:
                mglog.info(err)
                continue
            if 'python3.12 support is still experimental' in err:
                mglog.info(err)
                continue
            # silly ghostscript issue in 21.6.46 nightly
            if 'required by /lib64/libfontconfig.so' in err or\
               'required by /lib64/libgs.so' in err:
                mglog.info(err)
                continue
            if 'Error: Symbol' in err and 'has no IMPLICIT type' in err:
                bad_variables += [ err.split('Symbol ')[1].split(' at ')[0] ]
            # error output from tqdm (progress bar)
            if 'it/s' in err:
                mglog.info(err)
                continue
            mglog.error(err)
            unmasked_error = True
    # This is a bit clunky, but needed because we could be several places when we get here
    if my_debug_file is None:
        debug_files = glob.glob('*debug.log')+glob.glob('*/*debug.log')
        for debug_file in debug_files:
            # This protects against somebody piping their output to my_debug.log and it being caught here
            has_subproc = os.access(os.path.join(os.path.dirname(debug_file),'SubProcesses'),os.R_OK)
            if has_subproc:
                my_debug_file = debug_file
                break

    if my_debug_file is not None:
        if not unmasked_error:
            mglog.warning('Found a debug file at '+my_debug_file+' but no apparent error. Will terminate.')
        mglog.error('MadGraph5_aMC@NLO appears to have crashed. Debug file output follows.')
        with open(my_debug_file,'r') as error_output:
            for l in error_output:
                mglog.error(l.replace('\n',''))
        mglog.error('End of debug file output')

    if bad_variables:
        mglog.warning('Appeared to detect variables in your run card that MadGraph did not understand:')
        mglog.warning('  Check your run card / JO settings for %s',bad_variables)

    # Check the return code
    if return_code!=0:
        mglog.error(f'Detected a bad return code: {return_code}')
        unmasked_error = True

    # Now raise an error if we were in either of the error states
    if unmasked_error or my_debug_file is not None:
        write_test_script()
        raise RuntimeError('Error detected in MadGraphControl process')
    return


# Write a short test script for standalone debugging
def write_test_script():
    mglog.info('Will write a stand-alone debugging script.')
    mglog.info('This is an attempt to provide you commands that you can use')
    mglog.info('to reproduce the error locally. If you make additional')
    mglog.info('modifications by hand (not using MadGraphControl) in your JO,')
    mglog.info('make sure that you check and modify the script as needed.\n\n')
    global MADGRAPH_COMMAND_STACK
    mglog.info('# Script start; trim off columns left of the "#"')
    # Write offline stand-alone reproduction script
    with open('standalone_script.sh','w') as standalone_script:
        for command in MADGRAPH_COMMAND_STACK:
            for line in command.split('\n'):
                mglog.info(line)
                standalone_script.write(line+'\n')
    mglog.info('# Script end')
    mglog.info('Script also written to %s/standalone_script.sh',os.getcwd())

def setup_path_protection():
    # Addition for models directory
    global MADGRAPH_COMMAND_STACK
    if 'PYTHONPATH' in os.environ:
        if not any( [('Generators/madgraph/models' in x and 'shutil_patch' not in x) for x in os.environ['PYTHONPATH'].split(':') ]):
            os.environ['PYTHONPATH'] += ':/cvmfs/atlas.cern.ch/repo/sw/Generators/madgraph/models/latest'
            MADGRAPH_COMMAND_STACK += ['export PYTHONPATH=${PYTHONPATH}:/cvmfs/atlas.cern.ch/repo/sw/Generators/madgraph/models/latest']
    # Make sure that gfortran doesn't write to somewhere it shouldn't
    if 'GFORTRAN_TMPDIR' in os.environ:
        return
    if 'TMPDIR' in os.environ:
        os.environ['GFORTRAN_TMPDIR']=os.environ['TMPDIR']
        MADGRAPH_COMMAND_STACK += ['export GFORTRAN_TMPDIR=${TMPDIR}']
        return
    if 'TMP' in os.environ:
        os.environ['GFORTRAN_TMPDIR']=os.environ['TMP']
        MADGRAPH_COMMAND_STACK += ['export GFORTRAN_TMPDIR=${TMP}']
        return
    
def get_default_config_card(process_dir=MADGRAPH_GRIDPACK_LOCATION):

    lo_config_card=process_dir+'/Cards/me5_configuration.txt'
    nlo_config_card=process_dir+'/Cards/amcatnlo_configuration.txt'

    if os.access(lo_config_card,os.R_OK) and not os.access(nlo_config_card,os.R_OK):
        return lo_config_card
    elif os.access(nlo_config_card,os.R_OK) and not os.access(lo_config_card,os.R_OK):
        return nlo_config_card
    elif os.access(nlo_config_card,os.R_OK) and os.access(lo_config_card,os.R_OK):
        mglog.error('Found both types of config card in '+process_dir)
    else:
        mglog.error('No config card in '+process_dir)
    raise RuntimeError('Unable to locate configuration card')

def is_NLO_run(process_dir=MADGRAPH_GRIDPACK_LOCATION):
    # Very simple check based on the above config card grabbing
    return get_default_config_card(process_dir=process_dir)==process_dir+'/Cards/amcatnlo_configuration.txt'
