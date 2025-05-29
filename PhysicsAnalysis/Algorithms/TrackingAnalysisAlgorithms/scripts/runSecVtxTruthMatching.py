#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from glob import glob

def get_args():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Parser for SecVertexTruthMatching configuration')
    parser.add_argument("--filesInput", required=True)
    parser.add_argument("--maxEvents", help="Limit number of events. Default: all input events", default=-1, type=int)
    parser.add_argument("--skipEvents", help="Skip this number of events. Default: no events are skipped", default=0, type=int)
    parser.add_argument("--mergeLargeD0Tracks", help='Consider LRT tracks in the matching', action='store_true', default=False)
    parser.add_argument("--outputFile", help='Name of output file',default="TruthMatchHists.root")
    parser.add_argument("--pdgIds", help='List of pdgIds to match', nargs='+', type=int, default=[36,51,50, 72, 31, 32, 3000001])
    parser.add_argument("--vertexContainer", help='SG key of secondary vertex container',default='VrtSecInclusive_SecondaryVertices')
    parser.add_argument("--truthVertexContainer", help='SG key of truth vertex container',default='TruthVertices')
    parser.add_argument("--useMuSA", help='Use MuSA configuration instead of default', action='store_true', default=False)
    parser.add_argument("--doSMOrigin", help="Turn on optional SM origin matching", action='store_true', default=False)
    return parser.parse_args()

if __name__=='__main__':

    args = get_args()

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.Input.Files = []
    for path in args.filesInput.split(','):
        flags.Input.Files += glob(path)
    flags.Output.HISTFileName = args.outputFile

    flags.Exec.SkipEvents = args.skipEvents
    flags.Exec.MaxEvents = args.maxEvents

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    from TrackingAnalysisAlgorithms.TrackingAnalysisAlgorithmsConfig import SecVertexTruthMatchAlgCfg, SecVertexTruthMatchMuSaAlgCfg

    if args.useMuSA:
        acc.merge(SecVertexTruthMatchMuSaAlgCfg(flags,
                                               TargetPDGIDs = args.pdgIds,
                                               SecondaryVertexContainer = args.vertexContainer,
                                               TruthVertexContainer = args.truthVertexContainer,
                                               doSMOrigin = args.doSMOrigin,
                                              )
        )
    else:
        acc.merge(SecVertexTruthMatchAlgCfg(flags,
                                           useLRTTracks = args.mergeLargeD0Tracks,
                                           TargetPDGIDs = args.pdgIds,
                                           SecondaryVertexContainer = args.vertexContainer,
                                           TruthVertexContainer = args.truthVertexContainer,
                                           doSMOrigin = args.doSMOrigin,
                                          )
        )

    acc.printConfig(withDetails=True)

    # Execute and finish
    sc = acc.run()

    # Success should be 0
    import sys
    sys.exit(not sc.isSuccess())
