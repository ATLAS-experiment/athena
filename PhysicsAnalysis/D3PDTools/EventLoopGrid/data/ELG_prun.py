# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


import os.path
import subprocess
import re
import shlex
import time

# Force flushing print 
# Since this script is executed within a TPython::Exec() function call in the the PrunDriver class, 
# if not forcing flushing then no printed messages in this script would be displayed to the user 
# (unless an error is raised then the buffer would also be printed)
import functools
print = functools.partial(print, flush=True)


def ELG_prun(sample) :
    # Important: only return as integer 1 if the creation of the tarball was unsuccesful as the PrunDriver
    # relies on that to stop the submission if tarball creation was unsuccesful 

    try:
        from pandatools import PandaToolsPkgInfo  # noqa: F401
    except ImportError:
        print ("prun needs additional setup, try:")
        print ("    lsetup panda")
        return 99

    cmd = ["prun"]

    #These are options that can be set by the user
    opts = ['destSE',
            'site',
            'rootVer',
            'cmtConfig',
            'excludedSite',
            'nGBPerJob',
            'memory',
            'maxCpuCount',
            'nFiles',
            'nFilesPerJob',
            'nEventsPerJob',
            'nJobs',
            'maxFileSize',
            'maxNFilesPerJob',
            'addNthFieldOfInDSToLFN',
            'cpuTimePerEvent',
            'maxWalltime',
            'voms',
            'workingGroup',
            'tmpDir']

    #These are options that can be set by the user
    switches = ['express',
                'noSubmit',
                'skipScout',
                'disableAutoRetry',
                'useNewCode',
                'official',
                'mergeOutput',
                'useRootCore',
                'useAthenaPackages',
                'avoidVP']

    using_nEventsPerJob = False
    from ROOT import SH
    for opt in opts :
        arg = sample.meta().castDouble('nc_' + opt, -1, SH.MetaObject.CAST_NOCAST_DEFAULT)
        if abs(arg + 1) > 1e-6 :
            cmd += ["--" + opt + "=" + str(int(round(arg)))]
            if opt=="nEventsPerJob":
                using_nEventsPerJob=True
        else :
            arg = sample.meta().castString('nc_' + opt)
            if len(arg) :
                cmd += ["--" + opt + "=" + arg]

    # nGBPerJob and nEventsPerJob are incompatible to prun
    if using_nEventsPerJob:
        cmd = [ x for x in cmd if "nGBPerJob" not in x ]
        print(cmd)

    for switch in switches :
        arg = sample.meta().castDouble('nc_' + switch, 0, SH.MetaObject.CAST_NOCAST_DEFAULT)
        if arg != 0 :
            cmd += ["--" + switch]
        else :
            arg = sample.meta().castString('nc_' + switch)
            if len(arg) :
                if arg != "False" and arg != "false" and arg != "FALSE" :
                    cmd += ["--" + switch]

    #These options should normally not be touched by the user
    internalOpts = ['exec',
                    'inDS',
                    'outDS',
                    'outputs',
                    'writeInputToTxt',
                    'match',
                    'framework']

    for opt in internalOpts :
        value = sample.meta().castString('nc_' + opt)
        if opt == "exec" and using_nEventsPerJob:
            value += " %SKIPEVENTS %MAXEVENTS"
        cmd += ["--" + opt + "=" + value]

    if sample.meta().castDouble('nc_mergeOutput', 1, SH.MetaObject.CAST_NOCAST_DEFAULT) == 0 or sample.meta().castString('nc_mergeOutput').upper() == 'FALSE' :
        #don't set merge script 
        pass
    else :
        cmd += ["--mergeScript=" + sample.meta().castString('nc_mergeScript')]

    if len(sample.meta().castString('nc_EventLoop_SubmitFlags')) :
        cmd += shlex.split (sample.meta().castString('nc_EventLoop_SubmitFlags'))

    if sample.meta().castDouble('nc_showCmd', 0, SH.MetaObject.CAST_NOCAST_DEFAULT) != 0 :
        print (cmd)

    # If tarball is not existing create it 
    # In case of tarball creation issue return 1 
    if not os.path.isfile('jobcontents.tgz') : 
        import copy
        dummycmd = copy.deepcopy(cmd)
        dummycmd += ["--outTarBall=jobcontents.tgz"]
        if len(sample.meta().castString('nc_EventLoop_UserFiles')) :
            dummycmd += ["--extFile=jobdef.root,runjob.sh," + sample.meta().castString('nc_EventLoop_UserFiles').replace(" ",",")]
            pass
        else :
            dummycmd += ["--extFile=jobdef.root,runjob.sh"]
            pass
        dummycmd += ["--noSubmit"]

        try:
            out = subprocess.check_output(dummycmd, stderr=subprocess.STDOUT, encoding="utf-8")
        except subprocess.CalledProcessError as e: 
            # Handle a case where we couldn't get the grid nickname in advance
            if 'Need to generate a grid proxy' in e.output and any( ['%nickname%' in x for x in cmd ] ):
                print('Detected nickname still undefined. Trying to replace it.')
                try:
                    from pandatools import PsubUtils
                    nickname = PsubUtils.getNickname()
                    dummycmd = [ x.replace('%nickname%',nickname) for x in dummycmd ]
                    cmd = [ x.replace('%nickname%',nickname) for x in cmd ]
                except Exception as e_rep:
                    print(f'Nickname replacement failed with error {e_rep.returncode}: {e_rep.output}')
                # Now try the job again
                try:
                    out = subprocess.check_output(dummycmd, stderr=subprocess.STDOUT, encoding="utf-8")
                except subprocess.CalledProcessError as e_take2:
                    # Failed to create tarball thus returning 1 
                    print ("Command:")
                    print (e_take2.cmd)
                    print ("failed with return code " , e_take2.returncode)
                    print ("output was:")
                    print (e_take2.output)
                    return 1
                except Exception as e:
                    # Catch any other exception
                    # Failed to create tarball thus returning 1 
                    print ("Command:")
                    print (dummycmd)
                    print ("failed and output was:")
                    print (e)
                    return 1
            else:
                # Failed to create tarball thus returning 1 
                print ("Command:")
                print (e.cmd)
                print ("failed with return code " , e.returncode)
                print ("output was:")
                print (e.output)
                return 1
        
        except Exception as e:
            # Catch any other exception
            # Failed to create tarball thus returning 1 
            print ("Command:")
            print (dummycmd)
            print ("failed and output was:")
            print (e)
            return 1

    cmd += ["--inTarBall=jobcontents.tgz"]
    
    # If user has not specified this flag it will return -1
    nSubmitTries = int( sample.meta().castDouble("nc_prunNRetrySubmitToGrid", -1, SH.MetaObject.CAST_NOCAST_DEFAULT) )
    # Make sure nSubmitTries is not lower than 1
    # Could happen if user has specified a negative or 0 as value
    # or if user has not set the nc_prunNRetrySubmitToGrid value
    # In both cases assign the default value: 3 submission tries
    if nSubmitTries < 1:
        nSubmitTries = 3
    
    successSubmission = False
    iTry = 0
    out = ""
    
    listErrorsMessagesTries = []
    while (iTry < nSubmitTries) and (not successSubmission):
        if iTry > 0:
            # Wait for 2 seconds as issue occured on the past try
            # and it could be due to a transient issue
            time.sleep(2)
        try:
            out = subprocess.check_output(cmd, stderr=subprocess.STDOUT, encoding="utf-8")
            # In that case submission was succesful 
            successSubmission = True
        except subprocess.CalledProcessError as e:
            # Failed to submit job  
            # Keep track of error messages
            errorMsg = ""
            errorMsg += "-"*60 + "\n"
            errorMsg += f"iTry={iTry+1} out of nTries={nSubmitTries}\n"
            errorMsg += "-"*60 + "\n"
            errorMsg += "Command:\n"
            errorMsg += f"{e.cmd}\n"
            errorMsg += f"failed with return code {e.returncode}\n"
            errorMsg += "output was:\n"
            errorMsg += f"{e.output}\n"
            
            # Add error message to the list of error messages 
            listErrorsMessagesTries.append(errorMsg)
            # Increase index tries 
            iTry += 1
        
        except Exception as e:
            # Catch any other exception 
            # Failed to submit job  
            # Keep track of error messages
            errorMsg = ""
            errorMsg += "-"*60 + "\n"
            errorMsg += f"iTry={iTry+1} out of nTries={nSubmitTries}\n"
            errorMsg += "-"*60 + "\n"
            errorMsg += "Command:\n"
            errorMsg += f"{cmd}\n"
            errorMsg += f"failed and output was:\n"
            errorMsg += f"{e}"
            # Add error message to the list of error messages 
            listErrorsMessagesTries.append(errorMsg)
            # Increase index tries 
            iTry += 1
    
    # If after all tries submission was not succesful 
    # Then print error messages 
    if (not successSubmission):
        # NB: only print error messages if the submission failed 
        # Otherwise silence the issue 
        print(f"Failed submission after nTries={nSubmitTries}")
        print("\n".join(listErrorsMessagesTries))
        return 2
    
    jediTaskID = 0
    try:
        line = re.findall(r'TaskID=\d+', str(out))[0]
        jediTaskID = int(re.findall(r'\d+', line)[0])
    except IndexError:
        print (out)
        return 3

    return jediTaskID
