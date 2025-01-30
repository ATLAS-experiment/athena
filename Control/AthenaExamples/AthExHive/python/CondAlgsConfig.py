#!/usr/bin/env athena.py
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG

import sys

# --------------------------------------------------------------------
# Configure ASCIICondDbSvc

def ASCIICondDbSvcConf(flags,name="ASCIICondDbSvc",**kwargs):
    import os
    from AthenaCommon.Logging import log
    log.setLevel(flags.Exec.OutputLevel)
    
    condDbFile = "condDb.txt"
    found = 0
    for dir in (".:"+os.environ.get('DATAPATH')).split (':'):
        cdb = os.path.join(dir,condDbFile)
        if (os.path.isfile( cdb ) ) :
            found = 1
            break

    if (found == 0):
        log.fatal('ASCII condDb file \"' + condDbFile + '\" not found')
        sys.exit(1)
    else:
        log.info('using ASCIICondDb file from ' + cdb)

    kwargs.setdefault("CondFile",cdb)
    result = ComponentAccumulator()
    svc = CompFactory.ASCIICondDbSvc(name,**kwargs)
    result.addService(svc)
    return result

if __name__ == "__main__":
    # Setup configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.RunNumbers = [1]
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
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    cfg.merge(McEventSelectorCfg(flags,
        RunNumber=1,
        FirstEvent=1,
        InitialTimeStamp=0,
        TimeStampInterval=1,
        FirstLB=1))

    # Configure all algorithms used by the example
    # xAOD::EventInfo converter (because it is required by some algorithms in this example)
    from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
    cfg.merge(EventInfoCnvAlgCfg(flags, disableBeamSpot=True), sequenceName="AthAlgSeq")
    # Event algorithms
    cfg.addEventAlgo(CompFactory.AlgA(name="AlgA",OutputLevel=DEBUG))
    cfg.addEventAlgo(CompFactory.AlgB(name="AlgB",OutputLevel=DEBUG,Key_R1="a1",Key_W1="a3"))
    cfg.addEventAlgo(CompFactory.AlgC(name="AlgC1",OutputLevel=DEBUG,Key_R1="a2",Key_CH="X1"))
    cfg.addEventAlgo(CompFactory.AlgC(name="AlgC2",OutputLevel=DEBUG,Key_R1="a1",Key_CH="X2"))
    cfg.addEventAlgo(CompFactory.AlgD(name="AlgD1",OutputLevel=DEBUG,Key_R1="a3", Key_CH1="X1", Key_CH2="X2"))
    # Condition algorithms
    cfg.addCondAlgo(CompFactory.CondAlgX(name="CondAlgX1",OutputLevel=DEBUG,Key_CH="X1",Key_DB="X1"))
    cfg.addCondAlgo(CompFactory.CondAlgX(name="CondAlgX2",OutputLevel=DEBUG,Key_CH="X2",Key_DB="X2"))
    cfg.addCondAlgo(CompFactory.CondAlgY(name="CondAlgY1",OutputLevel=DEBUG,Key_CH1="Y1",Key_CH2="Y2",Key_DB1="Y1",Key_DB2="Y2"))

    # Configure ASCIICondDbSvc
    cfg.merge(ASCIICondDbSvcConf(flags,OutputLevel=DEBUG))

    # Run the example
    sys.exit(cfg.run().isFailure())
