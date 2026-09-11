#!/usr/bin/env python
"""Run tests on StripGeoModelXml configuration

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.Enums import Project
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
    flags.GeoModel.Align.Dynamic = False
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
    flags.lock()

    if flags.Common.Project is Project.AthSimulation:
        from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripSimulationGeometryCfg
        acc = ITkStripSimulationGeometryCfg(flags)
        f=open('ITkStripSimulationGeometryCfg.pkl','wb')
    else:
        from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
        acc = ITkStripReadoutGeometryCfg(flags)
        f=open('ITkStripReadoutGeometryCfg.pkl','wb')
    acc.store(f)
    f.close()
