#!/usr/bin/env athena.py
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
from InDetConfig.ConfigurationHelpers import OnlyTrackingPreInclude
from ActsConfig.ActsPhaseIIRawDataEdmConfig import (
    PhaseIIPixelRawDataContainerCfg,
    )
from ActsGPUGeometry.ActsGPUGeometryConfig import DeviceDetectorDescriptionCondAlgCfg
from ActsGPUEventCnv.ActsGPUEventCnvConfig import (
    PhaseIIRDOtoTracccCellConverterAlgCfg,
    TracccCellsToPhaseIIRDOConverterAlgCfg,
)
from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import PixelPhaseIIRDOValidationAlgCfg
from AthenaCommon.Constants import DEBUG
from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg

if __name__ == "__main__":
    flags = initConfigFlags()

    flags.Input.Files = defaultTestFiles.RDO_RUN4

    flags.Acts.TrackingGeometry.UseBlueprint = True
    flags.Acts.TrackingGeometry.BuildDetrayGeometry = True
    OnlyTrackingPreInclude(flags)

    flags.PerfMon.doFullMonMT = True

    flags.fillFromArgs()
    flags.lock()

    cfg = MainServicesCfg(flags)
    cfg.merge(PerfMonMTSvcCfg(flags))
    cfg.merge(PoolReadCfg(flags))
    msg_svc = cfg.getService('MessageSvc')
    msg_svc.Format = "%t % F%{:d}W%C%7W%R%T %0W%M".format(flags.Common.MsgSourceLength)

    cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
    # sg = cfg.getService("StoreGateSvc")
    # sg.Dump = True

    # Needed for PixelID and SCT_ID
    cfg.merge(ITkPixelReadoutGeometryCfg(flags))
    cfg.merge(ITkStripReadoutGeometryCfg(flags))

    # Detray detector description alg and service, and ID conversion maps
    cfg.merge(DeviceDetectorDescriptionCondAlgCfg(flags))

    # Make the PhaseII RDO containers (converted from legacy input RDOs)
    cfg.merge(PhaseIIPixelRawDataContainerCfg(flags))

    # Convert RDOs to Traccc cells
    cfg.merge(PhaseIIRDOtoTracccCellConverterAlgCfg(flags,
        TracccCells = "TracccCells",
        UsePixelToTForCellActivation = True,
        ConvertStripRDOs = False))

    # Convert (back) Traccc cells to RDOs
    cfg.merge(TracccCellsToPhaseIIRDOConverterAlgCfg(flags,
        TracccCells = "TracccCells",
        PixelRDOs = "monitored_pixel_rdos",
        StripRDOs = "monitored_strip_rdos",
        OutputLevel = DEBUG))

    # Validate by comparing the RDOs decoded from cells to the original Phase-II RDOs
    cfg.merge(PixelPhaseIIRDOValidationAlgCfg(flags,
        ReferenceRDOs = 'ITkPixelRDOs',
        MonitoredRDOs = 'monitored_pixel_rdos',
        OutputLevel = DEBUG))

    statusCode = cfg.run(1)
    assert statusCode.isSuccess(), "Application execution did not succeed"
