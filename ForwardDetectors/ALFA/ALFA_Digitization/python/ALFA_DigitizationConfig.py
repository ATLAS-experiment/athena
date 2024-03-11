# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from DigitizationConfig.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg

# The earliest bunch crossing time for which interactions will be sent
# to the ALFA Digitization code.
def ALFA_FirstXing():
    return 0 #Assume ALFA is only sensitive to the current bunch crossing, for the moment

######################################################################################

# The latest bunch crossing time for which interactions will be sent
# to the ALFA Digitization code.
def ALFA_LastXing():
    return 0 #Assume ALFA is only sensitive to the current bunch crossing, for the moment

######################################################################################

def ALFARangeCfg(flags, name="ALFARange", **kwargs):
    #this is the time of the xing in ns
    kwargs.setdefault('FirstXing', ALFA_FirstXing() )
    kwargs.setdefault('LastXing',  ALFA_LastXing() )
    kwargs.setdefault('CacheRefreshFrequency', 1.0 ) #default 0 no dataproxy reset
    kwargs.setdefault('ItemList', ["ALFA_HitCollection#ALFA_HitCollection",
                                   "ALFA_ODHitCollection#ALFA_ODHitCollection"] )
    return PileUpXingFolderCfg(flags, name, **kwargs)

######################################################################################

def ALFA_PileUpToolCfg(flags, name="ALFA_PileUpTool", **kwargs):
    """Return ComponentAccumulator with ALFA digitization tool."""
    acc = ComponentAccumulator()

    # Configure bunch-crossing envelope
    if flags.Digitization.PileUp:
        intervals = []
        if flags.Digitization.DoXingByXingPileUp:
            kwargs.setdefault("FirstXing", ALFA_FirstXing() )
            kwargs.setdefault("LastXing", ALFA_LastXing() )
        else:
            intervals += [acc.popToolsAndMerge(ALFARangeCfg(flags))]
        kwargs.setdefault("mergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
        #kwargs.setdefault("OnlyUseContainerName", True) # TODO in future MR
    else:
        kwargs.setdefault("mergeSvc", '')
        #kwargs.setdefault("OnlyUseContainerName", False) #TODO in future MR

    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)

    acc.setPrivateTools(CompFactory.ALFA_PileUpTool(name, **kwargs))
    return acc


def ALFA_OverlayPileUpToolCfg(flags, name="ALFA_OverlayPileUpTool", **kwargs):
    """Return ComponentAccumulator with ALFA_PileUpTool for Overlay"""
    acc = ComponentAccumulator()
    kwargs.setdefault("mergeSvc", '')
    #kwargs.setdefault("OnlyUseContainerName", False) #TODO in future MR
    tool = acc.popToolsAndMerge(ALFA_PileUpToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def ALFA_DigitizationOutputCfg(flags):
    """Return ComponentAccumulator with Output for ALFA. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["ALFA_DigitCollection#ALFA_DigitCollection",
                    "ALFA_ODDigitCollection#ALFA_ODDigitCollection"]
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags,"RDO", ItemList))
    return acc


def ALFA_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for ALFA digitization"""
    acc = ComponentAccumulator()
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(ALFA_PileUpToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    from DigitizationConfig.PileUpToolsConfig import PileUpToolsCfg
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def ALFA_OverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with ALFA_DigiAlg."""
    acc = ComponentAccumulator()
    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags,Load={('ALFA_HitCollection','ALFA_HitCollection'),
                                               ('ALFA_ODHitCollection','ALFA_ODHitCollection')} ) )
    if "DigitizationTool" not in kwargs:
        kwargs.setdefault("DigitizationTool", acc.popToolsAndMerge(ALFA_OverlayPileUpToolCfg(flags)))

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)

    acc.addEventAlgo(CompFactory.ALFA_DigiAlg("ALFA_DigiAlg", **kwargs))
    return acc


def ALFA_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for ALFA digitization and Output"""
    acc = ComponentAccumulator()
    acc.merge(ALFA_DigitizationBasicCfg(flags, **kwargs))
    acc.merge(ALFA_DigitizationOutputCfg(flags))

    return acc


def ALFA_DigitizationOverlayCfg(flags, **kwargs):
    """Return ComponentAccumulator for ALFA digitization and Output"""
    acc = ComponentAccumulator()
    acc.merge(ALFA_OverlayDigitizationBasicCfg(flags, **kwargs))
    acc.merge(ALFA_DigitizationOutputCfg(flags))

    return acc
