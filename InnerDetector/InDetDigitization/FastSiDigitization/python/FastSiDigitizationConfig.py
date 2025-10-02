# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from DigitizationConfig.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg


# The earliest bunch crossing time for which interactions will be sent
# to the Fast Pixel Digitization code.
def FastPixel_FirstXing():
    return 0


# The latest bunch crossing time for which interactions will be sent
# to the Fast Pixel Digitization code.
def FastPixel_LastXing():
    return 0


# The earliest bunch crossing time for which interactions will be sent
# to the Fast SCT Digitization code.
def FastSCT_FirstXing():
    return 0


# The latest bunch crossing time for which interactions will be sent
# to the Fast SCT Digitization code.
def FastSCT_LastXing():
    return 0


def commonPixelFastDigitizationCfg(flags, name,**kwargs):
    acc = ComponentAccumulator()
    from InDetConfig.SiClusterizationToolConfig import ClusterMakerToolCfg
    kwargs.setdefault("ClusterMaker", acc.popToolsAndMerge(ClusterMakerToolCfg(flags)))

    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    kwargs.setdefault("RndmEngine", "FastPixelDigitization")

    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", FastPixel_FirstXing())
        kwargs.setdefault("LastXing",  FastPixel_LastXing() )

    from PixelConditionsAlgorithms.PixelConditionsConfig import PixelConfigCondAlgCfg
    acc.merge(PixelConfigCondAlgCfg(flags, name="PixelConfigCondAlg", ReadDeadMapKey = ""))

    from SiLorentzAngleTool.PixelLorentzAngleConfig import PixelLorentzAngleToolCfg
    kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(PixelLorentzAngleToolCfg(flags)))

    acc.setPrivateTools(CompFactory.PixelFastDigitizationTool(name,**kwargs))
    return acc


def commonSCT_FastDigitizationCfg(flags, name,**kwargs):
    acc = ComponentAccumulator()
    from InDetConfig.SiClusterizationToolConfig import ClusterMakerToolCfg
    kwargs.setdefault("ClusterMaker", acc.popToolsAndMerge(ClusterMakerToolCfg(flags)))

    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    kwargs.setdefault("RndmEngine", "FastSCT_Digitization")

    if flags.Digitization.DoXingByXingPileUp:
        kwargs.setdefault("FirstXing", FastSCT_FirstXing())
        kwargs.setdefault("LastXing",  FastSCT_LastXing() )

    from SiLorentzAngleTool.SCT_LorentzAngleConfig import SCT_LorentzAngleToolCfg
    kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(SCT_LorentzAngleToolCfg(flags)))

    acc.setPrivateTools(CompFactory.SCT_FastDigitizationTool(name,**kwargs))
    return acc


######################################################################################
def PixelFastDigitizationToolCfg(flags, name="PixelFastDigitizationTool", **kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastPixelRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)))
    kwargs.setdefault("HardScatterSplittingMode", 0)
    acc.setPrivateTools(commonPixelFastDigitizationCfg(flags, name, **kwargs))
    return acc


######################################################################################
def PixelFastDigitizationToolHSCfg(flags, name="PixelFastDigitizationToolHS", **kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastPixelRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)))
    kwargs.setdefault("HardScatterSplittingMode", 1)
    acc.setPrivateTools(commonPixelFastDigitizationCfg(flags, name, **kwargs))
    return acc


######################################################################################
def PixelFastDigitizationToolPUCfg(flags, name="PixelFastDigitizationToolPU", **kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastPixelRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)))
    kwargs.setdefault("PixelClusterContainerName", "Pixel_PU_Clusters")
    kwargs.setdefault("TruthNamePixel", "PRD_MultiTruthPixel_PU")
    kwargs.setdefault("HardScatterSplittingMode", 2)
    acc.setPrivateTools(commonPixelFastDigitizationCfg(flags, name, **kwargs))
    return acc


######################################################################################
def PixelFastDigitizationToolSplitNoMergePUCfg(flags, name="PixelFastDigitizationToolSplitNoMergePU", **kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastPixelRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)))
    kwargs.setdefault("InputObjectName", "PileupPixelHits")
    kwargs.setdefault("PixelClusterContainerName", "PixelFast_PU_Clusters")
    kwargs.setdefault("TruthNamePixel", "PRD_MultiTruthPixel_PU")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    kwargs.setdefault("PixelClusterAmbiguitiesMapName", "PixelClusterAmbiguitiesMapPU")
    acc.setPrivateTools(commonPixelFastDigitizationCfg(flags, name, **kwargs))
    return acc


######################################################################################
def SCT_FastDigitizationToolCfg(flags, name="SCT_FastDigitizationTool", **kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastSCTRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)))
    kwargs.setdefault("HardScatterSplittingMode", 0)
    acc.setPrivateTools(commonSCT_FastDigitizationCfg(flags, name, **kwargs))
    return acc
    return commonSCT_FastDigitizationCfg(flags, name,**kwargs)


######################################################################################
def SCT_FastDigitizationToolHSCfg(flags, name="SCT_FastDigitizationToolHS",**kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastSCTRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)))
    kwargs.setdefault("HardScatterSplittingMode", 1)
    acc.setPrivateTools(commonSCT_FastDigitizationCfg(flags, name, **kwargs))
    return acc


######################################################################################
def SCT_FastDigitizationToolPUCfg(flags, name="SCT_FastDigitizationToolPU",**kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastSCTRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)))
    kwargs.setdefault("SCT_ClusterContainerName", "SCT_PU_Clusters")
    kwargs.setdefault("TruthNameSCT", "PRD_MultiTruthSCT_PU")
    kwargs.setdefault("HardScatterSplittingMode", 2)
    acc.setPrivateTools(commonSCT_FastDigitizationCfg(flags, name, **kwargs))
    return acc


######################################################################################
def SCT_FastDigitizationToolSplitNoMergePUCfg(flags, name="SCT_FastDigitizationToolSplitNoMergePU",**kwargs):
    acc = ComponentAccumulator()
    intervals = []
    if not flags.Digitization.DoXingByXingPileUp:
        intervals += [acc.popToolsAndMerge(FastSCTRangeCfg(flags))]
    kwargs.setdefault("MergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)))
    kwargs.setdefault("InputObjectName", "PileupSCT_Hits")
    kwargs.setdefault("SCT_ClusterContainerName", "SCT_PU_Clusters")
    kwargs.setdefault("TruthNameSCT", "PRD_MultiTruthSCT_PU")
    kwargs.setdefault("HardScatterSplittingMode", 0)
    acc.setPrivateTools(commonSCT_FastDigitizationCfg(flags, name, **kwargs))
    return acc


######################################################################################
def FastPixelRangeCfg(flags, name="FastPixelRange" , **kwargs):
    kwargs.setdefault('FirstXing', FastPixel_FirstXing() )
    kwargs.setdefault('LastXing',  FastPixel_LastXing() )
    kwargs.setdefault('CacheRefreshFrequency', 1.0 ) #default 0 no dataproxy reset
    kwargs.setdefault('ItemList', ["SiHitCollection#PixelHits"] )
    return PileUpXingFolderCfg(flags, name, **kwargs)


######################################################################################
def FastSCTRangeCfg(flags, name="FastSCTRange" , **kwargs):
    #this is the time of the xing in ns
    kwargs.setdefault('FirstXing', FastSCT_FirstXing() )
    kwargs.setdefault('LastXing',  FastSCT_LastXing()  )
    kwargs.setdefault('CacheRefreshFrequency', 1.0 ) #default 0 no dataproxy reset
    kwargs.setdefault('ItemList', ["SiHitCollection#SCT_Hits"] )
    return PileUpXingFolderCfg(flags, name, **kwargs)
