# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TrigHLTJetHypo.RepeatedConditionParams import RepeatedConditionParams
from TrigHLTJetHypo.FilterParams import FilterParams
from TrigHLTJetHypo.HelperConfigToolParams import HelperConfigToolParams
from TrigHLTJetHypo.ConditionDefaults import defaults
from TrigHLTJetHypo.make_treevec import make_treevec

import re


pattern_pt_threshold = r'^MULT(?P<multlo>\d+)mult(?P<multhi>\d+)'\
    r'(XX(?P<ptlo>\d*)pt(?P<pthi>\d*))?'

pattern_et_threshold = r'^MULT(?P<multlo>\d+)mult(?P<multhi>\d+)'\
    r'(XX(?P<etlo>\d*)et(?P<ethi>\d*))?'

pattern_common = r'(XX(?P<etalo>\d*)eta(?P<etahi>\d*))?'

pattern_ptfull = pattern_pt_threshold + pattern_common
rgx_pt         = re.compile(pattern_ptfull)

pattern_etfull = pattern_et_threshold + pattern_common
rgx_et         = re.compile(pattern_etfull)

def get_conditionfilter_args_from_matchdict(groupdict, threshold_var):
    """ Extract the arguments used by the filter of the MULT condition
    from the dictionary greated during ghe regex matching to the sceanario
    string. THESE CHAINS HAS DEFAULT CONDITION FILTERING"""
    
    # care! if et no match, etlo and etahi are None.
    #       if et match, missing etlo, ethi = '' 
    # same for eta

    if groupdict[threshold_var+'lo'] is None:  # then default filtering for threshold_var
        groupdict[threshold_var+'lo'] = ''
        groupdict[threshold_var+'hi'] = ''   # will be assigned default value

    if groupdict['etalo'] is None:  # then default filtering for eta
        groupdict['etalo'] = ''  # will be assigned default value
        groupdict['etahi'] = ''

    condargs = []
    vals = defaults(threshold_var,
                    groupdict[threshold_var+'lo'],
                    groupdict[threshold_var+'hi'])
    condargs.append((threshold_var, vals))
        
    vals = defaults('eta',
                    groupdict['etalo'],
                    groupdict['etahi'])
    condargs.append(('eta', vals))

    return condargs


def scenario_mult(scenario, chainPartInd):
    """calculate the parameters needed to generate a hypo helper config AlgTool
    starting from a the hypoScenario which appears in the chainname for
    an MULT condition. The MULT condition is filtered"""

    if not scenario.startswith('MULT'):
       raise ValueError( f'routing error, module {__name__}: bad scenario {scenario}')

    threshold_var = 'pt'
    m = rgx_pt.match(scenario)
    if m is None:
        threshold_var = 'et'
        m = rgx_et.match(scenario)
    groupdict = m.groupdict()

    condargs = []
    vals = defaults('mult',
                    groupdict['multlo'],
                    groupdict['multhi'])

    # find the constructor arguments for each elemental condition
    condargs.append(('mult', vals))
    
    # treeVec is [0, 0] handle non-root nodes here
    repcondargs = [RepeatedConditionParams(tree_id = 1,
                                           tree_pid=0,
                                           chainPartInd=chainPartInd,
                                           condargs=condargs)]

    # get the arguments needed for the MULT condition filter
    filterparams = None
    condargs = get_conditionfilter_args_from_matchdict(groupdict,threshold_var)

    if condargs:  # has default filterinf, so always True

        # make repeated conditions that filter the MULT condition
        repfiltargs = [RepeatedConditionParams(tree_id=1,
                                               tree_pid=0,
                                               condargs=condargs)]
        filterparams = [FilterParams(typename='ConditionFilter',
                                     args=repfiltargs)]
        filterparam_inds = [0]
        
    else:
        filterparams = []
        filterparam_inds = [-1] # no condition filter

    # parameters to initalise the AlgTool that initialises the helper AlgTool

    # treevec[i] gives the tree_id of the parent of the
    # node with tree_id = i
    treevec = make_treevec(repcondargs)
    assert treevec == [0, 0]

    assert len(repcondargs) == len(filterparam_inds)


    helper_params = HelperConfigToolParams(treevec=treevec,
                                           repcondargs=repcondargs,
                                           filterparams=filterparams,
                                           filterparam_inds=filterparam_inds)

    return [helper_params]  # a list with one entry per FastReduction tree

