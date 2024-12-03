# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from dataclasses import dataclass, field

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format, MetadataCategory, ProductionStep
from OutputStreamAthenaPool.OutputStreamConfig import addToMetaData, outputStreamName


@dataclass
class MetaDataHelperLists:
    """
    Helper class aggregating lists needed to setup metadata
    for the output stream configuration and metadata service.
    """

    helperTools: list = field(default_factory=list)
    mdTools: list = field(default_factory=list)
    mdToolNames: list = field(default_factory=list)
    mdItems: list = field(default_factory=list)

    def __iadd__(self, helperLists):
        self.helperTools += helperLists.helperTools
        self.mdTools += helperLists.mdTools
        self.mdToolNames += helperLists.mdToolNames
        self.mdItems += helperLists.mdItems
        return self


def createCutFlowMetaData(flags):
    tools = MetaDataHelperLists()
    result = ComponentAccumulator()
    from EventBookkeeperTools.EventBookkeeperToolsConfig import (
        CutFlowOutputList,
        CutFlowSvcCfg,
    )

    result.merge(CutFlowSvcCfg(flags))
    tools.mdItems += CutFlowOutputList(flags)
    return tools, result


def createByteStreamMetaData(flags):
    tools = MetaDataHelperLists()
    result = ComponentAccumulator()
    tools.mdItems += ["ByteStreamMetadataContainer#*"]
    if flags.Input.Format == Format.BS and not flags.Common.isOnline:
        from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg

        result.merge(ByteStreamReadCfg(flags))
    return tools, result


def createLumiBlockMetaData(flags):
    tools = MetaDataHelperLists()
    result = ComponentAccumulator()
    if flags.Input.Format == Format.BS and not flags.Common.isOnline:
        from LumiBlockComps.CreateLumiBlockCollectionFromFileConfig import (
            CreateLumiBlockCollectionFromFileCfg,
        )

        result.merge(CreateLumiBlockCollectionFromFileCfg(flags))
        tools.mdItems += [
            "xAOD::LumiBlockRangeContainer#*",
            "xAOD::LumiBlockRangeAuxContainer#*",
        ]
    return tools, result


def createTriggerMenuMetaData(flags):
    tools = MetaDataHelperLists()
    result = ComponentAccumulator()
    tools.mdTools.append(
        CompFactory.xAODMaker.TriggerMenuMetaDataTool("TriggerMenuMetaDataTool")
    )
    tools.mdItems += [
        "xAOD::TriggerMenuContainer#*",
        "xAOD::TriggerMenuAuxContainer#*",
        "xAOD::TriggerMenuJsonContainer#*",
        "xAOD::TriggerMenuJsonAuxContainer#*",
    ]
    return tools, result


def createTruthMetaData(flags):
    tools = MetaDataHelperLists()
    result = ComponentAccumulator()
    tools.mdItems += [
        "xAOD::TruthMetaDataContainer#TruthMetaData",
        "xAOD::TruthMetaDataAuxContainer#TruthMetaDataAux.",
    ]
    tools.mdTools.append(CompFactory.xAODMaker.TruthMetaDataTool("TruthMetaDataTool"))
    return tools, result


def propagateMetaData(flags, streamName="", category=None, *args, **kwargs):
    """
    Returns the tuple of MetaDataHelperLists and ComponentAccumulator.
    The former combines the lists needed to setup given metadata category
    for the output stream configuration and metadata service.
    The latter contains the CA needed for a given metadata category.
    """
    tools = MetaDataHelperLists()
    result = ComponentAccumulator()
    log = logging.getLogger("SetupMetaDataForStreamCfg")

    if category == MetadataCategory.FileMetaData:
        tools.mdToolNames.append("xAODMaker::FileMetaDataTool")
        tools.mdItems += [
            "xAOD::FileMetaData#FileMetaData",
            "xAOD::FileMetaDataAuxInfo#FileMetaDataAux.",
        ]
        tools.helperTools.append(
            CompFactory.xAODMaker.FileMetaDataCreatorTool(
                f"{outputStreamName(streamName)}_FileMetaDataCreatorTool",
                OutputKey="FileMetaData",
                StreamName=outputStreamName(streamName),
                EventInfoKey=f"{flags.Overlay.BkgPrefix}EventInfo"
                    if flags.Common.ProductionStep in [ProductionStep.PileUpPresampling, ProductionStep.PileUpPretracking, ProductionStep.MinbiasPreprocessing]
                    else "EventInfo",
            )
        )
    elif category == MetadataCategory.EventStreamInfo:
        esiTool = CompFactory.MakeEventStreamInfo(
            f"{outputStreamName(streamName)}_MakeEventStreamInfo",
            Key=outputStreamName(streamName),
            DataHeaderKey=outputStreamName(streamName),
            EventInfoKey=f"{flags.Overlay.BkgPrefix}EventInfo"
            if flags.Common.ProductionStep in [ProductionStep.PileUpPresampling, ProductionStep.PileUpPretracking, ProductionStep.MinbiasPreprocessing]
            else "EventInfo",
        )
        tools.mdItems += [
            f"EventStreamInfo#{outputStreamName(streamName)}",
        ]
        tools.helperTools.append(esiTool)

        # copy EventStreamInfo for merging jobs
        if kwargs.get("mergeJob", False):
            tools.mdTools += [
                CompFactory.CopyEventStreamInfo(
                    f"{outputStreamName(streamName)}_CopyEventStreamInfo"
                ),
            ]

    elif category == MetadataCategory.EventFormat:
        efTool = CompFactory.xAODMaker.EventFormatStreamHelperTool(
            f"{outputStreamName(streamName)}_EventFormatStreamHelperTool",
            Key=f"EventFormat{outputStreamName(streamName)}",
            DataHeaderKey=outputStreamName(streamName),
        )
        tools.mdItems += [
            f"xAOD::EventFormat#EventFormat{outputStreamName(streamName)}",
        ]
        tools.helperTools.append(efTool)
        tools.mdToolNames.append("xAODMaker::EventFormatMetaDataTool")
    elif category == MetadataCategory.CutFlowMetaData:
        if "CutBookkeepers" in flags.Input.MetadataItems:
            from EventBookkeeperTools.EventBookkeeperToolsConfig import (
                CutFlowOutputList,
            )

            tools.mdToolNames.append("BookkeeperTool")
            tools.mdItems += CutFlowOutputList(flags)

    elif category == MetadataCategory.TriggerMenuMetaData:
        if any("TriggerMenu" in item for item in flags.Input.MetadataItems):
            _tools, _ = createTriggerMenuMetaData(flags)
            tools.mdTools = _tools.mdTools
            tools.mdItems = _tools.mdItems

    elif category == MetadataCategory.TruthMetaData:
        if "TruthMetaData" in flags.Input.MetadataItems:
            tools.mdItems += [
                "xAOD::TruthMetaDataContainer#TruthMetaData",
                "xAOD::TruthMetaDataAuxContainer#TruthMetaDataAux.",
            ]
            tools.mdTools.append(
                CompFactory.xAODMaker.TruthMetaDataTool("TruthMetaDataTool")
            )
    elif category == MetadataCategory.ByteStreamMetaData:
        if "ByteStreamMetadata" in flags.Input.MetadataItems:
            tools.mdItems += ["ByteStreamMetadataContainer#*"]
    elif category == MetadataCategory.LumiBlockMetaData:
        if any(
            lb in flags.Input.MetadataItems
            for lb in ["SuspectLumiBlocks", "IncompleteLumiBlocks", "LumiBlocks"]
        ):
            tools.mdToolNames.append("LumiBlockMetaDataTool")
            tools.mdItems += [
                "xAOD::LumiBlockRangeContainer#*",
                "xAOD::LumiBlockRangeAuxContainer#*",
            ]

    else:
        log.warning(f"Requested metadata category: {category} could not be configured")
    return tools, result


def SetupMetaDataForStreamCfg(
    flags,
    streamName="",
    AcceptAlgs=None,
    createMetadata=None,
    propagateMetadataFromInput=True,
    *args,
    **kwargs,
):
    """
    Set up metadata for the stream named streamName

    It takes optional arguments: createMetadata to specify a list of metadata
    categories to create (empty by default) and propagateMetadataFromInput (bool)
    to propagate metadata existing in the input (True by default).

    The additional argument, AcceptAlgs, is needed for workflows with custom kernels.

    Returns CA to be merged
    """
    log = logging.getLogger("SetupMetaDataForStreamCfg")
    result = ComponentAccumulator()
    if not isinstance(streamName, str) or not streamName:
        return result
    if not AcceptAlgs:
        AcceptAlgs = []
    if not createMetadata:
        createMetadata = []

    helperLists = MetaDataHelperLists()

    if propagateMetadataFromInput:
        for mdCategory in MetadataCategory:
            lists, caConfig = propagateMetaData(
                flags,
                streamName,
                mdCategory,
                *args,
                **kwargs,
            )
            helperLists += lists
            result.merge(caConfig)

    for md in createMetadata:
        try:
            lists, caConfig = globals()[f"create{md.name}"](
                flags,
                *args,
                **kwargs,
            )
        except KeyError:
            log.warning(f"Requested metadata category: {md.name} could not be configured")
            continue
        helperLists += lists
        result.merge(caConfig)

    # Configure the relevant output stream
    result.merge(
        addToMetaData(
            flags,
            streamName=streamName,
            itemOrList=helperLists.mdItems,
            AcceptAlgs=AcceptAlgs,
            HelperTools=helperLists.helperTools,
            **kwargs
        )
    )
    # Configure the MetaDataSvc and pass the relevant tools
    from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg

    result.merge(
        MetaDataSvcCfg(
            flags, tools=helperLists.mdTools, toolNames=helperLists.mdToolNames
        )
    )
    return result
