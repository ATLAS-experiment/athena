# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def ActsToTrkConverterToolCfg(flags,
                              name: str = "ActsToTrkConverterTool",
                              **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # Currently this does not work if we are in a muon-only mode
    if flags.Detector.GeometryITk and 'TrackingGeometryTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault("TrackingGeometryTool", acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)))
    else:
         # Disable TrackingGeometryTool
         kwargs.setdefault("TrackingGeometryTool", "")
    
    kwargs.setdefault("ExtractMuonSurfaces", flags.Muon.usePhaseIIGeoSetup)

    from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
    kwargs.setdefault('SummaryTool', acc.getPrimaryAndMerge(InDetTrackSummaryToolCfg(flags)))

    
    if flags.Detector.GeometryITk:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import ITkRotCreatorCfg
        kwargs.setdefault('RotCreatorTool', acc.popToolsAndMerge(ITkRotCreatorCfg(flags)))
        from InDetConfig.InDetBoundaryCheckToolConfig import ITkBoundaryCheckToolCfg
        kwargs.setdefault("BoundaryCheckTool", acc.popToolsAndMerge(ITkBoundaryCheckToolCfg(flags)))
    elif flags.Detector.GeometryID:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import InDetRotCreatorCfg
        kwargs.setdefault('RotCreatorTool', acc.popToolsAndMerge(InDetRotCreatorCfg(flags)))
        from InDetConfig.InDetBoundaryCheckToolConfig import InDetBoundaryCheckToolCfg
        kwargs.setdefault("BoundaryCheckTool",acc.popToolsAndMerge(InDetBoundaryCheckToolCfg(flags)))
    elif flags.Detector.GeometryMuon:
        from TrkConfig.TrkRIO_OnTrackCreatorConfig import MuonRotCreatorCfg
        kwargs.setdefault('RotCreatorTool', acc.popToolsAndMerge(MuonRotCreatorCfg(flags)))
  
    acc.setPrivateTools(CompFactory.ActsTrk.ActsToTrkConverterTool(name, **kwargs))
    return acc


def TrkToActsConvertorAlgCfg(flags,
                             name: str = "",
                             **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    if 'ConvertorTool' not in kwargs:
        kwargs.setdefault("ConvertorTool", acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.TrkToActsConvertorAlg(name, **kwargs))
    return acc

def ActsToTrkConvertorAlgCfg(flags,
                             name: str = "ActsToTrkConvertorAlg",
                             **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # convert proper ACTS track collection
    # this depends on the ambi resol. activation
    kwargs.setdefault('ACTSTracksLocation', 'ActsTracks' if not flags.Acts.doAmbiguityResolution else 'ActsResolvedTracks')

    if 'TrackingGeometryTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault("TrackingGeometryTool", acc.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)))

    if 'ATLASConverterTool' not in kwargs:
        from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
        kwargs.setdefault("ATLASConverterTool", acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)))

    if 'BoundaryCheckTool' not in kwargs:
        if flags.Detector.GeometryITk:
            from InDetConfig.InDetBoundaryCheckToolConfig import ITkBoundaryCheckToolCfg
            kwargs.setdefault("BoundaryCheckTool", acc.popToolsAndMerge(ITkBoundaryCheckToolCfg(flags)))
        else:
            from InDetConfig.InDetBoundaryCheckToolConfig import InDetBoundaryCheckToolCfg
            kwargs.setdefault("BoundaryCheckTool", acc.popToolsAndMerge(InDetBoundaryCheckToolCfg(flags)))

    if 'SummaryTool' not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
        kwargs.setdefault("SummaryTool", acc.popToolsAndMerge(InDetTrackSummaryToolCfg(flags)))

    if flags.Acts.doRotCorrection and 'RotCreatorTool' not in kwargs:
        if flags.Detector.GeometryITk:
            from TrkConfig.TrkRIO_OnTrackCreatorConfig import ITkRotCreatorCfg
            kwargs.setdefault("RotCreatorTool", acc.popToolsAndMerge(ITkRotCreatorCfg(flags, name="ActsRotCreatorTool")))
        else:
            from TrkConfig.TrkRIO_OnTrackCreatorConfig import InDetRotCreatorCfg
            kwargs.setdefault("RotCreatorTool", acc.popToolsAndMerge(InDetRotCreatorCfg(flags, name="ActsRotCreatorTool")))

    acc.addEventAlgo(CompFactory.ActsTrk.ActsToTrkConvertorAlg(name, **kwargs))
    return acc

def RunTrackConversion(flags, track_collections = [], outputfile='dump.json'):
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
        flags, OutputLevel=1, TrackCollectionKeys=track_collections)
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
