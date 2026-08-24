#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
import TrigExamples.MTCalibPebConfig as Conf


def run(flags):
   """Test timeout handling in athenaEF"""

   # Setup flags
   Conf.set_flags(flags)
   flags.lock()
   cfg = ComponentAccumulator()

   # Configure the HLT algorithms
   hypo_tools = [Conf.make_hypo_tool('HLT_MTCalibPeb{:d}'.format(num)) for num in range(1, 4)]
   for tool in hypo_tools:
      # 100% accept rate, no ROB requests, sleeps for up to 1.4 seconds
      tool.RandomAcceptRate = 1.0
      tool.ROBAccessDict = {}
      tool.BurnTimePerCycleMillisec = 200
      tool.NumBurnCycles = 7
      tool.PEBROBList = [0x7c0000]

      hypo = Conf.make_hypo_alg('HypoAlg1')
      hypo.HypoTools = hypo_tools

   # SGInputLoader takes care of unmet input dependencies (e.g. triggering conversion from BS)
   from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
   cfg.merge(SGInputLoaderCfg(flags))

   # Configure the L1 and HLT sequences
   cfg.merge( Conf.l1_seq_cfg(flags) )
   cfg.merge( Conf.hlt_seq_cfg(flags,
                               num_chains=1,  # ignored if hypo_algs argument given
                               concurrent=False,  # ignored if hypo_algs argument given
                               hypo_algs=[hypo]) )

   return cfg
