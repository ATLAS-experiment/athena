# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from AthenaCommon.Constants import DEBUG

logger = logging.getLogger( __name__)
logger.setLevel(DEBUG)

import math
from copy import deepcopy

class ConditionDefaults:
    def __init__(self):

        # we have neta, ceta and p eta. Each is a key for
        # TrigJetConditionConfig_signed_eta. We need a way to
        # interprete the scenario, atking into account minus signs
        # cannot appear in the chain name.
        self.defaults = {
            'et': {'min': '0', 'max': 'inf'},
            'pt': {'min': '0', 'max': 'inf'},
            'eta': {'min': '0', 'max': 'inf'},
            'neta': {'min': '-inf', 'max': '0'},  # scenario etas: -eta  -eta
            'ceta': {'min': '-inf', 'max': 'inf'},  # scenario etas: -eta, +eta
            'peta': {'min': '0', 'max': 'inf'},  # scenario etas: +eta +eta
            'nphi': {'min': '-inf', 'max': '0'},  # scenario phis: -phi  -phi
            'cphi': {'min': '-inf', 'max': 'inf'},  # scenario phis: -phi, +phi
            'pphi': {'min': '0', 'max': 'inf'},  # scenario phis: +phi +phi
            'djmass': {'min': '0', 'max': 'inf'},
            'djdeta': {'min': '0', 'max': 'inf'},
            'djdphi': {'min': '0', 'max': str(math.pi)},
            'ht': {'min': '0', 'max': 'inf'},
            'dipz_mlpl': {'min': '-inf', 'max': 'inf'},
            'dipz_njet': {'min': '2', 'max': 'inf'},
            'mult': {'min': '0', 'max': 'inf'},
            'smc': {'min': '0', 'max': 'inf'},
            'jvt': {'min': '0', 'max': 'inf'},
            'nnJvt': {'min': '0', 'max': '0'}, # Not used; boolean pass/fail
            'bsel': {'min': '-inf', 'max': 'inf'},
            'tausel': {'min': '-inf', 'max': 'inf'},
            'clrsel': {'min': '-inf', 'max': 'inf'},
            'pileuprm': {'min': '-inf', 'max': 'inf'},
            'momCuts': {'min': '-inf', 'max': 'inf'},
            'timing': {'min': '0', 'max': 'inf'},
            'timeSig': {'min': '0', 'max': 'inf'},       
        }

        self.scale_factor = {
            'et': 1000.,
            'pt': 1000.,
            'eta': 0.01,
            'neta': 0.01,
            'ceta': 0.01,
            'peta': 0.01,
            'nphi': 0.01,
            'cphi': 0.01,
            'pphi': 0.01,
            'djmass': 1000.,
            'djdeta': 0.01, 
            'djdphi': 0.01,
            'ht': 1000.,
            'dipz_mlpl': -0.1,
            'dipz_njet': 1.0,
            'mult': 1,
            'smc': 1000.,
            'jvt': 0.01,
            'nnJvt': 0.,
            'clrsel': 0.01,
            'pileuprm': 0.01,
            'momCuts': 0.01,
            'timing': 1.0,
            'timeSig': 1.0,
        }
  
    def __call__(self, key, lo='', hi=''):
        vals = deepcopy(self.defaults[key])
        if lo: vals['min'] = self.scale(key, lo)
        if hi: vals['max'] = self.scale(key, hi)
        return vals

    def scale(self, key, val):
        return str(float(val) * self.scale_factor[key])

defaults = ConditionDefaults()
    
