#! /usr/bin/env athena.py
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Test that nesting of views does NOT work.
#

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import seqOR


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

   # View algorithms
   acc.addSequence( seqOR("l1ViewAlgorithms"), parentName="makeViewSequence" )
   acc.addEventAlgo( CompFactory.AthViews.ViewTestAlg("view_test"),
                     sequenceName="l1ViewAlgorithms" )

   nest_alg = CompFactory.AthViews.ViewSubgraphAlg(
      "nest_alg",
      ViewBaseName = "viewView",
      ViewNumber = 10,
      ViewNodeName = "l2ViewAlgorithms")
   acc.addEventAlgo( nest_alg, sequenceName="l1ViewAlgorithms" )

   acc.addSequence( seqOR("l2ViewAlgorithms"), parentName="l1ViewAlgorithms" )
   acc.addEventAlgo( CompFactory.AthViews.ViewTestAlg("viewView_test"),
                     sequenceName="l2ViewAlgorithms" )

   # Merge views
   acc.addEventAlgo( CompFactory.AthViews.ViewMergeAlg("merge_alg"),
                     sequenceName="makeViewSequence" )

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
