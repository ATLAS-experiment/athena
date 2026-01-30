# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

"""Configuration for writing parameters as in-file metadata"""


def writeParametersToMetaData(flags, folderName, parameters, beginRun, endRun):
    """
    Configure IOVDbMetaDataTool to write parameters as in-file metadata.

    Args:
        flags: Configuration flags
        folderName: Folder path (e.g., '/Simulation/Parameters')
        parameters: Dictionary of key-value pairs to store as strings
        beginRun: Begin run number for IOV
        endRun: End run number for IOV

    Returns:
        ComponentAccumulator with configured IOVDbSvc and IOVDbMetaDataTool
    """
    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg

    acc = IOVDbSvcCfg(flags, FoldersToMetaData=[folderName])

    # Get the IOVDbMetaDataTool and configure its Payloads property
    metaDataTool = acc.getPublicTool("IOVDbMetaDataTool")

    # Build the payload dict in "folder:key" -> "value" format
    payloadDict = {
        f"{folderName}:beginRun": str(beginRun),
        f"{folderName}:endRun": str(endRun)
    }
    for key, value in parameters.items():
        payloadDict[f"{folderName}:{key}"] = value

    # Set the Payloads property (map of string->string)
    metaDataTool.Payloads = payloadDict

    # Add TagInfo entry for consistency with sqlite mode
    from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
    tagAcc = TagInfoMgrCfg(flags)
    tagInfoMgr = tagAcc.getService("TagInfoMgr")
    currentPairs = dict(tagInfoMgr.ExtraTagValuePairs) if tagInfoMgr.ExtraTagValuePairs else {}
    currentPairs[folderName] = "HEAD"
    tagInfoMgr.ExtraTagValuePairs = currentPairs
    acc.merge(tagAcc)

    return acc