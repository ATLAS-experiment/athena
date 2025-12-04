# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#
# @file D3PDMakerConfig/python/D3PDMakerFlags.py
# @author scott snyder <snyder@bnl.gov>
# @date Aug, 2009
# @brief Configuration flags for D3PD making.
#

from AthenaConfiguration.AllConfigFlags import initConfigFlags


configFlags = initConfigFlags()
configFlags.addFlag ('D3PD.DoTruth', lambda f: f.Input.isMC)
D3PDMakerFlags = configFlags.D3PD


def _string_prop (p, v):
    configFlags.addFlag ('D3PD.' + p, v)
    return


##############################################################################
# SG key properties.
# 

_string_prop ('EventInfoSGKey',              'EventInfo')
_string_prop ('ElectronSGKey',               'AllElectrons')
_string_prop ('GSFTrackAssocSGKey',          'GSFTrackAssociation')
_string_prop ('PhotonSGKey',                 'Photons,PhotonCollection')
_string_prop ('MuonSGKey',                   'Muons')
_string_prop ('JetSGKey',                    'AntiKt4EMPFlowJets')
_string_prop ('TruthJetSGKey',               'AntiKt4H1TruthJets,' +
                                             #'Cone4TruthJets,' +
                                             'AntiKt4TruthJets')
_string_prop ('MissingETSGKey',              'MET_Reference_AntiKt4EMTopo,MET_RefFinal')
_string_prop ('TauSGKey',                    'TauRecContainer')
_string_prop ('CellsSGKey',                  'AllCalo')
_string_prop ('ClusterSGKey',                'CaloCalTopoClusters,CaloCalTopoCluster')
_string_prop ('EMTopoClusterSGKey',          'EMTopoCluster430,' +
                                             'EMTopoSW35' )
_string_prop ('TrackSGKey',                  'TrackParticleCandidate')
_string_prop ('VertexSGKey',                 'PrimaryVertices')
_string_prop ('MBTSSGKey',                   'MBTSContainer')
_string_prop ('TruthSGKey',                  'TruthParticles')
_string_prop ('TruthParticlesSGKey',         'TruthParticles')
_string_prop ('TruthEventSGKey',             'TruthEvents')
_string_prop ('RawClustersSGKeySuffix',      '_D3PDRawClusters')
_string_prop ('RawClustersAssocSGKeySuffix', '_D3PDRawClustersAssoc')
_string_prop ('LArCollisionTimeSGKey',       'LArCollisionTime')

#############################################################################

_string_prop ('OutputFile',                   'd3pd.root')

##############################################################################
# Trigger name pattern properties.
#

_string_prop ('egammaL1TrigPattern',   'L1_2?EM.*')
_string_prop ('ElectronL2TrigPattern', 'L2_2?e.*|L2_L1ItemStreamer_L1_2?EM.*|L2_SeededStreamerL1Calo|L2_SeededStreamerL1CaloEM')
_string_prop ('PhotonL2TrigPattern',   'L2_2?g.*|L2_L1ItemStreamer_L1_2?EM.*|L2_SeededStreamerL1Calo|L2_SeededStreamerL1CaloEM')
_string_prop ('ElectronEFTrigPattern', 'EF_2?e.*|EF_L1ItemStreamer_L1_EM.*|EF_SeededStreamerL1Calo|EF_SeededStreamerL1CaloEM')
_string_prop ('PhotonEFTrigPattern',   'EF_2?g.*|EF_L1ItemStreamer_L1_EM.*|EF_SeededStreamerL1Calo|EF_SeededStreamerL1CaloEM')

_string_prop ('MuonL1TrigPattern',     'L1_2?MU.*')
_string_prop ('MuonL2TrigPattern',     'L2_2?mu.*|L2_L1ItemStreamer_L1_2?MU.*')
_string_prop ('MuonEFTrigPattern',     'EF_2?mu.*|EF_L1ItemStreamer_L1_2?MU.*')


##############################################################################
# Muons.
#

configFlags.addFlag ('D3PD.Muons.doSingleMuons', False)
configFlags.addFlag ('D3PD.Muons.doNewChainOnly', True)
configFlags.addFlag ('D3PD.Muons.doSegmentTruth', False)


##############################################################################
# Tracking.
#

# General flags

configFlags.addFlag ('D3PD.Track.doTruth', True,
                     help = """Turn on filling of truth branches.""")

configFlags.addFlag ('D3PD.Track.storeDiagonalCovarianceAsErrors', False,
                     help = """store diagonal covariance matrix elements as errors ( err[i] = sqrt(cov[i][i]) )""")

# TrackD3PDObject flags

configFlags.addFlag ('D3PD.Track.storeHitTruthMatching', True,
                     help = """Turn on filling of hit truth matching branches""")

configFlags.addFlag ('D3PD.Track.storeDetailedTruth', False, 
    help = """Turn on filling of detailed truth branches""")

configFlags.addFlag ('D3PD.Track.trackParametersAtGlobalPerigeeLevelOfDetails', 0,
    help = """ Set level of details for track parameter at global perigee branches
        0: Don't store
        1: Store parameters only
        2: Store diagonal elements of the covariance matrix
        3: Store off diagonal elements of the covariance matrix
    """)

configFlags.addFlag ('D3PD.Track.trackParametersAtPrimaryVertexLevelOfDetails', 2,
    help = """ Set level of details for track parameter at primary vertex branches
        0: Don't store
        1: Store parameters only
        2: Store diagonal elements of the covariance matrix
        3: Store off diagonal elements of the covariance matrix
    """)

configFlags.addFlag ('D3PD.Track.trackParametersAtBeamSpotLevelOfDetails', 3,
    help = """ Set level of details for track parameter at beam spot branches
        0: Don't store
        1: Store parameters only
        2: Store diagonal elements of the covariance matrix
        3: Store off diagonal elements of the covariance matrix
    """)
    
configFlags.addFlag ('D3PD.Track.trackParametersAtBeamLineLevelOfDetails', 3,
    help = """ Set level of details for track parameter at beam spot branches
        0: Don't store
        1: Store parameters only
        2: Store diagonal elements of the covariance matrix
        3: Store off diagonal elements of the covariance matrix
    """)

configFlags.addFlag ('D3PD.Track.storeTrackParametersAtCalo', False, 
    help = """ Turn on filling of track parameters at Calo """)

configFlags.addFlag ('D3PD.Track.storeTrackParametersAtCalo2ndLayer', False, 
    help = """ Turn on filling of track parameters at Calo 2ndLayer """)

configFlags.addFlag ('D3PD.Track.storeTrackUnbiasedIPAtPV', False, 
    help = """ Turn on filling of track unbiased impact parameters at primary vertex branches """)

configFlags.addFlag ('D3PD.Track.storeTrackMomentum', True,
    help = """ Turn on filling of track momentum branches """)

configFlags.addFlag ('D3PD.Track.storeTrackInfo', True,
    help = """ Turn on filling of track info (fitter and pattern reco info) branches """)

configFlags.addFlag ('D3PD.Track.storeTrackFitQuality', True,
    help = """ Turn on filling of track fit quality (chi2 and ndof) branches """)

configFlags.addFlag ('D3PD.Track.storeTrackSummary', True,
    help = """ Turn on filling of track summary branches """)

configFlags.addFlag ('D3PD.Track.storeTrackSummaryFlags.FullInfo', False)
configFlags.addFlag ('D3PD.Track.storeTrackSummaryFlags.IDHits', True)
configFlags.addFlag ('D3PD.Track.storeTrackSummaryFlags.IDHoles', True)
configFlags.addFlag ('D3PD.Track.storeTrackSummaryFlags.IDSharedHits', True)
configFlags.addFlag ('D3PD.Track.storeTrackSummaryFlags.IDOutliers', False)
configFlags.addFlag ('D3PD.Track.storeTrackSummaryFlags.PixelInfoPlus', False)
configFlags.addFlag ('D3PD.Track.storeTrackSummaryFlags.SCTInfoPlus', False)
configFlags.addFlag ('D3PD.Track.storeTrackSummaryFlags.TRTInfoPlus', False)
configFlags.addFlag ('D3PD.Track.storeTrackSummaryFlags.InfoPlus', False)
configFlags.addFlag ('D3PD.Track.storeTrackSummaryFlags.MuonHits', True)
configFlags.addFlag ('D3PD.Track.storeTrackSummaryFlags.DBMHits', False)
configFlags.addFlag ('D3PD.Track.storeTrackSummaryFlags.ExpectBLayer', True)
configFlags.addFlag ('D3PD.Track.storeTrackSummaryFlags.HitSum', True)
configFlags.addFlag ('D3PD.Track.storeTrackSummaryFlags.HoleSum', True)
configFlags.addFlag ('D3PD.Track.storeTrackSummaryFlags.PixeldEdx', False)
configFlags.addFlag ('D3PD.Track.storeTrackSummaryFlags.ElectronPID', False)

configFlags.addFlag ('D3PD.Track.storePullsAndResiduals', False,
    help = """ Turn on filling of pulls and residuals of hits and outliers on track branches """)

configFlags.addFlag ('D3PD.Track.storeBLayerHitsOnTrack', False, 
    help = """ Turn on filling of Pixel hits on track branches """)

configFlags.addFlag ('D3PD.Track.storePixelHitsOnTrack', False, 
    help = """ Turn on filling of Pixel hits on track branches """)

configFlags.addFlag ('D3PD.Track.storeSCTHitsOnTrack', False, 
    help = """ Turn on filling of SCT hits on track branches """)

configFlags.addFlag ('D3PD.Track.storeTRTHitsOnTrack', False, 
    help = """ Turn on filling of TRT hits on track branches """)

configFlags.addFlag ('D3PD.Track.storeMDTHitsOnTrack', False, 
    help = """ Turn on filling of MDT hits on track branches """)
    
configFlags.addFlag ('D3PD.Track.storeCSCHitsOnTrack', False, 
    help = """ Turn on filling of CSC hits on track branches """)
    
configFlags.addFlag ('D3PD.Track.storeRPCHitsOnTrack', False, 
    help = """ Turn on filling of TRT hits on track branches """)

configFlags.addFlag ('D3PD.Track.storeTGCHitsOnTrack', False, 
    help = """ Turn on filling of TGC hits on track branches """)

configFlags.addFlag ('D3PD.Track.storeBLayerOutliersOnTrack', False, 
    help = """ Turn on filling of Pixel outliers on track branches """)

configFlags.addFlag ('D3PD.Track.storePixelOutliersOnTrack', False, 
    help = """ Turn on filling of Pixel outliers on track branches """)

configFlags.addFlag ('D3PD.Track.storeSCTOutliersOnTrack', False, 
    help = """ Turn on filling of SCT outliers on track branches """)

configFlags.addFlag ('D3PD.Track.storeTRTOutliersOnTrack', False, 
    help = """ Turn on filling of TRT outliers on track branches """)

configFlags.addFlag ('D3PD.Track.storeMDTOutliersOnTrack', False, 
    help = """ Turn on filling of MDT outliers on track branches """)

configFlags.addFlag ('D3PD.Track.storeCSCOutliersOnTrack', False, 
    help = """ Turn on filling of CSC outliers on track branches """)

configFlags.addFlag ('D3PD.Track.storeRPCOutliersOnTrack', False, 
    help = """ Turn on filling of RPC outliers on track branches """)
            
configFlags.addFlag ('D3PD.Track.storeTGCOutliersOnTrack', False, 
    help = """ Turn on filling of TGC outliers on track branches """)

configFlags.addFlag ('D3PD.Track.storeBLayerHolesOnTrack', False, 
    help = """ Turn on filling of Pixel holes on track branches """)
    
configFlags.addFlag ('D3PD.Track.storePixelHolesOnTrack', False, 
    help = """ Turn on filling of Pixel holes on track branches """)
    
configFlags.addFlag ('D3PD.Track.storeSCTHolesOnTrack', False, 
    help = """ Turn on filling of SCT holes on track branches """)
    
configFlags.addFlag ('D3PD.Track.storeTRTHolesOnTrack', False, 
    help = """ Turn on filling of TRT holes on track branches """)

configFlags.addFlag ('D3PD.Track.storeMDTHolesOnTrack', False, 
    help = """ Turn on filling of MDT holes on track branches """)

configFlags.addFlag ('D3PD.Track.storeCSCHolesOnTrack', False, 
    help = """ Turn on filling of CSC holes on track branches """)
        
configFlags.addFlag ('D3PD.Track.storeRPCHolesOnTrack', False, 
    help = """ Turn on filling of RPC holes on track branches """)
            
configFlags.addFlag ('D3PD.Track.storeTGCHolesOnTrack', False, 
    help = """ Turn on filling of TGC holes on track branches """)

configFlags.addFlag ('D3PD.Track.storeVertexAssociation', False, 
    help = """ Turn on filling of track to vertex association """)
    
configFlags.addFlag ('D3PD.Track.storeTrackPredictionAtBLayer', True,
    help = """ Turn on filling of track prediction at the B-Layer branches """)

configFlags.addFlag ('D3PD.Track.storeTruthInfo', False, 
    help = """ Turn on filling truth perigee and classification information. """)

# VertexD3PDObject flags

configFlags.addFlag ('D3PD.Track.vertexPositionLevelOfDetails', 3,
    help = """ Set level of details for vertex vertex position branches
        0: Don't store
        1: Store position only
        2: Store diagonal elements of the covariance matrix
        3: Store off diagonal elements of the covariance matrix
    """)

configFlags.addFlag ('D3PD.Track.storeVertexType', True,
    help = """ Turn on filling of vertex type as defined here
    https://svnweb.cern.ch/trac/atlasoff/browser/Tracking/TrkEvent/TrkEventPrimitives/trunk/TrkEventPrimitives/VertexType.h
    """)

configFlags.addFlag ('D3PD.Track.storeVertexFitQuality', True,
    help = """ Turn on filling of vertex fit quality (chi2 and ndof) branches """)

configFlags.addFlag ('D3PD.Track.storeVertexKinematics', True,
    help = """ Turn on filling of vertex kinematics (sumPT, ...) branches """)

configFlags.addFlag ('D3PD.Track.storeVertexPurity', False, 
    help = """ Turn on filling of vertex purity (truth matching) branches """)

configFlags.addFlag ('D3PD.Track.storeVertexTrackAssociation', False, 
    help = """ Turn on filling of vertex track association branches """)

configFlags.addFlag ('D3PD.Track.storeVertexTrackIndexAssociation', True,
    help = """ Turn on filling of vertex track index association branches """)



##############################################################################


configFlags.addFlag ('D3PD.AutoFlush', -30000000, None,
                     """Value to set for ROOT's AutoFlush parameter.
(For ROOT trees only; tells how often the tree baskets will be flushed.)
0 disables flushing.
-1 makes no changes to what THistSvc did.
Any other negative number gives the number of bytes after which to flush.
A positive number gives the number of entries after which to flush.""")


configFlags.addFlag ('D3PD.HaveEgammaUserData', False, None,
                     'If true, access results of egamma analysis stored in UserData.')


configFlags.addFlag ('D3PD.MakeEgammaUserData', True, None,
                     'If true, run egamma analysis to make UserData if not already done.')


configFlags.addFlag ('D3PD.EgammaUserDataPrefix', 'egammaD3PDAnalysis_', None,
                     'Prefix to use for UserData labels for egamma.')


configFlags.addFlag ('D3PD.TruthDoPileup', False, None,
                     'Set to true to include pileup in truth information.')


configFlags.addFlag ('D3PD.TruthWriteExtraJets', False, None,
                     'Set to true to include additional truth jet collections.')


configFlags.addFlag ('D3PD.PreD3PDAlgSeqName', 'PreD3PDAlgorithms', None,
                     'Sequence of algorithms to run before the D3PD maker.')


configFlags.addFlag ('D3PD.FilterAlgSeqSuffix', '_FilterAlgorithms', None,
    """Suffix for a sequence of algorithms to filter D3PD making.

The sequence name is formed by adding this string to the name of the
D3PD making algorithm.  If any of the algorithms in this sequence fail
their filter decision, then no D3PD entry will be made for this event.

The filter sequence may also be referenced by the filterSeq property
of the D3PD algorithm.
""")


configFlags.addFlag ('D3PD.SaveObjectMetadata', True, None,
                     'Control whether metadata about the D3PDObjects should be saved.')


configFlags.addFlag ('D3PD.CompressionLevel', 6, None,
                     'Controls the compression level of the ROOT file produced.')
