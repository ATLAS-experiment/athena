#!/usr/bin/env python

#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Runs a roundtrip of encoding and decoding for ITk Pixel byte streams
# RDO -> Encoder -> BS -> Decoder -> RDO
# And it checks that the resultins RDOs are the same as the input.

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
from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import PixelPhaseIIRDOValidationAlgCfg

PERFORMANCE_TESTING = False

if __name__=="__main__":
   flags = initConfigFlags()
   flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
   flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
   flags.Input.Files = defaultTestFiles.RDO_RUN4

   flags.Exec.MaxEvents = 10
   if PERFORMANCE_TESTING:
      flags.PerfMon.doFullMonMT = True
      flags.Exec.MaxEvents = 100

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
      pixelRDOKey = "decoded_rdos", FailOnDecodingError = True, OutputLevel=DEBUG))

   # Check that decoded RDOs are the same as the original ones
   cfg.merge(PixelPhaseIIRDOValidationAlgCfg(flags,
      ReferenceRDOs = 'ITkPixelRDOs',
      MonitoredRDOs = 'decoded_rdos',
      OutputLevel = DEBUG))

   if PERFORMANCE_TESTING:
      from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
      cfg.merge(PerfMonMTSvcCfg(flags))

   status = cfg.run()
   sys.exit(0 if status.isSuccess() else 1)
