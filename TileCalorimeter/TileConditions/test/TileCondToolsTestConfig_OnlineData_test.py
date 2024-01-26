#!/usr/bin/env python
"""Run a test on Tile conditions algorithms configuration on data online

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""

if __name__ == "__main__":

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    from TileConfiguration.TileConfigFlags import TileRunType
    
    # test setup
    log.setLevel(DEBUG)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Common.isOnline = True
    flags.Tile.RunType = TileRunType.PHY
    flags.lock()

    acc = ComponentAccumulator()

    from TileConditions.TileCondToolsTestConfig import TileCondToolsTestCfg
    acc.merge( TileCondToolsTestCfg(flags) )

    acc.printConfig(withDetails = True, summariseProps = True)
    print(acc.getService('IOVDbSvc'))
    acc.store( open('TileCondToolsOnlineData.pkl','wb') )

    print('All OK')
