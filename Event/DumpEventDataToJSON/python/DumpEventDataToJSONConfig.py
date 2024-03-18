# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import VERBOSE


def DumpEventDataToJSONAlgCfg(flags, doExtrap=False, doACTSEDM = True, **kwargs):
    result = ComponentAccumulator()
    extrapolationEngine = ""
    if doExtrap:
        from AtlasGeoModel.GeoModelConfig import GeoModelCfg
        gmsAcc = GeoModelCfg(flags)
        result.merge(gmsAcc)

        from TrkConfig.AtlasExtrapolationEngineConfig import AtlasExtrapolationEngineCfg
        extrapAcc = AtlasExtrapolationEngineCfg(flags)
        extrapolationEngine = extrapAcc.getPrimary()
        result.merge(extrapAcc)

        kwargs.setdefault('Extrapolator', extrapolationEngine)
    else:
        kwargs.setdefault('Extrapolator', '')


    if doACTSEDM:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault('TrackingGeometryTool', result.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)))
    else:
        kwargs.setdefault('TrackingGeometryTool', '')
        kwargs.setdefault('TrackContainerKeys', [])

    if 'StreamDAOD_PHYSLITE' in flags.Input.Collections:
        # AnalysisJets and AnalysisLargeRJets are the standard jet containers
        kwargs.setdefault('JetContainerKeys', ['AnalysisJets','AnalysisLargeRJets'])
        # AnalysisMuons are the standard calibrated muon container
        kwargs.setdefault('MuonContainerKeys', ['AnalysisMuons'])
        # AnalysisTauJets are the calibrated tau jet container
        kwargs.setdefault('TauJetContainerKeys', ['AnalysisTauJets'])
        # AnalysisElectrons are the standard calibrated electron container
        kwargs.setdefault('ElectronContainerKeys', ['AnalysisElectrons'])
        # AnalysisPhotons are the standard calibrated photon container
        kwargs.setdefault('PhotonContainerKeys', ['AnalysisPhotons'])
        # No clusters or cells in DAOD_PHYSLITE
        kwargs.setdefault('CaloClusterContainerKeys', [])
        kwargs.setdefault('CaloCellContainerKey', [])
        # No real track collections in DAOD_PHYSLITE
        kwargs.setdefault('TrackCollectionKeys', [])
        kwargs.setdefault('TrackContainerKeys', [])
        # InDetTrackParticles are heavily skimmed, so not useful output - keep only muons
        kwargs.setdefault('TrackParticleContainerKeys', []) #['CombinedMuonTrackParticles'])
        # No prep raw data in DAOD_PHYSLITE
        kwargs.setdefault('MdtPrepRawDataKey', '')
        kwargs.setdefault('RpcPrepRawDataKey', '')
        kwargs.setdefault('TgcPrepRawDataKey', '')
        kwargs.setdefault('PixelPrepRawDataKey', '')
        kwargs.setdefault('SctPrepRawDataKey', '')
        kwargs.setdefault('TrtPrepRawDataKey', '')
        # These are set based on detector flags and can simply be disabled (no point)
        kwargs.update( {'MMPrepRawDataKey':'', 'CscPrepRawDataKey':'', 'sTgcPrepRawDataKey':''} )

    dumpAlg = CompFactory.DumpEventDataToJsonAlg( **kwargs)
    result.addEventAlgo(dumpAlg)
    return result


if __name__ == "__main__":
    # Run this with python -m DumpEventDataToJSON.DumpEventDataToJSONConfig myESD.pool.root
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    parser = flags.getArgumentParser()
    parser.add_argument("-o", "--output", dest="output", default='Event.json',
                        help="write JSON to FILE", metavar="FILE")
    parser.add_argument("--prependCalib", help="Prepend a calibration event with some labelled objects at specific eta/phi.",
                        action="store_true")
    args = parser.parse_args()

    print('Running DumpEventDataToJSON on {} and outputting to {}. Prepend calib event is {}'.format(
        args.filesInput, args.output, args.prependCalib))

    from AthenaCommon.Logging import log
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    # Uncomment for debugging
    # from AthenaCommon.Constants import DEBUG
    # log.setLevel(DEBUG)

    args = flags.fillFromArgs(parser = parser)

    # This should run serially for the moment.
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1

    flags.dump()
    log.debug('Lock config flags now.')
    flags.lock()

    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    if flags.Detector.GeometryBpipe:
        from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
        cfg.merge(BeamPipeGeometryCfg(flags))

    if flags.Detector.GeometryPixel:
        from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
        cfg.merge(PixelReadoutGeometryCfg(flags))

    if flags.Detector.GeometrySCT:
        from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
        cfg.merge(SCT_ReadoutGeometryCfg(flags))

    if flags.Detector.GeometryTRT:
        from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
        cfg.merge(TRT_ReadoutGeometryCfg(flags))

    if flags.Detector.GeometryITkPixel:
        from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
        cfg.merge(ITkPixelReadoutGeometryCfg(flags))

    if flags.Detector.GeometryITkStrip:
        from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
        cfg.merge(ITkStripReadoutGeometryCfg(flags))

    if flags.Detector.GeometryLAr:
        from LArGeoAlgsNV.LArGMConfig import LArGMCfg
        cfg.merge(LArGMCfg(flags))

    if flags.Detector.GeometryTile:
        from TileGeoModel.TileGMConfig import TileGMCfg
        cfg.merge(TileGMCfg(flags))

    if flags.Detector.GeometryMuon:
        from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
        cfg.merge(MuonGeoModelCfg(flags))

    from TrkConfig.TrackCollectionReadConfig import TrackCollectionReadCfg
    cfg.merge(TrackCollectionReadCfg(flags, 'Tracks'))

    muon_edm_helper_svc = CompFactory.Muon.MuonEDMHelperSvc("MuonEDMHelperSvc")
    cfg.addService(muon_edm_helper_svc)

    # Disable doExtrap if you would prefer not to use the extrapolator.
    topoAcc = DumpEventDataToJSONAlgCfg(
        flags, doExtrap=True, doACTSEDM=False, OutputLevel=VERBOSE, DumpTestEvent=args.prependCalib, OutputLocation=args.output,
        CscPrepRawDataKey = "CSC_Clusters" if flags.Detector.EnableCSC else "",
        MMPrepRawDataKey = "MM_Measurements" if flags.Detector.EnableMM else "",
        sTgcPrepRawDataKey = "STGC_Measurements" if flags.Detector.EnablesTGC else "",
        )
    
    cfg.merge(topoAcc)

    cfg.run(2)
