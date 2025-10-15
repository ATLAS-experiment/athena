# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#
# Read a json file which is contains run 4 information (a first guess),
# and extract parameters used to initialise GlobalSim Hypo block
# Alforithms. For now, this is limited to eEmMultAlgTool
#

from AthenaCommon.Logging import logging
logger = logging.getLogger(__name__)

from pprint import pprint

import json as js

def eEmTipWriterParams_from_json(OutputLevel):

    logger.setLevel(OutputLevel)
    

    with open('/afs/cern.ch/work/p/peter/public/BDT1/build/x86_64-el9-gcc14-opt/share/L0GlobalTestMenuv1_Buttinger.json') as fh:
        menu = js.load(fh)


    eEm_names_l = get_eEm_names(menu)
    j_pars_l = [get_eEm_jpars(menu, n) for n in eEm_names_l]
    tlines_l = [get_eEm_triggerline(menu, n) for n in eEm_names_l]

    assert len(eEm_names_l) == len(j_pars_l)
    assert len(eEm_names_l) == len(tlines_l)

    return  [get_params(*z) for z in zip(eEm_names_l, j_pars_l, tlines_l)]

def get_eEm_names(menu):
    return menu['thresholds']['eEM']['thresholds'].keys()

def get_eEm_jpars(menu, name):
    return menu['thresholds']['eEM']['thresholds'][name]

def get_eEm_triggerline(menu, name):

    for tl in menu['connectors']['L0Global']['triggerlines']:
        if tl['name'] == name: return tl
    raise AssertionError('Triggerline not found for name ' +  name)

def get_params(name, j_pars, tl):
    print('get_params')
    print(name)
    pprint(j_pars)
    pprint(tl)

    conversion_factors = {
        'etmin' : 10,
        'etaMin' : 1, # not known at present (13/102025)
        'etaMax' : 1, # not known at present (13/102025)
    }
    
    params = {
        'name': name,
        'etmin': str(int(j_pars['etmin']*conversion_factors['etmin'])),
        'absEta': j_pars['absEta'],
        'etaMin': '0',
        'etaMax': 'inf',
        'rhad': 'unknown',
        'reta': 'unknown',
        'wstot': 'unknown',
        'rhad_op': '',
        'reta_op': '',
        'wstot_op': '',
        'startbit': tl['startbit'],
        'nbits': tl['nbits']
    }

    set_eEm_discriminants(params, j_pars['flags'])
    return params

def set_eEm_discriminants(params, discs):
    legal_ops = ('>=', '<=', '>', '<')
    for fkey, fval in discs.items():
        op_name = fkey+'_op'
        op = discs[fkey]['compOp']
        assert op in legal_ops, 'Unknown op ' + op
        assert op_name in params
        assert fkey in params
        params[fkey] = str(fval['value'])
        params[op_name] = op
        
 
