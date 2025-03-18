
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline
from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripGeoModelCfg


def ITkStripAlignCondAlgCfg(flags, name="ITkStripAlignCondAlg", **kwargs):
    """Return a configured SCT_AlignCondAlg for ITk"""
    acc = ITkStripGeoModelCfg(flags)
    if flags.GeoModel.Align.Dynamic:
        raise RuntimeError("Dynamic alignment not supported for ITk yet")
    else:
        acc.merge(addFoldersSplitOnline(flags, "INDET", "/Indet/Onl/Align", flags.ITk.Geometry.alignmentFolder, className="AlignableTransformContainer"))

    kwargs.setdefault("DetManagerName", "ITkStrip")
    kwargs.setdefault("WriteKey", "ITkStripAlignmentStore")
    kwargs.setdefault("UseDynamicAlignFolders", flags.GeoModel.Align.Dynamic)
    kwargs.setdefault("ReadKeyStatic",flags.ITk.Geometry.alignmentFolder)

    sctAlignCondAlg = CompFactory.SCT_AlignCondAlg(name, **kwargs)
    acc.addCondAlgo(sctAlignCondAlg)
    return acc


def getITkStripDAQConfigFolder(flags) :
    return "/ITkStrip/DAQ/Config/" if flags.IOVDb.DatabaseInstance != "COMP200" else "/ITkStrip/DAQ/Configuration/"


def ITkStripConfigurationCondAlgCfg(flags, name="ITkStripConfigurationCondAlg", **kwargs):
    acc = ComponentAccumulator()
    folder_prefix = getITkStripDAQConfigFolder(flags)
    channelFolder = folder_prefix + ("Chip" if flags.IOVDb.DatabaseInstance == "COMP200" else "ChipSlim")
    kwargs.setdefault("ReadKeyChannel", channelFolder)
    kwargs.setdefault("ReadKeyModule", f"{folder_prefix}Module")
    kwargs.setdefault("ReadKeyMur", f"{folder_prefix}MUR")

    acc.merge(addFoldersSplitOnline(flags,
                                    detDb="ITkStrip",
                                    onlineFolders=channelFolder,
                                    offlineFolders=channelFolder,
                                    className="CondAttrListVec",
                                    splitMC=True))
    acc.merge(addFoldersSplitOnline(flags,
                                    detDb="ITkStrip",
                                    onlineFolders=f"{folder_prefix}Module",
                                    offlineFolders=f"{folder_prefix}Module",
                                    className="CondAttrListVec",
                                    splitMC=True))
    acc.merge(addFoldersSplitOnline(flags,
                                    detDb="ITkStrip",
                                    onlineFolders=f"{folder_prefix}MUR",
                                    offlineFolders=f"{folder_prefix}MUR",
                                    className="CondAttrListVec",
                                    splitMC=True))

    from ITkStripCabling.ITkStripCablingConfig import ITkStripCablingToolCfg
    kwargs.setdefault("SCT_CablingTool", acc.popToolsAndMerge(ITkStripCablingToolCfg(flags)))

    from SCT_ConditionsTools.ITkStripConditionsToolsConfig import ITkStripReadoutToolCfg
    kwargs.setdefault("SCT_ReadoutTool", acc.popToolsAndMerge(ITkStripReadoutToolCfg(flags)))

    acc.addCondAlgo(CompFactory.SCT_ConfigurationCondAlg(name, **kwargs))
    return acc


def ITkStripDetectorElementCondAlgCfg(flags, name="ITkStripDetectorElementCondAlg", **kwargs):
    kwargs.setdefault("DetManagerName", "ITkStrip")
    kwargs.setdefault("ReadKey", "ITkStripAlignmentStore")
    kwargs.setdefault("WriteKey", "ITkStripDetectorElementCollection")

    acc = ITkStripAlignCondAlgCfg(flags)
    acc.addCondAlgo(CompFactory.SCT_DetectorElementCondAlg(name, **kwargs))
    return acc

def ITkStripDetectorElementStatusCondAlgNoByteStreamErrorsCfg(flags, name = "ITkStripDetectorElementStatusCondAlgNoByteStreamErrors", **kwargs) :
    '''
    Condition alg to precompute the strip detector element status.
    This algorithm does not consider the byte stream errors which are event data.
    '''
    acc = ComponentAccumulator()
    if 'ConditionsSummaryTool' not in kwargs :
        from SCT_ConditionsTools.ITkStripConditionsToolsConfig import ITkStripConditionsSummaryToolCfg
        kwargs.setdefault("ConditionsSummaryTool", acc.popToolsAndMerge( ITkStripConditionsSummaryToolCfg(flags)))
    kwargs.setdefault( "WriteKey", "ITkStripDetectorElementStatusNoByteStream")
    acc.addCondAlgo( CompFactory.InDet.SiDetectorElementStatusCondAlg(name, **kwargs) )
    return acc

def ITkStripDetectorElementStatusAlgCfg(flags, name="ITkStripDetectorElementStatusAlg",**kwargs) :
    '''
    Algorithm which adds status from the strip bytestream to the strip status conditions data
    '''
    acc = ComponentAccumulator()
    if 'ConditionsSummaryTool' not in kwargs :
        from SCT_ConditionsTools.ITkStripConditionsToolsConfig import ITkStripDetectorElementStatusAddByteStreamErrorsToolCfg
        # @TODO ITkStripDetectorElementStatusCondAlgNoByteStreamErrorsCfg should be moved to
        #   ITkStripDetectorElementStatusAddByteStreamErrorsToolCfg, but that would create
        #   circular dependencies..
        acc.merge(ITkStripDetectorElementStatusCondAlgNoByteStreamErrorsCfg(flags))
        kwargs.setdefault("ConditionsSummaryTool", acc.popToolsAndMerge(
          ITkStripDetectorElementStatusAddByteStreamErrorsToolCfg(flags,
                                                                  SCTDetElStatusCondDataBaseKey  = "ITkStripDetectorElementStatusNoByteStream",
                                                                  SCTDetElStatusEventDataBaseKey = ""
                                                                  )))
    kwargs.setdefault("WriteKey", "ITkStripDetectorElementStatus")

    # not a conditions algorithm since it combines conditions data and data from the bytestream
    acc.addEventAlgo( CompFactory.InDet.SiDetectorElementStatusAlg(name, **kwargs) )
    return acc
