#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def HIEventSelectionRun3Cfg(flags):
    acc = ComponentAccumulator()
    filterTool = CompFactory.HI.HIEventSelectionToolRun3()
    filterAlg = CompFactory.HI.HIEventFilterAlgRun3(name="HIEventFilterAlgRun3", SelectionTool=filterTool)
    acc.addEventAlgo(filterAlg)
    return acc

if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    inputAOD = f"{defaultTestFiles.d}/DerivationFrameworkART/data18_hi.00365602.physics_HardProbes.merge.AOD.f1021_m2037._lb0203._0001.1"
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    flags = initConfigFlags()
    flags.Exec.MaxEvents=10
    flags.Input.Files=[inputAOD]
    flags.lock()

    acc=MainServicesCfg(flags)
    # if need to read POOL file
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))
    from AthenaCommon.Constants import DEBUG

    acc.merge(HIEventSelectionRun3Cfg(flags))
    acc.foreach_component("**/AthAlgSeq/*Run3*").OutputLevel=DEBUG

    acc.printConfig(withDetails=True)
    # either
    status = acc.run()
    if status.isFailure():
        import sys
        sys.exit(-1)
