#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ITkStrip/v2/data_test.00242020.EventStorage_StreamBSFileOutput.daq.RAW._lb0010._Athena._0001.data"]
    from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
    flags.Input.isMC = True
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
    flags.Detector.GeometryITkStrip = True
    flags.ITk.Geometry.AllLocal = False

    # We want to keet the commented code for debugging
    #from AthenaCommon.Constants import DEBUG
    #flags.Exec.OutputLevel=DEBUG

    flags.Output.RDOFileName = "RDO.pool.root"    

    flags.fillFromArgs()
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    # For ByteStream file reading
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))

    from ITkStripsByteStreamCnv.ITkStripByteStreamCnvConfig import ITkStripRawDataProviderCfg
    acc.merge(ITkStripRawDataProviderCfg(flags))

    itemList = [] # items to store in RDO
    acceptAlgs = [] # skimming algs

    # We want to keet the commented code for debugging    
    itemList.append('xAOD::EventInfo#EventInfo')
    itemList.append('xAOD::EventAuxInfo#EventInfoAux.')
    itemList.append('SCT_RDO_Container#ITkStripRDOs')    
    itemList.append('IDCInDetBSErrContainer#SCT_ByteStreamErrs')
    
    from OutputStreamAthenaPool.OutputStreamConfig import (OutputStreamCfg, addToMetaData, outputStreamName)
    from AthenaConfiguration.ComponentFactory import CompFactory
    acc.merge(OutputStreamCfg(flags, 'RDO', itemList, AcceptAlgs=acceptAlgs))
    acc.merge(
        addToMetaData(
            flags,
            streamName="RDO",
            itemOrList=[
                f"xAOD::EventFormat#EventFormat{outputStreamName('RDO')}",
                "xAOD::FileMetaData#FileMetaData",
                "xAOD::FileMetaDataAuxInfo#FileMetaDataAux.",
            ],
            HelperTools=[
                CompFactory.xAODMaker.EventFormatStreamHelperTool(
                    f"{outputStreamName('RDO')}_EventFormatStreamHelperTool",
                    Key=f"EventFormat{outputStreamName('RDO')}",
                    DataHeaderKey=f"{outputStreamName('RDO')}",
                    TypeNames=[
                        "SCT_RDO_Container#ITkStripRDOs",
                        "IDCInDetBSErrContainer#SCT_ByteStreamErrs",
                    ],
                ),
                CompFactory.xAODMaker.FileMetaDataCreatorTool(
                    f"{outputStreamName('RDO')}_FileMetaDataCreatorTool",
                    OutputKey="FileMetaData",
                    StreamName=f"{outputStreamName('RDO')}",
                ),
            ],
        )
    )
              
    acc.run(maxEvents=1)

    
