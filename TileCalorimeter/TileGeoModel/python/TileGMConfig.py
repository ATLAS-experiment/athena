# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep, BeamType
from AtlasGeoModel.GeoModelConfig import GeoModelCfg

def TileGMCfg(flags):
    result=GeoModelCfg(flags)
    result.getPrimary().DetectorTools += [ CompFactory.TileDetectorTool() ]
    if flags.Common.ProductionStep not in [ProductionStep.Simulation, ProductionStep.FastChain]:
        result.getPrimary().DetectorTools["TileDetectorTool"].GeometryConfig = "RECO"
    if flags.Common.ProductionStep in [ProductionStep.Simulation, ProductionStep.Digitization] and flags.Beam.Type is BeamType.TestBeam:
        if (flags.TestBeam.Layout=='tb_Tile2000_2003_2B2EB'):
            # 2 Barrels + 2 Extended Barrels
            result.getPrimary().TileVersionOverride='TileTB-2B2EB-00'
        elif (flags.TestBeam.Layout=='tb_Tile2000_2003_2B1EB'):
            # 2 Barrels + 1 Extended Barrel
            result.getPrimary().TileVersionOverride='TileTB-2B1EB-00'
        elif (flags.TestBeam.Layout=='tb_Tile2000_2003_3B'):
            # 3 Barrels
            result.getPrimary().TileVersionOverride='TileTB-3B-00'
        elif (flags.TestBeam.Layout=='tb_Tile2000_2003_5B'):
            # 5 Barrels
            result.getPrimary().TileVersionOverride='TileTB-5B-00'

    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.lock()

    acc = TileGMCfg( flags )
    acc.store( open( "test.pkl", "wb" ) )
    print("All OK")
