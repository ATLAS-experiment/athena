# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SpacePointReaderCfg(flags,
                        name: str,
                        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.ActsTrk.SpacePointReader(name=name, **kwargs))
    return acc

def ActsPoolReadCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    import re
    typedCollections = flags.Input.TypedCollections
    for col in typedCollections:
        match = re.findall(r"xAOD::SpacePointContainer#(\S+)", col)
        if match:
            spCol = match[0]
            acc.merge(SpacePointReaderCfg(flags,
                                          name=f"{spCol}Reader",
                                          SpacePointKey=spCol))

    return acc

def ReadActsTracksCfg(flags, prefix=""):
    """
    Setup algorithm that reads xAOD track backends and produced TrackContainer
    name - the collections prefix, for consistency it also is the prefix of the output container name   
    """
    acc = ComponentAccumulator()
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
    acc.addEventAlgo(CompFactory.ActsTrk.TrackContainerReader("ActsTrkTrackContainerReader"+prefix,
                                                               TrackContainer=prefix+"TrackContainer",
                                                               TrackingGeometryTool=acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))
                                                              ))
    return acc

if __name__ == "__main__":
    # test reading
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    flags = initConfigFlags()
    flags.fillFromArgs()
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    cfg.merge(ReadActsTracksCfg(flags, prefix="SiSPSeededActs"))

    status = cfg.run()
    if status.isFailure():
        import sys
        sys.exit("Execution failed")
