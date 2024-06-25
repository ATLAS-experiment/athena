#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PRDxAODConvertorAlgCfg(flags,
                             name: str = "PRDxAODConvertorAlg",
                             **kwargs):
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.Muon.PRDxAODConvertorAlg(name, **kwargs))
    return acc

def RunPRDConversion():
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    flags = initConfigFlags()
    args = flags.fillFromArgs()

    flags.Input.Files = [
        '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/ESD/ATLAS-P2-RUN4-03-00-00/ESD.ttbar_mu0.pool.root']
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.CheckDependencies = True
    flags.lock()
    flags.dump()

    if not args.config_only:
        from AthenaConfiguration.MainServicesConfig import MainServicesCfg
        cfg = MainServicesCfg(flags)
    else:
        cfg = ComponentAccumulator()
    
    cfg.merge(PoolReadCfg(flags))
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg 
    cfg.merge( MuonGeoModelCfg(flags) )

    # Now setup the convertor
    acc = PRDxAODConvertorAlgCfg(
        flags, OutputLevel=1)
    cfg.merge(acc)

    if not args.config_only:
        sc = cfg.run()
        if not sc.isSuccess():
            import sys
            sys.exit("Execution failed")

if "__main__" == __name__:
    RunPRDConversion()

   

