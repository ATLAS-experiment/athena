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
        # Avoid putting call to parse_args() here! Otherwise it is hard to retrieve the parser infos

    @property
    def cfg(self):
        if self._cfg is None:
            raise ValueError('Service configuration not initialized, use initServiceCfg()')
        return self._cfg

    def addCustomArguments(self):
        # add arguments here
        derivedGroup = self.parser.add_argument_group('Athena specific arguments')
        derivedGroup.add_argument('--pool-file-reading', dest='pool_file_reading',
                                 action='store_true', help='Run the job with the POOL-based file reading')
        return

    def makeAlgSequence(self):
        from AthenaConfiguration.ComponentFactory import CompFactory
        algSeq = CompFactory.AthSequencer()
        self.logger.info("Configuring algorithms based on YAML file")
        configSeq =  self.config.configure()
        self.logger.info("Configuring common services")
        from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator
        configAccumulator = ConfigAccumulator(autoconfigFromFlags=self.flags,
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
        self.flags.lock()
        self.printFlags()

        self.initServiceCfg()
        if self.args.pool_file_reading:
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

        self.cfg.merge(self.makeAlgSequence())
        self.cfg.printConfig()

        sc = self.cfg.run(self.flags.Exec.MaxEvents)
        sys.exit(sc.isFailure())
