#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
flags = initConfigFlags()
flags.Common.MsgSuppression = False
flags.Exec.MaxEvents = 10
flags.Exec.EventPrintoutInterval = flags.Exec.MaxEvents/10
flags.addFlag('Test.AlgMode', 'cpp_cpp', help="Use C++/Python for producer_consumer")
flags.addFlag('Test.UseDataPool', False)
flags.fillFromArgs()

flags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
cfg = MainServicesCfg(flags)

class Options:
    DataName     = "MyData"
    NbrOfObjects = 10000
    ObjectsSize  = 100

# Add producer
if flags.Test.AlgMode.startswith('cpp_'):
    Producer = CompFactory.SgStressProducer
elif flags.Test.AlgMode.startswith('py_'):
    from StoreGateTests.Lib import PySgStressProducer as Producer

cfg.addEventAlgo( Producer(
    'SgStressProducer',
    DataName     = Options.DataName,
    NbrOfObjects = Options.NbrOfObjects,
    ObjectsSize  = Options.ObjectsSize,
    UseDataPool  = flags.Test.UseDataPool) )

# Add consumer
if flags.Test.AlgMode.endswith('_cpp'):
    Consumer = CompFactory.SgStressConsumer
elif flags.Test.AlgMode.endswith('_py'):
    from StoreGateTests.Lib import PySgStressConsumer as Consumer

cfg.addEventAlgo( Consumer(
    'SgStressConsumer',
    DataName     = Options.DataName,
    NbrOfObjects = Options.NbrOfObjects) )

# Run
import sys
sys.exit( cfg.run().isFailure() )
