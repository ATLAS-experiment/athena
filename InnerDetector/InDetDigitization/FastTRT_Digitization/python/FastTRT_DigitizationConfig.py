# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from DigitizationConfig.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg

# The earliest bunch crossing time for which interactions will be sent
# to the FastTRT Digitization code.
def FastTRT_FirstXing():
    return -50


# The latest bunch crossing time for which interactions will be sent
# to the FastTRT Digitization code.
def FastTRT_LastXing():
    return 50


def FastTRTRangeCfg(flags, name="FastTRTRange", **kwargs):
    #this is the time of the xing in ns
    kwargs.setdefault('FirstXing', FastTRT_FirstXing() )
    kwargs.setdefault('LastXing', FastTRT_LastXing() )
    kwargs.setdefault('CacheRefreshFrequency', 1.0 ) #default 0 no dataproxy reset
    kwargs.setdefault('ItemList', ["TRTUncompressedHitCollection#TRTUncompressedHits"] )
    return PileUpXingFolderCfg(flags, name, **kwargs)


def BasicTRTFastDigitizationToolCfg(flags, name, **kwargs):
    acc = ComponentAccumulator()
    from IOVDbSvc.IOVDbSvcConfig import addFolders, addFoldersSplitOnline
    acc.merge(addFoldersSplitOnline(flags, "TRT", "/TRT/Onl/Calib/errors", "/TRT/Calib/errors"))
    acc.merge(addFoldersSplitOnline(flags, "TRT","/TRT/Onl/Calib/PID_vector", "/TRT/Calib/PID_vector"))
    acc.merge(addFolders(flags, "TRT_OFL", "/TRT/Calib/ToT/ToTVectors"))
    acc.merge(addFolders(flags, "TRT_OFL", "/TRT/Calib/ToT/ToTValue"))
    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", FastTRT_FirstXing())
        kwargs.setdefault("LastXing",  FastTRT_LastXing())
    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    kwargs.setdefault("RandomStreamName", "FastTRTDigitization")
    tool = CompFactory.TRTFastDigitizationTool(name,**kwargs)
    acc.setPrivateTools(tool)
    return acc


def TRTFastDigitizationToolCfg(flags, name="TRTFastDigitizationTool",**kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastTRTRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    kwargs.setdefault("trtDriftCircleContainer", "TRT_DriftCircles")
    kwargs.setdefault("trtPrdMultiTruthCollection", "PRD_MultiTruthTRT")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    acc.setPrivateTools(BasicTRTFastDigitizationToolCfg(flags, name,**kwargs))
    return acc


def TRTFastDigitizationToolHS(flags, name="TRTFastDigitizationToolHS",**kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastTRTRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    kwargs.setdefault("OnlyUseContainerName", True)
    kwargs.setdefault("trtDriftCircleContainer", "TRT_DriftCircles")
    kwargs.setdefault("trtPrdMultiTruthCollection", "PRD_MultiTruthTRT")
    kwargs.setdefault("HardScatterSplittingMode", 1)
    acc.setPrivateTools(BasicTRTFastDigitizationToolCfg(flags, name,**kwargs))
    return acc


def TRTFastDigitizationToolPU(flags, name="TRTFastDigitizationToolPU",**kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastTRTRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    kwargs.setdefault("trtDriftCircleContainer", "TRT_PU_DriftCircles")
    kwargs.setdefault("trtPrdMultiTruthCollection", "PRD_MultiTruthTRT_PU")
    kwargs.setdefault("HardScatterSplittingMode", 2)
    acc.setPrivateTools(BasicTRTFastDigitizationToolCfg(flags, name,**kwargs))
    return acc


def TRTFastDigitizationToolSplitNoMergePU(flags, name="TRTFastDigitizationToolSplitNoMergePU",**kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastTRTRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    kwargs.setdefault("trtHitCollectionName", "PileupTRTUncompressedHits")
    kwargs.setdefault("trtDriftCircleContainer", "TRT_PU_DriftCircles")
    kwargs.setdefault("trtPrdMultiTruthCollection", "PRD_MultiTruthTRT_PU")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    acc.setPrivateTools(BasicTRTFastDigitizationToolCfg(flags, name,**kwargs))
    return acc
