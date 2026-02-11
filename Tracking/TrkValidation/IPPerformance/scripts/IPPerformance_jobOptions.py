#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from glob import glob
import IPPerformance

def get_args():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Parser for IPPerformance configuration')
    #parser.add_argument("--filesInput", required=True, default="/eos/atlas/atlascerngroupdisk/perf-idtracking/CTIDEOfficial/CI_samples/")
    parser.add_argument("--filesInput", help="Input file", default="/eos/atlas/atlascerngroupdisk/perf-idtracking/CTIDEOfficial/CI_samples/DAOD_IDTIDE.28461204._000175.pool.root.1")  #default DAOD_IDTIDE file
    #parser.add_argument("--filesInput", help="Input file", default="/eos/home-w/wenjingw/public/athena/run/mc23_13p6TeV/DAOD_IDTIDE.43036379._000577.pool.root.1")
    #parser.add_argument("--filesInput", help="Input file", default="/afs/cern.ch/user/w/wenjingw/public/athena/run/noIP9/DAOD_IDTIDE_noIP.pool.root")
    #parser.add_argument("--filesInput", help="Input file", default="/eos/home-w/wenjingw/public/condor_output/out_noIP/run481968/0168._SFO-17._0002.root")
    #parser.add_argument("--filesInput", help="Input file", default="/eos/home-w/wenjingw/public/athena/run/DAOD_IDTIDE.43036379._004114.pool.root.1")
    #parser.add_argument("--filesInput", help="Input file", default="/eos/home-w/wenjingw/public/athena/run/Data_DAOD/DAOD_IDTIDE.34111069._000147.pool.root.1") #Test Data file
    #parser.add_argument("--filesInput", help="Input file", default="/eos/home-w/wenjingw/public/athena/run/HITtoDAOD/DAOD_IDTIDE.43036379._000577.pool9.root.1")
    parser.add_argument("--maxEvents", help="Limit number of events. Default: all input events", default=-1, type=int)
    parser.add_argument("--skipEvents", help="Skip this number of events. Default: no events are skipped", default=0, type=int)
    #parser.add_argument("--mergeLargeD0Tracks", help='Consider LRT tracks in the matching', action='store_true', default=False)
    parser.add_argument("--outputFile", help='Name of output file',default="IPPerformanceHists.root")
    #parser.add_argument("--pdgIds", help='List of pdgIds to match', nargs='+', type=int, default=[36,51])
    #parser.add_argument("--vertexContainer", help='SG key of secondary vertex container',default='VrtSecInclusive_SecondaryVertices')
    #parser.add_argument("--truthVertexContainer", help='SG key of truth vertex container',default='TruthVertices')
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

    from AthenaConfiguration.ComponentFactory import CompFactory
    histSvc = CompFactory.THistSvc()
    histSvc.Output += ["MYSTREAM DATAFILE='" + args.outputFile + "' OPT='RECREATE'"]
    acc.addService(histSvc)

    from IPPerformance.IPPerformanceConfig import EventSelectorAlgCfg
    acc.merge(EventSelectorAlgCfg(flags))

    from IPPerformance.IPPerformanceConfig import JetCalibratorCfg
    acc.merge(JetCalibratorCfg(flags, filesInput=args.filesInput))

    from IPPerformance.IPPerformanceConfig import JetSelectorCfg
    acc.merge(JetSelectorCfg(flags))

    from IPPerformance.IPPerformanceConfig import IPNtupleDumperCfg
    acc.merge(IPNtupleDumperCfg(flags))

    #This works   
    #from PhotonVertexSelection.PhotonVertexSelectionConfig import DecoratePhotonPointingAlgCfg
    #acc.merge(DecoratePhotonPointingAlgCfg(flags)
                                        #,
                                        #useLRTTracks = args.mergeLargeD0Tracks,
                                        #TargetPDGIDs = args.pdgIds,
                                        #SecondaryVertexContainer = args.vertexContainer,
                                        #TruthVertexContainer = args.truthVertexContainer
                                        #)
    #)

    acc.printConfig(withDetails=True)

    # Execute and finish
    sc = acc.run()

    # Success should be 0
    import sys
    sys.exit(not sc.isSuccess())
