# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import os,glob,subprocess
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
    

def modify_param_card(param_card_input=None,param_card_backup=None,process_dir=MADGRAPH_GRIDPACK_LOCATION,params={},output_location=None):
    """Build a new param_card.dat from an existing one.
    Params should be a dictionary of dictionaries. The first key is the block name, and the second in the param name.
    Keys can include MASS (for masses) and DECAY X (for decays of particle X)"""
    # Grab the old param card and move it into place

    # Check for the default run card location
    if param_card_input is None:
        param_card_input=process_dir+'/Cards/param_card.dat'
    elif param_card_input is not None and not os.access(param_card_input,os.R_OK):
        paramcard = subprocess.Popen(['get_files','-data',param_card_input])
        paramcard.wait()
        if not os.access(param_card_input,os.R_OK):
            raise RuntimeError('Could not get param card '+param_card_input)
        mglog.info('Using input param card at '+param_card_input)

    #ensure all blocknames and paramnames are upper case
    paramsUpper = {}
    for blockName in list(params.keys()):
       paramsUpper[blockName.upper()] = {}
       for paramName in list(params[blockName].keys()):
          paramsUpper[blockName.upper()][paramName.upper()] = params[blockName][paramName]

    if param_card_backup is not None:
        mglog.info('Keeping backup of original param card at '+param_card_backup)
        param_card_old = param_card_backup
    else:
        param_card_old = param_card_input+'.old_to_be_deleted'
    if os.path.isfile(param_card_old):
        os.unlink(param_card_old) # delete old backup
    os.rename(param_card_input, param_card_old) # change name of original card

    oldcard = open(param_card_old,'r')
    param_card_location= process_dir+'/Cards/param_card.dat' if output_location is None else output_location
    newcard = open(param_card_location,'w')
    decayEdit = False #only becomes true in a DECAY block when specifying the BR
    blockName = ""
    doneParams = {} #tracks which params have been done
    for linewithcomment in oldcard:
        line=linewithcomment.split('#')[0]
        if line.strip().upper().startswith('BLOCK') or line.strip().upper().startswith('DECAY')\
                    and len(line.strip().split()) > 1:
            if decayEdit and blockName == 'DECAY':
                decayEdit = False # Start a new DECAY block
            pos = 0 if line.strip().startswith('DECAY') else 1
            if blockName=='MASS' and 'MASS' in paramsUpper:
                # Any residual masses to set?
                if "MASS" in doneParams:
                    leftOvers = [ x for x in paramsUpper['MASS'] if x not in doneParams['MASS'] ]
                else:
                    leftOvers = [ x for x in paramsUpper['MASS'] ]

                for pdg_id in leftOvers:
                    mglog.warning('Adding mass line for '+str(pdg_id)+' = '+str(paramsUpper['MASS'][pdg_id])+' which was not in original param card')
                    newcard.write('   '+str(pdg_id)+'  '+str(paramsUpper['MASS'][pdg_id])+'\n')
                    doneParams['MASS'][pdg_id]=True
            if blockName=='DECAY' and 'DECAY' not in line.strip().upper() and 'DECAY' in paramsUpper:
                # Any residual decays to include?
                leftOvers = [ x for x in paramsUpper['DECAY'] if x not in doneParams['DECAY'] ]
                for pdg_id in leftOvers:
                    mglog.warning('Adding decay for pdg id '+str(pdg_id)+' which was not in the original param card')
                    newcard.write( paramsUpper['DECAY'][pdg_id].strip()+'\n' )
                    doneParams['DECAY'][pdg_id]=True
            blockName = line.strip().upper().split()[pos]
        if decayEdit:
            continue #skipping these lines because we are in an edit of the DECAY BR

        akey = None
        if blockName != 'DECAY' and len(line.strip().split()) > 0:
            # The line is already without the comment.
            # In the case of mixing matrices this is a bit tricky
            if len(line.split())==2:
                akey = line.upper().strip().split()[0]
            else:
                # Take everything but the last word
                akey = line.upper().strip()[:line.strip().rfind(' ')].strip()
        elif blockName == 'DECAY' and len(line.strip().split()) > 1:
            akey = line.strip().split()[1]
        if akey is None:
           newcard.write(linewithcomment)
           continue

        #check if we have params for this block
        if blockName not in paramsUpper:
           newcard.write(linewithcomment)
           continue
        blockParams = paramsUpper[blockName]
        # Check the spacing in the key
        akey = find_key_and_update(akey,blockParams)

        # look for a string key, which would follow a #
        stringkey = None
        if '#' in linewithcomment: #ignores comment lines
           stringkey = linewithcomment[linewithcomment.find('#')+1:].strip()
           if len(stringkey.split()) > 0:
               stringkey = stringkey.split()[0].upper()

        if akey not in blockParams and not (stringkey is not None and stringkey in blockParams):
           newcard.write(linewithcomment)
           continue

        if akey in blockParams and (stringkey is not None and stringkey in blockParams):
           raise RuntimeError('Conflicting use of numeric and string keys '+akey+' and '+stringkey)

        theParam = blockParams.get(akey,blockParams[stringkey] if stringkey in blockParams else None)
        if blockName not in doneParams:
            doneParams[blockName] = {}
        if akey in blockParams:
            doneParams[blockName][akey]=True
        elif stringkey is not None and stringkey in blockParams:
            doneParams[blockName][stringkey]=True

        #do special case of DECAY block
        if blockName=="DECAY":
           if theParam.splitlines()[0].split()[0].upper()=="DECAY":
               #specifying the full decay block
               for newline in theParam.splitlines():
                    newcard.write(newline+'\n')
                    mglog.info(newline)
               decayEdit = True
           else: #just updating the total width
              newcard.write('DECAY   '+akey+'    '+str(theParam)+'  # '+(linewithcomment[linewithcomment.find('#')+1:].strip() if linewithcomment.find('#')>0 else "")+'\n')
              mglog.info('DECAY   '+akey+'    '+str(theParam)+'  # '+(linewithcomment[linewithcomment.find('#')+1:].strip() if linewithcomment.find('#')>0 else "")+'\n')
        # second special case of QNUMBERS
        elif blockName=='QNUMBERS':
           #specifying the full QNUMBERS block
           for newline in theParam.splitlines():
                newcard.write(newline+'\n')
                mglog.info(newline)
           decayEdit = True
        else: #just updating the parameter
           newcard.write('   '+akey+'    '+str(theParam)+'  # '+(linewithcomment[linewithcomment.find('#')+1:].strip() if linewithcomment.find('#')>0 else "")+'\n')
           mglog.info('   '+akey+'    '+str(theParam)+'  # '+(linewithcomment[linewithcomment.find('#')+1:].strip() if linewithcomment.find('#')>0 else "")+'\n')
        # Done editing the line!

    #check that all specified parameters have been updated (helps to catch typos)
    for blockName in paramsUpper:
       if blockName not in doneParams and len(paramsUpper[blockName].keys())>0:
          raise RuntimeError('Did not find any of the parameters for block '+blockName+' in param_card')
       for paramName in paramsUpper[blockName]:
          if paramName not in doneParams[blockName]:
            raise RuntimeError('Was not able to replace parameter '+paramName+' in param_card')

    # Close up and return
    oldcard.close()
    newcard.close()

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
