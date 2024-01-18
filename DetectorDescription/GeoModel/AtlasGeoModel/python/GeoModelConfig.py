# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from AthenaCommon import Logging


def GeoModelCfg(flags):
    from PyUtils.Helpers import release_metadata
    rel_metadata = release_metadata()
    relversion = rel_metadata['release'].split('.')
    if len(relversion) < 3:
        relversion = rel_metadata['base release'].split('.')

    result=ComponentAccumulator()

    #Get DetDescrCnvSvc (for identifier dictionaries (identifier helpers)
    from DetDescrCnvSvc.DetDescrCnvSvcConfig import DetDescrCnvSvcCfg
    result.merge(DetDescrCnvSvcCfg(flags))

    #TagInfoMgr used by GeoModelSvc but no ServiceHandle. Relies on string-name
    from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
    result.merge(TagInfoMgrCfg(flags))

    gms=CompFactory.GeoModelSvc(AtlasVersion=flags.GeoModel.AtlasVersion,
                                SQLiteDB=flags.GeoModel.SQLiteDB,
                                IgnoreTagDifference=flags.GeoModel.IgnoreTagDifference,
                                SupportedGeometry=int(relversion[0]))
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
