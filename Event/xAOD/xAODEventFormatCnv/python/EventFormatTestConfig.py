# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration import AllConfigFlags, MainServicesConfig
from AthenaConfiguration.ComponentFactory import CompFactory
from OutputStreamAthenaPool.OutputStreamConfig import (
    OutputStreamCfg,
    addToMetaData,
    outputStreamName,
)


def EventFormatTestFlags(eventsPerFile=5, inputFiles=None):
    if inputFiles is None:
        inputFiles = []

    flags = AllConfigFlags.initConfigFlags()
    flags.Input.Files = inputFiles
    flags.Concurrency.NumThreads = 8
    flags.Exec.MaxEvents = eventsPerFile

    return flags


def EventFormatTestOutputCfg(
    flags,
    streamName="Test",
    numberOfStreams=5,
    itemList=None,
):
    if itemList is None:
        itemList = []

    acc = MainServicesConfig.MainServicesCfg(flags)

    for i in range(numberOfStreams):
        acc.merge(
            OutputStreamCfg(
                flags,
                streamName=f"{streamName}{i}",
                ItemList=itemList,
                disableEventTag=True,
            )
        )

        acc.merge(
            addToMetaData(
                flags,
                streamName=f"{streamName}{i}",
                itemOrList=[
                    f"xAOD::EventFormat#EventFormat{outputStreamName(streamName)}{i}",
                    "xAOD::FileMetaData#FileMetaData",
                    "xAOD::FileMetaDataAuxInfo#FileMetaDataAux.",
                ],
                HelperTools=[
                    CompFactory.xAODMaker.EventFormatStreamHelperTool(
                        f"{outputStreamName(streamName)}{i}_EventFormatStreamHelperTool",
                        Key=f"EventFormat{outputStreamName(streamName)}{i}",
                        DataHeaderKey=f"{outputStreamName(streamName)}{i}",
                        TypeNames=[".*xAODMakerTest::.*"],
                        OutputLevel=1,
                    ),
                    CompFactory.xAODMaker.FileMetaDataCreatorTool(
                        f"{outputStreamName(streamName)}{i}_FileMetaDataCreatorTool",
                        OutputKey="FileMetaData",
                        StreamName=f"{outputStreamName(streamName)}{i}",
                    ),
                ],
            )
        )

    return acc
