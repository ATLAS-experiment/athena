# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType
from ISF_Algorithms.CollectionMergerConfig import CollectionMergerCfg


def TileGeoG4SDCfg(flags, name="TileGeoG4SD", **kwargs):
    bare_collection_name = "TileHitVec"
    mergeable_collection_suffix = "_G4"
    merger_input_property = "TileHits"
    region = "CALO"

    result, hits_collection_name = CollectionMergerCfg(flags, bare_collection_name, mergeable_collection_suffix, merger_input_property, region)
    kwargs.setdefault("LogicalVolumeNames", ["Tile::Scintillator"])
    kwargs.setdefault("OutputCollectionNames", [hits_collection_name])

    kwargs.setdefault("TileCalculator", result.getPrimaryAndMerge(TileGeoG4SDCalcCfg(flags)).name )

    result.setPrivateTools(CompFactory.TileGeoG4SDTool(name, **kwargs))
    return result


def TileCTBGeoG4SDCfg(flags, name="TileCTBGeoG4SD", **kwargs):
    kwargs.setdefault("LogicalVolumeNames", ["Tile::Scintillator"])
    kwargs.setdefault("OutputCollectionNames", ["TileHitVec"])

    result = ComponentAccumulator()
    kwargs.setdefault("TileCalculator", result.getPrimaryAndMerge(TileCTBGeoG4SDCalcCfg(flags)).name )

    result.setPrivateTools(CompFactory.TileGeoG4SDTool(name, **kwargs))
    return result


def TileGeoG4SDCalcCfg(flags, name="TileGeoG4SDCalc", **kwargs):
    result = ComponentAccumulator()

    if flags.Beam.Type is BeamType.Cosmics or flags.Sim.ReadTR:
        kwargs.setdefault("DeltaTHit", [1])
        kwargs.setdefault("DoTOFCorrection", False)
    kwargs.setdefault("DoCalibHitParticleID", flags.Sim.ParticleID )

    result.addService(CompFactory.TileGeoG4SDCalc(name, **kwargs), primary=True)
    return result


def TileCTBGeoG4SDCalcCfg(flags, name="TileCTBGeoG4SDCalc", **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("TileTB", True)
    kwargs.setdefault("DoCalibHitParticleID", flags.Sim.ParticleID )

    if flags.Tile.Sim.DeltaTHit != 'NONE':
        kwargs.setdefault("DeltaTHit", flags.Tile.Sim.DeltaTHit)
    if flags.Tile.Sim.TimeCut != 'NONE':
        kwargs.setdefault("TimeCut", flags.Tile.Sim.TimeCut)
    if flags.Tile.Sim.PlateToCell != 'NONE':
        kwargs.setdefault("PlateToCell", flags.Tile.Sim.PlateToCell)
    if flags.Tile.Sim.doTileRow != 'NONE':
        kwargs.setdefault("DoTileRow", flags.Tile.Sim.doTileRow)
    if flags.Tile.Sim.doTOFCorrection != 'NONE':
        kwargs.setdefault("DoTOFCorrection", flags.Tile.Sim.doTOFCorrection)
    if flags.Tile.Sim.doBirk != 'NONE':
        kwargs.setdefault("DoBirk", flags.Tile.Sim.doBirk)
    if any([flags.Tile.Sim.OldBirk != 'NONE', flags.Tile.Sim.Birk1 != 'NONE', flags.Tile.Sim.Birk2 != 'NONE']):
        import AthenaCommon.SystemOfUnits as Units
        gramsPerMeVcmSq = Units.g / (Units.MeV * Units.cm2)
        if flags.Tile.Sim.OldBirk != 'NONE':
            kwargs.setdefault("birk1", 0.0130 * gramsPerMeVcmSq)
            kwargs.setdefault("birk2", 9.6e-6 * gramsPerMeVcmSq * gramsPerMeVcmSq)
        if flags.Tile.Sim.Birk1 != 'NONE':
            kwargs.setdefault("birk1", flags.Tile.Sim.Birk1 * gramsPerMeVcmSq)
        if flags.Tile.Sim.Birk2 != 'NONE':
            kwargs.setdefault("birk2", flags.Tile.Sim.Birk2 *gramsPerMeVcmSq * gramsPerMeVcmSq)

    result.addService(CompFactory.TileGeoG4SDCalc(name, **kwargs), primary=True)
    return result
