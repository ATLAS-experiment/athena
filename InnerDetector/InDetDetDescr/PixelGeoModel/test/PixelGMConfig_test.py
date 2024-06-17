#!/usr/bin/env python
"""Run tests on PixelGeoModel configuration

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.Enums import Project
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.GeoModel.Align.Dynamic = False
    flags.lock()

    if flags.Common.Project is Project.AthSimulation:
        from PixelGeoModel.PixelGeoModelConfig import PixelSimulationGeometryCfg
        acc = PixelSimulationGeometryCfg(flags)
        f=open('PixelSimulationGeometryCfg.pkl','wb')
    else:
        from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
        acc = PixelReadoutGeometryCfg(flags)
        f=open('PixelReadoutGeometryCfg.pkl','wb')
    acc.store(f)
    f.close()
