# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
'''
@file FPGATrackSimBankGenConfig.py
@author Riley Xu - rixu@cern.ch
@date Sept 22, 2020
@brief This file declares functions to configure components in FPGATrackSimBankGen
'''

def applyTag(obj, bank_tag):
    '''
    Applies the parameters in the supplied tags to the given FPGATrackSimBankGen object.
    '''

    # List of configurable parameters for the given object type
    params = {
        'AtRanluxGenSvc': [
            # pass
        ],
        'FPGATrackSimMatrixGenAlgo': [
            'WCmax',
        ],
    }

    for param in params[obj.getType()]:
        setattr(obj, param, bank_tag[param])

    if obj.getType() == 'AtRanluxGenSvc':
        obj.Seeds = [ bank_tag['rndStreamName'] + " %d %d" % (bank_tag['seed'], bank_tag['seed2']) ]
        obj.EventReseeding = False

    elif obj.getType() == 'FPGATrackSimMatrixGenAlgo':

        obj.par_c_slices    = 250
        obj.par_phi_slices  = 10
        obj.par_d0_slices   = 250
        obj.par_z0_slices   = 100
        obj.par_eta_slices  = 10
