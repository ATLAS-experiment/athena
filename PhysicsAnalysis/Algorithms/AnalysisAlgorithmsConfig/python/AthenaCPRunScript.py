# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
import sys
from AnalysisAlgorithmsConfig.CPBaseRunner import CPBaseRunner


class AthenaCPRunScript(CPBaseRunner):
    def __init__(self):
        super().__init__()
        self.logger.info("AthenaCPRunScript initialized")
        self._cfg = None
        self.addCustomArguments()
        self.configSeq = None
        self._algSeqName = None
        # Avoid putting call to parse_args() here! Otherwise it is hard to retrieve the parser infos

    @property
    def cfg(self):
        if self._cfg is None:
            raise ValueError('Service configuration not initialized, use initServiceCfg()')
        return self._cfg

    def addCustomArguments(self):
        # add arguments here
        derivedGroup = self.parser.add_argument_group('Athena specific arguments')
        derivedGroup.add_argument('--config-only', dest='config_only',
                                 action='store_true', help='Only generate the configuration and save it to a pickle file')
        derivedGroup.add_argument('--perfmon', dest='perfmon', default='none',
                                  help='Run PerfMon to measure the job performance')
        derivedGroup.add_argument('--pool-file-reading', dest='pool_file_reading',
                                 action='store_true', help='Run the job with the POOL-based file reading')
        derivedGroup.add_argument('--test-mt-dependencies', dest='test_mt_dependencies',
                                 type=int, default=None,
                                 help='Print out multithreading dependencies, and run with the given number of threads')
        derivedGroup.add_argument('--invert-alg-order', dest='invert_alg_order',
                                 metavar='DEPFILE', default=None,
                                 help='Reorder the analysis algorithm sequence into a '
                                      'maximally-inverted order that still respects the '
                                      'dependencies in the given JSON file (produced by '
                                      'extract_alg_dependencies.py), as a check that the '
                                      'declared dependencies enforce a correct ordering')
        return

    def makeAlgSequence(self):
        from AthenaConfiguration.ComponentFactory import CompFactory
        algSeq = CompFactory.AthSequencer()
        self._algSeqName = algSeq.getName()
        self.logger.info("Configuring algorithms based on YAML file")
        configSeq =  self.config.configure()
        self.logger.info("Configuring common services")
        from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator
        configAccumulator = ConfigAccumulator(flags=self.flags,
                                              algSeq=algSeq,
                                              noSystematics=self.args.no_systematics)
        if not self.args.merge_output_files:
            configAccumulator.setDefaultHistogramStream('ANALYSIS_HIST')
        self.logger.info("Configuring algorithms")
        configSeq.fullConfigure(configAccumulator)
        self.configSeq = configSeq
        self.modifyAlgSequence()
        return configAccumulator.CA

    def initServiceCfg(self):
        if not self.flags.locked():
            raise ValueError('Flags must be locked before initializing services')
        from AthenaConfiguration.MainServicesConfig import MainServicesCfg
        self._cfg = MainServicesCfg(self.flags)

    def run(self):
        self.setup()

        # PerfMon
        from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
        setPerfmonFlagsFromRunArgs(self.flags, self.args)

        if self.args.test_mt_dependencies is not None:
            self.flags.Concurrency.NumThreads = self.args.test_mt_dependencies
            self.flags.Scheduler.ShowControlFlow = True
            self.flags.Scheduler.ShowDataDeps = True
        self.flags.lock()
        self.printFlags()

        self.initServiceCfg()
        if self.args.pool_file_reading or self.args.test_mt_dependencies is not None:
            from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
            self.cfg.merge(PoolReadCfg(self.flags))
        else:
            from AthenaRootComps.xAODEventSelectorConfig import xAODReadCfg
            self.cfg.merge(xAODReadCfg(self.flags))
        from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowSvcCfg
        self.cfg.merge(CutFlowSvcCfg(self.flags))

        outputFile = f"ANALYSIS DATAFILE='{self.outputName}.root' OPT='RECREATE'"
        from AthenaConfiguration.ComponentFactory import CompFactory
        self.cfg.addService(CompFactory.THistSvc(Output=[outputFile]))
        if not self.args.merge_output_files:
            outputFileHist = f"ANALYSIS_HIST DATAFILE='hist-{self.outputName}.root' OPT='RECREATE'"
            from AthenaConfiguration.ComponentFactory import CompFactory
            self.cfg.addService(CompFactory.THistSvc(Output=[outputFileHist]))

        # Make the main analysis configuration
        self.cfg.merge(self.makeAlgSequence())

        # Optionally invert the algorithm order to check that the declared
        # dependencies by themselves enforce a correct ordering.
        if self.args.invert_alg_order:
            from AnalysisAlgorithmsConfig.InvertAlgOrder import invertAlgOrder
            invertAlgOrder(self.cfg, self.args.invert_alg_order,
                           sequenceName=self._algSeqName, logger=self.logger)

        # Performance monitoring and profiling:
        if self.flags.PerfMon.doFastMonMT or self.flags.PerfMon.doFullMonMT:
            from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
            self.cfg.merge(PerfMonMTSvcCfg(self.flags))

        self.cfg.printConfig()

        # dump pickle if requested
        if self.args.config_only:
            with open("CPRunConfig.pkl", "wb") as f:
                self.cfg.store(f)
            sys.exit(0)

        sc = self.cfg.run()
        sys.exit(sc.isFailure())
