# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from AthenaCommon import Logging


def GeoDbTagSvcCfg(flags, name = "GeoDbTagSvc", **kwargs):
    result =ComponentAccumulator()
    from RDBAccessSvc.RDBAccessSvcConfig import RDBAccessSvcCfg
    result.merge(RDBAccessSvcCfg(flags))
    
    result.addService(CompFactory.GeoDbTagSvc(name, **kwargs))
    return result
def GeoModelCfg(flags):
    if not flags.GeoModel.AtlasVersion:
        raise ValueError('No geometry tag specified')

    from PyUtils.Helpers import release_metadata
    rel_metadata = release_metadata()
    relversion = rel_metadata['release'].split('.')
    if len(relversion) < 3:
        relversion = rel_metadata['base release'].split('.')

    result=ComponentAccumulator()

    from RDBAccessSvc.RDBAccessSvcConfig import RDBAccessSvcCfg
    result.merge(RDBAccessSvcCfg(flags))
    #Get DetDescrCnvSvc (for identifier dictionaries (identifier helpers)
    from DetDescrCnvSvc.DetDescrCnvSvcConfig import DetDescrCnvSvcCfg
    result.merge(GeoDbTagSvcCfg(flags))
    result.merge(DetDescrCnvSvcCfg(flags))
    


    #TagInfoMgr used by GeoModelSvc but no ServiceHandle. Relies on string-name
    from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
    result.merge(TagInfoMgrCfg(flags))
    nThreads = 0
    ### Use the try catch pattern as there's no clear indication whether the job
    #### is configured in the trigger environment or not. isOnine fails on MC...
    try:
        nThreads = flags.Concurrency.NumThreads
    except Exception:
        pass
    gms=CompFactory.GeoModelSvc(AtlasVersion=flags.GeoModel.AtlasVersion,
                                SQLiteDB=flags.GeoModel.SQLiteDB,
                                SQLiteDBFullPath=flags.GeoModel.SQLiteDBFullPath,
                                EMECStandard=flags.GeoModel.EMECStandard,
                                IgnoreTagDifference=flags.GeoModel.IgnoreTagDifference,
                                SupportedGeometry=int(relversion[0]),
                                nThreads = nThreads)
    if flags.Common.ProductionStep == ProductionStep.Simulation:
        ## Protects GeoModelSvc in the simulation from the AlignCallbacks
        gms.AlignCallbacks = False
    result.addService(gms, primary=True, create=True)

    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags

    flags = initConfigFlags()
    flags.Input.Files = []
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.lock()

    acc = GeoModelCfg(flags)
    with open("test.pkl", "wb") as f:
        acc.store(f)

    Logging.log.info("All OK")
