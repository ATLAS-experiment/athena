#!/usr/bin/env python
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# File: AthExStoreGateExample/test/test_DecorHandle.py
# Author: Frank Winklmeier
# Date: July, 2022
# Brief: Test for DecorHandleKey depending on a regular handle key
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
import sys

flags = initConfigFlags()
flags.Input.Files = []
flags.Exec.MaxEvents = 1
flags.lock()

cfg = MainEvgenServicesCfg(flags)

cfg.addEventAlgo( CompFactory.AthEx.HandleTestAlg ('testalg',
   Tool1 = CompFactory.AthEx.HandleTestTool3 ('testool',
                                              RHKey="myrcont",
                                              RDecorKey="myrdecor",
                                              WHKey="mywcont",
                                              WDecorKey="mywdecor",
                                              WDecorKey2="mywdecor2")) )

sys.exit(cfg.run().isFailure())
