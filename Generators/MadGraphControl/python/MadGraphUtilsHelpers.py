# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import os,glob
#The Import line  is  temporary for backwards compatibility of clients.
from AthenaCommon import Logging
mglog = Logging.logging.getLogger('MadGraphUtils')

# Magic name of gridpack directory
MADGRAPH_GRIDPACK_LOCATION='madevent'
# For error handling
MADGRAPH_CATCH_ERRORS=True
MADGRAPH_COMMAND_STACK = []



def settingIsTrue(setting):
    if setting.replace("'",'').replace('"','').replace('.','').lower() in ['t','true']:
        return True
    return False

def get_mg5_version():
    """Return MadGraph version string (e.g. '3.5.1')

    Used to include MG version in gridpack names for better traceability.
    Reads version from $MADPATH/VERSION file.
    """
    with open(os.environ['MADPATH']+'/VERSION','r') as version_file:
        for line in version_file:
            if 'version' in line:
                return line.split('=')[1].strip()
    raise RuntimeError('Failed to find MadGraph/MadGraph5_aMC@NLO version')

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

def error_check(errors_a, return_code):
    if not MADGRAPH_CATCH_ERRORS:
        return
    if errors_a is None:
        # stderr is not always captured (e.g. catch_errors=False).
        # Still fail on non-zero return code.
        if return_code != 0:
            mglog.error(f'Detected a bad return code: {return_code}')
            write_test_script()
            raise RuntimeError('Error detected in MadGraphControl process')
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
            # Another new python 3.12 message in MG5_aMC 3.6
            if 'python3.12+ support: For reweighting feature, please use 3.6.X release.' in err:
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
    mglog.info('# Script start; trim off columns left of the "#"')
    # Write offline stand-alone reproduction script
    with open('standalone_script.sh','w') as standalone_script:
        for command in MADGRAPH_COMMAND_STACK:
            for line in command.split('\n'):
                mglog.info(line)
                standalone_script.write(line+'\n')
    mglog.info('# Script end')
    mglog.info('Script also written to %s/standalone_script.sh',os.getcwd())


def find_key_and_update(akey,dictionary):
    """ Helper function when looking at param cards
    In some cases it's tricky to match keys - they may differ
    only in white space. This tries to sort out when we have
    a match, and then uses the one in blockParams afterwards.
    In the case of no match, it returns the original key.
    """
    test_key = ' '.join(akey.strip().replace('\t',' ').split())
    for key in dictionary:
        mod_key = ' '.join(key.strip().replace('\t',' ').split())
        if mod_key==test_key:
            return key
    return akey
