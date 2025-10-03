#!/usr/bin/env athena.py
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# A demonstration of inheriting data from a parent EventView
# and the use of ViewDataVerifier to allow DataFlow with
# the inherited data.
#

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import parOR, seqOR


def viewCfg(flags):
   totalViews = 3
   acc = ComponentAccumulator()
   acc.addSequence( seqOR("viewSequence" ) )

   # Make views
   view_make_1 = CompFactory.AthViews.ViewSubgraphAlg(
      "view_make_1",
      ViewBaseName = "view_1",
      ViewStart = "view_data_1",
      ViewNumber = totalViews,
      AllViews = "view_collection_1",
      ViewNodeName = "view_1" )

   # View 1 algorithm
   view_verify_1 = CompFactory.AthViews.ViewDataVerifier(
      "view_verify_1",
      DataObjects = { ('int','view_data_1') } )

   # Make child views
   view_make_2 = CompFactory.AthViews.ViewSubgraphAlg(
      "view_make_2",
      ViewBaseName = "view_2",
      ViewStart = "view_data_2",
      ViewNumber = totalViews,
      AllViews = "view_collection_2",
      ParentViews = "view_collection_1",
      ViewNodeName = "view_2" )

   # View 2 algorithm - should find both pieces of data
   view_verify_2 = CompFactory.AthViews.ViewDataVerifier(
      "view_verify_2",
      DataObjects = { ('int','view_data_1'), 
                      ('int', 'view_data_2') } )

   view_test = CompFactory.AthViews.ViewTestAlg("view_test")

   # Build sequences
   acc.addEventAlgo(view_make_1, sequenceName="viewSequence")

   acc.addSequence( parOR("view_1"), parentName="viewSequence" )
   acc.addEventAlgo(view_verify_1, sequenceName="view_1")

   acc.addEventAlgo(view_make_2, sequenceName="viewSequence")
   acc.addSequence( parOR("view_2"), parentName="viewSequence" )
   acc.addEventAlgo(view_verify_2, sequenceName="view_2")
   acc.addEventAlgo(view_test, sequenceName="view_2")

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
