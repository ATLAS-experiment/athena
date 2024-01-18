# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep

def TrackingGeometrySvcCfg(flags , name = 'AtlasTrackingGeometrySvc',
                           doMaterialValidation=False,
                           **kwargs):
    """
    Sets up the Tracking Geometry Service
    """
    if not (flags.Common.ProductionStep in [ProductionStep.Simulation, ProductionStep.FastChain] or
            flags.Sim.ISFRun is True):
        from AthenaCommon.Logging import logging
        mlog = logging.getLogger("TrackingGeometrySvcCfg")
        mlog.warning(
            " TrackingGeometrySvc is to be deprecated in favour of TrackingGeometryCondAlg")

    result = ComponentAccumulator()

    from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import (
        GeometryBuilderCfg)
    kwargs.setdefault("GeometryBuilder", result.popToolsAndMerge(
        GeometryBuilderCfg(flags, useCond=False)))

    # Now set up processors
    atlas_geometry_processors=[] # array of private ToolHandles

    if flags.TrackingGeometry.MaterialSource == 'COOL':
        CoolDataBaseFolder = '/GLOBAL/TrackingGeo/LayerMaterialV2'
        if flags.Detector.GeometryITk:
            CoolDataBaseFolder = '/GLOBAL/TrackingGeo/LayerMaterialITK'

        from TrackingGeometryCondAlg.TrkDetDescrToolsConfig import (
            LayerMaterialProviderCfg)
        atlas_geometry_processors += [ result.popToolsAndMerge(
            LayerMaterialProviderCfg(flags, name='AtlasMaterialProvider',
                                     LayerMaterialMapName = CoolDataBaseFolder,
                                     LayerMaterialMapKey = '')) ] # Different from AtlasTrackingGeometryCondAlgConfig

        # Setup DBs
        from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import _setupCondDB
        result.merge(_setupCondDB(flags, CoolDataBaseFolder))

    elif  flags.TrackingGeometry.MaterialSource == 'Input':
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
    kwargs.setdefault("TrackingGeometryName", 'AtlasTrackingGeometry')
    kwargs.setdefault("BuildGeometryFromTagInfo", True)

    result.addService(CompFactory.Trk.TrackingGeometrySvc(name, **kwargs),
                      primary = True)

    return result

if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()

    acc = TrackingGeometrySvcCfg(flags)

    f=open('TrackingGeometrySvcCfg.pkl','wb')
    acc.store(f)
    f.close()
