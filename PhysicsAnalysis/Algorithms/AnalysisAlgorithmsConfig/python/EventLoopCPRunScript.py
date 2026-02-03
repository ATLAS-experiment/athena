# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AnalysisAlgorithmsConfig.CPBaseRunner import CPBaseRunner
import os
import sys

class EventLoopCPRunScript(CPBaseRunner):
    def __init__(self):
        super().__init__()
        self.logger.info("EventLoopCPRunScript initialized")
        self.addCustomArguments()
        self.algSeq = None
        # Avoid putting call to parse_args() here! Otherwise it is hard to retrieve the parser infos

    def addCustomArguments(self):
        # add arguments here
        derivedGroup = self.parser.add_argument_group('EventLoop specific arguments')
        derivedGroup.add_argument('--direct-driver', dest='direct_driver',
                                 action='store_true', help='Run the job with the direct driver')
        derivedGroup.add_argument('--work-dir', dest='work_dir', nargs='?', const='workDir', default=None,
                                  help='The work directory for the EL job. defaults to "workDir".')
        derivedGroup.add_argument('--merge-output-files', dest='merge_output_files', action='store_true', help='Merge the output histogram and n-tuple files into a single file.')
        derivedGroup.add_argument('--dump-full-config', dest='dump_full_config', action='store_true', help='Save the full CP configuration log to a json file. This can be useful for debugging purposes.')
        
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
        configAccumulator = ConfigAccumulator(flags=self.flags,
                                              algSeq=algSeq,
                                              noSystematics=self.args.no_systematics)
        self.logger.info("Configuring algorithms")
        configSeq.fullConfigure(configAccumulator)
        self.algSeq = algSeq
        self.modifyAlgSequence()
        return algSeq
    
    def readSamples(self):
        import ROOT
        self.sampleHandler = ROOT.SH.SampleHandler()
        sampleFiles = ROOT.SH.SampleLocal(f"{self.outputName}")
        self.logger.info("Adding files to the sample handler")
        for file in self.inputList:
            sampleFiles.add(file)
        self.sampleHandler.add(sampleFiles)
    
    # This functionality should not be in the runscript, instead should be put into PrintConfiguration alg.
    # This is a temporary solution to dump the full config until PrintConfiguration alg is completely ready.
    def _dumpFullConfig(self):
        from AnalysisAlgorithmsConfig.SaveConfigUtils import save_algs_from_sequence_ELjob, combine_tools_and_algorithms_ELjob
        import json
        with(open("_alg_sequence.json", 'w', encoding='utf-8')) as seq_out_file:
            output_dict = {}
            try:
                save_algs_from_sequence_ELjob(self.algSeq, output_dict)
                json.dump(output_dict, seq_out_file, ensure_ascii=False, indent=4) 
            except Exception as e: 
                self.logger.warning(f'Dumping full config failed with: {e}')
                self.logger.warning('Please also check if "PrintConfiguration" is enabled in the text config.')
            try:
                combine_tools_and_algorithms_ELjob(combine_dictionaries=False, alg_file="_alg_sequence.json", output_file="full_config.json")
                self.logger.info("Combining full config to full_config.json succeeded")
                
            except Exception as e:
                self.logger.warning(f'Combining full config failed with: {e}')
                self.logger.warning('Please also check if "PrintConfiguration" is enabled in the text config.')
            finally:
                os.remove("_alg_sequence.json")
            
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
            self.logger.info(f"renaming the hist-{self.outputName}.root to {self.outputName}.root")
            newHistFile.rename(currentDir / f"{self.outputName}.root")
        
    def driverSubmit(self, driver):
        '''
        Important if you want to run code after submitting the job, with external driver e.g., ExecDriver.
        Assistant function to call driver submit. Move the submission to a child process to avoid the main process being terminated.
        Directly calling external driver submission will not return controls to the main process, the main thread will be terminated.
        '''
        if (pid := os.fork()) == 0: # child process
            name = self.args.work_dir if self.args.work_dir else 'workDir'
            driver.submit(self.job, name)
            exit(0)
        else:
            os.waitpid(pid, 0) # parent waits for child process to finish
            return
        
    def getExitCode(self):
        import ROOT
        statusCode = ROOT.EL.Driver.retrieve(self.args.work_dir if self.args.work_dir else 'workDir') 
        if statusCode:
            return 0
        return 1
    
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
        
        if self.args.run_perf_stat:
            self.job.options().setBool(ROOT.EL.Job.optXAODPerfStats, 1)
        if self.args.algorithm_timers:
            self.job.options().setBool(ROOT.EL.Job.optAlgorithmTimer, 1)
        if self.args.algorithm_memory_monitoring:
            self.job.options().setBool(ROOT.EL.Job.optAlgorithmMemoryMonitor, 1)

        driver = ROOT.EL.DirectDriver() if self.args.direct_driver else ROOT.EL.ExecDriver()
        self.driverSubmit(driver)
        
        if self.args.dump_full_config:
            self._dumpFullConfig()
        exitCode = self.getExitCode()
        
        if self.args.work_dir is None: # move output if work_dir is not used
            self.moveOutputFiles()

        sys.exit(exitCode)
