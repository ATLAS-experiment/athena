# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
import sys
from AnalysisAlgorithmsConfig.CPBaseRunner import CPBaseRunner


class AthenaCPRunScript(CPBaseRunner):
    def __init__(self):
        super().__init__()
        self.logger.info("AthenaCPRunScript initialized")
        self._cfg = None
        self.addCustomArguments()
        # Avoid putting call to parse_args() here! Otherwise it is hard to retrieve the parser infos
        
    @property
    def cfg(self):
        if self._cfg is None:
            raise ValueError('Service configuration not initialized, use initServiceCfg()')
        return self._cfg
    
    def addCustomArguments(self):
        # derivedGroup = self.parser.add_argument_group('Athena specific arguments') # commented out for now to avoid compilation warning in Athena, add it back when needed
        # add arguments here derivedGroup.add_argument(...)
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
        self.logger.info("Configuring algorithms")
        configSeq.fullConfigure(configAccumulator)
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
        
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowSvcCfg
        self.initServiceCfg()
        self.cfg.merge(PoolReadCfg(self.flags))
        self.cfg.merge(CutFlowSvcCfg(self.flags))
        
        outputFile = f"ANALYSIS DATAFILE='{self.outputName}.root' OPT='RECREATE'"
        from AthenaConfiguration.ComponentFactory import CompFactory
        self.cfg.addService(CompFactory.THistSvc(Output=[outputFile]))
        self.cfg.merge(self.makeAlgSequence())
        self.cfg.printConfig()
        
        sc = self.cfg.run(self.flags.Exec.MaxEvents)
        sys.exit(sc.isFailure())
    