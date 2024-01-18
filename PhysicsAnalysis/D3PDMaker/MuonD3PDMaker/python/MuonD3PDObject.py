# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


from D3PDMakerCoreComps.D3PDObject        import make_SGDataVector_D3PDObject
from D3PDMakerCoreComps.SimpleAssociation import SimpleAssociation
from D3PDMakerConfig.D3PDMakerFlags       import D3PDMakerFlags
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD


MuonD3PDObject = \
           make_SGDataVector_D3PDObject ('DataVector<xAOD::Muon_v1>',
                                         D3PDMakerFlags.MuonSGKey,
                                         'mu_', 'MuonD3PDObject')

MuonD3PDObject.defineBlock (0, 'Kinematics',
                            D3PD.FourMomFillerTool,
                            WriteRect = True,
                            WriteE = True,
                            WriteM = True)
MuonD3PDObject.defineBlock (
    0, 'AllAuthor',
    D3PD.AuxDataFillerTool,
    Vars = ['allauthor = allAuthors'])

MuonD3PDObject.defineBlock (
    1, 'Authors',
    D3PD.AuxDataFillerTool,
    Vars = ['author'])
MuonD3PDObject.defineBlock (1, 'NumberOfSegments',
                            D3PD.MuonNumberOfSegmentsFillerTool)

MuonD3PDObject.defineBlock (
    1, 'Isolation',
    D3PD.AuxDataFillerTool,
    Vars = ['etcone20 < float: 0',
            'etcone30 < float: 0',
            'etcone40 < float: 0',
            'ptcone20 < float: 0',
            'ptcone30 < float: 0',
            'ptcone40 < float: 0',
            'scatteringCurvatureSignificance < float: 0',
            'scatteringNeighbourSignificance < float: 0',
            'momentumBalanceSignificance < float: 0'])

MuonD3PDObject.defineBlock (
    1, 'CaloEnergyLoss',
    D3PD.AuxDataFillerTool,
    Vars = ['energyLossType'])
MuonD3PDObject.defineBlock (
    1, 'MuonType',
    D3PD.AuxDataFillerTool,
    Vars = ['muonType'])

MuonD3PDObject.defineBlock (2, 'MuonHitSummary',
                            D3PD.MuonTrkHitFillerTool)

MuonD3PDObject.defineBlock (
    2, 'MuonSpectrometerHitSummary',
    D3PD.AuxDataFillerTool,
    Vars = ['primarySector',
            'secondarySector',
            'innerSmallHits',
            'innerLargeHits',
            'middleSmallHits',
            'middleLargeHits',
            'outerSmallHits',
            'outerLargeHits',
            'extendedSmallHits',
            'extendedLargeHits',
            'innerSmallHoles',
            'innerLargeHoles',
            'middleSmallHoles',
            'middleLargeHoles',
            'outerSmallHoles',
            'outerLargeHoles',
            'extendedSmallHoles',
            'extendedLargeHoles',
            'phiLayer1Hits',
            'phiLayer2Hits',
            'phiLayer3Hits',
            'phiLayer4Hits',
            'etaLayer1Hits',
            'etaLayer2Hits',
            'etaLayer3Hits',
            'etaLayer4Hits',
            'phiLayer1Holes',
            'phiLayer2Holes',
            'phiLayer3Holes',
            'phiLayer4Holes',
            'etaLayer1Holes',
            'etaLayer2Holes',
            'etaLayer3Holes',
            'etaLayer4Holes',
            ])

MuonD3PDObject.defineBlock (
    4, 'MuonSpectrometerFieldIntegral',
    D3PD.AuxDataFillerTool,
    Vars = ['MSFieldIntegral = spectrometerFieldIntegral < float: 0'])


############################################################################
# TrackParticle variables
############################################################################


MuonTPAssoc = SimpleAssociation \
    (MuonD3PDObject,
     D3PD.MuonTrackParticleAssociationTool,
     matched = 'hastrack',
     blockname = 'TrkInfo')
MuonTPAssoc.defineBlock (
    0, 'Charge',
    # TrackD3PDMaker
    D3PD.TrackParticleChargeFillerTool)
TrackParticlePerigeeAssoc = SimpleAssociation \
    (MuonTPAssoc,
     # TrackD3PDMaker
     D3PD.TrackParticlePerigeeAtOOAssociationTool,
     prefix = 'track',
     blockname = 'TrackParticlePerigeeAssoc')
TrackParticlePerigeeAssoc.defineBlock (1, 'TrkParameters',
                                       # TrackD3PDMaker
                                       D3PD.PerigeeFillerTool)

TrackParticleCovarAssoc = SimpleAssociation (TrackParticlePerigeeAssoc,
                                             # TrackD3PDMaker
                                             D3PD.PerigeeCovarianceAssociationTool,
                                             blockname = 'TrackParticleCovarAssoc')
TrackParticleCovarAssoc.defineBlock (1, 'TrkCovDiag',
                                     # TrackD3PDMaker
                                     D3PD.CovarianceFillerTool,
                                     IsTrackPerigee = True,
                                     Error = False,
                                     DiagCovariance = True)
TrackParticleCovarAssoc.defineBlock (2, 'TrkCovOffDiag',
                                     # TrackD3PDMaker
                                     D3PD.CovarianceFillerTool,
                                     IsTrackPerigee = True,
                                     Error = False,
                                     OffDiagCovariance = True)

MuonTPAssoc.defineBlock (
    2, 'TrkFitQuality',
    D3PD.AuxDataFillerTool,
    Vars = ['chi2 = chiSquared',
            'ndof = numberDoF'],
    prefix = 'trackfit')


MuonIDTPAssoc = SimpleAssociation \
                (MuonD3PDObject,
                 D3PD.MuonTrackParticleAssociationTool,
                 blockname = 'TrkInfoInDet',
                 Type = 'InDet')

if not D3PDMakerFlags.Muons.doSingleMuons:
    #
    # Impact parameter variables.  Use the ID track for this.
    # These are likely not very useful for standalone muons anyway.
    #
    from TrackD3PDMaker.TrackParticleImpactParameters \
         import TrackParticleImpactParameters
    TrackParticleImpactParameters (MuonIDTPAssoc)


MuonPVPerigeeAssoc = SimpleAssociation (
    MuonTPAssoc,
    # TrackD3PDMaker
    D3PD.TrackParticlePerigeeAtPVAssociationTool,
    suffix = '_exPV',
    blockname = 'MuonPVPerigeeAssoc')
def _trackToVertexHook (c, flags, acc, *args, **kw):
    from TrackToVertex.TrackToVertexConfig import InDetTrackToVertexCfg
    c.Associator.TrackToVertexTool = acc.popToolsAndMerge (InDetTrackToVertexCfg (flags))
    return
MuonPVPerigeeAssoc.defineHook (_trackToVertexHook)
MuonPVPerigeeAssoc.defineBlock (
    1, 'ParametersAtPV',
    # TrackD3PDMaker
    D3PD.PerigeeFillerTool)
MuonPVCovarAssoc = SimpleAssociation (
    MuonPVPerigeeAssoc,
    # TrackD3PDMaker
    D3PD.PerigeeCovarianceAssociationTool,
    blockname = 'MuonPVCovarAssoc')
MuonPVCovarAssoc.defineBlock (
    1, 'PrimaryTrackDiagonalCovarianceAtPV',
    # TrackD3PDMaker
    D3PD.CovarianceFillerTool,
    IsTrackPerigee = True,
    Error = True)
MuonPVCovarAssoc.defineBlock (
    1, 'PrimaryTrackOffDiagonalCovarianceAtPV',
    # TrackD3PDMaker
    D3PD.CovarianceFillerTool,
    IsTrackPerigee = True,
    Error = False,
    OffDiagCovariance = True)

MuonCBTPAssoc = SimpleAssociation \
    (MuonD3PDObject,
     D3PD.MuonTrackParticleAssociationTool,
     Type = 'Combined',
     prefix = 'cb_',
     blockname = 'CBTrkInfo')
MuonCBPVPerigeeAssoc = SimpleAssociation (
    MuonCBTPAssoc,
    # TrackD3PDMaker
    D3PD.TrackParticlePerigeeAtPVAssociationTool,
    suffix = '_exPV',
    blockname = 'MuonCBPVPerigeeAssoc')
MuonCBPVPerigeeAssoc.defineHook (_trackToVertexHook)
MuonCBPVPerigeeAssoc.defineBlock (
    1, 'CBParametersAtPV',
    # TrackD3PDMaker
    D3PD.PerigeeFillerTool)

MuonIDTPAssoc = SimpleAssociation \
    (MuonD3PDObject,
     D3PD.MuonTrackParticleAssociationTool,
     Type = 'InDet',
     prefix = 'id_',
     blockname = 'IDTrkInfo')
MuonIDPerigeeAssoc = SimpleAssociation (
    MuonIDTPAssoc,
    # TrackD3PDMaker
    D3PD.TrackParticlePerigeeAtOOAssociationTool,
    blockname = 'MuonIDPerigeeAssoc')
MuonIDPerigeeAssoc.defineBlock (
    1, 'IDParameters',
    # TrackD3PDMaker
    D3PD.PerigeeFillerTool)
MuonIDPVPerigeeAssoc = SimpleAssociation (
    MuonIDTPAssoc,
    # TrackD3PDMaker
    D3PD.TrackParticlePerigeeAtPVAssociationTool,
    suffix = '_exPV',
    blockname = 'MuonIDPVPerigeeAssoc')
MuonIDPVPerigeeAssoc.defineHook (_trackToVertexHook)
MuonIDPVPerigeeAssoc.defineBlock (
    1, 'IDParametersAtPV',
    # TrackD3PDMaker
    D3PD.PerigeeFillerTool)

MuonIDPVCovarAssoc = SimpleAssociation (
    MuonIDPVPerigeeAssoc,
    # TrackD3PDMaker
    D3PD.PerigeeCovarianceAssociationTool,
    blockname = 'MuonIDPVCovarAssoc')
MuonIDPVCovarAssoc.defineBlock (
    2, 'IDTrackDiagonalCovarianceAtPV',
    # TrackD3PDMaker
    D3PD.CovarianceFillerTool,
    IsTrackPerigee = True,
    Error = True)
MuonIDPVCovarAssoc.defineBlock (
    3, 'IDTrackOffDiagonalCovarianceAtPV',
    # TrackD3PDMaker
    D3PD.CovarianceFillerTool,
    IsTrackPerigee = True,
    Error = False,
    OffDiagCovariance = True)


MuonMETPAssoc = SimpleAssociation \
    (MuonD3PDObject,
     D3PD.MuonTrackParticleAssociationTool,
     Type = 'MuonSpectrometer',
     prefix = 'me_',
     blockname = 'METrkInfo')
MuonMEPerigeeAssoc = SimpleAssociation (
    MuonMETPAssoc,
    # TrackD3PDMaker
    D3PD.TrackParticlePerigeeAtOOAssociationTool)
MuonMEPerigeeAssoc.defineBlock (
    1, 'MEParametersAt',
    # TrackD3PDMaker
    D3PD.PerigeeFillerTool)
MuonMEPVPerigeeAssoc = SimpleAssociation (
    MuonMETPAssoc,
    # TrackD3PDMaker
    D3PD.TrackParticlePerigeeAtPVAssociationTool,
    suffix = '_exPV',
    blockname = 'MuonMEPVPerigeeAssoc')
MuonMEPVPerigeeAssoc.defineHook (_trackToVertexHook)
MuonMEPVPerigeeAssoc.defineBlock (
    1, 'MEParametersAtPV',
    # TrackD3PDMaker
    D3PD.PerigeeFillerTool)

MuonMEPVCovarAssoc = SimpleAssociation (
    MuonMEPVPerigeeAssoc,
    # TrackD3PDMaker
    D3PD.PerigeeCovarianceAssociationTool,
    blockname = 'MuonMEPVCovarAssoc')
MuonMEPVCovarAssoc.defineBlock (
    2, 'METrackDiagonalCovarianceAtPV',
    # TrackD3PDMaker
    D3PD.CovarianceFillerTool,
    IsTrackPerigee = True,
    Error = True)
MuonMEPVCovarAssoc.defineBlock (
    3, 'METrackOffDiagonalCovarianceAtPV',
    # TrackD3PDMaker
    D3PD.CovarianceFillerTool,
    IsTrackPerigee = True,
    Error = False,
    OffDiagCovariance = True)


############################################################################
# Truth matching
############################################################################
if D3PDMakerFlags.DoTruth:
    truthClassification = \
        MuonD3PDObject.defineBlock (1, 'TruthClassification',
                                    D3PD.MuonTruthClassificationFillerTool)
    def _truthClassificationHook (c, flags, acc, *args, **kw):
        from TruthD3PDMaker.MCTruthClassifierConfig \
            import D3PDMCTruthClassifierCfg
        acc.merge (D3PDMCTruthClassifierCfg (flags))
        c.Classifier = acc.getPublicTool ('D3PDMCTruthClassifier')
        return
    truthClassification.defineHook (_truthClassificationHook)

    MuonTruthPartAssoc = SimpleAssociation \
        (MuonD3PDObject,
         D3PD.MuonGenParticleAssociationTool,
         blockname = 'TruthAssoc',
         DRVar = 'dr')
    def _truthClassificationAssocHook (c, flags, acc, *args, **kw):
        from TruthD3PDMaker.MCTruthClassifierConfig \
            import D3PDMCTruthClassifierCfg
        acc.merge (D3PDMCTruthClassifierCfg (flags))
        c.Associator.Classifier = acc.getPublicTool ('D3PDMCTruthClassifier')
        return
    MuonTruthPartAssoc.defineHook (_truthClassificationAssocHook)
    MuonTruthPartAssoc.defineBlock (0, 'TruthKin',
                                    D3PD.FourMomFillerTool,
                                    WriteE = True,
                                    WriteM = False,
                                    prefix = 'truth_')
    MuonTruthPartAssoc.defineBlock (0, 'Truth',
                                    D3PD.TruthParticleFillerTool,
                                    PDGIDVariable = 'type',
                                    prefix = 'truth_')
    MuonTruthPartMotherAssoc = SimpleAssociation \
      (MuonTruthPartAssoc,
       D3PD.FirstAssociationTool,
       # TruthD3PDMaker
       Associator = D3PD.TruthParticleParentAssociationTool
         ('MuonTruthPartMotherAssoc2'),
       blockname = 'MuonTruthPartMotherAssoc',
       prefix = 'truth_mother')
    MuonTruthPartMotherAssoc.defineBlock (0, 'MotherTruth',
                                          # TruthD3PDMaker
                                          D3PD.TruthParticleFillerTool,
                                          PDGIDVariable = 'type')
    MuonTruthPartAssoc.defineBlock (0, 'TruthAssocIndex',
                                    D3PD.IndexFillerTool,
                                    prefix = 'mc_',
                                    Target = 'mc_')


