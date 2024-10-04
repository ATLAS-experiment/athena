#!/usr/bin/env python
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''@file dumpTruth.py
@author RD Schaffer
@date 2023-08-04
@brief Script to print out truth events for HepMC (HITS, RDO) or TruthEvent (AOD)
'''

if __name__=='__main__':
    # from AthenaCommon.Constants import INFO
    from argparse import RawTextHelpFormatter
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    parser = flags.getArgumentParser(description='Run ASCII dumper of Truth on files with either HepMC or TruthEvent format.\
    User MUST provide file(s) to read (--filesInput).\
    Also, use --doPtEtaPhi for pt/eta/phi printout, and --skipEvents, --evtMax to select events.')
    parser.add_argument('--doPUEventPrintout', action='store_true',
                        help='Print out PU event for xAODTruthReader')
    parser.add_argument('--doPtEtaPhi', action='store_true',
                        help='Print out particle 4-mom as pt,eta,phi. Default is px,py,pz.')
    parser.set_defaults(threads=1)
    args, _ = parser.parse_known_args()

    from AthenaCommon.Logging import log

    flags.Input.Files = args.filesInput    
    log.info("Checking collections in the input file")
    collTypes = [ c.split("#")[0] for c in flags.Input.TypedCollections]
    MCCollectionName = None
    TruthCollectionName = None
    getName = lambda t: [ c.split("#")[1] for c in flags.Input.TypedCollections if c.split("#")[0] == t][0]

    if "McEventCollection" in collTypes:
        MCCollectionName = getName("McEventCollection")
        log.info("Found McEventCollection#%s in the input, assuming it is HepMC", MCCollectionName)
    elif "xAOD::TruthEventContainer" in collTypes:
        TruthCollectionName = getName("xAOD::TruthEventContainer")
        log.info("Found xAOD::TruthEventContainer#%s in the input", TruthCollectionName)
    else:
        log.error("Neither McEventCollection or xAOD::TruthEventContainer in the input file, can not dump truth")
        import sys
        sys.exit(1)


    # set up defaults for either HepMCReader or xAODTruthReader
    if MCCollectionName:
        flags.addFlag("HepMCContainerKey", MCCollectionName)
    else:
        flags.addFlag("xAODTruthEventContainerKey", "TruthEvents")
        flags.addFlag("xAODTruthPileupEventContainerKey", "TruthPileupEvents")
        if args.doPUEventPrintout:
            flags.addFlag("DoPUEventPrintout", True)
        else:
            flags.addFlag("DoPUEventPrintout", False)

        # default file name
        flags.Input.Files = ['truth.pool.root']
    if args.doPtEtaPhi:
        flags.addFlag("Do4momPtEtaPhi", True)
    else:
        flags.addFlag("Do4momPtEtaPhi", False)

    flags.fillFromArgs(parser=parser)
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc=MainServicesCfg(flags)
    acc.merge(PoolReadCfg(flags)) 
    if MCCollectionName:
        from xAODTruthCnv.xAODTruthCnvConfig import HepMCTruthReaderCfg
        acc.merge(HepMCTruthReaderCfg(flags))
    else:
        from xAODTruthCnv.xAODTruthCnvConfig import xAODTruthReaderCfg
        acc.merge(xAODTruthReaderCfg(flags))

    acc.store(open("HepMCTruthReader.pkl","wb"))

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, acc)

    statusCode = acc.run()
    assert statusCode.isSuccess() is True, "Application execution did not succeed"
