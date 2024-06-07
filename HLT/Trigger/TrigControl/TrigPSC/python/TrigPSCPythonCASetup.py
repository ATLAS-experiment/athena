# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# This module is used as a Python bootstrap in athenaHLT for ComponentAccumulator
# based configurations. In CA-mode, we always dump the configuraton to JSON and
# re-launch ourselves from that file.
#
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg
from AthenaConfiguration.MainServicesConfig import addMainSequences
from AthenaCommon.Logging import logging

from TrigCommon import AthHLT
from TrigPSC import PscConfig
from TrigServices.TriggerUnixStandardSetup import commonServicesCfg

import os
import sys

log = logging.getLogger("athenaHLT")

# Ensure we are really in CA-mode:
if not isComponentAccumulatorCfg():
   raise RuntimeError("isComponentAccumulatorCfg()==False but trying to run in CA mode")

# For standalone testing:
if __name__ == "__main__":
   if len(sys.argv)!=2:
      print("Syntax: python %s module.cfgFnc" % os.path.basename(sys.argv[0]))
      sys.exit(1)

   PscConfig.exitAfterDump = True  # no execution for standalone testing
   PscConfig.optmap = {"MESSAGESVCTYPE"    : "TrigMessageSvc",
                       "JOBOPTIONSSVCTYPE" : "TrigConf::JobOptionSvc",
                       "JOBOPTIONSPATH"    : sys.argv[1],
                       "PRECOMMAND"        : "",
                       "POSTCOMMAND"       : ""}


def execCommands(cmds, stage):
   """Helper to execute a command string"""
   if cmds:
      log.info("-"*80)
      log.info("Executing %scommand: %s", stage, cmds)
      exec(cmds, globals())
      log.info("-"*80)


from TrigPSC.PscDefaultFlags import defaultOnlineFlags
flags = defaultOnlineFlags()

# Run pre-command before user CA:
execCommands(PscConfig.optmap["PRECOMMAND"], "pre-")

# Now clone and use locked flags for services configuration:
locked_flags = flags.clone()
locked_flags.lock()

# Setup sequences and framework services:
cfg = ComponentAccumulator(CompFactory.AthSequencer("AthMasterSeq",Sequential=True))
cfg.setAppProperty('ExtSvcCreates', False)
cfg.setAppProperty("MessageSvcType", PscConfig.optmap["MESSAGESVCTYPE"])
cfg.setAppProperty("JobOptionsSvcType", PscConfig.optmap["JOBOPTIONSSVCTYPE"])

addMainSequences(locked_flags, cfg)
cfg.merge( commonServicesCfg(locked_flags) )

# User CA merge (with unlocked flags)
cfg.merge( AthHLT.getCACfg(PscConfig.optmap["JOBOPTIONSPATH"])(flags) )

# Run post-command after user CA:
execCommands(PscConfig.optmap["POSTCOMMAND"], "post-")

# Dump and convert job configuration:
fname = "HLTJobOptions"
with open(f"{fname}.pkl","wb") as f:
   cfg.store(f)

from TrigConfIO.JsonUtils import create_joboptions_json
create_joboptions_json(f"{fname}.pkl", f"{fname}.json")

# Exit or re-launch:
if PscConfig.exitAfterDump:
   log.info("Configuration dumped to %s.json Exiting...", fname)
   sys.exit(0)

AthHLT.reload_from_json(f"{fname}.json",
                        suppress_args = PscConfig.unparsedArguments)
