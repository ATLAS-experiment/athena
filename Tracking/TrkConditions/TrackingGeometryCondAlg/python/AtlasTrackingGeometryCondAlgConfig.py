# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

def _setupCondDB(flags, CoolDataBaseFolder, quiet=True):

    result = ComponentAccumulator()

    # the tag names
    materialTagBase = 'AtlasLayerMat_v'
    version = 21
    sub_version = ''

    # Tag an input tag of form TagInfo{Major|Minor}/<tag> or TagInfo(Major|Minor}/<prefix>/<tag>
    # The tag information string defines how to handle the GeoAtlas in the TagInfo manager
    # TagInfo      : uses the geometry tag as is
    # TagInfoMajor : only uses the major version of the geometry tag
    # TagInfoMinor : uses both major and minor version of the geometry tag

    atlasMaterialTag = materialTagBase + str(version) + sub_version
    cfolder = CoolDataBaseFolder + '<tag>TagInfoMajor/' + \
        atlasMaterialTag + '_/GeoAtlas</tag>'

    if flags.GeoModel.Run < LHCPeriod.Run4:
        # load the right folders
        from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline
        result.merge(addFoldersSplitOnline(
            flags, 'GLOBAL', [cfolder], [cfolder],
            splitMC=True, className='Trk::LayerMaterialMap'))

    #redefine tag and folder for ITk, as we use a different schema
    else:
        atlasMaterialTag = flags.ITk.trackingGeometry.materialTag + \
            str(flags.ITk.trackingGeometry.version)
        if flags.ITk.trackingGeometry.loadLocalDbForMaterialMaps:
            DataBaseName = flags.ITk.trackingGeometry.localDatabaseName
            from IOVDbSvc.IOVDbSvcConfig import addFolders
            result.merge(addFolders(flags,
                                    "/GLOBAL/TrackingGeo/LayerMaterialITK",
                                    detDb=DataBaseName,
                                    tag=atlasMaterialTag,
                                    className='Trk::LayerMaterialMap'))
        else:
            materialFileTag = atlasMaterialTag + '_'+ \
                              flags.GeoModel.AtlasVersion
            from IOVDbSvc.IOVDbSvcConfig import addFolders
            result.merge(addFolders(flags,
                                    "/GLOBAL/TrackingGeo/LayerMaterialITK",
                                    "GLOBAL_OFL",
                                    tag=materialFileTag,
                                    db="OFLP200",
                                    className="Trk::LayerMaterialMap"))
    return result

def GeometryBuilderCfg(flags, name='AtlasGeometryBuilder',
                       useCond=True,
                       **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("WorldDimension", [])
    kwargs.setdefault("WorldMaterialProperties", [])

    if "TrackingVolumeArrayCreator" not in kwargs:
        from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import (
            TrackingVolumeArrayCreatorCfg)
        TrackingVolumeArrayCreator = result.popToolsAndMerge(
            TrackingVolumeArrayCreatorCfg(flags))
        result.addPublicTool(TrackingVolumeArrayCreator)
        kwargs.setdefault("TrackingVolumeArrayCreator",
                          TrackingVolumeArrayCreator)

    if "TrackingVolumeHelper" not in kwargs:
        from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import (
            TrackingVolumeHelperCfg)
        TrackingVolumeHelper = result.popToolsAndMerge(
            TrackingVolumeHelperCfg(flags))
        result.addPublicTool(TrackingVolumeHelper)
        kwargs.setdefault("TrackingVolumeHelper", TrackingVolumeHelper)

    # Depending on the job configuration, setup the various detector builders,
    # and add to atlas_geometry_builder
    if flags.Detector.GeometryID:
        from TrackingGeometryCondAlg.InDetTrackingGeometryConfig import (
            InDetTrackingGeometryBuilderCfg)
        kwargs.setdefault(
            "InDetTrackingGeometryBuilder", result.popToolsAndMerge(
                InDetTrackingGeometryBuilderCfg(flags, useCond=useCond)))

    elif flags.Detector.GeometryITk:
        from TrackingGeometryCondAlg.InDetTrackingGeometryConfig import (
            ITkTrackingGeometryBuilderCfg)
        kwargs.setdefault(
            "InDetTrackingGeometryBuilder", result.popToolsAndMerge(
                ITkTrackingGeometryBuilderCfg(flags, useCond=useCond)))

    ### The HGTD geometry is not available for the soon deprecated TrackingGeometrySvcConfig
    if flags.Detector.GeometryHGTD and useCond:
       from TrackingGeometryCondAlg.HGTD_TrackingGeometryConfig import (
            HGTD_TrackingGeometryBuilderCfg)
       kwargs.setdefault(
           "HGTD_TrackingGeometryBuilder", result.popToolsAndMerge(
               HGTD_TrackingGeometryBuilderCfg(flags)))

    if flags.Detector.GeometryCalo:
       from TrackingGeometryCondAlg.CaloTrackingGeometryConfig import (
            CaloTrackingGeometryBuilderCfg)
       kwargs.setdefault(
           "CaloTrackingGeometryBuilder", result.popToolsAndMerge(
               CaloTrackingGeometryBuilderCfg(flags, useCond=useCond)))

    if flags.Detector.GeometryMuon:
        from TrackingGeometryCondAlg.MuonTrackingGeometryConfig import (
            MuonTrackingGeometryBuilderCfg)
        kwargs.setdefault(
            "MuonTrackingGeometryBuilder", result.popToolsAndMerge(
                MuonTrackingGeometryBuilderCfg(flags, useCond=useCond)))

    geometryBuilder = CompFactory.Trk.GeometryBuilderCond(name, **kwargs) \
                      if useCond else \
                         CompFactory.Trk.GeometryBuilder(name, **kwargs)
    result.setPrivateTools(geometryBuilder)
    return result

@AccumulatorCache
def TrackingGeometryCondAlgCfg(flags, name='AtlasTrackingGeometryCondAlg',
                               doMaterialValidation=False,
                               **kwargs):
    """
    Sets up the Tracking Geometry Conditions Algorithm
    """
    result = ComponentAccumulator()

    kwargs.setdefault("GeometryBuilder", result.popToolsAndMerge(
        GeometryBuilderCfg(flags)))

    # Now set up processors
    atlas_geometry_processors = []

    if flags.TrackingGeometry.MaterialSource == 'COOL':
        if flags.GeoModel.Run < LHCPeriod.Run4:
            CoolDataBaseFolder = '/GLOBAL/TrackingGeo/LayerMaterialV2'
        else:
            CoolDataBaseFolder = '/GLOBAL/TrackingGeo/LayerMaterialITK'

        from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import (
            LayerMaterialProviderCfg)
        atlas_geometry_processors += [ result.popToolsAndMerge(
            LayerMaterialProviderCfg(flags,
                                     LayerMaterialMapName = CoolDataBaseFolder,
                                     LayerMaterialMapKey = CoolDataBaseFolder)) ]

        # Setup DBs
        result.merge(_setupCondDB(flags, CoolDataBaseFolder))

    elif flags.TrackingGeometry.MaterialSource == 'Input':
        from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import (
            InputLayerMaterialProviderCfg)
        atlas_geometry_processors += [ result.popToolsAndMerge(
            InputLayerMaterialProviderCfg(flags)) ]

    if doMaterialValidation:
        from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import (
            LayerMaterialInspectorCfg)
        atlas_geometry_processors += [ result.popToolsAndMerge(
            LayerMaterialInspectorCfg(flags)) ]

    from GaudiKernel.GaudiHandles import PrivateToolHandleArray
    kwargs.setdefault("GeometryProcessors", PrivateToolHandleArray(
        atlas_geometry_processors))
    kwargs.setdefault("TrackingGeometryWriteKey", 'AtlasTrackingGeometry')

    result.addCondAlgo(CompFactory.Trk.TrackingGeometryCondAlg(name, **kwargs),
                       primary=True)

    return result


if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()

    acc = TrackingGeometryCondAlgCfg(flags)

    f = open('TrackingGeometryCondAlgCfg.pkl', 'wb')
    acc.store(f)
    f.close()
