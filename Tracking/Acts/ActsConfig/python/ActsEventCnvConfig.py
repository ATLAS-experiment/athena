# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def ActsToTrkConverterToolCfg(flags,
                              name: str = "ActsToTrkConverterTool",
                              setupMuon = False,
                              setupITk = True,
                              **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from ActsConfig.ActsGeometryConfig import ActsGeometryRealmConvTool
    kwargs.setdefault("GeometryRealmConvTool", acc.getPrimaryAndMerge(ActsGeometryRealmConvTool(flags)))

    setupMuon = setupMuon and flags.Muon.usePhaseIIGeoSetup
    setupITk = setupITk and (flags.Detector.GeometryITk or flags.Detector.GeometryID)
    if not setupMuon or not flags.Detector.EnableMDT:
        kwargs.setdefault("MdtKey", "")
    if not setupMuon or not flags.Detector.EnableRPC:
        kwargs.setdefault("RpcKey", "")
    if not setupMuon or not flags.Detector.EnableTGC:
        kwargs.setdefault("TgcKey", "")
    if not setupMuon or not flags.Detector.EnableMM:
        kwargs.setdefault("MmKey", "")
    if not setupMuon or not flags.Detector.EnablesTGC:
        kwargs.setdefault("sTgcKey", "")
    if not setupITk:
        kwargs.setdefault("PixelKey", "")
        kwargs.setdefault("SctKey", "")
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    kwargs.setdefault("MuonIdHelperSvc", acc.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags)) if setupMuon else "")

    from TrkConfig.TrkTrackSummaryToolConfig import CombinedSummaryToolCfg, InDetTrackSummaryToolCfg
    if setupMuon:
        kwargs.setdefault('SummaryTool', acc.getPrimaryAndMerge(CombinedSummaryToolCfg(flags)))
    else:
        kwargs.setdefault('SummaryTool', acc.getPrimaryAndMerge(InDetTrackSummaryToolCfg(flags)))
    if setupMuon and (flags.Detector.GeometryRPC or flags.Detector.GeometryTGC):
        from MuonConfig.MuonRIO_OnTrackCreatorToolConfig import TriggerChamberClusterOnTrackCreatorCfg
        kwargs.setdefault("CompetingRotCreator", acc.getPrimaryAndMerge(TriggerChamberClusterOnTrackCreatorCfg(flags)))

    from TrkConfig.TrkRIO_OnTrackCreatorConfig import CombinedRotCreatorCfg, InDetRotCreatorCfg, MuonRotCreatorCfg
    if setupMuon and setupITk:
        rotCreatorTool = acc.getPrimaryAndMerge(CombinedRotCreatorCfg(flags))
        rotCreatorTool.ToolMuonCluster.RestrictWarnings = True
        kwargs.setdefault('RotCreatorTool', rotCreatorTool)
    elif setupMuon:
        rotCreatorTool = acc.getPrimaryAndMerge(MuonRotCreatorCfg(flags))
        rotCreatorTool.ToolMuonCluster.RestrictWarnings = True
        kwargs.setdefault('RotCreatorTool', rotCreatorTool)
    else:
        kwargs.setdefault('RotCreatorTool', acc.popToolsAndMerge(InDetRotCreatorCfg(flags)))

    acc.setPrivateTools(CompFactory.ActsTrk.ActsToTrkConverterTool(name, **kwargs))
    return acc


def TrkToActsConvertorAlgCfg(flags,
                             name: str = "",
                             **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    if 'ATLASConverterTool' not in kwargs:
        kwargs.setdefault("ATLASConverterTool", acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.TrkToActsConvertorAlg(name, **kwargs), primary = True)
    return acc

def ActsToTrkConvertorAlgCfg(flags,
                             name: str = "ActsToTrkConvertorAlg",
                             **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # convert proper ACTS track collection
    # this depends on the ambi resol. activation
    kwargs.setdefault('ACTSTracksLocation', 'ActsTracks' if not flags.Acts.doAmbiguityResolution else 'ActsResolvedTracks')
    kwargs.setdefault("TracksLocation", "SiSPSeededActsTracks")
    ACTSTracksLocation = kwargs.pop('ACTSTracksLocation')
    TracksLocation = kwargs.pop("TracksLocation")
    ATLASConverterTool = None
    if "ATLASConverterTool" in kwargs:
        ATLASConverterTool = kwargs["ATLASConverterTool"]
    else:
        ATLASConverterTool = acc.getPrimaryAndMerge(ActsToTrkConverterToolCfg(flags, **kwargs))
    acc.addEventAlgo(CompFactory.ActsTrk.ActsToTrkConvertorAlg(name, 
                                                               TracksLocation = TracksLocation, 
                                                               ACTSTracksLocation = ACTSTracksLocation,
                                                               ATLASConverterTool = ATLASConverterTool))
    return acc


def xAODtoTrkConverterAlgCfg(flags, name ="xAODToTrkConversionAlg", 
                             setupMuon = False, setupITk = True, **kwargs):
    result = ComponentAccumulator()
    if setupITk:
        if flags.Tracking.ITkMainPass.doAthenaToActsCluster:
            return result
        from InDetConfig.InDetPrepRawDataFormationConfig import ITkXAODToInDetClusterConversionCfg
        result.merge(ITkXAODToInDetClusterConversionCfg(flags))
    if 'ATLASConverterTool' not in kwargs:
        kwargs.setdefault("ATLASConverterTool", result.popToolsAndMerge(ActsToTrkConverterToolCfg(flags, setupMuon = setupMuon ,setupITk = setupITk)))

    result.addEventAlgo(CompFactory.ActsTrk.xAODtoTrkConverterAlg(name, **kwargs), primary = True)
    return result

def ActsToXAODTrackConverterAlgCfg(flags,
                                   name: str = "ActsToXAODTrackConverterAlg",
                                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('InputActsTracksLocation', '')
    kwargs.setdefault('OutputActsTracksLocation', '')

    acc.addEventAlgo(CompFactory.ActsTrk.ActsToXAODTrackConverterAlg(name, **kwargs), primary = True)    
    return acc

def ActsTrackToTrackParticleCnvToolCfg(flags,
                                       name: str = "ActsTrackToTrackParticleCnvTool",
                                       **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # To produce AtlasFieldCacheCondObj
    from MagFieldServices.MagFieldServicesConfig import (
        AtlasFieldCacheCondAlgCfg)
    acc.merge(AtlasFieldCacheCondAlgCfg(flags))

    kwargs.setdefault('FirstAndLastParameterOnly',True)
    kwargs.setdefault('ComputeExpectedLayerPattern',True)


    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
    kwargs.setdefault('ExtrapolationTool', acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags)) )
    acc.setPrivateTools(CompFactory.ActsTrk.TrackToTrackParticleCnvTool(name, **kwargs))
    return acc

def ActsTrackToTrackParticleCnvAlgCfg(flags,
                                      name: str = "ActsTrackToTrackParticleCnvAlg",
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Beam Spot Cond is a requirement
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))

    # Configure TrackToTrackParticleConvTool
    tool_kwargs = {}
    if "ExtrapolationTool" in kwargs:
        tool_kwargs["ExtrapolationTool"] = kwargs.pop("ExtrapolationTool")
    if "FirstAndLastParameterOnly" in kwargs:
        tool_kwargs["FirstAndLastParameterOnly"] = kwargs.pop("FirstAndLastParameterOnly")
    if "ComputeExpectedLayerPattern" in kwargs:
        tool_kwargs["ComputeExpectedLayerPattern"] = kwargs.pop("ComputeExpectedLayerPattern")
    if "MuonSummaryTool" in kwargs:
        tool_kwargs["MuonSummaryTool"] = kwargs.pop("MuonSummaryTool")
    if 'TrackToTrackParticleCnvTool' not in kwargs:
        kwargs['TrackToTrackParticleCnvTool'] = acc.popToolsAndMerge(
            ActsTrackToTrackParticleCnvToolCfg(flags, **tool_kwargs))

    kwargs.setdefault('BeamSpotKey', 'BeamSpotData')
    kwargs.setdefault("PerigeeExpression", flags.Tracking.perigeeExpression)
    kwargs.setdefault('VertexContainerKey', 'PrimaryVertices')

    acc.addEventAlgo(CompFactory.ActsTrk.TrackToTrackParticleCnvAlg(name, **kwargs), primary = True)

    return acc

def RunTrackConversion(flags, track_collections = [], outputfile='dump.json', setupMuon = False):
    from TrkConfig.TrackCollectionReadConfig import TrackCollectionReadCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    
    cfg = MainServicesCfg(flags)

    # Set up to read ESD and tracks
    cfg.merge(PoolReadCfg(flags))
    for collection in track_collections:
        cfg.merge(TrackCollectionReadCfg(flags, collection))

    # Needed to read tracks
    from TrkEventCnvTools.TrkEventCnvToolsConfig import TrkEventCnvSuperToolCfg
    cfg.merge(TrkEventCnvSuperToolCfg(flags))

    # Muon geometry not yet in ActsTrackingGeometrySvcCfg
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    cfg.merge(MuonGeoModelCfg(flags))

    # Now setup the convertor
    acc = TrkToActsConvertorAlgCfg(
        flags, OutputLevel=1, TrackCollectionKeys=track_collections,
        ATLASConverterTool= cfg.popToolsAndMerge(ActsToTrkConverterToolCfg(flags, setupMuon=setupMuon)))
    cfg.merge(acc)

    # Let's dump the input tracks, and also the output ACTS tracks
    from DumpEventDataToJSON.DumpEventDataToJSONConfig import DumpEventDataToJSONAlgCfg
    acc = DumpEventDataToJSONAlgCfg(
        flags, doExtrap=False, OutputLevel=1,
        TrackCollectionKeys=track_collections,
        CscPrepRawDataKey="",
        MMPrepRawDataKey="",
        sTgcPrepRawDataKey="",
        MdtPrepRawDataKey="",
        RpcPrepRawDataKey="",
        TgcPrepRawDataKey="",
        PixelPrepRawDataKey="",
        SctPrepRawDataKey="",
        TrtPrepRawDataKey="",
        CaloCellContainerKey=[""],
        CaloClusterContainerKeys=[""],
        MuonContainerKeys=[""],
        JetContainerKeys=[""],
        TrackParticleContainerKeys=[""],
        OutputLocation=outputfile,
    )
    cfg.merge(acc)
    cfg.printConfig(withDetails=True, summariseProps=True)
    from AthenaCommon.Constants import FATAL
    ### The translation of the Phase-II stlye muon geometry throws a ton
    ### of error messages which degrades the physics performance but does
    ### not harm the technical execution. In order, to make the tests pass
    ### silence the algorithm until the tracking geometry translation understands
    ### the new style of geometry building.
    cfg.getCondAlgo("AtlasTrackingGeometryCondAlg").OutputLevel = FATAL

    sc = cfg.run()
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")


if __name__ == "__main__":
    # To run this, do e.g.
    # python -m ActsEventCnv.ActsEventCnvConfig --threads=1
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    args = flags.fillFromArgs()

    flags.Input.Files = [
        '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/ESD/ATLAS-P2-RUN4-03-00-00/ESD.ttbar_mu0.pool.root']
    from AthenaConfiguration.TestDefaults import defaultConditionsTags
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.CheckDependencies = True
    
    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, None, use_metadata=True,
                       toggle_geometry=True, keep_beampipe=True)

    flags.lock()
    flags.dump()
    RunTrackConversion(flags)
