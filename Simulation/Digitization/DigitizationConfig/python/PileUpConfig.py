"""ComponentAccumulator confguration for pileup digitization

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from RngComps.RandomServices import dSFMT, AthRNGSvcCfg
from DigitizationConfig import PileUpEventType
from DigitizationConfig.RunDependentConfig import (
    maxNevtsPerXing,
    LumiProfileSvcCfg, NoProfileSvcCfg,
)
from BeamEffects.BeamEffectsAlgConfig import BeamSpotFixerAlgCfg


def PileUpConfigdSFMT(name):
    """Local wrapper for dSFMT RNG service"""
    seedOffset = " OFFSET 340 123 345"
    if name=="PileUpCollXingStream":
        seedOffset = " OFFSET 340 123 345"
    if name=="BEAMINT":
        seedOffset = " OFFSET 340 678 91011"
    return dSFMT(name + seedOffset)


def StepArrayBMCfg(flags, name="StepArrayBM", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("IntensityPattern", flags.Digitization.PU.BeamIntensityPattern)
    kwargs.setdefault("SignalPattern", flags.Digitization.PU.SignalPatternForSteppingCache)
    acc.addService(CompFactory.StepArrayBM(name, **kwargs), primary=True)
    return acc


def FixedArrayBMCfg(flags, name="FixedArrayBM", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("IntensityPattern", flags.Digitization.PU.BeamIntensityPattern)
    kwargs.setdefault("T0Offset", flags.Digitization.PU.FixedT0BunchCrossing)
    acc.addService(CompFactory.FixedArrayBM(name, **kwargs), primary=True)
    return acc


def ArrayBMCfg(flags, name="ArrayBM", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("IntensityPattern", flags.Digitization.PU.BeamIntensityPattern)
    kwargs.setdefault("RandomSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    acc.addService(CompFactory.ArrayBM(name, **kwargs), primary=True)
    return acc


def GenericBackgroundEventSelectorCfg(flags, name="GenericBackgroundEventSelector", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("KeepInputFilesOpen", True)
    kwargs.setdefault("ProcessMetadata", False)
    acc.addService(CompFactory.EventSelectorAthenaPool(name, **kwargs), primary=True)
    return acc


def LowPtMinBiasEventSelectorCfg(flags, name="LowPtMinBiasEventSelector", **kwargs):
    kwargs.setdefault("InputCollections", flags.Digitization.PU.LowPtMinBiasInputCols)
    return GenericBackgroundEventSelectorCfg(flags, name, **kwargs)


def HighPtMinBiasEventSelectorCfg(flags, name="HighPtMinBiasEventSelector", **kwargs):
    kwargs.setdefault("InputCollections", flags.Digitization.PU.HighPtMinBiasInputCols)
    kwargs.setdefault('SkipEvents', flags.Digitization.PU.HighPtMinBiasInputColOffset)
    return GenericBackgroundEventSelectorCfg(flags, name, **kwargs)


def CavernEventSelectorCfg(flags, name="cavernEventSelector", **kwargs):
    kwargs.setdefault("InputCollections", flags.Digitization.PU.CavernInputCols)
    return GenericBackgroundEventSelectorCfg(flags, name, **kwargs)


def BeamGasEventSelectorCfg(flags, name="BeamGasEventSelector", **kwargs):
    kwargs.setdefault("InputCollections", flags.Digitization.PU.BeamGasInputCols)
    return GenericBackgroundEventSelectorCfg(flags, name, **kwargs)


def BeamHaloEventSelectorCfg(flags, name="BeamHaloEventSelector", **kwargs):
    kwargs.setdefault("InputCollections", flags.Digitization.PU.BeamHaloInputCols)
    return GenericBackgroundEventSelectorCfg(flags, name, **kwargs)


def MinBiasCacheCfg(flags, name="MinBiasCache", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("CollPerXing", flags.Digitization.PU.NumberOfLowPtMinBias + flags.Digitization.PU.NumberOfHighPtMinBias)
    kwargs.setdefault("FractionOfCache1Collisions", (flags.Digitization.PU.NumberOfLowPtMinBias/
                                                     (flags.Digitization.PU.NumberOfLowPtMinBias + flags.Digitization.PU.NumberOfHighPtMinBias)))
    # may need to have a separate type in the future
    kwargs.setdefault("PileUpEventType", PileUpEventType.MinimumBias)
    if flags.Digitization.DoXingByXingPileUp or flags.Digitization.PU.SignalPatternForSteppingCache:
        kwargs.setdefault("Cache1ReadDownscaleFactor", 1)
    kwargs.setdefault("Cache1EventSelector", acc.getPrimaryAndMerge(LowPtMinBiasEventSelectorCfg(flags)).name)
    kwargs.setdefault("Cache2ReadDownscaleFactor", 1)
    kwargs.setdefault("Cache2EventSelector", acc.getPrimaryAndMerge(HighPtMinBiasEventSelectorCfg(flags)).name)

    kwargs.setdefault("OccupationFraction", (float(flags.Digitization.PU.BunchSpacing)/
                                             float(flags.Beam.BunchSpacing)))

    RndmStreamName = "PileUpCollXingStream"
    acc.merge(PileUpConfigdSFMT(RndmStreamName))
    kwargs.setdefault("RndmGenSvc", acc.getService("AtDSFMTGenSvc"))
    kwargs.setdefault("RndmStreamName", RndmStreamName)

    # FIXME migrated, but SplitBkgStreamsCache does not exist
    acc.setPrivateTools(CompFactory.SplitBkgStreamsCache(name, **kwargs))
    return acc


def LowPtMinBiasCacheCfg(flags, name="LowPtMinBiasCache", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("CollPerXing", flags.Digitization.PU.NumberOfLowPtMinBias)
    # may need to have a separate type in the future
    kwargs.setdefault("PileUpEventType", PileUpEventType.MinimumBias)
    if flags.Digitization.DoXingByXingPileUp or flags.Digitization.PU.SignalPatternForSteppingCache:
        kwargs.setdefault("ReadDownscaleFactor", 1)
    kwargs.setdefault("EventSelector", acc.getPrimaryAndMerge(LowPtMinBiasEventSelectorCfg(flags)).name)
    kwargs.setdefault("OccupationFraction", (float(flags.Digitization.PU.BunchSpacing)/
                                             float(flags.Beam.BunchSpacing)))

    RndmStreamName = "PileUpCollXingStream"
    acc.merge(PileUpConfigdSFMT(RndmStreamName))
    kwargs.setdefault("RndmGenSvc", acc.getService("AtDSFMTGenSvc"))
    kwargs.setdefault("RndmStreamName", RndmStreamName)

    # Use BkgStreamsStepCaches when using the StepArrayBM and BkgStreamsCache otherwise
    if flags.Digitization.PU.SignalPatternForSteppingCache:
        # FIXME migrated, but BkgStreamsStepCache does not exist
        tool = CompFactory.BkgStreamsStepCache(name, **kwargs)
    else:
        tool = CompFactory.BkgStreamsCache(name, **kwargs)

    acc.setPrivateTools(tool)
    return acc


def HighPtMinBiasCacheCfg(flags, name="HighPtMinBiasCache", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("CollPerXing", flags.Digitization.PU.NumberOfHighPtMinBias)
    # may need to have a separate type in the future
    kwargs.setdefault("PileUpEventType", PileUpEventType.HighPtMinimumBias)
    kwargs.setdefault("ReadDownscaleFactor", 1)
    kwargs.setdefault("EventSelector", acc.getPrimaryAndMerge(HighPtMinBiasEventSelectorCfg(flags)).name)
    kwargs.setdefault("OccupationFraction", (float(flags.Digitization.PU.BunchSpacing)/
                                             float(flags.Beam.BunchSpacing)))

    RndmStreamName = "PileUpCollXingStream"
    acc.merge(PileUpConfigdSFMT(RndmStreamName))
    kwargs.setdefault("RndmGenSvc", acc.getService("AtDSFMTGenSvc"))
    kwargs.setdefault("RndmStreamName", RndmStreamName)

    # Use BkgStreamsStepCaches when using the StepArrayBM and BkgStreamsCache otherwise
    if flags.Digitization.PU.SignalPatternForSteppingCache:
        # FIXME migrated, but BkgStreamsStepCache does not exist
        tool = CompFactory.BkgStreamsStepCache(name, **kwargs)
    else:
        tool = CompFactory.BkgStreamsCache(name, **kwargs)

    acc.setPrivateTools(tool)
    return acc


def CavernCacheCfg(flags, name="CavernCache", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("CollPerXing", flags.Digitization.PU.NumberOfCavern)
    kwargs.setdefault("CollDistribution", "Fixed")
    kwargs.setdefault("PileUpEventType", PileUpEventType.Cavern)
    if flags.Digitization.DoXingByXingPileUp or flags.Digitization.PU.SignalPatternForSteppingCache:
        kwargs.setdefault("ReadDownscaleFactor", 1)
    # Cavern Background Cache Should Ignore Bunch Structure
    OccupationFraction = (float(flags.Digitization.PU.BunchSpacing)/
                          float(flags.Beam.BunchSpacing))
    if flags.Digitization.PU.BeamIntensityPattern:
        kwargs.setdefault("IgnoreBeamInt", flags.Digitization.PU.CavernIgnoresBeamInt)
        if flags.Digitization.PU.CavernIgnoresBeamInt:
            OccupationFraction = 1.0
    kwargs.setdefault("OccupationFraction", OccupationFraction)
    kwargs.setdefault("EventSelector", acc.getPrimaryAndMerge(CavernEventSelectorCfg(flags)).name)

    RndmStreamName = "PileUpCollXingStream"
    acc.merge(PileUpConfigdSFMT(RndmStreamName))
    kwargs.setdefault("RndmGenSvc", acc.getService("AtDSFMTGenSvc"))
    kwargs.setdefault("RndmStreamName", RndmStreamName)

    # Use BkgStreamsStepCaches when using the StepArrayBM and BkgStreamsCache otherwise
    if flags.Digitization.PU.SignalPatternForSteppingCache:
        # FIXME migrated, but BkgStreamsStepCache does not exist
        tool = CompFactory.BkgStreamsStepCache(name, **kwargs)
    else:
        tool = CompFactory.BkgStreamsCache(name, **kwargs)
    acc.setPrivateTools(tool)
    return acc


def BeamGasCacheCfg(flags, name="BeamGasCache", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("IgnoreBeamLumi", True)
    kwargs.setdefault("CollPerXing", flags.Digitization.PU.NumberOfBeamGas)
    kwargs.setdefault("PileUpEventType", PileUpEventType.HaloGas)
    kwargs.setdefault("CollDistribution", "Poisson")
    kwargs.setdefault("ReadDownscaleFactor", 1)

    kwargs.setdefault("EventSelector", acc.getPrimaryAndMerge(BeamGasEventSelectorCfg(flags)).name)

    RndmStreamName = "PileUpCollXingStream"
    acc.merge(PileUpConfigdSFMT(RndmStreamName))
    kwargs.setdefault("RndmGenSvc", acc.getService("AtDSFMTGenSvc"))
    kwargs.setdefault("RndmStreamName", RndmStreamName)

    # Use BkgStreamsStepCaches when using the StepArrayBM and BkgStreamsCache otherwise
    if flags.Digitization.PU.SignalPatternForSteppingCache:
        # FIXME migrated, but BkgStreamsStepCache does not exist
        tool = CompFactory.BkgStreamsStepCache(name, **kwargs)
    else:
        tool = CompFactory.BkgStreamsCache(name, **kwargs)

    acc.setPrivateTools(tool)
    return acc


def BeamHaloCacheCfg(flags, name="BeamHaloCache", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("IgnoreBeamLumi", True)
    kwargs.setdefault("CollPerXing", flags.Digitization.PU.NumberOfBeamHalo)
    kwargs.setdefault("PileUpEventType", PileUpEventType.HaloGas)
    kwargs.setdefault("CollDistribution", "Poisson")
    kwargs.setdefault("ReadDownscaleFactor",  1)

    kwargs.setdefault("EventSelector", acc.getPrimaryAndMerge(BeamHaloEventSelectorCfg(flags)).name)

    RndmStreamName = "PileUpCollXingStream"
    acc.merge(PileUpConfigdSFMT(RndmStreamName))
    kwargs.setdefault("RndmGenSvc", acc.getService("AtDSFMTGenSvc"))
    kwargs.setdefault("RndmStreamName", RndmStreamName)

    #Use BkgStreamsStepCaches when using the StepArrayBM and BkgStreamsCache otherwise
    if flags.Digitization.PU.SignalPatternForSteppingCache:
        # FIXME migrated, but BkgStreamsStepCache does not exist
        tool = CompFactory.BkgStreamsStepCache(name, **kwargs)
    else:
        tool = CompFactory.BkgStreamsCache(name, **kwargs)

    acc.setPrivateTools(tool)
    return acc


def PileUpEventLoopMgrCfg(flags, name="PileUpEventLoopMgr", **kwargs):
    acc = BeamSpotFixerAlgCfg(flags) # Needed currently for running on 21.0 HITS

    # SubDet By SubDet (default) or Xing By Xing Pile-up?
    kwargs.setdefault("XingByXing", flags.Digitization.DoXingByXingPileUp)
    # Bunch Structure
    if flags.Digitization.PU.BeamIntensityPattern:
        if flags.Digitization.PU.SignalPatternForSteppingCache:
            # Simulate Bunch Structure with events sliding backwards on a conveyor belt
            kwargs.setdefault("BeamInt", acc.getPrimaryAndMerge(StepArrayBMCfg(flags)).name)
        elif flags.Digitization.PU.FixedT0BunchCrossing:
            # Simulate Bunch Structure using a fixed point for the central bunch crossing
            kwargs.setdefault("BeamInt", acc.getPrimaryAndMerge(FixedArrayBMCfg(flags)).name)
        else:
            # Simulate Bunch Structure and allow the central bunch crossing to vary
            kwargs.setdefault("BeamInt", acc.getPrimaryAndMerge(ArrayBMCfg(flags)).name)

    # define inputs
    assert not flags.Input.SecondaryFiles, ("Found ConfigFlags.Input.SecondaryFiles = %r; "
                                            "double event selection is not supported "
                                            "by PileUpEventLoopMgr" % (not flags.Input.SecondaryFiles))
    acc.merge(PoolReadCfg(flags))
    kwargs.setdefault("OrigSelector", acc.getService("EventSelector"))
    BackgroundCaches = []
    # Note: experimentalDigi not migrated
    if flags.Digitization.PU.LowPtMinBiasInputCols:
        BackgroundCaches += [acc.popToolsAndMerge(LowPtMinBiasCacheCfg(flags))]
    if flags.Digitization.PU.HighPtMinBiasInputCols:
        BackgroundCaches += [acc.popToolsAndMerge(HighPtMinBiasCacheCfg(flags))]
    if flags.Digitization.PU.CavernInputCols:
        BackgroundCaches += [acc.popToolsAndMerge(CavernCacheCfg(flags))]
    if flags.Digitization.PU.BeamGasInputCols:
        BackgroundCaches += [acc.popToolsAndMerge(BeamGasCacheCfg(flags))]
    if flags.Digitization.PU.BeamHaloInputCols:
        BackgroundCaches += [acc.popToolsAndMerge(BeamHaloCacheCfg(flags))]
    kwargs.setdefault("bkgCaches", BackgroundCaches)
    # xing frequency in ns
    kwargs.setdefault("XingFrequency", flags.Digitization.PU.BunchSpacing)
    # define time range to be studied. t0 at t" ,0, xing" ,0
    kwargs.setdefault("firstXing", flags.Digitization.PU.InitialBunchCrossing)
    kwargs.setdefault("lastXing", flags.Digitization.PU.FinalBunchCrossing)

    if flags.Input.RunAndLumiOverrideList:
        kwargs.setdefault("MaxMinBiasCollPerXing", maxNevtsPerXing(flags))
        kwargs.setdefault("BeamLuminosity", acc.getPrimaryAndMerge(LumiProfileSvcCfg(flags)).name)
    else:
        kwargs.setdefault("MaxMinBiasCollPerXing", flags.Digitization.PU.NumberOfCollisions)
        kwargs.setdefault("BeamLuminosity", acc.getPrimaryAndMerge(NoProfileSvcCfg(flags)).name)

    from AthenaKernel.EventIdOverrideConfig import EvtIdModifierSvcCfg
    kwargs.setdefault("EvtIdModifierSvc", acc.getPrimaryAndMerge(EvtIdModifierSvcCfg(flags))) # TODO make configurable?
    kwargs.setdefault("EventInfoName", "Input_EventInfo")
    # Note that this is a hack. It is needed to fix beam spot information
    # as original xAOD::EventInfo is created before conditions data could
    # be read. Only the "EventInfoName" should change.

    if flags.Input.MCChannelNumber > 0:
        kwargs.setdefault("MCChannelNumber", flags.Input.MCChannelNumber)

    # write PileUpEventInfo
    if flags.Output.doWriteRDO:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, "RDO", ItemList=[
            "xAOD::EventInfoContainer#PileUpEventInfo",
            "xAOD::EventInfoAuxContainer#PileUpEventInfo*",
        ]))

    acc.addService(CompFactory.PileUpEventLoopMgr(name, **kwargs))
    return acc


def NoPileUpMuWriterCfg(flags, name="NoPileUpMuWriter", **kwargs):
    """NoPileUpMuWriter configuration."""
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.NoPileUpMuWriter(name, **kwargs))
    return acc
