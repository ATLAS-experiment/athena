#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ITkStrip/data_test.00242020.EventStorage_StreamBSFileOutput.daq.RAW._lb0002._Athena._0001.data"]
    flags.IOVDb.GlobalTag = "OFLCOND-MC15c-SDR-14-05"
    flags.Input.isMC = True
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
    flags.Detector.GeometryITkStrip = True
    flags.ITk.Geometry.AllLocal = False

    # for debugging
    from AthenaCommon.Constants import INFO
    flags.Exec.OutputLevel=INFO

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    # For ByteStream file reading
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))

    from ITkStripsByteStreamCnv.ITkStripRawDataByteStreamCnvConfig import ITkStripRawDataProviderCfg
    acc.merge(ITkStripRawDataProviderCfg(flags))
              
    acc.run(maxEvents=2)

    
