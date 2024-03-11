# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from DigitizationConfig.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg

# The earliest bunch crossing time for which interactions will be sent
# to the LUCID Digitization code.
def LUCID_FirstXing():
    return 0


# The latest bunch crossing time for which interactions will be sent
# to the LUCID Digitization code.
def LUCID_LastXing():
    return 0

######################################################################################

def LucidRangeCfg(flags, name="LucidRange" , **kwargs):
    #this is the time of the xing in ns
    kwargs.setdefault('FirstXing', LUCID_FirstXing() ) #LUCID is only sensitive to the current bunch crossing.
    kwargs.setdefault('LastXing', LUCID_LastXing() ) #LUCID is only sensitive to the current bunch crossing.
    kwargs.setdefault('CacheRefreshFrequency', 1.0 ) #default 0 no dataproxy reset
    kwargs.setdefault('ItemList', ["LUCID_SimHitCollection#LucidSimHitsVector"] )
    return PileUpXingFolderCfg(flags, name, **kwargs)

######################################################################################

def LUCID_PileUpToolCfg(flags, name="LUCID_PileUpTool",**kwargs):
    acc = ComponentAccumulator()

    if flags.Digitization.PileUp:
        intervals = []
        if flags.Digitization.DoXingByXingPileUp:
            kwargs.setdefault("FirstXing", LUCID_FirstXing() )
            kwargs.setdefault("LastXing", LUCID_LastXing() )
        else:
            intervals += [acc.popToolsAndMerge(LucidRangeCfg(flags))]
        kwargs.setdefault("mergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
        #kwargs.setdefault("OnlyUseContainerName", True) # FIXME in future MR
    else:
        kwargs.setdefault("mergeSvc", '')
        #kwargs.setdefault("OnlyUseContainerName", False) # FIXME in future MR

    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)

    kwargs.setdefault('pmtSmearing', [0.317, 0.000, 0.292, 0.316, 0.208, 0.178, 0.204, 0.281, 0.233, 0.261, 0.223, 0.250, 0.254, 0.239, 0.202, 0.224,  1,  1,  1,  1,
                          0.268, 0.277, 0.297, 0.310, 0.203, 0.347, 0.269, 0.241, 0.234, 0.234, 0.277, 0.297, 0.225, 0.297, 0.238, 0.000,  1,  1,  1,  1] )
    kwargs.setdefault('pmtScaling', [1.010, 0.000, 0.991, 0.948, 1.152, 1.221, 1.160, 0.988, 1.092, 1.063, 1.143, 1.091, 1.109, 1.117, 1.185, 1.142,  1,  1,  1,  1,
                          1.023, 1.127, 1.043, 0.986, 1.148, 0.899, 0.898, 1.098, 1.115, 1.109, 1.127, 1.043, 1.085, 1.043, 1.063, 0.000,  1,  1,  1,  1] )
    kwargs.setdefault('gasScaling', [1.176, 0.000, 1.217, 1.101, 1.143, 1.105, 1.103, 1.144, 1.075, 1.069, 1.100, 1.208, 1.212, 1.125, 1.026, 1.037,  1,  1,  1,  1,
                          1.064, 0.956, 0.975, 0.938, 1.205, 1.095, 1.137, 1.222, 1.262, 1.160, 0.923, 0.969, 1.132, 0.969, 1.174, 0.000,  1,  1,  1,  1] )
    kwargs.setdefault('npeThreshold', [   17,    15,    16,    16,    18,    16,    16,    18,    17,    16,    16,    17,    19,    16,    16,    17, 15, 15, 15, 15,
                             17,    16,    16,    17,    17,    15,    16,    16,    17,    16,    15,    17,    17,    15,    16,    15, 15, 15, 15, 15] )

    acc.setPrivateTools(CompFactory.LUCID_PileUpTool(name,**kwargs))
    return acc


def LUCID_OverlayPileUpToolCfg(flags, name="LUCID_OverlayPileUpTool", **kwargs):
    """Return ComponentAccumulator with LUCID_PileUpTool for Overlay"""
    acc = ComponentAccumulator()
    kwargs.setdefault("mergeSvc", '')
    #kwargs.setdefault("OnlyUseContainerName", False) #TODO in future MR
    tool = acc.popToolsAndMerge(LUCID_PileUpToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def LUCID_DigitizationOutputCfg(flags):
    """Return ComponentAccumulator with Output for LUCID. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["LUCID_DigitContainer#LucidDigitsCnt"]
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags,"RDO", ItemList))
    return acc


def LUCID_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for LUCID digitization"""
    acc = ComponentAccumulator()
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(LUCID_PileUpToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    from DigitizationConfig.PileUpToolsConfig import PileUpToolsCfg
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def LUCID_OverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with LUCID_DigiTop."""
    acc = ComponentAccumulator()
    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags,Load={('LUCID_SimHitCollection','LucidSimHitsVector')} ) )
    if "DigitizationTool" not in kwargs:
        kwargs.setdefault("DigitizationTool", acc.popToolsAndMerge(LUCID_OverlayPileUpToolCfg(flags)))

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)

    acc.addEventAlgo(CompFactory.LUCID_DigiTop("LUCID_DigiTop", **kwargs))
    return acc


def LUCID_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for LUCID digitization and Output"""
    acc = ComponentAccumulator()
    acc.merge(LUCID_DigitizationBasicCfg(flags, **kwargs))
    acc.merge(LUCID_DigitizationOutputCfg(flags))

    return acc


def LUCID_DigitizationOverlayCfg(flags, **kwargs):
    """Return ComponentAccumulator for LUCID digitization and Output"""
    acc = ComponentAccumulator()
    acc.merge(LUCID_OverlayDigitizationBasicCfg(flags, **kwargs))
    acc.merge(LUCID_DigitizationOutputCfg(flags))

    return acc
