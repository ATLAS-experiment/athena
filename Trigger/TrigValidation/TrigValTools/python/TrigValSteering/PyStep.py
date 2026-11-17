#
# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
#

"""
Step implemented as python function
"""

from TrigValTools.TrigValSteering.Step import Step
import contextlib
import os
import sys

class PyStep(Step):
   """Step calling a python function"""

   def __init__(self, func, **kwargs):
      name = kwargs.get('name') or func.__name__
      super(PyStep, self).__init__(name)
      self.func = func
      self.func_kwargs = dict([(k,v) for k,v in kwargs.items() if k != 'name'])
      self.output_stream = Step.OutputStream.STDOUT_ONLY

   def run(self, dry_run=False):

      self.log.info('Running %s step', self.name)

      dest = sys.stdout
      if self.output_stream == self.OutputStream.NO_PRINT:
         dest = None
      elif self.output_stream in [self.OutputStream.FILE_ONLY, self.OutputStream.FILE_AND_STDOUT]:
         dest = open(self.get_log_file_name(), 'w')

      if dry_run:
         self.result = 0
      else:
         try:
            with contextlib.redirect_stdout(dest), contextlib.redirect_stderr(dest):
               self.result = self.func(**self.func_kwargs)

            # Poor man's implementation of 'tee'
            if self.output_stream == self.OutputStream.FILE_AND_STDOUT:
               dest.close()
               print(open(dest.name).read())

            # In case function does not return a value, assume success
            if self.result is None:
               self.result = 0
         except Exception as e:
            self.log.error('Exception calling %s: %s', self.func.__name__, e)
            self.result = 1

      return self.result, f'# (internal) {self.func.__name__}'

class RmStep(PyStep):
   """PyStep removing a file, suppressing FileNotFoundError

   Usage:
      ex_rm = RmStep('created.BS')
   """

   def __init__(self, filename, **kwargs):
      name = kwargs.pop('name', None) or filename

      def _rm():
         with contextlib.suppress(FileNotFoundError):
            os.remove(filename)

      super(RmStep, self).__init__(_rm, name=name, **kwargs)

class MvStep(PyStep):
   """PyStep moving (renaming) a file

   Usage:
      ex_mv = MvStep('in.BS', 'out.BS')
   """

   def __init__(self, sourceFile, destFile, **kwargs):
      name = kwargs.pop('name', None) or f'{sourceFile} -> {destFile}'

      def _mv():
         os.rename(sourceFile, destFile)

      super(MvStep, self).__init__(_mv, name=name, **kwargs)
