#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator, ConfigurationError
from AthenaConfiguration.ComponentFactory import CompFactory
import AthenaCommon.SystemOfUnits as Units
from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_HILoose_Cfg

def HIEventSelectionRun3Cfg(flags):
    acc = ComponentAccumulator()
    # in future decide cut level for tracks depending on input dataset
    # for now default to HILoose cuts set with 0.5 GeV
    trackSelectionTool = acc.popToolsAndMerge(InDetTrackSelectionTool_HILoose_Cfg(flags,
                                                                                  minPt=0.5*Units.GeV))
    filterTool = CompFactory.HI.HIEventSelectionToolRun3(TrackSelectionTool=trackSelectionTool)

    # ZDC modules collection change its name
    zdcKey=None
    zdcNeeded=True  # in the future add check if ZDC info is required in fact
    if zdcNeeded and "ZDCModules" in flags.Input.Collections:
        zdcKey="ZDCModules"
    elif zdcNeeded and "ZdcSums" in flags.Input.Collections:
        zdcKey="ZdcSums"

    if zdcNeeded and not zdcKey:
        raise ConfigurationError("The input file does not have any ZDCModules (any capitalisation) container and ZDC info is needed for selection")

    filterAlg = CompFactory.HI.HIEventFilterAlgRun3(name="HIEventFilterAlgRun3",
                                                    SelectionTool=filterTool,
                                                    ZDC=zdcKey)
    acc.addEventAlgo(filterAlg)
    return acc

if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    data_hi="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/data_hi/"
    test_files = {"23":"data23_hi.00463364.physics_HardProbes.AOD.r16069_p6447_skim",
                  "24":"data24_hi.00490145.physics_HardProbes.AOD.f1550_m2267_skim",
                  "25OO": "data25_hi.00501859.physics_MinBias.AOD.f1606_m2272_skim", 
                  "25NeNe": "data25_hi.00502008.physics_MinBias.AOD.f1606_m2272_skim",
                  "25pO": "data25_hip.00501607.physics_MinBias.AOD.f1604_m2272_skim" }
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    flags = initConfigFlags()
    flags.addFlag("HIPeriodToTest", "23")
    flags.Exec.MaxEvents=10
    flags.Input.Files=lambda fl: [data_hi+test_files[fl.HIPeriodToTest]]
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
