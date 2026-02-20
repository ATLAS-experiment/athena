#!/usr/bin/env athena.py
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG, INFO

# --------------------------------------------------------------------
# Configure HistAlg and NtupAlg algorithms

# helper function to add xAOD::EventInfo creating algorythm
# both HistAlg and NtupAlg access it as input data
def addEventInfoConv(flags):
    result = ComponentAccumulator()
    from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
    EICA = EventInfoCnvAlgCfg(flags, inputKey="McEventInfo", outputKey="EventInfo", disableBeamSpot=True, OutputLevel=DEBUG)
    result.merge(EICA, sequenceName="AthAlgSeq")
    # Select HBOOK or ROOT persistency (ROOT is default)
    result.setAppProperty("HistogramPersistency","ROOT")
    return result


def HistAlgConf(flags):
    result = ComponentAccumulator()

    result.merge( addEventInfoConv(flags) )

    alg = CompFactory.AthEx.Hist("Hist", OutputLevel=DEBUG )
    result.addEventAlgo(alg)
    
    # Configure the histogram output file, alias 'stat'
    svc = CompFactory.THistSvc("THistSvc", OutputLevel=INFO)
    svc.Output += [ "stat DATAFILE='hist.root' OPT='RECREATE'" ]    
    result.addService(svc)

    return result

def NtupAlgConf(flags):
    result = ComponentAccumulator()

    result.merge( addEventInfoConv(flags) )

    alg = CompFactory.AthEx.Ntup("Ntup", OutputLevel=DEBUG)
    result.addEventAlgo(alg)

    svc = CompFactory.THistSvc("THistSvc", OutputLevel=INFO)
    # Ntuples output file, alias 'rec'
    svc.Output += [ "rec DATAFILE='ntuple.root' OPT='RECREATE'" ]
    
    result.addService(svc)
    return result


# _______________ main _______________
if __name__ == "__main__":
    # Setup configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Input.RunNumbers = [12345]
    flags.Input.TimeStamps = [1] # dummy value
    flags.Input.TypedCollections = [] # workaround for building xAOD::EventInfo without input files
    flags.Exec.MaxEvents = 10
    flags.Scheduler.ShowControlFlow = True
    flags.Scheduler.ShowDataDeps = True
    flags.fillFromArgs()
    flags.lock()
    
    # Setup python logging level
    from AthenaCommon.Logging import log
    log.setLevel(flags.Exec.OutputLevel)
    
    # The example runs with no input file. We configure it with the McEventSelector
    from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
    cfg = MainEvgenServicesCfg(flags,withSequences=True)

    # Configure the C++ message service
    msgSvc = CompFactory.MessageSvc("MessageSvc", useColors=True)
    cfg.addService(msgSvc)

    # Configure all algorithms used by the example
    cfg.merge(HistAlgConf(flags))
    cfg.merge(NtupAlgConf(flags))
    
    # Run the example
    import sys
    sys.exit(cfg.run().isFailure())

