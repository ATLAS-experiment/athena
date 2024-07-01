# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Python steering script for Herwig7_i to shower input LHE files
# generated with a given UFO model
#   written by Yoran Yeh <yoran.yeh@cern.ch>

import os, shutil, subprocess

def generateFRModel(UFOModel, overWriteLocalDir=False):
    """
        UFOModel (string) :: name of UFO model that is imported for the event generation
    """

    # find UFO directory
    MGModelsLatest = '/cvmfs/atlas.cern.ch/repo/sw/Generators/madgraph/models/latest/'
    MGModelDirCVMFS = os.path.join(MGModelsLatest, UFOModel)
    if not MGModelDirCVMFS:
        athMsgLog.error(f'could not find UFO model {UFOModel} on cvmfs.' \
                        f' Are you sure it is available on {MGModelsLatest} ?')
        exit(1)

    try:
        shutil.copytree(MGModelDirCVMFS, UFOModel)
    except FileExistsError:
        if not overWriteLocalDir:
            athMsgLog.error(f'{UFOModel} already exists in your local directory.' \
                            ' Please (re)move this directory yourself to avoid it' \
                            ' being overwritten, or set `overWriteLocalDir` to True.' \
                            ' Exiting gracefully...')
            exit(1)
        else:
            shutil.rmtree(UFOModel)
            shutil.copytree(MGModelDirCVMFS, UFOModel)

    # find ufo2herwig command
    try:
      HERWIG7_PATH = os.environ['HERWIG7_PATH']
    except KeyError:
      raise RuntimeError("HERWIG7_PATH environment variable not set")    
    ufo2herwig   = os.path.join(HERWIG7_PATH, 'bin/ufo2herwig')
    if not ufo2herwig: 
        athMsgLog.error(f'could not find Herwig7 ufo2herwig command: {ufo2herwig}')
        exit(1)

    # generate FRModel.model file
    try:
        subprocess.run([ufo2herwig, UFOModel], check = True)
    
    # it might be needed to convert python2 models to python3
    except subprocess.CalledProcessError:
    
        # fresh copy of UFO model, as the previous attempt for `ufo2herwig` 
        # will have made modifications in this directory
        shutil.rmtree(UFOModel)
        shutil.copytree(MGModelDirCVMFS, UFOModel)
        subprocess.run([ufo2herwig, '--convert', UFOModel], check = True)

    subprocess.run(['make'], shell = True, check = True)
    
    # add "read FRModel.model" to the Herwig7 command
    Herwig7Config.add_commands("""
read FRModel.model
#""")

  
