# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""Make chain dicts for testing jet hypo config modules"""

from TriggerMenuMT.HLT.Menu.Physics_pp_run3_v1 import (
    SingleJetGroup,
    MultiJetGroup,
    PhysicsStream,
    SupportLegGroup,
)

DevGroup = ['Development']


from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from TriggerMenuMT.HLT.Config.Utility.DictFromChainName import dictFromChainName

from TrigHLTJetHypo.hypoToolDisplay import hypoToolDisplay

import sys

from pprint import pprint

# from TrigHLTJetHypo.xmlConfig import hypotool_from_chaindict
from TrigHLTJetHypo.hypoConfigBuilder import hypotool_from_chaindict

from AthenaCommon.Logging import logging
from AthenaCommon.Constants import DEBUG
logger = logging.getLogger( __name__)

logger.setLevel(DEBUG)

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()
flags.Input.Files = []
flags.lock()



chains = [

    ChainProp(name='HLT_j45a_pf_ftf_preselj20_L1J15', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J15'], monGroups=['jetMon:t0','jetMon:online','idMon:shifter','caloMon:t0']),

    ChainProp(name='HLT_j0_DIPZ50XX4j20_roiftf_preselj20_L13J35p0ETA23', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MultiJetGroup+SupportLegGroup, monGroups=['jetMon:t0','jetMon:online']),
    ChainProp(name='HLT_j0_DIPZ50XX4j20_MAXMULT20_roiftf_preselj20_L13J35p0ETA23', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MultiJetGroup+SupportLegGroup, monGroups=['jetMon:t0','jetMon:online']),
    ChainProp(name='HLT_4j110_pf_ftf_presel4jDIPZ50X20pt_L13J50', l1SeedThresholds=['FSNOSEED'], stream=[PhysicsStream,'express'], groups=MultiJetGroup+SupportLegGroup, monGroups=['jetMon:t0','jetMon:online']),

    ChainProp(name='HLT_j0_HT400XX15ptXX0eta490_j45a_pf_ftf_preselj20_L1J15', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J15'], monGroups=['jetMon:t0','jetMon:online','idMon:shifter','caloMon:t0']),

 
    ChainProp(name='HLT_j0_HT400XX15ptXX0eta490XXveto_j45a_pf_ftf_preselj20_L1J15', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J15'], monGroups=['jetMon:t0','jetMon:online','idMon:shifter','caloMon:t0']),

    ChainProp(name='HLT_j0_MULT0mult11XX15ptXX0eta490_j45a_pf_ftf_preselj20_L1J15', l1SeedThresholds=['FSNOSEED']*2, stream=[PhysicsStream,'express'], groups=SingleJetGroup+SupportLegGroup+['RATE:CPS_J15'], monGroups=['jetMon:t0','jetMon:online','idMon:shifter','caloMon:t0']),
   ]

def testChainDictMaker(idict):

    if idict >= 0:
        chain_props = [chains[idict]]
    else:
        chain_props = chains

    result = []
    for cp in chain_props:
        logger.debug(cp)
        chain_dict = dictFromChainName(flags, cp)
        result.append((cp.name, chain_dict))

    return result


def list_chains():
    for i, c in enumerate(chains):
        logger.debug('%2d ' + c.name, i)


if __name__ == '__main__':
    import argparse
    import os

    parser = argparse.ArgumentParser()

    parser.add_argument(
        '-i',
        '--iprop',
        help='index of chainProp to run, -1: do all',
        type=int)

    parser.add_argument('-d', '--dot', help='write out a dot file',
                        action='store_true')

    parser.add_argument(
        '--dotdir',
        help='specify directory to which dot file is to be written [$HOME]',
        default=os.environ['HOME'])

   
    args = parser.parse_args()

    if args.iprop is None:
        list_chains()
        sys.exit()

    iprop = args.iprop
    dicts = testChainDictMaker(iprop)

    def order_chainparts(d):
        cdict = d[1]
        # crass "fix" for out of order chainparts
        # these errors probably arise from calling
        # not-quite-correct menu code.
        chain_part_inds = [cp['chainPartIndex'] for cp in cdict['chainParts']]
        fix = chain_part_inds == sorted(chain_part_inds)
        if not fix:
            fix = chain_part_inds[-1] - chain_part_inds[0] == len(chain_part_inds)

            
        if fix:
            cpi = 0
            for cp in cdict['chainParts']:
                cp['chainPartIndex'] = cpi
                cpi += 1
                           

    for d in dicts:
        order_chainparts(d)
        pprint(d)

    do_dot = args.dot
    if args.dotdir:
        do_dot = True
        
    for cn, d in dicts:
        hypo_tool = hypotool_from_chaindict(d)
        hypoToolDisplay(hypo_tool,
                        do_dot=do_dot,
                        dotdir=args.dotdir)

        
