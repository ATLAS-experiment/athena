#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from glob import glob

def get_args():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Parser for IPPerformance configuration')
    #parser.add_argument("--filesInput", help="Input file", default="/eos/atlas/atlascerngroupdisk/perf-idtracking/CTIDEOfficial/CI_samples/DAOD_IDTIDE.28461204._000175.pool.root.1")  #default DAOD_IDTIDE file
    parser.add_argument("--filesInput", help="Input file", default="/eos/home-w/wenjingw/public/athena/run/Data_DAOD/DAOD_IDTIDE.34111069._000147.pool.root.1")
    parser.add_argument("--maxEvents", help="Limit number of events. Default: all input events", default=-1, type=int)
    parser.add_argument("--skipEvents", help="Skip this number of events. Default: no events are skipped", default=0, type=int)
    parser.add_argument("--outputFile", help='Name of output file',default="IPPerformanceHists.root")
    return parser.parse_args()


if __name__=='__main__':

    args = get_args()

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.Input.Files = []
    for path in args.filesInput.split(','):
        flags.Input.Files += glob(path)
    #Does this work? 
    #flags.Output.HISTFileName = args.outputFile


    flags.Exec.SkipEvents = args.skipEvents
    flags.Exec.MaxEvents = args.maxEvents

    flags.lock()
    

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    from IPPerformance.IPPerformanceConfig import IPPerformanceCfg
    acc.merge(IPPerformanceCfg(flags))

    
    acc.printConfig(withDetails=True)

    # Execute and finish
    sc = acc.run()

    # Success should be 0
    import sys
    sys.exit(not sc.isSuccess())
