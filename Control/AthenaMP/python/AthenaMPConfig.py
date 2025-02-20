# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AllConfigFlags import initConfigFlags, GetFileMD
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format, ProductionStep
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

from AthenaCommon.Logging import log as msg

import os, shutil, uuid


def athenaMPRunArgsToFlags(runArgs, flags):
    """Fill MP configuration flags from run arguments."""
    if hasattr(runArgs, "athenaMPWorkerTopDir"):
        flags.MP.WorkerTopDir = runArgs.athenaMPWorkerTopDir

    if hasattr(runArgs, "athenaMPOutputReportFile"):
        flags.MP.OutputReportFile = runArgs.athenaMPOutputReportFile

    if hasattr(runArgs, "athenaMPCollectSubprocessLogs"):
        flags.MP.CollectSubprocessLogs = runArgs.athenaMPCollectSubprocessLogs

    if hasattr(runArgs, "athenaMPStrategy"):
        flags.MP.Strategy = runArgs.athenaMPStrategy

    if hasattr(runArgs, "athenaMPReadEventOrders"):
        flags.MP.ReadEventOrders = runArgs.athenaMPReadEventOrders

    if hasattr(runArgs, "athenaMPEventOrdersFile"):
        flags.MP.EventOrdersFile = runArgs.athenaMPEventOrdersFile

    if hasattr(runArgs, "athenaMPEventsBeforeFork"):
        flags.MP.EventsBeforeFork = runArgs.athenaMPEventsBeforeFork

    if hasattr(runArgs, "sharedWriter"):
        flags.MP.UseSharedWriter = runArgs.sharedWriter

    if hasattr(runArgs, "sharedReader"):
        flags.MP.UseSharedReader = runArgs.sharedReader

    if hasattr(runArgs, "parallelCompression"):
        flags.MP.UseParallelCompression = runArgs.parallelCompression

    if hasattr(runArgs, "eventService"):
        flags.MP.Strategy = "EventService"

def AthenaMPCfg(flags):

    os.putenv('XRD_ENABLEFORKHANDLERS','1')
    os.putenv('XRD_RUNFORKHANDLER','1')

    result = ComponentAccumulator()

    # Configure MP Event Loop Manager
    mpevtloop = CompFactory.AthMpEvtLoopMgr(EventPrintoutInterval = flags.Exec.EventPrintoutInterval)

    mpevtloop.NWorkers = flags.Concurrency.NumProcs
    mpevtloop.Strategy = flags.MP.Strategy
    mpevtloop.WorkerTopDir = flags.MP.WorkerTopDir
    mpevtloop.OutputReportFile = flags.MP.OutputReportFile
    mpevtloop.CollectSubprocessLogs = flags.MP.CollectSubprocessLogs
    mpevtloop.PollingInterval = flags.MP.PollingInterval
    mpevtloop.MemSamplingInterval = flags.MP.MemSamplingInterval
    mpevtloop.IsPileup = flags.Common.ProductionStep in [ProductionStep.Digitization, ProductionStep.PileUpPresampling, ProductionStep.FastChain] and flags.Digitization.PileUp
    mpevtloop.EventsBeforeFork = 0 if flags.MP.Strategy == 'EventService' else flags.MP.EventsBeforeFork

    # Configure Gaudi File Manager
    filemgr = CompFactory.FileMgr(LogFile="FileManagerLog")
    result.addService(filemgr)

    # Save PoolFileCatalog.xml if exists in the run directory
    # The saved file will be copied over to workers' run directories just after forking
    if os.path.isfile('PoolFileCatalog.xml'):
        shutil.copyfile('PoolFileCatalog.xml','PoolFileCatalog.xml.AthenaMP-saved')

    # Compute event chunk size
    chunk_size = getChunkSize(flags)
    if chunk_size < 1:
        msg.warning('Nonpositive ChunkSize (%i) caught, setting it to 1', chunk_size)
        chunk_size = 1

    # Configure Strategy
    debug_worker = flags.Concurrency.DebugWorkers
    event_range_channel = flags.MP.EventRangeChannel
    use_shared_reader = flags.MP.UseSharedReader
    use_shared_writer = flags.MP.UseSharedWriter
    unique_id = f"{str(os.getpid())}-{uuid.uuid4().hex}"

    if flags.MP.Strategy == 'SharedQueue' or flags.MP.Strategy == 'RoundRobin':
        if use_shared_reader:
            AthenaSharedMemoryTool = CompFactory.AthenaSharedMemoryTool

            if flags.Input.Format is Format.BS:
                evSel = CompFactory.EventSelectorByteStream("EventSelector")

                from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
                bscfg = ByteStreamReadCfg(flags)
                result.merge(bscfg)
            else:
                evSel = CompFactory.EventSelectorAthenaPool("EventSelector")

                inputStreamingTool = AthenaSharedMemoryTool("InputStreamingTool",
                                                            SharedMemoryName=f"InputStream{unique_id}")

                from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
                result.merge(PoolReadCfg(flags))
                from AthenaPoolCnvSvc.PoolCommonConfig import AthenaPoolCnvSvcCfg
                result.merge(AthenaPoolCnvSvcCfg(flags, InputStreamingTool=inputStreamingTool))

            evSel.SharedMemoryTool = AthenaSharedMemoryTool("EventStreamingTool",
                                                            SharedMemoryName=f"EventStream{unique_id}")
            result.addService(evSel)

        if use_shared_writer:
            if any((flags.Output.doWriteESD,
                    flags.Output.doWriteAOD,
                    flags.Output.doWriteDAOD,
                    flags.Output.doWriteRDO)) or flags.Output.HITSFileName!='':
                AthenaSharedMemoryTool = CompFactory.AthenaSharedMemoryTool
                outputStreamingTool = AthenaSharedMemoryTool("OutputStreamingTool",
                                                             SharedMemoryName=f"OutputStream{unique_id}")

                from AthenaPoolCnvSvc.PoolCommonConfig import AthenaPoolCnvSvcCfg
                result.merge(AthenaPoolCnvSvcCfg(flags, OutputStreamingTool=outputStreamingTool))

        if flags.MP.Strategy == 'SharedQueue':
            queue_provider = CompFactory.SharedEvtQueueProvider(UseSharedReader=use_shared_reader,
                                                                IsPileup=mpevtloop.IsPileup,
                                                                EventsBeforeFork=mpevtloop.EventsBeforeFork,
                                                                ChunkSize=chunk_size)

        if flags.Concurrency.NumThreads > 0:
            if mpevtloop.IsPileup:
                raise Exception('Running pileup digitization in mixed MP+MT currently not supported')
            from AthenaConfiguration.MainServicesConfig import AthenaMtesEventLoopMgrCfg
            result.merge(AthenaMtesEventLoopMgrCfg(flags))
            queue_consumer = CompFactory.SharedHiveEvtQueueConsumer(UseSharedWriter=use_shared_writer,
                                                                    EventsBeforeFork=mpevtloop.EventsBeforeFork,
                                                                    Debug=debug_worker)
        else:
            queue_consumer = CompFactory.SharedEvtQueueConsumer(UseSharedReader=use_shared_reader,
                                                                UseSharedWriter=use_shared_writer,
                                                                IsPileup=mpevtloop.IsPileup,
                                                                IsRoundRobin=(flags.MP.Strategy=='RoundRobin'),
                                                                EventsBeforeFork=mpevtloop.EventsBeforeFork,
                                                                ReadEventOrders=flags.MP.ReadEventOrders,
                                                                EventOrdersFile=flags.MP.EventOrdersFile,
                                                                Debug=debug_worker)
        mpevtloop.Tools += [ queue_provider, queue_consumer ]

        if use_shared_writer:
            shared_writer = CompFactory.SharedWriterTool(MotherProcess=(mpevtloop.EventsBeforeFork>0),
                                                         IsPileup=mpevtloop.IsPileup,
                                                         Debug=debug_worker)
            mpevtloop.Tools += [ shared_writer ]

    elif flags.MP.Strategy=='EventService':
        channelScatterer2Processor = "AthenaMP_Scatterer2Processor"
        channelProcessor2EvtSel = "AthenaMP_Processor2EvtSel"

        mpevtloop.Tools += [ CompFactory.EvtRangeScatterer(ProcessorChannel = channelScatterer2Processor,
                                                           EventRangeChannel = event_range_channel,
                                                           DoCaching=flags.MP.EvtRangeScattererCaching) ]
        mpevtloop.Tools += [ CompFactory.EvtRangeProcessor(IsPileup=mpevtloop.IsPileup,
                                                           Channel2Scatterer = channelScatterer2Processor,
                                                           Channel2EvtSel = channelProcessor2EvtSel,
                                                           Debug=debug_worker) ]

        from AthenaServices.OutputStreamSequencerSvcConfig import OutputStreamSequencerSvcCfg
        result.merge(OutputStreamSequencerSvcCfg(flags,incidentName="NextEventRange"))
    else:
        msg.warning("Unknown strategy %s. No MP tools will be configured", flags.MP.Strategy)

    result.addService(mpevtloop, primary=True)

    return result

def getChunkSize(flags) -> int:
    chunk_size = 1
    if flags.MP.ChunkSize > 0:
        chunk_size = flags.MP.ChunkSize
        msg.info('Chunk size set to %i', chunk_size)
    else:
        if '_ATHENA_GENERIC_INPUTFILE_NAME_' in flags.Input.Files:
            msg.info('Running an input-less job. Setting Chunk Size to 1')
        else:
            md = GetFileMD(flags.Input.Files)
            #Don't use auto flush for shared reader
            if flags.MP.UseSharedReader:
                msg.info('Shared Reader in use, chunk_size set to default (%i)', chunk_size)
            #Use auto flush only if file is compressed with LZMA, else use default chunk_size
            elif flags.MP.ChunkSize == -1:
                if md.get('file_comp_alg',-1) == 2:
                    chunk_size = md.get('auto_flush',-1)
                    msg.info('Chunk size set to auto flush (%i)', chunk_size)
                else:
                    msg.info('LZMA algorithm not in use, chunk_size set to default (%i)', chunk_size)
            #Use auto flush only if file is compressed with LZMA or ZLIB, else use default chunk_size
            elif flags.MP.ChunkSize == -2:
                if md.get('file_comp_alg',-1) in [1,2]:
                    chunk_size = md.get('auto_flush',-1)
                    msg.info('Chunk size set to auto flush (%i)', chunk_size)
                else:
                    msg.info('LZMA nor ZLIB in use, chunk_size set to default (%i)', chunk_size)
            #Use auto flush only if file is compressed with LZMA, ZLIB or LZ4, else use default chunk_size
            elif flags.MP.ChunkSize == -3:
                if md.get('file_comp_alg',-1) in [1,2,4]:
                    chunk_size = md.get('auto_flush',-1)
                    msg.info('Chunk size set to auto flush (%i)', chunk_size)
                else:
                    msg.info('LZMA, ZLIB nor LZ4 in use, chunk_size set to (%i)', chunk_size)
            #Use auto flush value for chunk_size, regardless of compression algorithm
            elif flags.MP.ChunkSize <= -4:
                chunk_size = md.get('auto_flush',-1)
                msg.info('Chunk size set to auto flush (%i)', chunk_size)
            else:
                msg.warning('Invalid ChunkSize, Chunk Size set to default (%i)', chunk_size)

    return chunk_size


if __name__=="__main__":

    # -----------------  Example with input file --------------
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.ESD
    flags.Exec.MaxEvents=10
    flags.Concurrency.NumProcs=2

    cfg = MainServicesCfg(flags)
    from AthenaPoolCnvSvc.PoolReadConfig import EventSelectorAthenaPoolCfg
    cfg.merge(EventSelectorAthenaPoolCfg(flags))
    cfg.run()
    # -----------------  Example with input file --------------

    msg.info('All OK!')
