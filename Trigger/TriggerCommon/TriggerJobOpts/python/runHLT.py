# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
CA module to configure the (standalone) HLT for athena and athenaHLT.
There is a separate entry point for each application to tailor some
flags and services. All common code should go to runLTCfg(flags).

Usage:
  athena --CA [options] TriggerJobOpts/runHLT.py [flags]
  athenaHLT [options] TriggerJobOpts.runHLT [flags]

  python -m TriggerJobOpts.runHLT  # not recommended (due to missing LD_PRELOADs)
"""

from AthenaConfiguration.ComponentFactory import CompFactory

def lock_and_restrict(flags):
   """Deny access to a few flags and lock"""

   def bomb(x):
      raise RuntimeError("Concurrency flags cannot be used in the HLT to ensure "
                         "that the configuration is portable across different CPUs")

   flags.Concurrency.NumProcs = bomb
   flags.Concurrency.NumThreads = bomb
   flags.Concurrency.NumConcurrentEvents = bomb

   #-------------------------------------------------------------
   # Switch off CPS mechanism if we only run selected
   # signatures or chains, to avoid single-chain sets
   #-------------------------------------------------------------
   if flags.Trigger.selectChains or len(flags.Trigger.enabledSignatures) == 1:
      flags.Trigger.disableCPS = True

   flags.lock()


def set_flags(flags):
   """Set default flags for running HLT"""

   from AthenaConfiguration.Enums import BeamType

   flags.Trigger.doHLT = True    # needs to be set early as other flags depend on it
   flags.Trigger.EDMVersion = 3  # Run-3 EDM
   flags.Beam.Type = BeamType.Collisions
   flags.InDet.useDCS = False    # DCS is in general not available online
   flags.Muon.MuonTrigger = True # Setup muon reconstruction for trigger

   # Disable some forward detetors
   flags.Detector.GeometryALFA = False
   flags.Detector.GeometryFwdRegion = False
   flags.Detector.GeometryLucid = False

   # Increase scheduler checks and verbosity
   flags.Scheduler.CheckDependencies = True
   flags.Scheduler.ShowControlFlow = True
   flags.Scheduler.ShowDataDeps = True
   flags.Scheduler.EnableVerboseViews = True
   flags.Input.FailOnUnknownCollections = True
   flags.Scheduler.AutoLoadUnmetDependencies = False


def runHLTCfg(flags):
   """Main function to configure the HLT in athena and athenaHLT"""
   from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
   from AthenaCommon.Logging import logging

   log = logging.getLogger('runHLT')
   cfg = ComponentAccumulator()

   # Load these objects from StoreGate
   loadFromSG = [('xAOD::EventInfo', 'StoreGateSvc+EventInfo'),
                 ('TrigConf::L1Menu','DetectorStore+L1TriggerMenu'),
                 ('TrigConf::HLTMenu','DetectorStore+HLTTriggerMenu')]

   from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
   cfg.merge(SGInputLoaderCfg(flags, loadFromSG))

   from TriggerJobOpts.TriggerHistSvcConfig import TriggerHistSvcConfig
   cfg.merge(TriggerHistSvcConfig(flags))

   # Menu
   from TriggerMenuMT.HLT.Config.GenerateMenuMT_newJO import generateMenuMT
   from TriggerJobOpts.TriggerConfig import triggerRunCfg
   menu = triggerRunCfg(flags, menu=generateMenuMT)
   cfg.merge(menu)

   from LumiBlockComps.LumiBlockMuWriterConfig import LumiBlockMuWriterCfg
   cfg.merge(LumiBlockMuWriterCfg(flags), sequenceName="HLTBeginSeq")

   if flags.Trigger.doTransientByteStream and flags.Trigger.doCalo:
       from TriggerJobOpts.TriggerTransBSConfig import triggerTransBSCfg_Calo
       cfg.merge(triggerTransBSCfg_Calo(flags), sequenceName="HLTBeginSeq")

   # L1 simulation
   if flags.Trigger.doLVL1:
       from TriggerJobOpts.Lvl1SimulationConfig import Lvl1SimulationCfg
       cfg.merge(Lvl1SimulationCfg(flags), sequenceName="HLTBeginSeq")

   # Track overlay needs this to ensure that the collections are copied correctly
   # (due to the hardcoding of the name in the converters)
   if flags.Overlay.doTrackOverlay:
       from TrkEventCnvTools.TrkEventCnvToolsConfigCA import TrkEventCnvSuperToolCfg
       cfg.merge(TrkEventCnvSuperToolCfg(flags))

   if flags.Common.isOnline:
     from TrigOnlineMonitor.TrigOnlineMonitorConfig import trigOpMonitorCfg
     cfg.merge( trigOpMonitorCfg(flags) )

   # Print config and statistics
   if log.getEffectiveLevel() <= logging.DEBUG:
       cfg.printConfig(withDetails=False, summariseProps=True, printDefaults=True)

   from AthenaConfiguration.AccumulatorCache import AccumulatorDecorator
   AccumulatorDecorator.printStats()

   return cfg


def athenaHLTCfg(flags):
   """Top-level cfg function when running in athenaHLT"""
   # Set default flags for running HLT
   set_flags(flags)

   # Decoding the flags from the command line is already done in athenaHLT.
   # But we have to do it again in case some of the flags from set_flags
   # get overwritten by the user.
   from TrigPSC import PscConfig
   for flag_arg in PscConfig.unparsedArguments:
      flags.fillFromString(flag_arg)

   # Lock flags
   lock_and_restrict(flags)

   # Configure HLT
   cfg = runHLTCfg(flags)
   return cfg


def athenaCfg():
   """Top-level cfg function when running in athena"""
   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   from AthenaConfiguration.Enums import Format

   # Set default flags for running HLT
   flags = initConfigFlags()
   set_flags(flags)

   # To allow running from MC
   flags.Common.isOnline = lambda f: not f.Input.isMC

   # Add options to command line parser
   parser = flags.getArgumentParser()
   parser.add_argument('--postExec', metavar='CMD',
                       help='Commands executed after Python configuration')

   # Fill flags from command line
   args = flags.fillFromArgs(parser=parser)

   if flags.Trigger.writeBS:
      flags.Output.doWriteBS = True
   else:  # RDO writing is default in athena
      flags.Output.doWriteRDO = True
      if not flags.Output.RDOFileName:
         flags.Output.RDOFileName = 'RDO_TRIG.pool.root'

   # Configure main services
   _allflags = flags.clone()   # copy including Concurrency flags
   _allflags.lock()
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   cfg = MainServicesCfg(_allflags)
   del _allflags

   # Lock flags
   lock_and_restrict(flags)

   if flags.Input.isMC:
      # Disable spurious warnings from HepMcParticleLink (ATR-21838)
      cfg.addService(CompFactory.MessageSvc(setError=["HepMcParticleLink"]))

   if flags.Input.Format is Format.BS:
       from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
       cfg.merge(ByteStreamReadCfg(flags))
   else:
       from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
       cfg.merge(PoolReadCfg(flags))

   # Configure HLT
   cfg.merge(runHLTCfg(flags))

   if args.postExec:
      exec(args.postExec)

   # Apply flags.Exec.XXXMessageComponents logic to configured job
   from AthenaConfiguration.Utils import setupLoggingLevels
   setupLoggingLevels(flags, cfg)

   return cfg


def main(flags=None):
   """This method is called by athena (no flags) and athenaHLT (with pre-populated flags)"""

   # Make sure nobody uses deprecated global ConfigFlags
   import AthenaConfiguration.AllConfigFlags
   del AthenaConfiguration.AllConfigFlags.ConfigFlags

   return athenaCfg() if flags is None else athenaHLTCfg(flags)


# This entry point is only used when running in athena
if __name__ == "__main__":
   import sys
   sys.exit(main().run().isFailure())
