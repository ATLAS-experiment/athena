# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

'''
@author Riley Xu - rixu@cern.ch
@date Feb 6th 2020
@brief This file declares functions to make and configure the bank service.
'''

from FPGATrackSimBanks.FPGATrackSimBanksConf import FPGATrackSimBankSvc
from FPGATrackSimConfTools import FPGATrackSimConfigCompInit

import os

def addBankSvc(mapTag, bankTag):
    '''
    Creates and returns a FPGATrackSimBankSvc object, configured with the supplied tag.

    This function adds the returned bank service
    instance to SvcMgr.
    '''

    from AthenaCommon.AppMgr import ServiceMgr
    MyFPGATrackSimBankSvc = FPGATrackSimBankSvc()

    _applyTag(MyFPGATrackSimBankSvc, mapTag, bankTag)

    if "EOS_MGM_URL_FPGATrackSim" in os.environ: # Set by the gitlab yaml
        _applyGitlabOverrides(MyFPGATrackSimBankSvc)

    ServiceMgr += MyFPGATrackSimBankSvc

    return MyFPGATrackSimBankSvc


def _applyTag(MyFPGATrackSimBankSvc, mapTag, bankTag):
    '''
    Helper function that sets the filepaths of the BankSvc using the supplied tag.
    '''

    bankDir = bankTag['bankDir']

    filepaths = [
            'constants_1st',
            'constants_2nd',
            'sectorBank_1st',
            'sectorBank_2nd',
            'sectorSlices'
    ]
    filelists = [
            'constantsNoGuess_1st',
            'constantsNoGuess_2nd',
    ]

    formats = {
            'region': FPGATrackSimConfigCompInit.getRegionIndex(mapTag),
            'regionName': mapTag['regionNames'][FPGATrackSimConfigCompInit.getRegionIndex(mapTag)],            
    }

    for param in filepaths:
        if bankTag['formatted']:
            value = (bankDir + bankTag[param]).format(**formats)
        else:
            value = bankDir + bankTag[param]
        setattr(MyFPGATrackSimBankSvc, param, value)

    for param in filelists:
        if bankTag['formatted']:
            value = [(bankDir + path).format(**formats) for path in bankTag[param]]
        else:
            value = [bankDir + path for path in bankTag[param]]
        setattr(MyFPGATrackSimBankSvc, param, value)



def _applyGitlabOverrides(MyFPGATrackSimBankSvc):
    '''
    Alters the filepaths for running on Gitlab CI
    '''

    # The non-root files need to be copied to the pwd
    for param in ['sectorBank_1st', 'sectorBank_2nd', 'constants_1st', 'constants_2nd']:
        filepath = getattr(MyFPGATrackSimBankSvc, param)
        newpath = os.path.basename(filepath)

        # This doesn't work for some reason, gets a file not found
        #import subprocess
        #subprocess.call(['xrdcp', '-f', "'" + os.environ["EOS_MGM_URL_FPGATrackSim"] + filepath + "'", "'" + newpath + "'"])
        setattr(MyFPGATrackSimBankSvc, param, newpath)

    for param in ['sectorSlices']:
        setattr(MyFPGATrackSimBankSvc, param, os.environ["EOS_MGM_URL_FPGATrackSim"] + getattr(MyFPGATrackSimBankSvc, param))
