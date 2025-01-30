# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
import sys
from AnalysisAlgorithmsConfig.CPBaseRunner import CPBaseRunner
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowSvcCfg

from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


class AthenaCPRunScript(CPBaseRunner):
    def __init__(self):
        super().__init__()
        self.logger.info("AthenaCPRunScript initialized")
        self._cfg = None
        self.addCustomArguments()
        
    @property
    def cfg(self):
        if self._cfg is None:
            raise ValueError('Service configuration not initialized, use initServiceCfg()')
        return self._cfg
    
    def addCustomArguments(self):
        pass
    
    def makeAlgSequence(self):
        algSeq = CompFactory.AthSequencer()
        self.logger.info("Configuring algorithms based on YAML file")
        configSeq =  self.config.configure()
        self.logger.info("Configuring common services")
        configAccumulator = ConfigAccumulator(autoconfigFromFlags=self.flags,
                                              algSeq=algSeq,
                                              noSystematics=self.args.no_systematics)
        self.logger.info("Configuring algorithms")
        configSeq.fullConfigure(configAccumulator)
        return configAccumulator.CA
    
    def initServiceCfg(self):
        if not self.flags.locked():
            raise ValueError('Flags must be locked before initializing services')
        self._cfg = MainServicesCfg(self.flags)
    
    def run(self):
        self.flags.lock()
        self.printFlags()
        
        self.initServiceCfg()
        self.cfg.merge(PoolReadCfg(self.flags))
        self.cfg.merge(CutFlowSvcCfg(self.flags))
        
        outputFile = f"ANALYSIS DATAFILE='{self.args.work_dir}' OPT='RECREATE'"
        self.cfg.addService(CompFactory.THistSvc(Output=[outputFile]))
        self.cfg.merge(self.makeAlgSequence())
        self.cfg.printConfig()
        
        sc = self.cfg.run(self.flags.Exec.MaxEvents)
        sys.exit(sc.isFailure())
    