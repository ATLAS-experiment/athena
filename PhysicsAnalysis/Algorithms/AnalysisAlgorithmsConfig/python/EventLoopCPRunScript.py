# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
import ROOT
from AnalysisAlgorithmsConfig.CPBaseRunner import CPBaseRunner
from AnaAlgorithm.AlgSequence import AlgSequence
from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator


class EventLoopCPRunScript(CPBaseRunner):
    def __init__(self):
        super().__init__()
        self.logger.info("EventLoopCPRunScript initialized")
        self.sampleHandler = ROOT.SH.SampleHandler()
        self.job = ROOT.EL.Job()
        self.addCustomArguments()
        
    def addCustomArguments(self):
        self.parser.add_argument('--direct-driver', dest='direct_driver',
                                 action='store_true', help='Run the job with the direct driver')
    def makeAlgSequence(self):
        algSeq = AlgSequence()
        self.logger.info("Configuring algorithms based on YAML file")
        configSeq =  self.config.configure()
        self.logger.info("Configuring common services")
        configAccumulator = ConfigAccumulator(autoconfigFromFlags=self.flags,
                                              algSeq=algSeq,
                                              noSystematics=self.args.no_systematics)
        self.logger.info("Configuring algorithms")
        configSeq.fullConfigure(configAccumulator)
        return algSeq
    
    def readSamples(self):
        sampleFiles = ROOT.SH.SampleLocal()
        self.logger.info("Adding files to the sample handler")
        for file in self.inputList:
            sampleFiles.add(file)
        self.sampleHandler.add(sampleFiles)
    
    def makeJob(self):
        job = ROOT.EL.Job()
        job.sampleHandler(self.sampleHandler)
        return job
    
    def run(self):
        ROOT.xAOD.Init().ignore()
        self.readSamples()
        self.flags.lock()
        self.printFlags()
        
        self.job.sampleHandler(self.sampleHandler)
        self.job.options().setDouble(ROOT.EL.Job.optMaxEvents, self.flags.Exec.MaxEvents)
        self.job.options().setString(ROOT.EL.Job.optSubmitDirMode, 'unique-link')
        for alg in self.makeAlgSequence():
            self.job.algsAdd(alg)
        self.job.outputAdd(ROOT.EL.OutputStream('ANALYSIS'))
        
        driver = ROOT.EL.DirectDriver() if self.args.direct_driver else ROOT.EL.ExecDriver()
        driver.submit(self.job, self.args.work_dir)
