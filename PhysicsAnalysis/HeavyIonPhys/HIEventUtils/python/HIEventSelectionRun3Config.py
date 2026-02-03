#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator, ConfigurationError
from AthenaConfiguration.ComponentFactory import CompFactory

def HIEventSelectionRun3Cfg(flags):
    acc = ComponentAccumulator()
    filterTool = CompFactory.HI.HIEventSelectionToolRun3()

    # ZDC modules collection change its name
    zdcKey=None
    zdcNeeded=True  # in the future add check if ZDC info is required in fact
    if zdcNeeded and "ZDCModules" in flags.Input.Collections:
        zdcKey="ZDCModules"
    elif zdcNeeded and "ZdcSums" in flags.Input.Collections:
        zdcKey="ZdcSums"

    if zdcNeeded and not zdcKey:
        raise ConfigurationError("The input file does not have any zdcmodules (any capitalisation) container and ZDC info is needed for selection")

    filterAlg = CompFactory.HI.HIEventFilterAlgRun3(name="HIEventFilterAlgRun3",
                                                    SelectionTool=filterTool,
                                                    ZDC=zdcKey)
    acc.addEventAlgo(filterAlg)
    return acc

if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    # test needs to wait for files to be on CVMFS
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

    filterAlg = acc.getEventAlgo("HIEventFilterAlgRun3")
    # test if we can set custom selection (required python access to enums defined in C++)
    import ROOT
    filterAlg.SelectionMask=ROOT.HI.SelectionMask.NoEventError & ROOT.HI.SelectionMask.PUFCalVsZDCAny
    # test, in order to realy run needs to wait for files to be on CVMFS, 
    filterAlg.SelectionMask=ROOT.HI.SelectionMask.NoEventError
    filterAlg.UseIonDataTypeDefaultMask=False
    acc.printConfig(withDetails=True)
    # either
    status = acc.run()
    if status.isFailure():
        import sys
        sys.exit(-1)
