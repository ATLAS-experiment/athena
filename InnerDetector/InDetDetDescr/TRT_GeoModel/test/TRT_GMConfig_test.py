#!/usr/bin/env python
"""Run tests on TRT_GeoModel configuration

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
        from TRT_GeoModel.TRT_GeoModelConfig import TRT_SimulationGeometryCfg
        acc = TRT_SimulationGeometryCfg(flags)
        f=open('TRT_SimulationGeometryCfg.pkl','wb')
    else:
        from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
        acc = TRT_ReadoutGeometryCfg(flags)
        f=open('TRT_ReadoutGeometryCfg.pkl','wb')
    acc.store(f)
    f.close()
