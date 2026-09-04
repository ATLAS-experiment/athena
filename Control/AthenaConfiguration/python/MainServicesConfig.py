# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import INFO, WARNING


def AppMgrCfg(flags):
    """Top-level CA with TopAlg and ApplicationMgr settings"""

    topSeq = CompFactory.AthSequencer('AthMasterSeq', Sequential=True)
    cfg = ComponentAccumulator(sequence=topSeq)
    cfg.setAsTopLevel()

    # AppMgr properties:
    cfg.setAppProperty('AuditAlgorithms', True)
    cfg.setAppProperty('InitializationLoopCheck', False)
    cfg.setAppProperty('ExtSvcCreates', False)
    cfg.setAppProperty('JobOptionsType', 'NONE')
    cfg.setAppProperty('EvtMax', flags.Exec.MaxEvents)
    cfg.setAppProperty('TopAlg', [topSeq.getFullJobOptName()])
    cfg.setAppProperty('PrintAlgsSequence', flags.Exec.PrintAlgsSequence)
    cfg.setAppProperty('OutputLevel', flags.Exec.OutputLevel)

    if flags.Exec.OutputLevel > INFO:
        # this turns off the AppMgr splash
        cfg.setAppProperty('AppName', '')

    if flags.Exec.StopOnSignal:
        cfg.setAppProperty("StopOnSignal", True)
        cfg.addService(CompFactory.Gaudi.Utils.StopSignalHandler(Signals=flags.Exec.StopOnSignal))

    return cfg


def AvalancheSchedulerSvcCfg(flags, **kwargs):
    kwargs.setdefault("CheckDependencies", flags.Scheduler.CheckDependencies)
    kwargs.setdefault("CheckOutputUsage", flags.Scheduler.CheckOutputUsage)
    kwargs.setdefault("ShowDataDependencies", flags.Scheduler.ShowDataDeps)
    kwargs.setdefault("ShowDataFlow", flags.Scheduler.ShowDataFlow)
    kwargs.setdefault("ShowControlFlow", flags.Scheduler.ShowControlFlow)
    kwargs.setdefault("VerboseSubSlots", flags.Scheduler.EnableVerboseViews)
    kwargs.setdefault("ThreadPoolSize", flags.Concurrency.NumThreads)
    kwargs.setdefault("DataDepsGraphFile", flags.Scheduler.DataDepsGraphFile)
    kwargs.setdefault("DataDepsGraphAlgPattern", flags.Scheduler.DataDepsGraphAlgPattern)
    kwargs.setdefault("DataDepsGraphObjectPattern", flags.Scheduler.DataDepsGraphObjectPattern)
    kwargs.setdefault("NumOffloadThreads", flags.Concurrency.NumOffloadThreads)

    cfg = ComponentAccumulator()
    scheduler = CompFactory.AvalancheSchedulerSvc(**kwargs)
    cfg.addService(scheduler, primary=True)

    from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
    # FailIfNoProxy=False makes it a warning, not an error, if unmet data
    # dependencies are not found in the store.  It should probably be changed
    # to True eventually.
    inputloader_ca = SGInputLoaderCfg(flags, FailIfNoProxy=flags.Input.FailOnUnknownCollections)
    cfg.merge(inputloader_ca, sequenceName="AthAlgSeq")

    # Specifying DataLoaderAlg makes the Scheduler automatically assign
    # all unmet data dependencies to that algorithm.
    if flags.Scheduler.AutoLoadUnmetDependencies:
        scheduler.DataLoaderAlg = inputloader_ca.getPrimary().getName()

    return cfg


def OutputUsageIgnoreCfg(flags, algorithm):
    cfg = ComponentAccumulator()
    if flags.Concurrency.NumThreads > 0 and flags.Scheduler.CheckOutputUsage:
       cfg.merge(AvalancheSchedulerSvcCfg(flags, CheckOutputUsageIgnoreList=[algorithm]))
    return cfg


def AthenaEventLoopMgrCfg(flags):
    cfg = ComponentAccumulator()
    elmgr = CompFactory.AthenaEventLoopMgr(EventPrintoutInterval = flags.Exec.EventPrintoutInterval)
    cfg.setAppProperty('EventLoop', elmgr.name)

    if flags.Input.OverrideRunNumber:
        from AthenaKernel.EventIdOverrideConfig import EvtIdModifierSvcCfg
        elmgr.EvtIdModifierSvc = cfg.getPrimaryAndMerge( EvtIdModifierSvcCfg(flags) )

    if flags.Common.isOverlay:
        if not flags.Overlay.DataOverlay:
            elmgr.RequireInputAttributeList = True
            elmgr.UseSecondaryEventNumber = True

    cfg.addService( elmgr )

    return cfg


def MPIHiveEventLoopMgrCfg(flags):
    """Sets up an MPIHive EventLoopMgr along with it's dependencies"""
    from SQLiteDBSvc.SQLiteDBSvcConfig import SQLiteDBSvcCfg
    cfg = ComponentAccumulator()
    nConcurrentEvents = flags.Concurrency.NumConcurrentEvents
    nThreads = flags.Concurrency.NumThreads

    hivesvc = CompFactory.SG.HiveMgrSvc("EventDataSvc", NSlots=nConcurrentEvents)
    cfg.addService(hivesvc)

    arp = CompFactory.AlgResourcePool(
        TopAlg=["AthMasterSeq"]
    )  # this should enable control flow
    cfg.addService(arp)

    scheduler = cfg.getPrimaryAndMerge(
        AvalancheSchedulerSvcCfg(flags, ThreadPoolSize=nThreads)
    )

    # Log DBs are rank-exclusive so no locking is needed
    cfg.merge(
        SQLiteDBSvcCfg(
            flags, name="LogDBSvc", dbPath="file:mpilog.db?nolock=1"
        )
    )
    cfg.addService(CompFactory.MPIClusterSvc("MPIClusterSvc", LogDatabaseSvc="SQLiteDBSvc/LogDBSvc"))
    elmgr = CompFactory.MPIHiveEventLoopMgr(
        MPIClusterSvc="MPIClusterSvc",
        WhiteboardSvc="EventDataSvc",
        SchedulerSvc=scheduler.getName(),
        FirstEventIndex=flags.Exec.SkipEvents,
    )
    cfg.setAppProperty('EventLoop', elmgr.name)

    from AthenaServices.OutputStreamSequencerSvcConfig import OutputStreamSequencerSvcCfg

    cfg.merge(
        OutputStreamSequencerSvcCfg(
            flags, incidentName="BeginInputFile", reportingOn=False, replaceRangeMode=True
        )
    )
    if flags.Input.OverrideRunNumber:
        from AthenaKernel.EventIdOverrideConfig import EvtIdModifierSvcCfg
        elmgr.EvtIdModifierSvc = cfg.getPrimaryAndMerge(EvtIdModifierSvcCfg(flags)).name

    if flags.Common.isOverlay and not flags.Overlay.DataOverlay:
        elmgr.RequireInputAttributeList = True
        elmgr.UseSecondaryEventNumber = True

    cfg.addService(elmgr)

    return cfg


def AthenaHiveEventLoopMgrCfg(flags):
    cfg = ComponentAccumulator()
    hivesvc = CompFactory.SG.HiveMgrSvc("EventDataSvc",
                                        NSlots = flags.Concurrency.NumConcurrentEvents)
    cfg.addService( hivesvc )

    arp = CompFactory.AlgResourcePool(TopAlg = ["AthMasterSeq"]) #this should enable control flow
    cfg.addService( arp )

    scheduler = cfg.getPrimaryAndMerge(AvalancheSchedulerSvcCfg(flags))

    elmgr = CompFactory.AthenaHiveEventLoopMgr(
        WhiteboardSvc = "EventDataSvc",
        SchedulerSvc = scheduler.getName(),
        EventPrintoutInterval = flags.Exec.EventPrintoutInterval)

    cfg.setAppProperty('EventLoop', elmgr.name)

    if flags.Input.OverrideRunNumber:
        from AthenaKernel.EventIdOverrideConfig import EvtIdModifierSvcCfg
        elmgr.EvtIdModifierSvc = cfg.getPrimaryAndMerge(EvtIdModifierSvcCfg(flags))

    if flags.Common.isOverlay and not flags.Overlay.DataOverlay:
        elmgr.RequireInputAttributeList = True
        elmgr.UseSecondaryEventNumber = True

    cfg.addService( elmgr )

    return cfg


def AthenaMpEventLoopMgrCfg(flags):
    cfg = ComponentAccumulator()
    if flags.Common.isOverlay and not flags.Overlay.DataOverlay:
        # Configure the AthenaEventLoopMgr, which is used by AthMpEvtLoopMgr,
        # but do NOT set it as the main EventLoopMgr:
        elmgr = CompFactory.AthenaEventLoopMgr(
            EventPrintoutInterval = flags.Exec.EventPrintoutInterval,
            RequireInputAttributeList = True,
            UseSecondaryEventNumber = True)
        cfg.addService( elmgr )

    from AthenaMP.AthenaMPConfig import AthenaMPCfg
    mploop = AthenaMPCfg(flags)
    cfg.merge( mploop )

    return cfg


def AthenaMtesEventLoopMgrCfg(flags, mtEs=False, channel=''):
    cfg = ComponentAccumulator()

    hivesvc = CompFactory.SG.HiveMgrSvc("EventDataSvc",
                                        NSlots = flags.Concurrency.NumConcurrentEvents)
    cfg.addService( hivesvc )

    arp = CompFactory.AlgResourcePool(TopAlg = ["AthMasterSeq"]) #this should enable control flow
    cfg.addService( arp )

    scheduler = cfg.getPrimaryAndMerge(AvalancheSchedulerSvcCfg(flags))

    elmgr = CompFactory.AthenaMtesEventLoopMgr(
        WhiteboardSvc = "EventDataSvc",
        SchedulerSvc = scheduler.getName(),
        EventRangeChannel = channel,
        EventPrintoutInterval = flags.Exec.EventPrintoutInterval)

    cfg.setAppProperty('EventLoop', elmgr.name)

    if flags.Input.OverrideRunNumber:
        from AthenaKernel.EventIdOverrideConfig import EvtIdModifierSvcCfg
        elmgr.EvtIdModifierSvc = cfg.getPrimaryAndMerge(EvtIdModifierSvcCfg(flags))

    if flags.Common.isOverlay and not flags.Overlay.DataOverlay:
        elmgr.RequireInputAttributeList = True
        elmgr.UseSecondaryEventNumber = True

    if mtEs:
        from AthenaServices.OutputStreamSequencerSvcConfig import OutputStreamSequencerSvcCfg
        cfg.merge(OutputStreamSequencerSvcCfg(flags,
                                              incidentName="NextEventRange",
                                              reportingOn = True))

    cfg.addService( elmgr )

    return cfg


def PyAthenaEventLoopMgrCfg(flags):
    cfg = ComponentAccumulator()
    cfg.setAppProperty('EventLoop', "PyAthenaEventLoopMgr")
    return cfg


def MessageSvcCfg(flags):
    cfg = ComponentAccumulator()
    defaultLimit = 500

    msgsvc = CompFactory.MessageSvc(
        OutputLevel = flags.Exec.OutputLevel,
        Format = (f"% F%{flags.Common.MsgSourceLength}W%C%6W%R%e%s%8W%R%T %0W%M" if flags.Concurrency.NumThreads>0 else
                  f"% F%{flags.Common.MsgSourceLength}W%C%7W%R%T %0W%M"),
        enableSuppression = flags.Common.MsgSuppression,
        showStats = flags.Common.ShowMsgStats,
        statLevel = WARNING,
        # Disable suppression limit if we are debugging components
        verboseLimit = 0 if flags.Exec.VerboseMessageComponents else defaultLimit,
        debugLimit = 0 if flags.Exec.DebugMessageComponents else defaultLimit,
        infoLimit = 0 if flags.Exec.InfoMessageComponents else defaultLimit,
        warningLimit = 0 if flags.Exec.WarningMessageComponents else defaultLimit,
        errorLimit = 0 if flags.Exec.ErrorMessageComponents else defaultLimit,
    )

    # Temporary to match legacy configuration for serial simulation/digitization/overlay jobs (FIXME)
    from AthenaConfiguration.Enums import ProductionStep
    if flags.Common.ProductionStep not in (ProductionStep.Default,
                                           ProductionStep.Reconstruction,
                                           ProductionStep.Derivation):
        msgsvc.Format = "% F%18W%S%7W%R%T %0W%M"

    cfg.addService(msgsvc)
    return cfg


def addMainSequences(flags, cfg):
    """Add the standard sequences to cfg"""

    topSeqName = cfg.getSequence().name  # usually AthMasterSeq

    # Build standard sequences:
    AthSequencer = CompFactory.AthSequencer
    cfg.addSequence(AthSequencer('AthAlgEvtSeq', Sequential=True, StopOverride=True), parentName=topSeqName)
    cfg.addSequence(AthSequencer('AthOutSeq', StopOverride=True), parentName=topSeqName)

    cfg.addSequence(AthSequencer('AthBeginSeq', Sequential=True), parentName='AthAlgEvtSeq')
    cfg.addSequence(AthSequencer('AthAllAlgSeq', StopOverride=True), parentName='AthAlgEvtSeq')

    athAlgSeq = AthSequencer('AthAlgSeq', IgnoreFilterPassed=True, StopOverride=True, ProcessDynamicDataDependencies=True, ExtraDataForDynamicConsumers=[])
    athCondSeq = AthSequencer('AthCondSeq',StopOverride=True)

    if flags.Concurrency.NumThreads==0:
        # For serial execution, we need the CondAlgs to execute first.
        cfg.addSequence(athCondSeq, parentName='AthAllAlgSeq')
        cfg.addSequence(athAlgSeq, parentName='AthAllAlgSeq')
    else:
        # In MT, the order of execution is irrelevant (determined by data deps).
        # We add the conditions sequence later such that the CondInputLoader gets
        # initialized after all other user Algorithms for MT, so the base classes
        # of data deps can be correctly determined.
        cfg.addSequence(athAlgSeq, parentName='AthAllAlgSeq')
        cfg.addSequence(athCondSeq, parentName='AthAllAlgSeq')

    cfg.addSequence(AthSequencer('AthEndSeq', Sequential=True), parentName='AthAlgEvtSeq')

    # Set up incident firing:
    AthIncFirerAlg = CompFactory.AthIncFirerAlg
    IncidentProcAlg = CompFactory.IncidentProcAlg

    previousPerfmonDomain = cfg.getCurrentPerfmonDomain()
    cfg.flagPerfmonDomain('Incidents')

    cfg.addEventAlgo(AthIncFirerAlg("BeginIncFiringAlg", FireSerial=False, Incidents=['BeginEvent']),
                     sequenceName='AthBeginSeq')

    cfg.addEventAlgo(IncidentProcAlg('IncidentProcAlg1'),
                     sequenceName='AthBeginSeq')

    cfg.addEventAlgo(AthIncFirerAlg('EndIncFiringAlg', FireSerial=False, Incidents=['EndEvent']),
                     sequenceName="AthEndSeq")

    cfg.addEventAlgo(IncidentProcAlg('IncidentProcAlg2'),
                     sequenceName="AthEndSeq")

    # Should be after all other algorithms:
    cfg.addEventAlgo(AthIncFirerAlg('EndAlgorithmsFiringAlg', FireSerial=False, Incidents=['EndAlgorithms']),
                     sequenceName=topSeqName)

    cfg.addEventAlgo(IncidentProcAlg('IncidentProcAlg3'),
                     sequenceName=topSeqName)

    cfg.flagPerfmonDomain(previousPerfmonDomain)


def addEvgenSequences(flags, cfg):
    from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory
    cfg.addSequence(EvgenSequenceFactory(EvgenSequence.Main), parentName="AthAlgSeq")
    cfg.addSequence(EvgenSequenceFactory(EvgenSequence.Generator), parentName=EvgenSequence.Main.value)
    cfg.addSequence(EvgenSequenceFactory(EvgenSequence.Fix), parentName=EvgenSequence.Main.value)
    cfg.addSequence(EvgenSequenceFactory(EvgenSequence.PreFilter), parentName=EvgenSequence.Main.value)
    cfg.addSequence(EvgenSequenceFactory(EvgenSequence.Test), parentName=EvgenSequence.Main.value)
    cfg.addSequence(EvgenSequenceFactory(EvgenSequence.Filter), parentName=EvgenSequence.Main.value)
    cfg.addSequence(EvgenSequenceFactory(EvgenSequence.Post), parentName=EvgenSequence.Main.value)


def MainServicesCfg(flags, createEventLoopMgr=True):
    """Configuration of main services. An appropriate EventLoopMgr is configured (MT/MP/etc).
    If you have configured your own EventLoopMgr set createEventLoopMgr to False."""

    # Set the Python OutputLevel on the root logger
    from AthenaCommon.Logging import log
    log.setLevel(flags.Exec.OutputLevel)

    # Guard incorrect configurations
    if flags.Exec.MPI:
        if flags.Concurrency.NumThreads < 1:
            raise Exception("Erroneous configuration for MPI: "
                            f"Concurrency.NumThreads = {flags.Concurrency.NumThreads}, must be >= 1")
        if flags.Concurrency.NumProcs > 0:
            raise Exception("Erroneous configuration for MPI: "
                            f"Concurrency.NumProcs = {flags.Concurrency.NumProcs}, must be 0")

    if flags.Concurrency.NumThreads > 0 and flags.Concurrency.NumConcurrentEvents==0:
        raise Exception("Requested Concurrency.NumThreads>0 and Concurrency.NumConcurrentEvents==0, "
                        "which will not process events!")

    # Top-level CA with ApplicationMgr settings
    cfg = AppMgrCfg(flags)

    # Main sequences and incident handling:
    addMainSequences(flags, cfg)

    # Basic services:
    cfg.addService(CompFactory.ClassIDSvc(CLIDDBFiles = ['clid.db','Gaudi_clid.db']))

    cfg.addService(CompFactory.AlgContextSvc(BypassIncidents=True))
    cfg.addAuditor(CompFactory.AlgContextAuditor())

    cfg.addService(CompFactory.StoreGateSvc(Dump=flags.Debug.DumpEvtStore))
    cfg.addService(CompFactory.StoreGateSvc("DetectorStore",Dump=flags.Debug.DumpDetStore))
    cfg.addService(CompFactory.StoreGateSvc("HistoryStore"))
    cfg.addService(CompFactory.StoreGateSvc("ConditionStore",Dump=flags.Debug.DumpCondStore))

    cfg.merge(MessageSvcCfg(flags))

    from AthenaServices.FPEAndCoreDumpConfig import FPEAndCoreDumpCfg
    cfg.merge(FPEAndCoreDumpCfg(flags))

    if flags.Debug.NameAuditor:
        cfg.addAuditor(CompFactory.NameAuditor())

    # Avoid stack traces to the exception handler. These traces
    # aren't very useful since they just point to the handler, not
    # the original bug.
    cfg.addService(CompFactory.ExceptionSvc(Catch="NONE"))

    # Miscellaneous environment settings.
    # This includes fixing the cache sizes that Eigen assumes, so that
    # operations on large matrices will give identical results across
    # hardware with differing cache sizes.
    cfg.addService(CompFactory.AthEnvironmentSvc(), create=True)

    if flags.Exec.DebugStage != "":
        cfg.setDebugStage(flags.Exec.DebugStage)

    cfg.interactive = flags.Exec.Interactive

    # Timeout
    if flags.Exec.EventTimeOut > 0:
        timeoutAlg = CompFactory.TimeoutAlg(
            Timeout = flags.Exec.EventTimeOut,
            AbortJob = True,
            DumpSchedulerState = False)
        cfg.addEventAlgo(timeoutAlg, sequenceName='AthBeginSeq')

    # Configure EventLoopMgr:
    if flags.Exec.Interactive == "run":
        cfg.merge(PyAthenaEventLoopMgrCfg(flags))
        log.info("Interactive mode, switching to PyAthenaEventLoopMgr")

    elif createEventLoopMgr is False:
        pass  # the user will have to configure one

    elif flags.Concurrency.NumProcs > 0:
        cfg.merge(AthenaMpEventLoopMgrCfg(flags))

    elif flags.Concurrency.NumThreads > 0:
        # Setup SGCommitAuditor to sweep new DataObjects at end of Alg execute
        cfg.addAuditor( CompFactory.SGCommitAuditor() )

        if flags.Exec.MTEventService:
            cfg.merge(AthenaMtesEventLoopMgrCfg(flags,True,flags.Exec.MTEventServiceChannel))
        elif flags.Exec.MPI:
            cfg.merge(MPIHiveEventLoopMgrCfg(flags))
        else:
            cfg.merge(AthenaHiveEventLoopMgrCfg(flags))

    elif flags.Concurrency.NumThreads == 0:
        cfg.merge(AthenaEventLoopMgrCfg(flags))

    # Performance monitoring and profiling:
    if flags.PerfMon.doFastMonMT or flags.PerfMon.doFullMonMT:
        from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
        cfg.merge(PerfMonMTSvcCfg(flags))

    if flags.PerfMon.doGPerfProf:
        from PerfMonGPerfTools.GPT_ProfilerServiceConfig import GPT_ProfilerServiceCfg
        cfg.merge(GPT_ProfilerServiceCfg(flags))

    if len(flags.PerfMon.Valgrind.ProfiledAlgs)>0:
        from Valkyrie.ValkyrieConfig import ValgrindServiceCfg
        cfg.merge(ValgrindServiceCfg(flags))

    return cfg


def MainEvgenServicesCfg(flags, withSequences=True):
    """ComponentAccumulator-based equivalent of:
    import AthenaCommon.AtlasUnixGeneratorJob

    NB Must have set flags.Input.RunNumbers and
    flags.Input.TimeStamps before calling to avoid
    attempted auto-configuration from an input file.
    """
    cfg = MainServicesCfg(flags)
    if not flags.Input.Files:
        from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
        cfg.merge(McEventSelectorCfg(flags))

    if withSequences:
        addEvgenSequences(flags, cfg)

    return cfg


def JobOptionsDumpCfg(flags, fileName="JobOptsConfig.txt"):
    """Job options service configuration - mainly to dump the config."""
    acc = ComponentAccumulator()
    acc.addService(CompFactory.JobOptionsSvc(DUMPFILE=fileName))
    return acc


if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    try:
        flags.Input.RunNumbers = [284500] # Set to either MC DSID or MC Run Number
        flags.Input.TimeStamps = [1] # dummy value
        cfg = MainEvgenServicesCfg(flags, withSequences=True)
    except ModuleNotFoundError:
        #  The McEventSelector package required by MainEvgenServicesCfg is not part of the AthAnalysis project
        cfg = MainServicesCfg(flags)
    cfg.wasMerged()   # to avoid errror that CA was not merged
