#!/usr/bin/env python3
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""
Dumps the trigger menu, optionally running some checks for name
consistency.
"""
# help strings
_h_names = "print the names of all chains"
_h_parse = "parse names to dictionary"
_h_menu = "name of the menu to dump (default Physics_pp_run3_v1)"
_h_l1check = "do check of L1 items vs L1 menu"
_h_stream = "filter by stream"
_h_dump_dicts = "dump dicts to json"
_h_check_CPS = "verify that Support chains are in CPS"

import sys
from AthenaCommon.Logging import logging

# hack to turn off logging on imports
logging.INFO = logging.DEBUG

from TriggerMenuMT.HLT.Config.Utility.DictFromChainName import dictFromChainName

import importlib

MENU_ALIASES = {
    'dev': 'Dev_pp_run3_v1',
    'hi': 'PhysicsP1_HI_run3_v1',
    'mc': 'MC_pp_run3_v1'
}

def get_parser(flags):
    aliases = '\n'.join(f'{a} -> {f}' for a, f in MENU_ALIASES.items())
    epi='menu aliases:\n' + aliases
    parser = flags.getArgumentParser(
        description=__doc__, epilog=epi,
    )
    parser.add_argument('-m', '--menu',
                        default='Physics_pp_run3_v1',
                        help=_h_menu)
    output = parser.add_mutually_exclusive_group()
    output.add_argument('-n', '--names', action='store_true',
                        help=_h_names)
    output.add_argument('-p', '--parse-names', action='store_true',
                        help=_h_parse)
    parser.add_argument('-L', '--check-l1', action='store_true',
                        help=_h_l1check)
    parser.add_argument('-C', '--check-CPS', action='store_true',
                        help=_h_check_CPS)
    parser.add_argument('-s', '--stream', const='Main', nargs='?',
                        help=_h_stream)
    parser.add_argument('-D', '--dump-dicts', action='store_true',
                        help=_h_dump_dicts)
    group = parser.add_mutually_exclusive_group()
    group.add_argument(
        '--primary',
        dest='group',
        action='store_const',
        const='Primary:',
    )
    group.add_argument(
        '--support',
        dest='group',
        action='store_const',
        const='Support:',
    )
    return parser

def run():
    # The Physics menu (at least) depends on a check of the menu name
    # in order to decide if PS:Online chains should be retained.
    # Should do this in a more explicit way
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    parser = get_parser(flags)

    args = flags.fillFromArgs(parser=parser)
    menu_name = MENU_ALIASES.get(args.menu, args.menu)

    # Can't do these without parsing
    if args.check_l1 or args.check_CPS or args.dump_dicts:
        args.parse_names = True

    flags.Trigger.triggerConfig='FILE'
    flags.Input.Files=[]
    flags.Trigger.triggerMenuSetup=menu_name
    flags.lock()

    if args.parse_names:
        from TrigConfigSvc.TrigConfigSvcCfg import generateL1Menu
        generateL1Menu(flags)

    # Import menu by name
    menumodule = importlib.import_module(f'TriggerMenuMT.HLT.Menu.{menu_name}')
    menu = menumodule.setupMenu(menu_name)

    # filter chains
    if args.stream:
        def filt(chain):
            return args.stream in chain.stream
    else:
        def filt(x):
            return True

    if args.group:
        groupstr = args.group
        def filt(x, old=filt):
            if not old(x):
                return False
            for group in x.groups:
                if group.startswith(groupstr):
                    return True
            return False

    chains = chain_iter(menu, filt)
    if args.names:
        dump_chains(chains)
    elif args.parse_names:
        chain_to_dict, failed = get_chain_dicts(flags, chains)
        if failed:
            sys.exit(1)
        if args.check_l1:
            l1items = get_l1_list(args.menu)
            missingl1 = set()
            for chain, chain_dict in chain_to_dict.items():
                if not chain_dict['L1item']: # Exception for L1All
                    continue
                # Handle comma-separated list for multiseed
                this_l1items = chain_dict['L1item'].split(',')
                for this_l1 in this_l1items:
                    if this_l1 not in l1items:
                        sys.stderr.write(f'L1 item not in menu for HLT item {chain}\n')
                        missingl1.add(chain)
                        break
            if missingl1:
                sys.exit(1)

        if args.check_CPS:
            # Need to regenerate this because we already iterated through
            chains = chain_iter(menu, filt)
            def match_group(expr,chain):
                for group in chain.groups:
                    if expr in group:
                        return True
                return False

            cps_to_chains = {}
            support_L1_counts = {}

            for chain in chains:
                chain_dict = chain_to_dict[chain.name]
                if (
                    not chain_dict['L1item'] # Exception for L1All
                    or not match_group('Support',chain)
                ):
                    continue
                
                # Increment the number of support chains seeded by this L1
                if chain_dict['L1item'] not in support_L1_counts:
                    support_L1_counts[chain_dict['L1item']] = []
                support_L1_counts[chain_dict['L1item']] += 1

                cps_item = None
                for group in chain.groups:
                    if group.startswith('CPS:'):
                        cps_item = group[4:]
                if cps_item not in cps_to_chains:
                    cps_to_chains[cps_item] = []
                cps_to_chains[cps_item] = chain.name

        if args.dump_dicts:
            dump_chain_dicts(chain_to_dict,args.menu)

def chain_iter(menu, filt=lambda x: True):
    for group, chains in menu.items():
        for chain in chains:
            if filt(chain):
                yield chain

def dump_chains(chains):
    try:
        for chain in chains:
            sys.stdout.write(f'{chain.name}\n')
    except BrokenPipeError:
        # this might happen if e.g. you are piping the output
        pass

def get_l1_list(menu):
    from TriggerMenuMT.L1.Menu import MenuMapping
    l1menuname = MenuMapping.menuMap[menu][0]
    l1module = importlib.import_module(f'TriggerMenuMT.L1.Menu.Menu_{l1menuname}')
    l1module.defineMenu()
    return set(l1module.L1MenuFlags.items())

def get_chain_dicts(flags, chains):
    """
    returns map of chain names to dictionaries with a set of failed chains
    """
    # disable even more useless output
    logging.WARNING = logging.DEBUG
    passed = set()
    known_failure = set()
    new_failure = set()
    chain_to_dict = {}
    for chain in chains:
        chain_dict = dictFromChainName(flags, chain)
        name = chain_dict['chainName']
        chain_to_dict[name] = chain_dict
        passed.add(name)
    sys.stdout.write(
        f'Passed: {len(passed)}, Known failures: {len(known_failure)}\n')

    return chain_to_dict, new_failure

def dump_chain_dicts(chain_to_dict,menu):
    import json
    fname = f'dictdump_{menu}.json'
    sys.stdout.write(f'Dumping chain dicts to file "{fname}"')
    fdict = open(fname,'w')
    json.dump(chain_to_dict,fdict,indent=2)
    fdict.close()

if __name__ == '__main__':
    run()
    sys.exit(0)
