# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AnalysisAlgorithmsConfig.CPBaseRunner import CPBaseRunner
import os

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
        derivedGroup.add_argument('--work-dir', dest='work_dir', nargs='?', const='workDir', default=None,
                                  help='The work directory for the EL job. defaults to "workDir".')
        derivedGroup.add_argument('--no-factory-preload', dest='no_factory_preload', action='store_true', help='Do not preload the component factories for the EL job. The component factories save memory and sidestep some technical issues, so you should not disable them unless you have a good reason to do so.')
        derivedGroup.add_argument('--merge-output-files', dest='merge_output_files', action='store_true', help='Merge the output histogram and n-tuple files into a single file.')
        
        expertGroup = self.parser.add_argument_group('Experts arguments')
        expertGroup.add_argument('--run-perf-stat', dest='run_perf_stat', action='store_true', help='Run xAOD::PerfStats to get input branch access data. This is mostly useful for AMG experts wanting to understand branch access patterns.')
        expertGroup.add_argument('--algorithm-timers', dest='algorithm_timers', action='store_true', help='Enable algorithm timers. This is mostly useful for AMG experts wanting to understand tool performance.')
        expertGroup.add_argument('--algorithm-memory-monitoring', dest='algorithm_memory_monitoring', action='store_true', help='Enable algorithm memory monitoring. This is mostly useful for AMG experts wanting to understand tool memory usage. Note that this is imperfect and may in cases assign memory to the wrong algorithm.')
        return
        
    def makeAlgSequence(self):
        from AnaAlgorithm.AlgSequence import AlgSequence
        from AnalysisAlgorithmsConfig.ConfigAccumulator import ConfigAccumulator
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
        sampleFiles = ROOT.SH.SampleLocal(f"{self.outputName}")
        self.logger.info("Adding files to the sample handler")
        for file in self.inputList:
            sampleFiles.add(file)
        self.sampleHandler.add(sampleFiles)
            
    def moveOutputFiles(self):
        from pathlib import Path
        import shutil
        self.logger.info("Moving the analysis root file and the hist file to the top level.")
        workDir = Path(self.args.work_dir) if self.args.work_dir else Path('workDir')
        rootfileSymlink = (workDir / 'data-ANALYSIS' / f'{self.outputName}.root')
        rootfilePath = rootfileSymlink.resolve()
        histfileSymlink = (workDir / f'hist-{self.outputName}.root')
        histfilePath = histfileSymlink.resolve()
        currentDir = Path.cwd()
        # move ntuple file if it exists
        if rootfilePath.exists():
            self.logger.info(f"Moving {rootfilePath} to {currentDir / f'{self.outputName}.root'}")
            if rootfileSymlink.is_symlink(): # The check is needed to avoid FileNotFoundError if using direct driver
                rootfileSymlink.unlink()
            shutil.move(str(rootfilePath), str(currentDir / f"{self.outputName}.root"))
        else:
            self.logger.warning(f"Root file {rootfilePath} does not exist or merging is enabled, skipping move.")
        #move histogram file if it exists    
        if histfilePath.exists():
            self.logger.info(f"Moving {histfilePath} to {currentDir / f'hist-{self.outputName}.root'}")
            if histfileSymlink.is_symlink(): # The check is needed to avoid FileNotFoundError if using direct driver
                histfileSymlink.unlink()
            shutil.move(str(histfilePath), str(currentDir / f"hist-{self.outputName}.root"))
        else:
            self.logger.warning(f"Histogram file {histfilePath} does not exist or merging, skipping move.")
            
        newHistFile = currentDir / f"hist-{self.outputName}.root"
        # rename merged hist-ntuple to output_name.root
        if self.args.merge_output_files and newHistFile.exists():
            self.logger.info(f"renmaing the hist-{self.outputName}.root to {self.outputName}.root")
            newHistFile.rename(currentDir / f"{self.outputName}.root")
        
    def driverSubmit(self, driver):
        '''
        Important if you want to run code after submitting the job, with external driver e.g., ExecDriver.
        Assistant function to call driver submit. Move the submission to a child process to avoid the main process being terminated.
        Directly calling external driver submission will not return controls to the main process, the main thread will be terminated.
        '''
        import os
        if (pid := os.fork()) == 0: # child process
            name = self.args.work_dir if self.args.work_dir else 'workDir'
            driver.submit(self.job, name)
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
        self.job.options().setDouble(ROOT.EL.Job.optSkipEvents, self.flags.Exec.SkipEvents)
        
        for alg in self.makeAlgSequence():
            self.job.algsAdd(alg)
        if self.args.merge_output_files:
            self.job.options().setString(ROOT.EL.Job.optStreamAliases, "ANALYSIS=" + ROOT.EL.Job.histogramStreamName)
        else:
            self.job.outputAdd(ROOT.EL.OutputStream('ANALYSIS'))
        if not self.args.no_factory_preload:
            preload = os.getenv('EL_FACTORY_PRELOAD', 'libComponentFactoryPreloaderDict.so,CP::preloadComponentFactories')
            self.logger.info(f"Preloading factories: {preload}")
            self.job.options().setString(ROOT.EL.Job.optFactoryPreload, preload)
        
        if self.args.run_perf_stat:
            self.job.options().setBool(ROOT.EL.Job.optXAODPerfStats, 1)
        if self.args.algorithm_timers:
            self.job.options().setBool(ROOT.EL.Job.optAlgorithmTimer, 1)
        if self.args.algorithm_memory_monitoring:
            self.job.options().setBool(ROOT.EL.Job.optAlgorithmMemoryMonitor, 1)

        driver = ROOT.EL.DirectDriver() if self.args.direct_driver else ROOT.EL.ExecDriver()
        self.driverSubmit(driver)
        if self.args.work_dir is None: # move output if work_dir is not used
            self.moveOutputFiles()
