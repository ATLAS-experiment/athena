# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def FPEAndCoreDumpCfg(flags):
   """Configure FPE handling and CoreDumpSvc"""

   cfg = ComponentAccumulator()
   cds = CompFactory.CoreDumpSvc(FastStackTrace=True)

   if flags.Exec.FPE != -2:      # -2: disabled
      if flags.Exec.FPE == -1:  # -1: abort with core dump
         cfg.addService(CompFactory.FPEControlSvc(), create=True)
      else:
         try:
            fpe_aud = CompFactory.FPEAuditor(NStacktracesOnFPE=flags.Exec.FPE)
         except AttributeError:
            pass  # FPEAuditor not available in all projects
         else:
            cfg.addAuditor(fpe_aud)
            # If FPEAuditor is used, the CoreDumpSvc should not catch SIGFPE.
            # Need to copy the list first as we cannot modify a default value.
            from signal import SIGFPE
            signalsToCatch = list(cds.Signals)
            try:
               signalsToCatch.remove(SIGFPE)
               cds.Signals = signalsToCatch
            except ValueError:
               pass  # SIGFPE was not in the list

   cfg.addService(cds, create=True)
   return cfg
