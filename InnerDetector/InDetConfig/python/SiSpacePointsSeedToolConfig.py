# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of SiSpacePointsSeedTool_xk package
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType


def SiSpacePointsSeedMaker_CosmicCfg(
        flags, name="InDetSpSeedsMaker_Cosmic", **kwargs):
    acc = ComponentAccumulator()

    acc.setPrivateTools(CompFactory.InDet.SiSpacePointsSeedMaker_Cosmic(
        name, **kwargs))
    return acc


def SiSpacePointsSeedMaker_HeavyIonCfg(
        flags, name="InDetSpSeedsMaker_HeavyIon", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("maxdImpact",
                      flags.Tracking.ActiveConfig.maxPrimaryImpact)
    kwargs.setdefault("maxZ", flags.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("minZ", -flags.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("maxdImpactPPS",
                      flags.Tracking.ActiveConfig.maxdImpactPPSSeeds)
    kwargs.setdefault("maxdImpactSSS",
                      flags.Tracking.ActiveConfig.maxdImpactSSSSeeds)

    acc.setPrivateTools(CompFactory.InDet.SiSpacePointsSeedMaker_HeavyIon(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def SiSpacePointsSeedMaker_LowMomentumCfg(
        flags, name="InDetSpSeedsMaker_LowMomentum", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("maxdImpact",
                      flags.Tracking.ActiveConfig.maxPrimaryImpact)
    kwargs.setdefault("maxZ", flags.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("minZ", -flags.Tracking.ActiveConfig.maxZImpact)

    kwargs.setdefault("maxRadius1",
                      0.75*flags.Tracking.ActiveConfig.radMax)
    kwargs.setdefault("maxRadius2",
                      flags.Tracking.ActiveConfig.radMax)
    kwargs.setdefault("pTmax", flags.Tracking.ActiveConfig.maxPT)
    kwargs.setdefault("mindRadius", 4.0)

    acc.setPrivateTools(CompFactory.InDet.SiSpacePointsSeedMaker_LowMomentum(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def SiSpacePointsSeedMaker_BeamGasCfg(
        flags, name="InDetSpSeedsMaker_BeamGas", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("maxdImpact",
                      flags.Tracking.ActiveConfig.maxPrimaryImpact)
    kwargs.setdefault("maxZ", flags.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("minZ", -flags.Tracking.ActiveConfig.maxZImpact)

    kwargs.setdefault("maxRadius1",
                      0.75*flags.Tracking.ActiveConfig.radMax)
    kwargs.setdefault("maxRadius2",
                      flags.Tracking.ActiveConfig.radMax)
    kwargs.setdefault("maxRadius3",
                      flags.Tracking.ActiveConfig.radMax)

    acc.setPrivateTools(CompFactory.InDet.SiSpacePointsSeedMaker_BeamGas(
        name, **kwargs))
    return acc


def SiSpacePointsSeedMaker_ATLxkCfg(
        flags, name="InDetSpSeedsMaker", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("maxdImpact",
                      flags.Tracking.ActiveConfig.maxPrimaryImpact)
    kwargs.setdefault("maxZ", flags.Tracking.ActiveConfig.maxZImpact)
    kwargs.setdefault("minZ", -flags.Tracking.ActiveConfig.maxZImpact)

    if flags.Tracking.ActiveConfig.extension in ["", "Forward"]:
        kwargs.setdefault("maxdImpactSSS",
                          flags.Tracking.ActiveConfig.maxdImpactSSSSeeds)
        kwargs.setdefault("maxSeedsForSpacePointStrips",
                          flags.Tracking.ActiveConfig.maxSeedsPerSP_Strips)
        kwargs.setdefault("maxSeedsForSpacePointPixels",
                          flags.Tracking.ActiveConfig.maxSeedsPerSP_Pixels)
        kwargs.setdefault("alwaysKeepConfirmedStripSeeds",
                          flags.Tracking.ActiveConfig.keepAllConfirmedStripSeeds)
        kwargs.setdefault("alwaysKeepConfirmedPixelSeeds",
                          flags.Tracking.ActiveConfig.keepAllConfirmedPixelSeeds)
        kwargs.setdefault("mindRadius", 10)
        kwargs.setdefault("maxSizeSP", 200)
        kwargs.setdefault("dImpactCutSlopeUnconfirmedSSS", 1.25)
        kwargs.setdefault("dImpactCutSlopeUnconfirmedPPP", 2.0)

        if flags.Tracking.ActiveConfig.extension == "Forward":
            kwargs.setdefault("checkEta", True)
            kwargs.setdefault("etaMin", flags.Tracking.ActiveConfig.minEta)

    elif flags.Tracking.ActiveConfig.extension == "R3LargeD0":
        kwargs.setdefault("optimisePhiBinning", False)
        kwargs.setdefault("maxSeedsForSpacePointStrips",
                          flags.Tracking.ActiveConfig.maxSeedsPerSP_Strips)
        kwargs.setdefault("alwaysKeepConfirmedStripSeeds",
                          flags.Tracking.ActiveConfig.keepAllConfirmedStripSeeds)
        kwargs.setdefault("maxdRadius", 150)
        kwargs.setdefault("seedScoreBonusConfirmationSeed", -2000)

    if flags.Tracking.writeSeedValNtuple:
        kwargs.setdefault("WriteNtuple", True)
        acc.addService(CompFactory.THistSvc(
            Output=["valNtuples DATAFILE='SeedMakerValidation.root' OPT='RECREATE'"]))

    acc.setPrivateTools(CompFactory.InDet.SiSpacePointsSeedMaker_ATLxk(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc


def SiSpacePointsSeedMakerCfg(flags, **kwargs):
    # Properties valid for all of the classes
    kwargs.setdefault("pTmin", flags.Tracking.ActiveConfig.minPT)
    kwargs.setdefault("radMax", flags.Tracking.ActiveConfig.radMax)
    kwargs.setdefault("etaMax", flags.Tracking.ActiveConfig.maxEta)
    kwargs.setdefault("usePixel", flags.Tracking.ActiveConfig.usePixelSeeding)
    kwargs.setdefault("SpacePointsPixelName", 'PixelSpacePoints')
    kwargs.setdefault("useSCT", flags.Tracking.ActiveConfig.useSCTSeeding)
    kwargs.setdefault("SpacePointsSCTName", 'SCT_SpacePoints')
    kwargs.setdefault("useOverlapSpCollection",
                      flags.Tracking.ActiveConfig.useSCTSeeding)
    kwargs.setdefault("SpacePointsOverlapName", 'OverlapSpacePoints')

    if flags.Tracking.ActiveConfig.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault("PRDtoTrackMap", (
            'InDetPRDtoTrackMap' + flags.Tracking.ActiveConfig.extension))

    #
    # --- Space points seeds maker, use different ones for cosmics and collisions
    #
    if flags.Beam.Type is BeamType.Cosmics:
        return SiSpacePointsSeedMaker_CosmicCfg(flags, **kwargs)
    elif flags.Tracking.ActiveConfig.isLowPt:
        return SiSpacePointsSeedMaker_LowMomentumCfg(flags, **kwargs)
    elif flags.Tracking.ActiveConfig.extension == "HeavyIon":
        return SiSpacePointsSeedMaker_HeavyIonCfg(flags, **kwargs)
    elif flags.Tracking.ActiveConfig.extension == "BeamGas":
        return SiSpacePointsSeedMaker_BeamGasCfg(flags, **kwargs)
    else:
        return SiSpacePointsSeedMaker_ATLxkCfg(flags, **kwargs)


def TrigSiSpacePointsSeedMakerCfg(flags, **kwargs):
    
    # Properties valid for all of the classes
    kwargs.setdefault("pTmin", flags.Tracking.ActiveConfig.minPT)
    kwargs.setdefault("radMax", flags.Tracking.ActiveConfig.radMax)
    kwargs.setdefault("etaMax", flags.Tracking.ActiveConfig.maxEta)
    kwargs.setdefault("usePixel", flags.Tracking.ActiveConfig.usePixelSeeding)
    kwargs.setdefault("SpacePointsPixelName", 'PixelTrigSpacePoints')
    kwargs.setdefault("useSCT", flags.Tracking.ActiveConfig.useSCTSeeding)
    kwargs.setdefault("SpacePointsSCTName", 'SCT_TrigSpacePoints')
    kwargs.setdefault("useOverlapSpCollection",
                      flags.Tracking.ActiveConfig.useSCTSeeding)
    kwargs.setdefault("SpacePointsOverlapName", 'OverlapSpacePoints')

    if flags.Tracking.ActiveConfig.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault("PRDtoTrackMap", 'TrigPRDtoTrackMap') #2023fix + flags.Tracking.ActiveConfig.extension))

    #
    # --- Space points seeds maker, use different ones for cosmics and collisions
    #
    if flags.Tracking.ActiveConfig.input_name=="cosmics":
        return SiSpacePointsSeedMaker_CosmicCfg(flags, **kwargs)
    # elif flags.Reco.EnableHI:   #2023fix - in principle minBias and HI should use specific versions too
    #     return SiSpacePointsSeedMaker_HeavyIonCfg(flags, **kwargs)
    # elif flags.Tracking.ActiveConfig.isLowPt:
    #     return SiSpacePointsSeedMaker_LowMomentumCfg(flags, **kwargs)
    else:
        kwargs.setdefault("maxdImpact", flags.Tracking.ActiveConfig.maxRPhiImpact)
        kwargs.setdefault("maxZ", flags.Tracking.ActiveConfig.maxZImpact)
        kwargs.setdefault("minZ", -flags.Tracking.ActiveConfig.maxZImpact)
                              
        return SiSpacePointsSeedMaker_ATLxkCfg(flags, 
                                               name = 'TrigSiSpacePointsSeedMaker_'+flags.Tracking.ActiveConfig.input_name,
                                               **kwargs)

    
def ITkSiSpacePointsSeedMakerCfg(flags, name="ITkSpSeedsMaker", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("pTmin", flags.Tracking.ActiveConfig.minPTSeed)
    kwargs.setdefault("maxdImpact",
                      flags.Tracking.ActiveConfig.maxPrimaryImpactSeed)
    kwargs.setdefault("maxZ", flags.Tracking.ActiveConfig.maxZImpactSeed)
    kwargs.setdefault("minZ", -flags.Tracking.ActiveConfig.maxZImpactSeed)
    kwargs.setdefault("radMax", flags.Tracking.ActiveConfig.radMax)
    kwargs.setdefault("etaMax", flags.Tracking.ActiveConfig.maxEta)

    kwargs.setdefault("usePixel",
                      flags.Tracking.ActiveConfig.useITkPixel and
                      flags.Tracking.ActiveConfig.useITkPixelSeeding)
    kwargs.setdefault("SpacePointsPixelName", 'ITkPixelSpacePoints')
    kwargs.setdefault("useStrip",
                      flags.Tracking.ActiveConfig.useITkStrip and
                      flags.Tracking.ActiveConfig.useITkStripSeeding)
    kwargs.setdefault("SpacePointsStripName", 'ITkStripSpacePoints')
    kwargs.setdefault("useOverlapSpCollection",
                      flags.Tracking.ActiveConfig.useITkStrip and
                      flags.Tracking.ActiveConfig.useITkStripSeeding)
    kwargs.setdefault("SpacePointsOverlapName", 'ITkOverlapSpacePoints')

    if flags.Tracking.ActiveConfig.usePrdAssociationTool:
        # not all classes have that property !!!
        kwargs.setdefault("PRDtoTrackMap", (
            'ITkPRDtoTrackMap' + flags.Tracking.ActiveConfig.extension))

    if flags.Tracking.doITkFastTracking:
        kwargs.setdefault("useFastTracking", True)
        kwargs.setdefault("maxSeedsForSpacePoint", 3)

    if flags.Tracking.ActiveConfig.extension == "LargeD0":
        kwargs.setdefault("maxSeedsForSpacePoint", 5)
        kwargs.setdefault("isLRT", True)
        kwargs.setdefault("maxZPPP",
                          flags.Tracking.ActiveConfig.maxZSpacePointsPPPSeeds)
        kwargs.setdefault("maxZSSS",
                          flags.Tracking.ActiveConfig.maxZSpacePointsSSSSeeds)

    if flags.Tracking.writeSeedValNtuple:
        kwargs.setdefault("WriteNtuple", True)
        acc.addService(CompFactory.THistSvc(
            Output=["valNtuples DATAFILE='SeedMakerValidation.root' OPT='RECREATE'"]))

    acc.setPrivateTools(CompFactory.ITk.SiSpacePointsSeedMaker(
        name+flags.Tracking.ActiveConfig.extension, **kwargs))
    return acc
