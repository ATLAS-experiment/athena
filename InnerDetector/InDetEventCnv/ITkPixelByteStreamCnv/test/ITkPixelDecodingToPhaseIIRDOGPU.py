#!/usr/bin/env python

#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Compare CPU and GPU decoding to Ph2 RDOs:
# RDO -> Encoder -> BS -> GPU Decoder -> Traccc cells -> Ph2 RDO
#                     \-> CPU decoder ---------------/ 

import sys
from AthenaCommon.Constants import DEBUG
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import (
   defaultGeometryTags, defaultConditionsTags, defaultTestFiles)
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from PixelReadoutGeometry.PixelReadoutGeometryConfig import ITkPixelReadoutManagerCfg
from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelEncodingAlgCfg
from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamToRawEventAlgCfg
from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelDecodingPhaseIIRDOAlgCfg
from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import PixelPhaseIIRDOVsPhaseIIRDOValidationAlgCfg
from ActsGPUEventCnv.ActsGPUEventCnvConfig import TracccCellsToPhaseIIRDOConverterAlgCfg
from InDetConfig.ConfigurationHelpers import OnlyTrackingPreInclude
from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelDecodingTracccCellsCUDAAlgCfg

if __name__=="__main__":
   flags = initConfigFlags()
   flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
   flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
   flags.Input.Files = defaultTestFiles.RDO_RUN4
   flags.PerfMon.doFullMonMT = True

   # The Detray (GPU) detector geometry is required by TracccCellsToPhaseIIRDOConverterAlgCfg
   flags.Acts.TrackingGeometry.UseBlueprint = True
   flags.Acts.TrackingGeometry.BuildDetrayGeometry = True
   OnlyTrackingPreInclude(flags)

   flags.lock()

   cfg = MainServicesCfg(flags)

   # Read RDOs
   cfg.merge(PoolReadCfg(flags))

   # Encode RDOs to BS
   cfg.merge(ITkPixelReadoutManagerCfg(flags, name="ITkPixelReadoutManager"))
   cfg.merge(ITkPixelEncodingAlgCfg(flags))

   # Feed the encoded BS to the ROBDataProvider
   cfg.merge(ByteStreamToRawEventAlgCfg(flags, RawContainers = ["ITkPixelRawCont"]))

   # Decode BS to Phase-II RDOs
   cfg.merge(ITkPixelDecodingPhaseIIRDOAlgCfg(flags, nRDOs = 1200000,
      pixelRDOKey = "cpu_decoded_rdos", FailOnDecodingError = True,
      # OutputLevel=DEBUG
      ))

   # Decode BS with a GPU; two steps: BS->cells on GPU, cells->RDOs on CPU
   cfg.merge(ITkPixelDecodingTracccCellsCUDAAlgCfg(flags,
      OutputLevel = DEBUG))
   cfg.merge(TracccCellsToPhaseIIRDOConverterAlgCfg(flags,
      PixelRDOs = "gpu_decoded_rdos",
      # OutputLevel = DEBUG
      ))

   # Check that decoded RDOs are the same as the original ones
   cfg.merge(PixelPhaseIIRDOVsPhaseIIRDOValidationAlgCfg(flags,
      ReferenceRDOs = 'cpu_decoded_rdos',
      MonitoredRDOs = 'gpu_decoded_rdos',
      # OutputLevel = DEBUG
      ))

   # Performance monitoring
   cfg.merge(PerfMonMTSvcCfg(flags))

   status = cfg.run(1)
   sys.exit(0 if status.isSuccess() else 1)
