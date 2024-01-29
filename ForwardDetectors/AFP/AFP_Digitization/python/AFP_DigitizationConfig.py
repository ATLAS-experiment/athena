# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#==============================================================
# Job options file for the AFP_Digitization package
#==============================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from Digitization.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg


# The earliest bunch crossing time for which interactions will be sent to the AFP Digitization code.
def AFP_FirstXing():
    return 0 #Assume AFP is only sensitive to the current bunch crossing, for the moment


# The latest bunch crossing time for which interactions will be sent to the AFP Digitization code.
def AFP_LastXing():
    return 0 #Assume AFP is only sensitive to the current bunch crossing, for the moment


def AFP_SIDPUXinfFolderCfg(flags, name="AFP_SIDXinfFolder", **kwargs):
    """Return a PileUpXingFoldertool for AFP SID"""
    # bunch crossing range in ns
    kwargs.setdefault("FirstXing", AFP_FirstXing())
    kwargs.setdefault("LastXing", AFP_LastXing())
    kwargs.setdefault("ItemList", ["AFP_SIDSimHitCollection#AFP_SIDSimHitCollection"])
    return PileUpXingFolderCfg(flags, name, **kwargs)


def AFP_TDPUXinfFolderCfg(flags, name="AFP_TDXinfFolder", **kwargs):
    """Return a PileUpXingFoldertool for AFP TD"""
    # bunch crossing range in ns
    kwargs.setdefault("FirstXing", AFP_FirstXing())
    kwargs.setdefault("LastXing", AFP_LastXing())
    kwargs.setdefault("ItemList", ["AFP_TDSimHitCollection#AFP_TDSimHitCollection"])
    return PileUpXingFolderCfg(flags, name, **kwargs)


def AFP_DigitizationToolCfg(flags, name="AFP_PileUpTool", **kwargs):
    """Return ComponentAccumulator with AFP digitization tool."""
    acc = ComponentAccumulator()

    if flags.Digitization.PileUp:
        intervals = []
        if flags.Digitization.DoXingByXingPileUp:
            kwargs.setdefault("FirstXing", AFP_FirstXing())
            kwargs.setdefault("LastXing", AFP_LastXing())
        else:
            intervals += [acc.popToolsAndMerge(AFP_SIDPUXinfFolderCfg(flags))]
            intervals += [acc.popToolsAndMerge(AFP_TDPUXinfFolderCfg(flags))]
        kwargs.setdefault("mergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
        kwargs.setdefault("OnlyUseContainerName", True)
    else:
        kwargs.setdefault("mergeSvc", '')
        kwargs.setdefault("OnlyUseContainerName", False)

    from RngComps.RandomServices import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)

    acc.setPrivateTools(CompFactory.AFP_PileUpTool(name, **kwargs))
    return acc


def AFP_OverlayDigitizationToolCfg(flags, name="AFP_OverlayDigitizationTool", **kwargs):
    """Return ComponentAccumulator with AFP_DigitizationTool for Overlay"""
    acc = ComponentAccumulator()
    kwargs.setdefault("mergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", False)
    tool = acc.popToolsAndMerge(AFP_DigitizationToolCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def AFP_DigitizationOutputCfg(flags):
    """Return ComponentAccumulator with Output for AFP. Not standalone."""
    acc = ComponentAccumulator()
    if flags.Output.doWriteRDO:
        ItemList = ["AFP_TDDigiCollection#AFP_TDDigiCollection",
                                               "AFP_SiDigiCollection#AFP_SiDigiCollection",
                                               "xAOD::AFPSiHitContainer#AFPSiHitContainer", "xAOD::AFPSiHitAuxContainer#AFPSiHitContainerAux.",
                                               "xAOD::AFPToFHitContainer#AFPToFHitContainer", "xAOD::AFPToFHitAuxContainer#AFPToFHitContainerAux."]
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags,"RDO", ItemList))
    return acc


def AFP_DigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator for AFP digitization"""
    acc = ComponentAccumulator()
    if "PileUpTools" not in kwargs:
        PileUpTools = acc.popToolsAndMerge(AFP_DigitizationToolCfg(flags))
        kwargs["PileUpTools"] = PileUpTools
    from Digitization.PileUpToolsConfig import PileUpToolsCfg
    acc.merge(PileUpToolsCfg(flags, **kwargs))
    return acc


def AFP_OverlayDigitizationBasicCfg(flags, **kwargs):
    """Return ComponentAccumulator with AFP_DigiTop."""
    acc = ComponentAccumulator()
    if flags.Common.ProductionStep != ProductionStep.FastChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags,Load={('AFP_TDSimHitCollection','StoreGateSvc+AFP_TDSimHitCollection'),
                                               ('AFP_SIDSimHitCollection','StoreGateSvc+AFP_SIDSimHitCollection')} ) )
    if "DigitizationTool" not in kwargs:
        kwargs.setdefault("DigitizationTool", acc.popToolsAndMerge(AFP_OverlayDigitizationToolCfg(flags)))

    if flags.Concurrency.NumThreads > 0:
        kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)

    # Set common overlay extra inputs
    kwargs.setdefault("ExtraInputs", flags.Overlay.ExtraInputs)

    acc.addEventAlgo(CompFactory.AFP_DigiTop("AFP_DigiTop", **kwargs))
    return acc


def AFP_DigitizationCfg(flags, **kwargs):
    """Return ComponentAccumulator for AFP digitization and Output"""
    acc = ComponentAccumulator()
    acc.merge(AFP_DigitizationBasicCfg(flags, **kwargs))
    acc.merge(AFP_DigitizationOutputCfg(flags))

    return acc


def AFP_DigitizationOverlayCfg(flags, **kwargs):
    """Return ComponentAccumulator for AFP digitization and Output"""
    acc = ComponentAccumulator()
    acc.merge(AFP_OverlayDigitizationBasicCfg(flags, **kwargs))
    acc.merge(AFP_DigitizationOutputCfg(flags))

    return acc
