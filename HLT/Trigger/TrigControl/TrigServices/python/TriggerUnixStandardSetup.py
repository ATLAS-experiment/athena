# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

## @file TriggerUnixStandardSetup.py
## @brief py-module to configure the Athena AppMgr for trigger
## @author Werner Wiedenmann <Werner.Wiedenmann@cern.ch>
###############################################################


def setupCommonServices(flags):
    from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
    CAtoGlobalWrapper(commonServicesCfg, flags)


def commonServicesCfg(flags):
    from AthenaCommon.Constants import INFO
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory

    # set ROOT to batch mode (ATR-21890)
    from PyUtils.Helpers import ROOT6Setup
    ROOT6Setup(batch=True)

    # Basic services
    cfg = ComponentAccumulator()
    cfg.addService(CompFactory.ClassIDSvc(CLIDDBFiles = ['clid.db','Gaudi_clid.db']))

    cfg.addService(CompFactory.AlgContextSvc(BypassIncidents=True))
    cfg.addAuditor(CompFactory.AlgContextAuditor())

    cfg.addService(CompFactory.StoreGateSvc())
    cfg.addService(CompFactory.StoreGateSvc("DetectorStore"))
    cfg.addService(CompFactory.StoreGateSvc("HistoryStore"))
    cfg.addService(CompFactory.StoreGateSvc("ConditionStore"))

    cfg.addService( CompFactory.SG.HiveMgrSvc(
        "EventDataSvc",
        NSlots = flags.Concurrency.NumConcurrentEvents) )

    cfg.addService( CompFactory.AlgResourcePool(
        OutputLevel = INFO,
        TopAlg=["AthSequencer/AthMasterSeq"]) )

    from AthenaConfiguration.MainServicesConfig import AvalancheSchedulerSvcCfg
    cfg.merge( AvalancheSchedulerSvcCfg(flags, maxParallelismExtra=1 if flags.Trigger.enableAsyncIO else 0) )

    # SGCommitAuditor to sweep new DataObjects at end of Alg execute
    cfg.addAuditor( CompFactory.SGCommitAuditor() )

    # CoreDumpSvc
    cfg.addService( CompFactory.CoreDumpSvc(
        CoreDumpStream = "stdout",
        CallOldHandler = False,  # avoid calling e.g. ROOT signal handler
        FastStackTrace = True,   # first produce a fast stacktrace
        StackTrace = True,       # then produce full stacktrace using gdb
        DumpCoreFile = True,     # also produce core file (if allowed by ulimit -c)
        FatalHandler = 0,        # no extra fatal handler
        TimeOut = 120e9) )       # timeout for stack trace generation changed to 120s (ATR-17112,ATR-25404)

    # IOVSvc
    cfg.addService( CompFactory.IOVSvc(
        updateInterval = "RUN",
        preLoadData = True,
        preLoadExtensibleFolders = False,  # ATR-19392
        forceResetAtBeginRun = False) )

    # PerfMon
    if flags.PerfMon.doFastMonMT or flags.PerfMon.doFullMonMT:
        from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
        cfg.merge( PerfMonMTSvcCfg(flags) )

    from TrigServices.TrigServicesConfig import TrigServicesCfg
    cfg.merge( TrigServicesCfg(flags) )

    # ApplicationMgr properties
    cfg.setAppProperty('AuditAlgorithms', True)
    cfg.setAppProperty('InitializationLoopCheck', False)

    return cfg
