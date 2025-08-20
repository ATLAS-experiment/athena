# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TrigHLTJetHypo.RepeatedConditionParams import RepeatedConditionParams
from TrigHLTJetHypo.HelperConfigToolParams import HelperConfigToolParams
from TrigHLTJetHypo.ConditionDefaults import defaults
from TrigHLTJetHypo.make_treevec import make_treevec

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)


# make a list of all possible cut items for the simple scenario
all_elemental_keys = ('etaRange', 'nnJvt', 'jvt', 'smc',
                      'threshold', 'momCuts', 'bsel', 'gntau', 'uht1tau',
                      'clrsel','pileuprm', 'timing', 'timeSig')

# Extract moment cuts
def _cuts_from_momCuts(momCuts):
    separator = 'XX'
    args      = momCuts.split(separator)
    if len(args) > 1:
        return args
    return ''

def get_condition_args_from_chainpart(cp):
    # determine which cut variable are present in this chain part,
    # and find their chain part values
    elemental_keys = [k for k in all_elemental_keys if k  in cp]
    cp_elemental_args = {k : cp[k] for k in elemental_keys if cp[k]}

    # remove place holders
    todelete = []
    for k, v in cp_elemental_args.items():
        if v == 'nosmc': todelete.append(k)

    for k in todelete: del cp_elemental_args[k]

    # decode the chain part cut values to find the numerical cut values

    condargs = list()

    for k, v in cp_elemental_args.items():
        if k == 'threshold':
            key = 'pt'
            vals = defaults(key, lo=v)
            condargs.append((key, vals))

        if k == 'etaRange':
            key='eta'
            lo, hi = v.split(key)
            vals = defaults(key, lo=lo, hi=hi)
            condargs.append((key, vals))

        if k == 'smc':
            key = 'smc'
            lo, hi = v.split(key)
            vals = defaults(key, lo=lo, hi=hi)
            condargs.append((key, vals))

        # Capitalisation to avoid clash with pure 'jvt' key
        if k == 'nnJvt':
            key    = 'nnJvt'
            values = v.split(key)
            assert values[0]=='','nnJvt condition takes only version post-argument'
            assert values[1].startswith('v'),'nnJvt condition expects version as post-argument'

            version_to_name = dict(
                v1='NNJvtTrkAugV1',
            )

            vals = {
                'min': '', #not used
                'max': '', #not used
                'nnJvtName': version_to_name[values[1]]
            }

            condargs.append((key, vals))

        if k == 'jvt':
            key    = 'jvt'
            values = v.split(key)
            assert values[1] == '','jvt condition takes only one argument, two were given' # protection when an upper (not supported) cut is requested
            lo   = values[0]
            vals = defaults(key, lo=lo)
            condargs.append((key, vals))

        if k == 'timing':
            key    = 'timing'
            values = v.split(key)
            lo, hi = v.split(key)
            vals = defaults(key, lo=lo, hi=hi)
            condargs.append((key, vals))

        if k == 'timeSig':
            key    = 'timeSig'
            values = v.split(key)
            lo, hi = v.split(key)
            vals = defaults(key, lo=lo, hi=hi)
            condargs.append((key, vals))

        if k =='clrsel':
            key    = 'clrsel'
            values = v.split(key)
            lo   = values[0]
            vals = defaults(key, lo=lo)
            condargs.append((key, vals))

        if k == 'bsel':
            if 'bgnone' in v:
                key = 'bgnone'
                values = v.split(key)
                assert values[1] == '', 'bgn1 condition takes only one argument, two were given'

                gn1_WPs = {
                    '': float('-inf'),
                    "95": -2.0886,
                    "90": -0.7981,
                    "85": 0.4462,
                    "82": 1.0965,
                    "80": 1.4991,
                    "77": 2.0717,
                    "75": 2.4110,
                    "60": 4.5465,
                }

                assert (values[0] in gn1_WPs.keys()),f"The efficiency of the specified GN1 cut \'{v}\' can not be found in the WP dictionary. Please add or remove the WP from the GN1 WP dictionary."

                lo   = gn1_WPs[values[0]]

                vals = {
                    'min': str(lo),
                    'max': '',
                    'cfrac': '0.018',
                    'namePb': 'fastGN120230331_pb',
                    'namePc': 'fastGN120230331_pc',
                    'namePu': 'fastGN120230331_pu',
                    'nameValid': 'TracksForMinimalJetTag_isValid'
                }

                condargs.append((k, vals))

            # Retrained Xbb tagger
            # Has to go before the rest of "bgntwo*"
            elif 'bgntwoxt' in v:
                key = 'bgntwoxt'
                values = v.split(key)
                assert values[1] == '','bgntwoxt condition takes only one argument, two were given'

                #This dictionary maps the bgntwoxt efficiency into the WP cut to be applied to the GN2XTrig output
                gn2xt_WPs = {
                    '':   float('-inf'),
                    '95': -1.35026,
                    '90': -0.25403,
                    '85': 0.50616,
                    '80': 1.10348,
                    '75': 1.57464,
                    '70': 1.96391,
                    '65': 2.29533,
                    '60': 2.58698,
                }

                assert (values[0] in gn2xt_WPs.keys()),f"The efficiency of the specified gn2xt cut \'{v}\' can not be found in the WP dictionary. Please add or remove the WP from the gn2xt WP dictionary."

                lo   = gn2xt_WPs[values[0]]
                vals = {
                    'min': str(lo),
                    'max': '',
                    'cfrac': '0.25',
                    'namePb': 'GN2XTrig_phbb',
                    'namePc': 'GN2XTrig_ptop',
                    'namePu': 'GN2XTrig_pqcd',
                }
                condargs.append((k, vals))

            # GN2x has to go before GN2 in order to avoid ` elif 'bgntwo' in v:` l149 to pass 'bgntwox'
            elif 'bgntwox' in v:
                key = 'bgntwox'
                values = v.split(key)
                assert values[1] == '','bgntwox condition takes only one argument, two were given'

                #This dictionary maps the bdips efficiency into the WP cut to be applied to the DIPS output
                gn2x_WPs = {
                    '':   float('-inf'),
                    '79': 3.1077,
                    '86': 2.2998,
                    '91': 1.2486,
                    '96': -0.4298,
                }

                assert (values[0] in gn2x_WPs.keys()),f"The efficiency of the specified gn2x cut \'{v}\' can not be found in the WP dictionary. Please add or remove the WP from the gn2x WP dictionary."

                lo   = gn2x_WPs[values[0]]
                vals = {
                    'min': str(lo),
                    'max': '',
                    'cfrac': '0.25',
                    'namePb': 'GN2Xv01_phbb',
                    'namePc': 'GN2Xv01_ptop',
                    'namePu': 'GN2Xv01_pqcd',
                }
                condargs.append((k, vals))

            elif 'bgntwo' in v:
                key = 'bgntwo'
                values = v.split(key)
                assert values[1] == '', 'bgn2 condition takes only one argument, two were given'

                gn2_WPs = {
                    '': float('-inf'),
                    "95": -2.432,
                    "90": -1.351,
                    "85": -0.150,
                    "82": 0.5105,
                    "80": 0.931,
                    "77": 1.471,
                    "75": 1.832,
                    "60": 4.054,
                }

                assert (values[0] in gn2_WPs.keys()),f"The efficiency of the specified GN2 cut \'{v}\' can not be found in the WP dictionary. Please add or remove the WP from the GN2 WP dictionary."

                lo   = gn2_WPs[values[0]]

                vals = {
                    'min': str(lo),
                    'max': '',
                    'cfrac': '0.018',
                    'namePb': 'fastGN220240122_pb',
                    'namePc': 'fastGN220240122_pc',
                    'namePu': 'fastGN220240122_pu',
                    'nameValid': 'TracksForMinimalJetTag_isValid'
                }

                condargs.append((k, vals))

            elif 'bdips' in v:
                key = 'bdips'
                values = v.split(key)
                assert values[1] == '','bdips condition takes only one argument, two were given' # protection when an upper (not supported) cut is requested

                #This dictionary maps the bdips efficiency into the WP cut to be applied to the DIPS output
                dips_WPs = {
                    '':   float('-inf'),
                    '95': -1.6495,
                    '90': -0.8703,
                    '85': -0.0295,
                    '82': 0.4560,
                    '80': 0.7470,
                    '77': 1.1540,
                    '75': 1.4086,
                    '60': 3.0447,
                }

                assert (values[0] in dips_WPs.keys()),f"The efficiency of the specified dips cut \'{v}\' can not be found in the WP dictionary. Please add or remove the WP from the dips WP dictionary."

                lo   = dips_WPs[values[0]]
                vals = {
                    'min': str(lo),
                    'max': '',
                    'cfrac': '0.018',
                    'namePb': 'fastDips_pb',
                    'namePc': 'fastDips_pc',
                    'namePu': 'fastDips_pu',
                    'nameValid': 'TracksForMinimalJetTag_isValid'
                }
                condargs.append((k, vals))

            else:
                raise ValueError(f'btagger {v} not supported')

        # for the love of all that is whole could we turn these into functions
        # instead of copy/paste?!
        elif k == "gntau":
            assert k in v
            values = v.split(k)
            assert values[1] == '' , 'gntau condition takes only one argument, two were given' # protection when an upper (not supported) cut is requested

            gntau_WPs = \
                { '':   float('-inf')
                , '90': -0.846
                , '85': 0.048
                , '80': 0.693
                , '75': 1.229
                }

            assert (values[0] in gntau_WPs.keys()),f"The efficiency of the specified gntau cut \'{v}\' can not be found in the WP dictionary. Please add or remove the WP from the gntau WP dictionary."

            lo   = gntau_WPs[values[0]]
            vals = {
                'min': str(lo),
                'max': '',
                'namePtau': 'fastGNTau20240216_ptau',
                'namePu': 'fastGNTau20240216_pu',
                'nameValid': 'TracksForMinimalJetTag_isValid'
            }
            condargs.append((k, vals))

            log.info("Adding HLT preselection: gntau %s", values[0])

        elif k == "uht1tau":
            assert k in v
            values = v.split(k)
            assert values[1] == '' , 'uht1tau condition takes only one argument, two were given' # protection when an upper (not supported) cut is requested

            uht1tau_WPs = \
                { '':   float('-inf')
                , '90': 0.556
                , '87': 1.379
                , '85': 1.936
                , '82': 2.752
                , '80': 3.259
                }

            assert (values[0] in uht1tau_WPs.keys()),f"The efficiency of the specified uht1tau cut \'{v}\' can not be found in the WP dictionary. Please add or remove the WP from the gntau WP dictionary."

            lo   = uht1tau_WPs[values[0]]
            vals = {
                'min': str(lo),
                'max': '',
                'namePtau': 'fastUHT120250605_ptau',
                'namePu': 'fastUHT120250605_pu',
                'nameValid': 'TracksForMinimalJetTag_isValid'
            }
            condargs.append((k, vals))

            log.info("Adding HLT preselection: uht1tau %s", values[0])


        elif k == 'momCuts':
            from TrigHLTJetHypo.FastReductionAlgToolFactory import jetMoments
            if 'XX' in v: # several moment cuts are requested

                # loop over requested moment strings
                for cutstr in _cuts_from_momCuts(v):
                    for moment in jetMoments: # loop over possible jet moments
                        if moment in cutstr:
                            key='mom{}'.format(moment)
                            lo, hi = cutstr.split(key)
                            vals   = defaults(k, lo=lo, hi=hi)
                            vals["moment"] = jetMoments[moment]
                            condargs.append((key, vals))
            else: # a single moment cut is requested
                for moment in jetMoments: # loop over possible jet moments
                    if moment in v:
                        key='mom{}'.format(moment)
                        lo, hi = v.split(key)
                        vals   = defaults(k, lo=lo, hi=hi)
                        vals["moment"] = jetMoments[moment]
                        condargs.append((key, vals))

        elif k =='pileuprm':
            key    = 'pileuprm'
            values = v.split(key)
            if "n" in values[0]:
                 lo=values[0].replace("n","-",1)
            if "n" in values[1]:
                 hi=values[1].replace("n","-",1)
            vals =  defaults(key, lo=lo, hi=hi)
            condargs.append((key, vals))

    return condargs


def scenario_simple(chain_parts):
    """build two lists of RepeatedConditionConfig objects
    one list contains the Conditions to be used by FastReducer,
    and the other Contains the conditions used to filter the Condition.
    Each list has one entry per chain part"""

    assert chain_parts, 'routing error, module %s: no chain parts' % __name__

    repcondargs = []
    filterparams = []
    filterparam_inds = []

    ncp = 0

    # keep track of identical cond_args, which are given the same
    # clique number.
    # the C++ code will use the clique number for optimisation of
    # the calculation of the combinations
    #
    # current implementation: condition filter not included.
    clique_list = []
    for cp in chain_parts:
        ncp += 1

        # get the Condition parameters (cut values) for the
        # elemental Conditions

        condargs = get_condition_args_from_chainpart(cp)

        multiplicity = int(cp['multiplicity'])
        chainPartInd = cp['chainPartIndex']

        # no condition filtering
        filterparam_inds.append(-1) # no Condition filter

        clique = None
        try:
            clique = clique_list.index(condargs)
        except ValueError:
            # seen for the first time
            clique_list.append(condargs)
            clique = len(clique_list)-1

        repcondargs.append(RepeatedConditionParams(tree_id = ncp,
                                                   tree_pid=0,
                                                   clique=clique,
                                                   chainPartInd=chainPartInd,
                                                   multiplicity=multiplicity,
                                                   condargs=condargs))


    # treevec[i] gives the tree_id of the parent of the
    # node with tree_id = i
    treevec = make_treevec(repcondargs)
    assert treevec == [0 for i in range(len(chain_parts) + 1)]

    assert len(repcondargs) == len(filterparam_inds)
    helper_params = HelperConfigToolParams(treevec=treevec,
                                           repcondargs=repcondargs,
                                           filterparams=filterparams,
                                           filterparam_inds=filterparam_inds)

    return [helper_params]  # a list is one entry per FastReduction tree
