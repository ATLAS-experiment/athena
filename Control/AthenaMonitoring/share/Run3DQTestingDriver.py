#!/usr/bin/env python
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

'''@file DQTestingDriver.py
@author C. D. Burton
@author P. Onyisi
@date 2019-06-20
@brief Driver script to run DQ with new-style configuration on an ESD/AOD
'''

if __name__=='__main__':
    import sys
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.Enums import Format
    flags = initConfigFlags()
    parser = flags.getArgumentParser()
    parser.add_argument('--preExec', help='Code to execute before locking configs')
    parser.add_argument('--postExec', help='Code to execute after setup')
    parser.add_argument('--dqOffByDefault', action='store_true',
                        help='Set all DQ steering flags to False, user must then switch them on again explicitly')
    # keep for compatibility reasons
    parser.add_argument('--inputFiles',
                        help='Comma-separated list of input files (alias for --filesInput)')
    # keep for compatibility reasons
    parser.add_argument('--maxEvents', type=int,
                        help='Maximum number of events to process (alias for --evtMax)')
    parser.add_argument('--printDetailedConfig', action='store_true',
                        help='Print detailed Athena configuration')

    # change default
    parser.set_defaults(threads=1)
    args, _ = parser.parse_known_args()

    # default input if nothing specified
    flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/AthenaMonitoring/q431/21.0/f946/myESD.pool.root']
    flags.Output.HISTFileName = 'ExampleMonitorOutput.root'    
    if args.dqOffByDefault:
        from AthenaMonitoring.DQConfigFlags import allSteeringFlagsOff
        allSteeringFlagsOff(flags)
    flags.fillFromArgs(parser=parser)

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon import Constants
    if args.loglevel:
        log.setLevel(getattr(Constants, args.loglevel))

    # override Input.Files with result from our own arguments
    # if --filesInput was specified as well (!) this will override
    if args.inputFiles is not None:
        flags.Input.Files = args.inputFiles.split(',')
    # if --evtMax was specified as well this will override
    if args.maxEvents is not None:
        flags.Exec.MaxEvents = args.maxEvents

    if flags.Input.Format is Format.BS:
        if flags.DQ.Environment not in ('tier0', 'tier0Raw', 'online'):
            log.warning('Reading RAW file, but DQ.Environment set to %s',
                        flags.DQ.Environment)
            log.warning('Will proceed but best guess is this is an error')
        log.info('Will schedule reconstruction, as best we know')
    else:
        if flags.DQ.Environment in ('tier0', 'tier0Raw', 'online'):
            log.warning('Reading POOL file, but DQ.Environment set to %s',
                        flags.DQ.Environment)
            log.warning('Will proceed but best guess is this is an error')

    if args.preExec:
        # bring things into scope
        from AthenaMonitoring.DQConfigFlags import allSteeringFlagsOff
        log.info('Executing preExec: %s', args.preExec)
        exec(args.preExec)

    if flags.hasCategory("DQ.Steering") and flags.hasCategory("Detector"):
        if flags.hasCategory("DQ.Steering.InDet"):
            if (flags.hasFlag("DQ.Steering.InDet.doAlignMon") and flags.DQ.Steering.InDet.doAlignMon) or \
               (flags.hasFlag("DQ.Steering.InDet.doGlobalMon") and flags.DQ.Steering.InDet.doGlobalMon) or \
               (flags.hasFlag("DQ.Steering.InDet.doPerfMon") and flags.DQ.Steering.InDet.doPerfMon):
                flags.Detector.GeometryID = True

    # Just assume we want the full ID geometry, if we are reading in geometry
    flags.Detector.GeometryPixel = True
    flags.Detector.GeometrySCT = True
    flags.Detector.GeometryTRT = True

    log.info('FINAL CONFIG FLAGS SETTINGS FOLLOW')
    if args.loglevel is None or getattr(Constants, args.loglevel) <= Constants.INFO:
        flags.dump()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    if flags.Input.Format is Format.BS:
        # attempt to start setting up reco ...
        from CaloRec.CaloRecoConfig import CaloRecoCfg
        cfg.merge(CaloRecoCfg(flags))
    else:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(flags))

    # load DQ
    from AthenaMonitoring.AthenaMonitoringCfg import AthenaMonitoringCfg
    dq = AthenaMonitoringCfg(flags)
    cfg.merge(dq)

    # Force loading of conditions in MT mode
    if flags.Concurrency.NumThreads > 0:
        from AthenaConfiguration.ComponentAccumulator import ConfigurationError
        for condalg, alg in [("PixelDetectorElementCondAlg", "ForceIDConditionsAlg")]:
            try:
                cfg.getCondAlgo(condalg)
            except ConfigurationError:
                pass  # do nothing if the CondAlg is not present
            else:
                beginseq = cfg.getSequence("AthBeginSeq")
                beginseq.Members.append(CompFactory.getComp(alg)())

    # any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    cfg.printConfig(withDetails=args.printDetailedConfig) # set True for exhaustive info

    sc = cfg.run()
    sys.exit(0 if sc.isSuccess() else 1)
