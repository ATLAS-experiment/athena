# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.flagTestLOD       import flagTestLOD
from D3PDMakerCoreComps.D3PDObject        import make_SGDataVector_D3PDObject
from D3PDMakerCoreComps.IndexAssociation  import IndexAssociation
from TrackD3PDMaker.PerigeeAssociation    import PerigeeAssociation
from D3PDMakerConfig.D3PDMakerFlags       import D3PDMakerFlags
from AthenaConfiguration.ComponentFactory import CompFactory

D3PD = CompFactory.D3PD


def xAODTrackD3PDObject(_label='trkTrack',
                        _prefix='trkTrack_',
                        _sgkey='TrackParticleCandidate',
                        _object_name='TrackD3PDObject',
                        vertexTarget='vx',
                        vertexPrefix='vx_',
                        vertexSGKey='VxPrimaryCandidate',
                        flags = D3PDMakerFlags.Track):

    object = make_SGDataVector_D3PDObject (
        'xAOD::TrackParticleContainer',
        _sgkey,
        _prefix,
        _object_name,
        default_allowMissing = True,
        default_label = _label,
        allow_args = ['storeTrackMomentum',
                      'trackParametersAtGlobalPerigeeLevelOfDetails',
                      'trackParametersAtPrimaryVertexLevelOfDetails',
                      'trackParametersAtBeamSpotLevelOfDetails',
                      'storeDiagonalCovarianceAsErrors',
                      'storeTrackFitQuality',
                      'storeTrackPredictionAtBLayer',
                      'storeTrackInfo',
                      'storeVertexAssociation',
                      'storeTrackSummary',
                      ])

    ## default perigee (at (0,0,0))
    PerigeeAssoc = PerigeeAssociation\
                   (object,
                    D3PD.TrackParticlePerigeeAtOOAssociationTool,
                    "GlobalPerigee",
                    fillMomName = 'storeTrackMomentum',
                    levelName = 'trackParametersAtGlobalPerigeeLevelOfDetails')

    perigeeBLPrediction = \
        PerigeeAssoc.defineBlock (flagTestLOD('storeTrackPredictionAtBLayer', flags),
                                  _prefix+'BLayerInfo',
                                  D3PD.PerigeeBLPredictionFillerTool)
    def _pixelLayerToolHook (c, flags, acc, *args, **kw):
        from InDetConfig.InDetTestPixelLayerConfig import InDetTestPixelLayerToolInnerCfg
        c.InDetTestPixelLayerTool = acc.popToolsAndMerge (InDetTestPixelLayerToolInnerCfg (flags))
        return
    perigeeBLPrediction.defineHook (_pixelLayerToolHook)

    # perigee at Primary Vertex
    PerigeeAtPVAssoc = PerigeeAssociation(object,  # noqa: F841
                        D3PD.TrackParticlePerigeeAtPVAssociationTool,
                        "PerigeeAtPV",
                        suffix='_wrtPV',
                        levelName = 'trackParametersAtPrimaryVertexLevelOfDetails')
    def _trackToVertexHook (c, flags, acc, *args, **kw):
        from TrackToVertex.TrackToVertexConfig import InDetTrackToVertexCfg
        c.Associator.TrackToVertexTool = acc.popToolsAndMerge (InDetTrackToVertexCfg (flags))
        return
    PerigeeAtPVAssoc.defineHook (_trackToVertexHook)

    # perigee at Beam Spot
    PerigeeAtBSAssoc = PerigeeAssociation(object,  # noqa: F841
                        D3PD.TrackParticlePerigeeAtBSAssociationTool,
                        "PerigeeAtBS",
                        suffix='_wrtBS', 
                        levelName = 'trackParametersAtBeamSpotLevelOfDetails')
    PerigeeAtBSAssoc.defineHook (_trackToVertexHook)

    object.defineBlock(flagTestLOD('storeTrackFitQuality', flags),
                       _prefix+'FitQuality',
                        D3PD.AuxDataFillerTool,
                        Vars = ['chiSquared',
                                'numberDoF'])

    # Track Summary
    class SumVars:
        HitSum         = 1 << 0
        HoleSum        = 1 << 1
        IDHits         = 1 << 2
        IDHoles        = 1 << 3
        IDSharedHits   = 1 << 4
        IDOutliers     = 1 << 5
        PixelInfoPlus  = 1 << 6
        SCTInfoPlus    = 1 << 7
        TRTInfoPlus    = 1 << 8
        InfoPlus       = 1 << 9
        ExpectBLayer   = 1 << 10
        MuonHits       = 1 << 11
        DBMHits        = 1 << 12


        varsTable = [
            # ID hits
            [IDHits,          'nBLHits',        'numberOfInnermostPixelLayerHits'],
            [IDHits + HitSum, 'nPixHits',       'numberOfPixelHits' ],
            [IDHits + HitSum, 'nSCTHits',       'numberOfSCTHits' ],
            [IDHits + HitSum, 'nTRTHits',       'numberOfTRTHits' ],
            [IDHits,          'nTRTHighTHits',  'numberOfTRTHighThresholdHits'],
            [IDHits,          'nTRTXenonHits',  'numberOfTRTXenonHits'],

            # ID holes + dead sensors - needed for appropriate cutting
            [IDHoles + HoleSum, 'nPixHoles',         'numberOfPixelHoles' ],
            [IDHoles + HoleSum, 'nSCTHoles',         'numberOfSCTHoles' ],
            [IDHoles + HoleSum, 'nTRTHoles',         'numberOfTRTHoles' ],
            [IDHoles,           'nPixelDeadSensors', 'numberOfPixelDeadSensors' ],
            [IDHoles,           'nSCTDeadSensors',   'numberOfSCTDeadSensors' ],

            # ID shared & Split hits
            [IDSharedHits,      'nBLSharedHits',     'numberOfInnermostPixelLayerSharedHits'],
            [IDSharedHits,      'nPixSharedHits',    'numberOfPixelSharedHits'],
            [IDSharedHits,      'nSCTSharedHits',    'numberOfSCTSharedHits'],
            [IDSharedHits,      'nBLayerSplitHits',  'numberOfInnermostPixelLayerSplitHits'],
            [IDSharedHits,      'nPixSplitHits',     'numberOfPixelSplitHits'],

            # ID outliers                                              
            [IDOutliers,        'nBLayerOutliers',   'numberOfInnermostPixelLayerOutliers' ],
            [IDOutliers,        'nPixelOutliers',    'numberOfPixelOutliers' ],
            [IDOutliers,        'nSCTOutliers',      'numberOfSCTOutliers' ],
            [IDOutliers,        'nTRTOutliers',      'numberOfTRTOutliers'],
            [IDOutliers,        'nTRTHighTOutliers', 'numberOfTRTHighThresholdOutliers'],

            # Pixel info plus                                          
            [PixelInfoPlus,     'nContribPixelLayers', 'numberOfContribPixelLayers' ],
            [PixelInfoPlus,     'nGangedPixels',       'numberOfGangedPixels' ],
            [PixelInfoPlus,     'nGangedFlaggedFakes', 'numberOfGangedFlaggedFakes' ],
            [PixelInfoPlus,     'nPixelSpoiltHits',    'numberOfPixelSpoiltHits' ],

            # SCT info plus                                            
            [SCTInfoPlus,       'nSCTDoubleHoles',     'numberOfSCTDoubleHoles' ],
            [SCTInfoPlus,       'nSCTSpoiltHits',      'numberOfSCTSpoiltHits' ],

            # TRT info plus                                            
            [TRTInfoPlus,       'nTRTDeadStraws',      'numberOfTRTDeadStraws' ],
            [TRTInfoPlus,       'nTRTTubeHits',        'numberOfTRTTubeHits' ],
            [TRTInfoPlus,       'nTRTSharedHits',      'numberOfTRTSharedHits' ],
            [TRTInfoPlus,       'nTRTHTHitsTotal',     'numberOfTRTHighThresholdHitsTotal' ],

            # Info plus                                                
            [InfoPlus,          'nOutliersOnTrack',           'numberOfOutliersOnTrack'],
            [InfoPlus,          'standardDeviationOfChi2OS',],

            # Expect BLayer hit                                        
            [ExpectBLayer,      'expectInnermostPixelLayerHit'],

            # Muon hits
            [MuonHits + HitSum, 'numberOfPrecisionLayers'],
            [MuonHits + HitSum, 'numberOfPrecisionHoleLayers'],
            [MuonHits + HitSum, 'numberOfPhiLayers'],
            [MuonHits + HitSum, 'numberOfPhiHoleLayers'],
            [MuonHits + HitSum, 'numberOfTriggerEtaLayers'],
            [MuonHits + HitSum, 'numberOfTriggerEtaHoleLayers'],
        ]


        @classmethod
        def varlist (cls,
                     FullInfo,
                     IDHits,
                     IDHoles,
                     IDSharedHits,
                     IDOutliers,
                     PixelInfoPlus,
                     SCTInfoPlus,
                     TRTInfoPlus,
                     InfoPlus,
                     ExpectBLayer,
                     MuonHits,
                     DBMHits,
                     HitSum,
                     HoleSum,
                     ElectronPID,
                     PixeldEdx):
            ret = []
            mask = 0
            if FullInfo: mask = ~0
            if IDHits:        mask |= cls.IDHits
            if IDHoles:       mask |= cls.IDHoles
            if IDSharedHits:  mask |= cls.IDSharedHits
            if IDOutliers:    mask |= cls.IDOutliers
            if PixelInfoPlus: mask |= cls.PixelInfoPlus
            if SCTInfoPlus:   mask |= cls.SCTInfoPlus
            if TRTInfoPlus:   mask |= cls.TRTInfoPlus
            if InfoPlus:      mask |= cls.InfoPlus
            if ExpectBLayer:  mask |= cls.ExpectBLayer
            if MuonHits:      mask |= cls.MuonHits
            if DBMHits:       mask |= cls.DBMHits
            if HitSum:        mask |= cls.HitSum
            if HoleSum:       mask |= cls.HoleSum

            for v in cls.varsTable:
                if (v[0] & mask) != 0:
                    var = v[1]
                    enum = v[2] if len(v) >= 3 else var
                    ret.append (var + ' = ' + enum)

            if ElectronPID or FullInfo:
                ret.append ('eProbabilityComb')
                ret.append ('eProbabilityHT')

            if PixeldEdx or FullInfo:
                ret.append ('pixeldEdx')
                ret.append ('numberOfUsedHitsdEdx')
                ret.append ('numberOfIBLOverflowsdEdx')
            return ret
            

    sumvarlist = SumVars.varlist (
        FullInfo = flags.storeTrackSummaryFlags.FullInfo,
        IDHits = flags.storeTrackSummaryFlags.IDHits,
        IDHoles = flags.storeTrackSummaryFlags.IDHoles,
        IDSharedHits = flags.storeTrackSummaryFlags.IDSharedHits,
        IDOutliers = flags.storeTrackSummaryFlags.IDOutliers,
        PixelInfoPlus = flags.storeTrackSummaryFlags.PixelInfoPlus,
        SCTInfoPlus = flags.storeTrackSummaryFlags.SCTInfoPlus,
        TRTInfoPlus = flags.storeTrackSummaryFlags.TRTInfoPlus,
        InfoPlus = flags.storeTrackSummaryFlags.InfoPlus,
        ExpectBLayer = flags.storeTrackSummaryFlags.ExpectBLayer,
        MuonHits = flags.storeTrackSummaryFlags.MuonHits,
        DBMHits = flags.storeTrackSummaryFlags.DBMHits,
        HitSum = flags.storeTrackSummaryFlags.HitSum,
        HoleSum = flags.storeTrackSummaryFlags.HoleSum,
        ElectronPID = flags.storeTrackSummaryFlags.ElectronPID,
        PixeldEdx = flags.storeTrackSummaryFlags.PixeldEdx,
        )

    object.defineBlock (flagTestLOD('storeTrackSummary', flags),
                        _prefix + 'TrackSummary',
                        D3PD.AuxDataFillerTool,
                        Vars = sumvarlist)


    # Track Info
    object.defineBlock (flagTestLOD('storeTrackInfo', flags),
                        _prefix + 'TrackInfo',
                        D3PD.AuxDataFillerTool,
                        Vars = ['trackFitter',
                                'particleHypothesis',
                                'trackProperties',
                                'patternRecoInfo'])

    # Vertex association
    VertexAssoc = IndexAssociation(object,  # noqa: F841
        D3PD.TrackParticleVertexAssociationTool,
        vertexTarget,
        prefix = vertexPrefix,
        VxSGKey = vertexSGKey,
        level = flagTestLOD('storeVertexAssociation', flags))

    return object


xAODTrackParticleD3PDObject = xAODTrackD3PDObject()
