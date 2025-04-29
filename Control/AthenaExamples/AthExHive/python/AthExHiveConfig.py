#!/usr/bin/env athena.py
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG

# --------------------------------------------------------------------
# Configure HiveAlgX-s. X in (A,B,C,D,E,F,G,V)

def HiveAlgAConf(flags):
    result = ComponentAccumulator()

    # HiveAlgA has input dependency on xAOD::EventInfo
    # Hence we need to add the EventInfo converter algorithm
    from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
    result.merge(EventInfoCnvAlgCfg(flags, disableBeamSpot=True), sequenceName="AthAlgSeq")

    alg = CompFactory.HiveAlgA("HiveAlgA",
                               OutputLevel=DEBUG,
                               Time=20,
                               Cardinality=flags.Concurrency.NumThreads)
    result.addEventAlgo(alg)
    return result

def HiveAlgBConf(flags):
    result = ComponentAccumulator()

    alg = CompFactory.HiveAlgB("HiveAlgB",
                               OutputLevel=DEBUG,
                               Time=10,
                               Cardinality=flags.Concurrency.NumThreads)
    result.addEventAlgo(alg)
    return result

def HiveAlgCConf(flags):
    result = ComponentAccumulator()

    alg = CompFactory.HiveAlgC("HiveAlgC",
                               OutputLevel=DEBUG,
                               Time=190,
                               Key_W1="C1",
                               Cardinality=flags.Concurrency.NumThreads)
    result.addEventAlgo(alg)
    return result

def HiveAlgDConf(flags):
    result = ComponentAccumulator()

    alg = CompFactory.HiveAlgD("HiveAlgD",
                               OutputLevel=DEBUG,
                               Time=10,
                               Cardinality=flags.Concurrency.NumThreads)
    result.addEventAlgo(alg)
    return result

def HiveAlgEConf(flags):
    result = ComponentAccumulator()

    alg = CompFactory.HiveAlgE("HiveAlgE",
                               OutputLevel=DEBUG,
                               Time=30,
                               Key_R1="C1",
                               Cardinality=flags.Concurrency.NumThreads)
    result.addEventAlgo(alg)
    return result

def HiveAlgFConf(flags):
    result = ComponentAccumulator()

    alg = CompFactory.HiveAlgF("HiveAlgF",
                               OutputLevel=DEBUG,
                               Time=30,
                               Cardinality=flags.Concurrency.NumThreads)
    result.addEventAlgo(alg)
    return result

def HiveAlgGConf(flags):
    result = ComponentAccumulator()

    alg = CompFactory.HiveAlgG("HiveAlgG",
                               OutputLevel=DEBUG,
                               Time=10,
                               Cardinality=flags.Concurrency.NumThreads)
    result.addEventAlgo(alg)
    return result

def HiveAlgVConf(flags):
    result = ComponentAccumulator()

    alg = CompFactory.HiveAlgV("HiveAlgV",
                               OutputLevel=DEBUG,
                               Time=30,
                               Key_RV = [ "a1", "a2", "d1", "e1", "C1" ],
                               Key_WV = [ "V1", "V2", "V3" ],
                               Cardinality=flags.Concurrency.NumThreads)
    result.addEventAlgo(alg)
    return result
# --------------------------------------------------------------------

# Configure ThreadPoolSvc
def ThreadPoolSvcCfg(flags):
    result = ComponentAccumulator()

    svc = CompFactory.ThreadPoolSvc(name="ThreadPoolSvc")
    svc.ThreadInitTools += [CompFactory.ThreadInitTool()]

    result.addService(svc)
    return result

# _______________ main _______________
if __name__ == "__main__":
    import sys
    
    from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg

    # Setup configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.RunNumbers = [284500]
    flags.Input.TimeStamps = [1] # dummy value
    flags.Input.TypedCollections = [] # workaround for building xAOD::EventInfo without input files
    flags.Exec.MaxEvents = 20
    flags.Scheduler.ShowControlFlow = True
    flags.Scheduler.ShowDataDeps = True
    flags.fillFromArgs()
    flags.lock()
    
    # Setup logging
    from AthenaCommon.Logging import log
    log.setLevel(flags.Exec.OutputLevel)
    
    # This example should run only in the multithreaded mode
    if flags.Concurrency.NumThreads < 1:
        log.fatal('The number of threads must be >0. Did you set the --threads=N option?')
        sys.exit(1)

    # The example runs with no input file. We configure it with the McEventSelector
    cfg = MainEvgenServicesCfg(flags,withSequences=True)

    # Configure all algorithms used by the example
    cfg.merge(HiveAlgAConf(flags))
    cfg.merge(HiveAlgBConf(flags))
    cfg.merge(HiveAlgCConf(flags))
    cfg.merge(HiveAlgDConf(flags))
    cfg.merge(HiveAlgEConf(flags))
    cfg.merge(HiveAlgFConf(flags))
    cfg.merge(HiveAlgGConf(flags))
    cfg.merge(HiveAlgVConf(flags))

    # Configure ThreadPoolSvc
    cfg.merge(ThreadPoolSvcCfg(flags))

    # Run the example
    sys.exit(cfg.run().isFailure())
