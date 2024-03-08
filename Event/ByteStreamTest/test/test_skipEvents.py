#!/usr/bin/env python
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.TestDefaults import defaultTestFiles

flags = initConfigFlags()
parser = flags.getArgumentParser()
parser.add_argument('--selectorTool', type=str, help='Name of EventSelector tool to use')

flags.Input.Files = defaultTestFiles.RAW_RUN3
flags.Exec.MaxEvents = 10
flags.Exec.DebugMessageComponents = ['*/EventSelector']
args = flags.fillFromArgs(parser=parser)
flags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)

from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
acc.merge( ByteStreamReadCfg(flags) )

if args.selectorTool:
   acc.getService('EventSelector').HelperTools.append(
      CompFactory.getComp(args.selectorTool)() )

from AthenaConfiguration.Utils import setupLoggingLevels
setupLoggingLevels(flags, acc)

import sys
sys.exit(acc.run().isFailure())
