# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from DigitizationConfig.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg

# The earliest bunch crossing time for which interactions will be sent
# to the ZDC Digitization code.
def ZDC_FirstXing():
    return 0 #Assume ZDC is only sensitive to the current bunch crossing, for the moment

######################################################################################

# The latest bunch crossing time for which interactions will be sent
# to the ZDC Digitization code.
def ZDC_LastXing():
    return 0 #Assume ZDC is only sensitive to the current bunch crossing, for the moment

######################################################################################

def ZDC_RangeCfg(flags, name="ZdcRange" , **kwargs):
    #this is the time of the xing in ns
    kwargs.setdefault('FirstXing', ZDC_FirstXing() )
    kwargs.setdefault('LastXing',  ZDC_LastXing() )
    kwargs.setdefault('CacheRefreshFrequency', 1.0 ) #default 0 no dataproxy reset
    kwargs.setdefault('ItemList', ["ZDC_SimFiberHit_Collection#ZDC_SimFiberHit_Collection" ] )
    return PileUpXingFolderCfg(flags, name, **kwargs)

######################################################################################

def ZDC_PileUpToolCfg(flags, name="ZDC_PileUpTool",**kwargs):
    """Return ComponentAccumulator with ZDC digitization tool."""
    acc = ComponentAccumulator()

    # Configure bunch-crossing envelope
    if flags.Digitization.PileUp:
        intervals = []
        if flags.Digitization.DoXingByXingPileUp:
            kwargs.setdefault("FirstXing", ZDC_FirstXing() )
            kwargs.setdefault("LastXing", ZDC_LastXing() )
        else:
            intervals += [acc.popToolsAndMerge(ZDC_RangeCfg(flags))]
        kwargs.setdefault("mergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
        #kwargs.setdefault("OnlyUseContainerName", True) # TODO in future MR
    else:
        kwargs.setdefault("mergeSvc", '')
        #kwargs.setdefault("OnlyUseContainerName", False) #TODO in future MR

    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)

    config = "PbPb2023"
    from AthenaConfiguration.Enums import LHCPeriod
    if flags.GeoModel.Run in [LHCPeriod.Run1, LHCPeriod.Run2]:
        config = "PbPb2015"

    acc.setPrivateTools(CompFactory.ZDC_PileUpTool(name, configuration = config, **kwargs))
    return acc


def ZDC_OverlayPileUpToolCfg(flags, name="ZDC_OverlayPileUpTool", **kwargs):
    """Return ComponentAccumulator with ZDC_PileUpTool for Overlay"""
    acc = ComponentAccumulator()
    kwargs.setdefault("mergeSvc", '')
    #kwargs.setdefault("OnlyUseContainerName", False) #TODO in future MR
    tool = acc.popToolsAndMerge(ZDC_PileUpToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def ZDC_DigitizationOutputCfg(flags):
    """Return ComponentAccumulator with Output for ZDC. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["xAOD::ZdcModuleContainer#*", "xAOD::ZdcModuleAuxContainer#*.", "CaloCalibrationHitContainer#*"]
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags,"RDO", ItemList))
    return acc


def ZDC_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for ZDC digitization"""
    acc = ComponentAccumulator()
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(ZDC_PileUpToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    from DigitizationConfig.PileUpToolsConfig import PileUpToolsCfg
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    from DetDescrCnvSvc.DetDescrCnvSvcConfig import DetDescrCnvSvcCfg
    acc.merge(DetDescrCnvSvcCfg(flags))
    from ZDC_GeoM.ZdcGeoModelConfig import ZDC_DetToolCfg
    acc.merge(ZDC_DetToolCfg(flags))
    return acc


def ZDC_OverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with ZDC_DigiAlg."""
    acc = ComponentAccumulator()
    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags,Load={
            ('ZDC_SimFiberHit_Collection','ZDC_SimFiberHit_Collection')} ) )
    if "DigitizationTool" not in kwargs:
        kwargs.setdefault("DigitizationTool", acc.popToolsAndMerge(ZDC_OverlayPileUpToolCfg(flags)))

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)

    acc.addEventAlgo(CompFactory.ZDC_DigiAlg("ZDC_DigiAlg", **kwargs))
    return acc


def ZDC_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for ZDC digitization and Output"""
    acc = ComponentAccumulator()
    acc.merge(ZDC_DigitizationBasicCfg(flags, **kwargs))
    acc.merge(ZDC_DigitizationOutputCfg(flags))

    return acc


def ZDC_DigitizationOverlayCfg(flags, **kwargs):
    """Return ComponentAccumulator for ZDC digitization and Output"""
    acc = ComponentAccumulator()
    acc.merge(ZDC_OverlayDigitizationBasicCfg(flags, **kwargs))
    acc.merge(ZDC_DigitizationOutputCfg(flags))

    return acc
