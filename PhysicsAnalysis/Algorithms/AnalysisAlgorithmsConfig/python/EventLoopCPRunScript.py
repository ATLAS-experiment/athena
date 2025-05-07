# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AnalysisAlgorithmsConfig.CPBaseRunner import CPBaseRunner
from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator

class EventLoopCPRunScript(CPBaseRunner):
    def __init__(self):
        super().__init__()
        self.logger.info("EventLoopCPRunScript initialized")
        self.addCustomArguments()
        # Avoid putting call to parse_args() here! Otherwise it is hard to retrieve the parser infos

    def addCustomArguments(self):
        # add arguments here
        derivedGroup = self.parser.add_argument_group('EventLoop specific arguments')
        derivedGroup.add_argument('--direct-driver', dest='direct_driver',
                                 action='store_true', help='Run the job with the direct driver')
        derivedGroup.add_argument('--strip', dest='strip', action='store_true', help='Move the analysis root file to the top level, and delete the work directory.'
                                  ' Mainly useful for standardizing the output with the Athena framework.')
        derivedGroup.add_argument('--work-dir', dest='work_dir', default='workDir', help='The work directory for the EL job')
        return
        
    def makeAlgSequence(self):
        from AnaAlgorithm.AlgSequence import AlgSequence
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
        import ROOT
        self.sampleHandler = ROOT.SH.SampleHandler()
        sampleFiles = ROOT.SH.SampleLocal(f"{self.args.output_name}")
        self.logger.info("Adding files to the sample handler")
        for file in self.inputList:
            sampleFiles.add(file)
        self.sampleHandler.add(sampleFiles)
        
    def stripPath(self):
        import os
        import shutil
        self.logger.info("Moving the analysis root file to the top level, and deleting the work directory. (--strip option)")
        workDir = os.path.realpath(self.args.work_dir)
        rootfilePath = os.path.realpath(os.path.join(workDir, 'data-ANALYSIS', f'{self.args.output_name}.root'))
        currentDir = os.getcwd()
        shutil.move(rootfilePath, os.path.join(currentDir, f"{self.args.output_name}.root"))
        shutil.rmtree(workDir)
        os.remove(os.path.join(currentDir, self.args.work_dir))
    
    def driverSubmit(self, driver):
        '''
        Important if you want to run code after submitting the job, with external driver e.g., ExecDriver.
        Assistant function to call driver submit. Move the submission to a child process to avoid the main process being terminated.
        Directly calling external driver submission will not return controls to the main process, the main thread will be terminated.
        '''
        import os
        if (pid := os.fork()) == 0: # child process
            driver.submit(self.job, self.args.work_dir)
            exit(0)
        else:
            os.waitpid(pid, 0) # parent waits for child process to finish
            return
    
    def run(self):
        self.setup()
        # importing ROOT has a long upfront time, so we do it here
        import ROOT
        ROOT.xAOD.Init().ignore()
        self.readSamples()
        self.flags.lock()
        self.printFlags()
        
        self.job = ROOT.EL.Job()
        self.job.sampleHandler(self.sampleHandler)
        self.job.options().setDouble(ROOT.EL.Job.optFilesPerWorker, 100)
        self.job.options().setDouble(ROOT.EL.Job.optMaxEvents, self.flags.Exec.MaxEvents)
        self.job.options().setString(ROOT.EL.Job.optSubmitDirMode, 'unique-link')
    
        for alg in self.makeAlgSequence():
            self.job.algsAdd(alg)
        self.job.outputAdd(ROOT.EL.OutputStream('ANALYSIS'))
        
        driver = ROOT.EL.DirectDriver() if self.args.direct_driver else ROOT.EL.ExecDriver()
        self.driverSubmit(driver)
        if self.args.strip: self.stripPath()
