# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#-----Python imports---#
import os, sys, shutil, uuid

#-----Athena imports---#
from AthenaCommon.Logging import log as msg

from AthenaMP.AthenaMPConf import AthMpEvtLoopMgr
class MpEvtLoopMgr(AthMpEvtLoopMgr):
    def __init__(self, name='AthMpEvtLoopMgr', isPileup=False, **kw):

        from AthenaCommon.AppMgr import theApp
        self.nThreads = theApp._opts.threads

        ## init base class
        kw['name'] = name
        super(MpEvtLoopMgr, self).__init__(**kw)

        os.putenv('XRD_ENABLEFORKHANDLERS','1')
        os.putenv('XRD_RUNFORKHANDLER','1')

        from .AthenaMPFlags import jobproperties as jp
        self.WorkerTopDir = jp.AthenaMPFlags.WorkerTopDir()
        self.OutputReportFile = jp.AthenaMPFlags.OutputReportFile()
        self.CollectSubprocessLogs = jp.AthenaMPFlags.CollectSubprocessLogs()
        self.Strategy = jp.AthenaMPFlags.Strategy()
        self.PollingInterval = jp.AthenaMPFlags.PollingInterval()
        self.MemSamplingInterval = jp.AthenaMPFlags.MemSamplingInterval()
        self.EventsBeforeFork = jp.AthenaMPFlags.EventsBeforeFork()
        self.IsPileup = isPileup

        if self.Strategy=='EventService':
            self.EventsBeforeFork = 0

        from AthenaCommon.AppMgr import theApp as app
        app.EventLoop = self.getFullJobOptName()

        # Enable FileMgr logging
        from GaudiSvc.GaudiSvcConf import FileMgr
        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        svcMgr+=FileMgr(LogFile="FileManagerLog")

        # Save PoolFileCatalog.xml if exists in the run directory
        if os.path.isfile('PoolFileCatalog.xml'):
            shutil.copyfile('PoolFileCatalog.xml','PoolFileCatalog.xml.AthenaMP-saved')

        self.configureStrategy(self.Strategy,self.IsPileup,self.EventsBeforeFork)
        
    def configureStrategy(self,strategy,pileup,events_before_fork) -> None :
        from .AthenaMPFlags import jobproperties as jp
        import AthenaCommon.ConcurrencyFlags # noqa: F401
        event_range_channel = jp.AthenaMPFlags.EventRangeChannel()

        chunk_size = getChunkSize()
        if chunk_size < 1:
            msg.warning('Nonpositive ChunkSize (%i) caught, setting it to 1', chunk_size)
            chunk_size = 1
        
        debug_worker = jp.ConcurrencyFlags.DebugWorkers()
        use_shared_reader = jp.AthenaMPFlags.UseSharedReader()
        use_shared_writer = jp.AthenaMPFlags.UseSharedWriter()
        use_parallel_compression = jp.AthenaMPFlags.UseParallelCompression()
        unique_id = f"{str(os.getpid())}-{uuid.uuid4().hex}"

        # For e.g. event generation, if we use SharedQueue the job does not complete correctly
        if strategy == 'SharedQueue':
            from AthenaCommon.AthenaCommonFlags import jobproperties as ajp
            if (not ajp.AthenaCommonFlags.FilesInput.statusOn) or ajp.AthenaCommonFlags.FilesInput == []:
                msg.info('MP strategy "SharedQueue" will not work without input files when maxEvents=-1. Switching to "RoundRobin" just in case')
                strategy = 'RoundRobin'

        if strategy=='SharedQueue' or strategy=='RoundRobin':
            if use_shared_reader:
                from AthenaCommon.AppMgr import ServiceMgr as svcMgr
                svcMgr.PoolSvc.MaxFilesOpen = 2
                from AthenaIPCTools.AthenaIPCToolsConf import AthenaSharedMemoryTool
                svcMgr.EventSelector.SharedMemoryTool = AthenaSharedMemoryTool("EventStreamingTool", SharedMemoryName=f"EventStream{unique_id}")
                if 'AthenaPoolCnvSvc.ReadAthenaPool' in sys.modules:
                    svcMgr.AthenaPoolCnvSvc.InputStreamingTool = AthenaSharedMemoryTool("InputStreamingTool", SharedMemoryName=f"InputStream{unique_id}")
            if use_shared_writer:
                from AthenaCommon.AppMgr import ServiceMgr as svcMgr
                if 'AthenaPoolCnvSvc.WriteAthenaPool' in sys.modules:
                    from AthenaIPCTools.AthenaIPCToolsConf import AthenaSharedMemoryTool
                    svcMgr.AthenaPoolCnvSvc.OutputStreamingTool = AthenaSharedMemoryTool("OutputStreamingTool", SharedMemoryName=f"OutputStream{unique_id}")
                    svcMgr.AthenaPoolCnvSvc.ParallelCompression=use_parallel_compression

            if strategy=='SharedQueue':
                from AthenaMPTools.AthenaMPToolsConf import SharedEvtQueueProvider
                self.Tools += [ SharedEvtQueueProvider(UseSharedReader=use_shared_reader,
                                                       IsPileup=pileup,
                                                       EventsBeforeFork=events_before_fork,
                                                       ChunkSize=chunk_size) ]

            # In pure MP, self.nThreads may be set to None - we want the pure MP setup in that case
            if self.nThreads is not None and self.nThreads >= 1:
                if(pileup):
                    raise Exception('Running pileup digitization in mixed MP+MT currently not supported')
                from AthenaMPTools.AthenaMPToolsConf import SharedHiveEvtQueueConsumer
                self.Tools += [ SharedHiveEvtQueueConsumer(UseSharedWriter=use_shared_writer, 
                                                           EventsBeforeFork=events_before_fork,
                                                           Debug=debug_worker)   ]
            else:
                from AthenaMPTools.AthenaMPToolsConf import SharedEvtQueueConsumer
                self.Tools += [ SharedEvtQueueConsumer(UseSharedReader=use_shared_reader,
                                                       UseSharedWriter=use_shared_writer,
                                                       IsPileup=pileup,
                                                       IsRoundRobin=(strategy=='RoundRobin'),
                                                       EventsBeforeFork=events_before_fork,
                                                       ReadEventOrders=jp.AthenaMPFlags.ReadEventOrders(),
                                                       EventOrdersFile=jp.AthenaMPFlags.EventOrdersFile(),
                                                       Debug=debug_worker)   ]
            if use_shared_writer:
                from AthenaMPTools.AthenaMPToolsConf import SharedWriterTool
                self.Tools += [ SharedWriterTool(MotherProcess=(events_before_fork>0),
                                                 IsPileup=pileup,
                                                 Debug=debug_worker) ]
        elif strategy=='EventService':
            channelScatterer2Processor = "AthenaMP_Scatterer2Processor"
            channelProcessor2EvtSel = "AthenaMP_Processor2EvtSel"

            from AthenaMPTools.AthenaMPToolsConf import EvtRangeScatterer
            self.Tools += [ EvtRangeScatterer(ProcessorChannel = channelScatterer2Processor,
                                              EventRangeChannel = event_range_channel,
                                              DoCaching=jp.AthenaMPFlags.EvtRangeScattererCaching()) ]

            from AthenaMPTools.AthenaMPToolsConf import EvtRangeProcessor
            self.Tools += [ EvtRangeProcessor(IsPileup=pileup,
                                              Channel2Scatterer = channelScatterer2Processor,
                                              Channel2EvtSel = channelProcessor2EvtSel,
                                              Debug=debug_worker) ]
        else:
            msg.warning("Unknown strategy. No MP tools will be configured")

def getChunkSize() -> int :
    from .AthenaMPFlags import jobproperties as jp
    from PyUtils.MetaReaderPeeker import metadata
    chunk_size = 1
    # In jobs without input (e.g. event generation), metadata is an empty dictionary
    if (jp.AthenaMPFlags.ChunkSize() > 0):
        chunk_size = jp.AthenaMPFlags.ChunkSize()
        msg.info('Chunk size set to %i', chunk_size)
    elif 'file_size' in metadata and metadata['file_size'] is not None:
        #Don't use auto flush for shared reader
        if (jp.AthenaMPFlags.UseSharedReader()):
            msg.info('Shared Reader in use, chunk_size set to default (%i)', chunk_size)
        #Use auto flush only if file is compressed with LZMA, else use default chunk_size
        elif (jp.AthenaMPFlags.ChunkSize() == -1):
            if (metadata['file_comp_alg'] == 2):
                chunk_size = metadata['auto_flush']
                msg.info('Chunk size set to auto flush (%i)', chunk_size)
            else:
                msg.info('LZMA algorithm not in use, chunk_size set to default (%i)', chunk_size)
        #Use auto flush only if file is compressed with LZMA or ZLIB, else use default chunk_size
        elif (jp.AthenaMPFlags.ChunkSize() == -2):
            if (metadata['file_comp_alg'] == 1 or metadata['file_comp_alg'] == 2):
                chunk_size = metadata['auto_flush']
                msg.info('Chunk size set to auto flush (%i)', chunk_size)
            else:
                msg.info('LZMA nor ZLIB in use, chunk_size set to default (%i)', chunk_size)
        #Use auto flush only if file is compressed with LZMA, ZLIB or LZ4, else use default chunk_size
        elif (jp.AthenaMPFlags.ChunkSize() == -3):
            if (metadata['file_comp_alg'] == 1 or metadata['file_comp_alg'] == 2 or metadata['file_comp_alg'] == 4):
                chunk_size = metadata['auto_flush']
                msg.info('Chunk size set to auto flush (%i)', chunk_size)
            else:
                msg.info('LZMA, ZLIB nor LZ4 in use, chunk_size set to (%i)', chunk_size)
        #Use auto flush value for chunk_size, regarldess of compression algorithm
        elif (jp.AthenaMPFlags.ChunkSize() <= -4):
            chunk_size = metadata['auto_flush']
            msg.info('Chunk size set to auto flush (%i)', chunk_size)
        else:
            msg.warning('Invalid ChunkSize, Chunk Size set to default (%i)', chunk_size)

    return chunk_size
