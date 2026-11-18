#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Check that the ITkPixel region covers its configured sensitive volumes."""

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.Enums import LHCPeriod, ProductionStep
from G4AtlasTools.G4PhysicsRegionConfig import ITkPixelPhysicsRegionToolCfg
from PixelG4_SD.PixelG4_SDToolConfig import ITkPixelSensorSDCfg


if __name__ == '__main__':
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Common.ProductionStep = ProductionStep.Simulation
    flags.GeoModel.Run = LHCPeriod.Run4
    flags.Detector.GeometryITkPixel = True
    flags.Sim.ISFRun = False
    flags.lock()

    region_acc = ITkPixelPhysicsRegionToolCfg(flags)
    region = region_acc.popPrivateTools()
    sensor_acc = ITkPixelSensorSDCfg(flags)
    sensor = sensor_acc.popPrivateTools()

    # Compare the independent SD and region configurations so that adding a
    # sensor cannot silently leave it with the inherited production cuts.
    # PLR has a separate SD configuration and is outside this check.
    missing = set(sensor.LogicalVolumeNames) - set(region.VolumeList)
    assert not missing, (
        f'ITkPixel sensitive volumes missing from region VolumeList: {sorted(missing)}')
    assert region.RegionName == 'ITkPixel'
    for cut in ('ElectronCut', 'PositronCut', 'GammaCut'):
        assert getattr(region, cut) == 0.05, f'Unexpected ITkPixel {cut}'
