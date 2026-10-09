# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

__doc__ = """
Create a sorted event-tag POOL file from a set of Athena files, using SortedEventTagWriter
"""


class SortedInputFileCreator:
   """Creates a POOL file of event references sorted by an EventInfo attribute"""

   def execute(self, inputFiles, outputFile, sortAttribute="LumiBlockN"):
      """Sort the events of the input files and write them to outputFile
      in ascending order of sortAttribute, read from the input attribute list"""
      inputs = [inputFiles] if isinstance(inputFiles, str) else list(inputFiles)

      from AthenaConfiguration.AllConfigFlags import initConfigFlags
      from AthenaConfiguration.ComponentFactory import CompFactory
      from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
      from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
      from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg

      flags = initConfigFlags()
      flags.Input.Files = inputs
      flags.lock()

      # Set up the main services and configure the input and output
      acc = MainEvgenServicesCfg(flags)
      acc.merge(PoolReadCfg(flags))
      acc.merge(PoolWriteCfg(flags))
      acc.addEventAlgo(CompFactory.SortedEventTagWriter(OutputFile=outputFile,
                                                        SortAttribute=sortAttribute))
      if acc.run().isFailure():
         raise RuntimeError("Sorting of input files failed")

   def _executeQuiet(self, *args, **kwargs):
      import os
      # Athena messaging writes to the file descriptor, so redirect it (child process only)
      devnull = os.open(os.devnull, os.O_WRONLY)
      os.dup2(devnull, 1)
      self.execute(*args, **kwargs)

   def executeInSubprocess(self, *args, quiet=True, **kwargs):
      """Run execute() in a forked process; quiet=True discards its stdout"""
      import multiprocessing
      # Default start method changed from fork to spawn in python 3.14.
      # Need to force it back.
      multiprocessing.set_start_method('fork', force=True)
      target = self._executeQuiet if quiet else self.execute
      process = multiprocessing.Process(target=target, args=args, kwargs=kwargs)
      process.start()
      process.join()  # Wait for completion
      return process.exitcode
