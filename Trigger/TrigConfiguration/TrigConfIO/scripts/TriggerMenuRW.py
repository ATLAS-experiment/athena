#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TrigConfIO.TriggerConfigAccessBase import getFileType, ConfigType
from TrigConfIO.L1TriggerConfigAccess import L1MenuAccess, L1PrescalesSetAccess, BunchGroupSetAccess
from TrigConfIO.HLTTriggerConfigAccess import HLTMenuAccess, HLTPrescalesSetAccess, HLTJobOptionsAccess, HLTMonitoringAccess
from AthenaCommon.Logging import logging
log = logging.getLogger('TriggerMenuRW.py')

def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("-v",       help="increase output verbosity", action="count", default=0)
    parser.add_argument("-f", "--file",   dest="filename", help="json file (menu, prescale set, or bunch group set)")
    parser.add_argument("--db",     dest="dbalias", help="TriggerDB connection alias")
    parser.add_argument("--smk",    type = int, help="SuperMaster key (when accessing menu or job options")
    parser.add_argument("--l1psk",  type = int, help="L1 PrescalesSet key (when accessing L1 prescales")
    parser.add_argument("--hltpsk", type = int, help="HLT PrescalesSet key (when accessing L1 prescales")
    parser.add_argument("--bgsk",   type = int, help="BunchGroupSet key (when accessing bunch groups")
    parser.add_argument("--monk",   type = int, help="Monitoring Groups key (when accessing  monitoring groups)")
    parser.add_argument("--print",  dest="doPrint", help="Prints the loaded information", action="store_true", default = False)
    parser.add_argument("-w", "--write",  dest="doWrite", help="Writes the loaded information to a file", action="store_true", default = False)
    parser.add_argument("-l", "--loglevel",  dest="loglevel", help="Log level [DEBUG, INFO, WARNING] (default INFO)", default = "INFO")
    parser.add_argument("--use-crest",  dest="useCrest", action="store_true",
                        help="Uses CREST to access the TriggerDB")
    parser.add_argument("--crest-server", dest="crestServer", type=str, default="http://crest.cern.ch/api-v5.0", 
                        help="CREST server (default: http://crest.cern.ch/api-v5.0)")
    
    args = parser.parse_args()

    logging.getLogger('TriggerConfigAccessBase.py').setLevel(args.loglevel)
    logging.getLogger('L1TriggerConfigAccess.py').setLevel(args.loglevel)
    logging.getLogger('HLTTriggerConfigAccess.py').setLevel(args.loglevel)
    log.setLevel(args.loglevel)


    if args.filename:
        filetype = getFileType(args.filename)
        if filetype == ConfigType.L1MENU.filetype:
            cfg = L1MenuAccess( filename = args.filename )
        elif filetype == ConfigType.L1PS.filetype:
            cfg = L1PrescalesSetAccess( filename = args.filename )
        elif filetype == ConfigType.BGS.filetype:
            cfg = BunchGroupSetAccess( filename = args.filename )
        elif filetype == ConfigType.HLTMENU.filetype:
            cfg = HLTMenuAccess( filename = args.filename )
        elif filetype == ConfigType.HLTPS.filetype:
            cfg = HLTPrescalesSetAccess( filename = args.filename )
        elif filetype == ConfigType.HLTJO.filetype:
            cfg = HLTJobOptionsAccess( filename = args.filename )
        elif filetype == ConfigType.HLTMON.filetype:
            cfg = HLTMonitoringAccess( filename = args.filename )
        else:
            log.error("Can't read file %s of unknown filetype '%s'", args.filename, filetype)
            return 1
    elif args.dbalias:
        cfg = []
        if args.smk:
            cfg += [ L1MenuAccess( dbalias = args.dbalias, smkey = args.smk, useCrest = args.useCrest, crestServer = args.crestServer) ]
            cfg += [ HLTMenuAccess( dbalias = args.dbalias, smkey = args.smk, useCrest = args.useCrest, crestServer = args.crestServer) ]
            cfg += [ HLTJobOptionsAccess( dbalias = args.dbalias, smkey = args.smk, useCrest = args.useCrest, crestServer = args.crestServer) ]
            cfg += [ HLTMonitoringAccess( dbalias = args.dbalias, smkey = args.smk, useCrest = args.useCrest, crestServer = args.crestServer) ]
        if args.l1psk:
            cfg += [ L1PrescalesSetAccess( dbalias = args.dbalias, l1pskey = args.l1psk, useCrest = args.useCrest, crestServer = args.crestServer) ]
        if args.hltpsk:
            cfg += [ HLTPrescalesSetAccess( dbalias = args.dbalias, hltpskey = args.hltpsk, useCrest = args.useCrest, crestServer = args.crestServer) ]
        if args.bgsk:
            cfg += [ BunchGroupSetAccess( dbalias = args.dbalias, bgskey = args.bgsk, useCrest = args.useCrest, crestServer = args.crestServer) ]
        if args.monk:
            cfg += [ HLTMonitoringAccess( dbalias = args.dbalias, monikey = args.monk, useCrest = args.useCrest, crestServer = args.crestServer) ]
    else:
        print("Either a file or dbalias and key need to be specified")
        return 1

    if args.doPrint:
        if type(cfg) is list:
            for c in cfg:
                c.printSummary()
        else:
            cfg.printSummary()

    if args.doWrite:
        if type(cfg) is list:
            for c in cfg:
                c.writeFile()
        else:
            cfg.writeFile()
        



    return 0

if __name__ == "__main__":
    import sys
    sys.exit(main())
