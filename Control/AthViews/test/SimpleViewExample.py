#!/usr/bin/env athena.py
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import parOR, seqOR


def viewCfg(flags):
   acc = ComponentAccumulator()

   # Sequence for view maker
   acc.addSequence( seqOR("makeViewSequence") )

   make_alg = CompFactory.AthViews.ViewSubgraphAlg(
      "make_alg",
      ViewBaseName = "view",
      ViewNumber = 10,
      ViewNodeName = "allViewAlgorithms")
   acc.addEventAlgo( make_alg, sequenceName="makeViewSequence" )

   # Sequence for algorithms
   acc.addSequence( parOR("allViewAlgorithms"), parentName="makeViewSequence" )
   acc.addEventAlgo( CompFactory.AthViews.ViewTestAlg("view_test"),
                     sequenceName="allViewAlgorithms" )
   return acc


flags = initConfigFlags()
flags.Scheduler.ShowControlFlow = True
flags.Scheduler.ShowDataDeps = True
flags.Exec.MaxEvents = 10
flags.fillFromArgs()
flags.lock()

cfg = MainServicesCfg(flags)
cfg.merge( viewCfg(flags) )

import sys
sys.exit(cfg.run().isFailure())
