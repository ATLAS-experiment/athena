#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import sys

def main():
    # Prevent usage of legacy job properties
    from AthenaCommon import JobProperties
    JobProperties.jobPropertiesDisallowed = True

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    parser = flags.getArgumentParser()
    parser.add_argument("menu", nargs="?", default='PhysicsP1_pp_run3_v1',
                        help="the menu to generate [%(default)s]")
    parser.add_argument("--bgrp", action="store_true",
                        help="generate default MC bunchgroup")
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="increase output verbosity")
    args = flags.fillFromArgs(parser=parser)

    # set menu
    flags.Input.Files = []
    flags.Trigger.triggerMenuSetup = args.menu
    flags.lock()

    # set verbosity
    if args.verbose:
        from AthenaCommon.Logging import logging
        logging.getLogger("TriggerMenuMT").setLevel(logging.DEBUG)

    # Bunchgroup generation
    if args.bgrp:
        from TriggerMenuMT.L1.Base.Limits import Limits
        from TriggerMenuMT.L1.Base.BunchGroupSet import createDefaultBunchGroupSet
        Limits.setLimits(CTPVersion=4)
        bgs = createDefaultBunchGroupSet(flags)
        bgs.writeJSON(outputFile = "L1BunchGroupSet.json")
    else:
        # L1 menu generation
        from TrigConfigSvc.TrigConfigSvcCfg import generateL1Menu
        generateL1Menu(flags)

    return 0

if __name__=="__main__":
    sys.exit( main() )
        
        
