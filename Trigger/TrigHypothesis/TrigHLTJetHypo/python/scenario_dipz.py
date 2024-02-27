# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TrigHLTJetHypo.RepeatedConditionParams import RepeatedConditionParams
from TrigHLTJetHypo.HelperConfigToolParams import HelperConfigToolParams
from TrigHLTJetHypo.ConditionDefaults import defaults
from TrigHLTJetHypo.make_treevec import make_treevec

import re

pattern_dipz = r'^Z(?P<WP>\d+)XX(?P<N>\d+)c(?P<ptlo>\d*)'
rgx_pattern = re.compile(pattern_dipz)


def get_kin_args_from_matchdict(groupdict):
    """ Get kinematic cuts on jets for DIPZ MLPL hypo """
    
    if not groupdict['ptlo']:  # then default filtering for pt
        groupdict['ptlo'] = '20'
        groupdict['pthi'] = ''
    
    condargs = []
    vals = defaults('pt',
                    groupdict['ptlo'],
                    '')  # will be assigned default value 
    condargs.append(('pt', vals))
        
    vals = defaults('eta',
                    '', # will be assigned default value 
                    '240')  
    condargs.append(('eta', vals))

    return condargs

def get_mult_args_from_matchdict(groupdict):
    """Get jet multiplicity for DIPZ MLPL hypo """
     
    if not groupdict['N']:
        raise ValueError('DIPZ scenario requires a pre-defined jet multiplicity.')

    condargs = []

    vals = defaults('dipz_njet', lo = groupdict['N'], hi = groupdict['N'])
    condargs.append(('dipz_njet', vals))
    
    return condargs

def get_dipz_mlpl_from_matchdict(groupdict, njets):
    """Get DIPz WP, capacity (njets) and decorator names"""
    
    if not groupdict['WP'] :
        raise ValueError('DIPZ scenario requires a pre-defined working point cut on MLPL.')

    condargs = []

    vals = defaults('dipz_mlpl', lo = groupdict['WP']) # Note: scale factor of -0.1 applied by default
    vals['decName_z']='dipz20231122_z'
    vals['decName_sigma']='dipz20231122_negLogSigma2'
    vals['capacity']=njets
    
    condargs.append(('dipz_mlpl', vals))
 
    return condargs


def scenario_dipz(scenario, chainPartInd):
    """calculate the parameters needed to generate a hypo helper config AlgTool
    starting from a the hypoScenario which appears in the chainname for
    a DIPZ mlpl condition. """

    assert scenario.startswith('Z'),\
        'routing error, module %s: bad scenario %s' % (__name__, scenario)

    m = rgx_pattern.match(scenario)

    assert m is not None, \
        'scenario_dipz.py - regex part %s does not match scenario %s' % (        
            pattern_dipz, scenario)

    groupdict = m.groupdict()

    # list for the repeatedCondition parameters, FilterParams and their indices
    # needed for the HelperConfigToolParams
    repcondargs = []
    filterparams = []
    filterparam_inds = []

    # treeVec is [0, 0, 1] handle non-root nodes here

    ## Get kinematic cuts, number of jets, DIPZ cut, ... 
    condargs_kin = get_kin_args_from_matchdict(groupdict)
    chooseN = float(get_mult_args_from_matchdict(groupdict)[0][1]['min']) 
    condargs = get_dipz_mlpl_from_matchdict(groupdict, str(chooseN))

    ## DIPz condition for N jets
    repcondargs.append(RepeatedConditionParams(tree_id=2,
                                               tree_pid=1,
                                               multiplicity=1,
                                               chainPartInd=chainPartInd,
                                               condargs=condargs))
    filterparam_inds.append(-1) # no filter

    # N jet: single jet condition with multiplicity N
    repcondargs.append(RepeatedConditionParams(tree_id=1,
                                           tree_pid=0,
                                           multiplicity=int(chooseN),
                                           chainPartInd=chainPartInd,
                                           condargs=condargs_kin))    
    filterparam_inds.append(-1)

    # treevec[i] gives the tree_id of the parent of the
    # node with tree_id = i
    treevec = make_treevec(repcondargs)
    assert treevec == [0, 0, 1]

    assert len(repcondargs) == len(filterparam_inds)


    helper_params = HelperConfigToolParams(treevec=treevec,
                                           repcondargs=repcondargs,
                                           filterparams=filterparams,
                                           filterparam_inds=filterparam_inds)

    return [helper_params]  # a list with one entry per FastReduction tree

