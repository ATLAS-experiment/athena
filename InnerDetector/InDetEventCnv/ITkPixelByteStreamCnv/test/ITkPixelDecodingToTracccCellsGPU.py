#!/usr/bin/env python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Runs the CPU and CUDA Phase-II ITk Pixel decoders on the same event of the
# same bytestream and compares their RDO containers (PixelRDOs vs ITkPixelRDOsCUDA).

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import (
    defaultConditionsTags,
    defaultGeometryTags,
    defaultTestFiles,
)
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from PixelReadoutGeometry.PixelReadoutGeometryConfig import ITkPixelReadoutManagerCfg
from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelEncodingAlgCfg
from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamToRawEventAlgCfg
from ActsGPUGeometry.ActsGPUGeometryConfig import DeviceDetectorDescriptionCondAlgCfg
from InDetConfig.ConfigurationHelpers import OnlyTrackingPreInclude
from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelDecodingTracccCellsCUDAAlgCfg
from ActsGPUEventCnv.ActsGPUEventCnvConfig import (
    RDOtoTracccCellConverterAlgCfg,
    TracccCellValidationAlgCfg,
)
from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
# from AthenaCommon.Constants import DEBUG
import sys

if __name__ == "__main__":
    flags = initConfigFlags()

    flags.Input.Files = defaultTestFiles.RDO_RUN4

    # We need the Detray geometry for conversion of athena to detray indices
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
    # Detector description service configuration: make detray geometry
    flags.Acts.TrackingGeometry.UseBlueprint = True
    flags.Acts.TrackingGeometry.BuildDetrayGeometry = True
    # Keep calo/muon out of the tracking geometry: the calo volumes cannot be
    # converted to a consistent Detray geometry
    OnlyTrackingPreInclude(flags)

    flags.Exec.MaxEvents = 1
    flags.fillFromArgs()
    flags.PerfMon.doFullMonMT = True

    flags.lock()

    cfg = MainServicesCfg(flags)

    # # For Athena Pixel module IDs to device detector indices
    cfg.merge(DeviceDetectorDescriptionCondAlgCfg(flags))

    # Read RDOs
    cfg.merge(PoolReadCfg(flags))

    # Encode RDOs to byte streams (BS)
    cfg.merge(ITkPixelReadoutManagerCfg(flags, name="ITkPixelReadoutManager"))
    cfg.merge(ITkPixelEncodingAlgCfg(flags))

    # Feed the encoded BS to the ROBDataProvider
    cfg.merge(ByteStreamToRawEventAlgCfg(flags, RawContainers = ["ITkPixelRawCont"]))

    reference_cells = "reference_cells"
    monitored_cells = "gpu_decoded_cells"
    # Decode BS to Traccc cells on GPU
    cfg.merge(ITkPixelDecodingTracccCellsCUDAAlgCfg(flags,
        TracccCells = monitored_cells,
        # OutputLevel = DEBUG
        ))
    # Convert RDOs to Traccc cells to create a reference
    cfg.merge(RDOtoTracccCellConverterAlgCfg(flags,
        TracccCells = reference_cells,
        ConvertStripRDOs = False,
        UsePixelToTForCellActivation = True,
        # OutputLevel = DEBUG
        ))
    # Validate by comparing the two produced containers
    cfg.merge(TracccCellValidationAlgCfg(flags,
        ReferenceCells = reference_cells,
        Cells = monitored_cells,
        SortBeforeCompare = True,
        # OutputLevel = DEBUG
    ))

    cfg.merge(PerfMonMTSvcCfg(flags))
    status = cfg.run()
    sys.exit(0 if status.isSuccess() else 1)
