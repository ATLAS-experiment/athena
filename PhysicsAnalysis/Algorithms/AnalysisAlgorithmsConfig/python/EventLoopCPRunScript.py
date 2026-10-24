# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AnalysisAlgorithmsConfig.CPBaseRunner import CPBaseRunner
import os
import sys

class EventLoopCPRunScript(CPBaseRunner):
    def __init__(self):
        super().__init__()
        self.logger.info("EventLoopCPRunScript initialized")
        self.addCustomArguments()
        self.algSeq = None
        self.outputStreams = []
        # Avoid putting call to parse_args() here! Otherwise it is hard to retrieve the parser infos

    def addCustomArguments(self):
        # add arguments here
        derivedGroup = self.parser.add_argument_group('EventLoop specific arguments')
        derivedGroup.add_argument('--direct-driver', dest='direct_driver',
                                 action='store_true', help='Run the job with the direct driver')
        derivedGroup.add_argument('--work-dir', dest='work_dir', nargs='?', const='workDir', default=None,
                                  help='The work directory for the EL job. defaults to "workDir".')
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
        # EventLoop writes histograms to its histogram file whatever their stream,
        # so they should not request the tree stream ANALYSIS
        import ROOT
        configAccumulator.setDefaultHistogramStream(ROOT.EL.Job.histogramStreamName)
        self.logger.info("Configuring algorithms")
        configSeq.fullConfigure(configAccumulator)
        self.algSeq = algSeq
        self.outputStreams = configAccumulator.outputStreams()
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
    #
    # The dump happens in two steps: the algorithm sequence is written out by
    # the process that did the configuration (before the job is submitted), and
    # the combination with the tool configuration written by the job happens
    # after the job has finished.
    _algSequenceFile = "_alg_sequence.json"

    def _dumpAlgSequence(self):
        from AnalysisAlgorithmsConfig.SaveConfigUtils import save_algs_from_sequence_ELjob
        import json
        with open(self._algSequenceFile, 'w', encoding='utf-8') as seq_out_file:
            output_dict = {}
            try:
                save_algs_from_sequence_ELjob(self.algSeq, output_dict)
                json.dump(output_dict, seq_out_file, ensure_ascii=False, indent=4)
            except Exception as e:
                self.logger.warning(f'Dumping full config failed with: {e}')
                self.logger.warning('Please also check if "PrintConfiguration" is enabled in the text config.')

    def _combineFullConfig(self):
        from AnalysisAlgorithmsConfig.SaveConfigUtils import combine_tools_and_algorithms_ELjob
        try:
            combine_tools_and_algorithms_ELjob(combine_dictionaries=True, alg_file=self._algSequenceFile, output_file="full_config.json")
            self.logger.info("Combining full config to full_config.json succeeded")

        except Exception as e:
            self.logger.warning(f'Combining full config failed with: {e}')
            self.logger.warning('Please also check if "PrintConfiguration" is enabled in the text config.')
        finally:
            # the file may be missing if the configuration failed before writing it
            if os.path.exists(self._algSequenceFile):
                os.remove(self._algSequenceFile)

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
        # move the other output stream files, named as in AthenaCPRunScript; taken from disk as
        # the streams are only known in the job process, which the exec driver forks off
        for streamDir in sorted(workDir.glob('data-*')):
            stream = streamDir.name.removeprefix('data-')
            if stream == 'ANALYSIS':
                continue
            streamSymlink = streamDir / f'{self.outputName}.root'
            streamPath = streamSymlink.resolve()
            target = currentDir / f"{self.outputName}_{stream.removeprefix('ANALYSIS_')}.root"
            if streamPath.exists():
                self.logger.info(f"Moving {streamPath} to {target}")
                if streamSymlink.is_symlink():
                    streamSymlink.unlink()
                shutil.move(str(streamPath), str(target))
            else:
                self.logger.warning(f"Root file {streamPath} does not exist, skipping move.")

        newHistFile = currentDir / f"hist-{self.outputName}.root"
        # rename merged hist-ntuple to output_name.root
        if self.args.merge_output_files and newHistFile.exists():
            self.logger.info(f"renaming the hist-{self.outputName}.root to {self.outputName}.root")
            newHistFile.rename(currentDir / f"{self.outputName}.root")

    @property
    def submitDir(self):
        return self.args.work_dir if self.args.work_dir else 'workDir'

    def getExitCode(self):
        '''
        Exit code for the direct driver, where the job ran in this process.
        For the exec driver the exit status of the worker process is used
        instead, so that this process never has to load ROOT.
        '''
        import ROOT
        statusCode = ROOT.EL.Driver.retrieve(self.submitDir)
        if statusCode:
            return 0
        return 1

    @staticmethod
    def _flushOutput():
        sys.stdout.flush()
        sys.stderr.flush()

    def configureAndSubmit(self):
        '''
        Do the full job configuration and submit the job.

        Everything that loads ROOT (and with it the ROOT-python
        dictionaries, which account for most of the configuration-time
        memory) happens in here.  With the exec driver, driver.submit()
        replaces the current process with the worker and never returns.
        '''
        self.setupConfig()
        # importing ROOT has a long upfront time and high memory
        # consumption, so we do it here
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
        # Every requested output stream not going to the histogram file gets its own file: the
        # default tree stream ANALYSIS only if something writes to it, plus e.g. the streams of
        # Output.splitBySelection
        histStreams = {ROOT.EL.Job.histogramStreamName}
        if self.args.merge_output_files:
            self.job.options().setString(ROOT.EL.Job.optStreamAliases, "ANALYSIS=" + ROOT.EL.Job.histogramStreamName)
            histStreams.add('ANALYSIS')
        for stream in [stream for stream in self.outputStreams if stream not in histStreams]:
            self.job.outputAdd(ROOT.EL.OutputStream(stream))

        if self.args.run_perf_stat:
            self.job.options().setBool(ROOT.EL.Job.optXAODPerfStats, 1)
        if self.args.algorithm_timers:
            self.job.options().setBool(ROOT.EL.Job.optAlgorithmTimer, 1)
        if self.args.algorithm_memory_monitoring:
            self.job.options().setBool(ROOT.EL.Job.optAlgorithmMemoryMonitor, 1)

        if self.args.dump_full_config:
            self._dumpAlgSequence()

        driver = ROOT.EL.DirectDriver() if self.args.direct_driver else ROOT.EL.ExecDriver()
        # the exec driver replaces this process, so anything still sitting in
        # the python output buffers would be lost without this flush
        self._flushOutput()
        driver.submit(self.job, self.submitDir)

    def runInChildProcess(self):
        '''
        Run configureAndSubmit() in a forked child process and return the
        exit code of the job.

        The fork has to happen here, before anything imports ROOT or reads
        the input file metadata through the flags.  The exec driver frees the
        configuration-time memory by replacing the configuring process with
        the worker, and that only helps if the process that stays behind to
        wait for the job (this one) never loaded ROOT in the first place.
        Forking after the configuration would just keep all that memory alive
        in the waiting parent for the duration of the job.

        The worker (eventloop_exec_worker) runs all job segments, retrieves
        the output and exits non-zero on any failure, so its exit status is
        the exit status of the job.
        '''
        self._flushOutput()
        if (pid := os.fork()) == 0: # child process
            try:
                self.configureAndSubmit()
                # the exec driver never returns from submit, so getting here
                # means the process was not replaced
                self.logger.error('Job submission returned without running the job')
            except Exception:
                import traceback
                traceback.print_exc()
                self.logger.error('Job configuration or submission failed')
            self._flushOutput()
            os._exit(1)

        _, status = os.waitpid(pid, 0) # parent waits for the job to finish
        exitCode = os.waitstatus_to_exitcode(status)
        if exitCode != 0:
            self.logger.error(f'Job in child process failed with exit code {exitCode}')
            return 1
        return 0

    def run(self):
        # only the argument parsing happens before the fork, see runInChildProcess()
        self.setupArguments()

        if self.args.direct_driver:
            try:
                self.configureAndSubmit()
            except Exception:
                import traceback
                traceback.print_exc()
                self.logger.error('Job submission failed')
            exitCode = self.getExitCode()
        else:
            exitCode = self.runInChildProcess()

        if self.args.dump_full_config:
            self._combineFullConfig()

        if self.args.work_dir is None: # move output if work_dir is not used
            self.moveOutputFiles()

        sys.exit(exitCode)
