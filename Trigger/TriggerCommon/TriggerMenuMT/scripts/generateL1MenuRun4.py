#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import argparse
import sys

from AthenaCommon.Logging import logging
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from TrigConfigSvc.TrigConfigSvcCfg import getL1MenuFileName, getBunchGroupSetFileName

from TriggerMenuMT.L1.L1MenuConfigRun4 import L1MenuConfig

def cmdl():
    parser = argparse.ArgumentParser(description='Generate L1 Menu for Run 4',
                                     formatter_class=
                                     lambda prog : argparse.HelpFormatter(prog, max_help_position=40, width=80))
    parser.add_argument("menu", nargs="?", default='Physics_pp_run4_v2',
                        help="the menu to generate [default: %(default)s]")
    parser.add_argument('-l', '--loglevel', metavar='LVL', type=str.upper, default='INFO',
                        choices=['ALL', 'VERBOSE', 'DEBUG', 'INFO', 'WARNING', 'ERROR', 'FATAL'],
                        help='logging level: %(choices)s [default: %(default)s]')
    return parser.parse_args()


def main():

    args = cmdl()

    # set verbosity
    log = logging.getLogger("TriggerMenuMT")
    log.setLevel(args.loglevel)

    flags = initConfigFlags()

    # set L1 menu
    flags.Input.Files = []
    flags.Trigger.triggerMenuSetup = args.menu
    flags.lock()

    # L1 menu generation
    log.info("Generating L1 menu %s", flags.Trigger.triggerMenuSetup)
    l1cfg = L1MenuConfig(flags)
    l1cfg.writeJSON(outputFile    = getL1MenuFileName(flags),
                    bgsOutputFile = getBunchGroupSetFileName(flags))


    return 0


if __name__=="__main__":
    sys.exit( main() )
        
        
