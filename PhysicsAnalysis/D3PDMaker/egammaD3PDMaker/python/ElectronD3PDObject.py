# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
# @file egammaD3PDMaker/python/ElectronD3PDObject.py
# @author scott snyder <snyder@bnl.gov>
# @date 2009
# @brief Configure electron D3PD object.
#

from egammaD3PDMaker.defineBlockAndAlg      import defineBlockAndAlg
from EventCommonD3PDMaker.DRAssociation     import DRAssociation
from TrackD3PDMaker.xAODTrackSummaryFiller  import xAODTrackSummaryFiller
from D3PDMakerCoreComps.D3PDObject          import make_SGDataVector_D3PDObject
from D3PDMakerCoreComps.D3PDObject          import DeferArg
from D3PDMakerCoreComps.SimpleAssociation   import SimpleAssociation
from D3PDMakerCoreComps.IndexMultiAssociation import IndexMultiAssociation
from D3PDMakerCoreComps.resolveSGKey        import testSGKey
from D3PDMakerCoreComps.resolveSGKey        import resolveSGKey # noqa: F401
from D3PDMakerConfig.D3PDMakerFlags         import D3PDMakerFlags, configFlags # noqa: F401
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD


ElectronD3PDObject = \
           make_SGDataVector_D3PDObject ('xAOD::ElectronContainer',
                                         D3PDMakerFlags.ElectronSGKey,
                                         'el_', 'ElectronD3PDObject')

# AuxPrefix args need to be deferred in order to add in the sgkey.
auxprefix = DeferArg ('D3PDMakerFlags.EgammaUserDataPrefix + "_" +'
                      'resolveSGKey (configFlags, sgkey) + "_"',
                      globals())


ElectronD3PDObject.defineBlock (0, 'Kinematics',
                                D3PD.FourMomFillerTool,
                                WriteE  = True,
                                WriteEt = True,
                                WriteRect = True)
ElectronD3PDObject.defineBlock (
    0, 'Charge',
    D3PD.AuxDataFillerTool,
    Vars = ['charge'])
ElectronD3PDObject.defineBlock (
    0, 'Author',
    D3PD.AuxDataFillerTool,
    Vars = ['author'])
ElectronD3PDObject.defineBlock (
    0, 'Pass',
    D3PD.AuxDataFillerTool,
    # These are present only for central electrons.
    Vars = ['loose = Loose < int8_t: 0',
            'medium = Medium < int8_t: 0',
            'tight = Tight < int8_t : 0'
            ])
ElectronD3PDObject.defineBlock (
    0, 'OQ',
    D3PD.AuxDataFillerTool,
    Vars = ['OQ'])

defineBlockAndAlg \
    (ElectronD3PDObject,
     999, 'MaxEcell',
     D3PD.AuxDataFillerTool,
     'egammaMaxECellAlg',
     AuxPrefix = auxprefix,
     Vars = ['maxEcell_time',
             'maxEcell_energy',
             'maxEcell_onlId',
             'maxEcell_gain',
             'maxEcell_x',
             'maxEcell_y',
             'maxEcell_z',
             ])

defineBlockAndAlg \
    (ElectronD3PDObject,
     999, 'SumCellsGain',
     D3PD.AuxDataFillerTool,
     'egammaSumCellsGainAlg',
     AuxPrefix = auxprefix,
     Vars = [
         'Es0LowGain',
         'Es0MedGain',
         'Es0HighGain',
         'Es1LowGain',
         'Es1MedGain',
         'Es1HighGain',
         'Es2LowGain',
         'Es2MedGain',
         'Es2HighGain',
         'Es3LowGain',
         'Es3MedGain',
         'Es3HighGain',
     ])


defineBlockAndAlg \
    (ElectronD3PDObject,
     999, 'NbCellsGain',
     D3PD.AuxDataFillerTool,
     'egammaNbCellsGainAlg',
     AuxPrefix = auxprefix,
     Vars = [
         'nbCells_s0LowGain',
         'nbCells_s0MedGain',
         'nbCells_s0HighGain',
         'nbCells_s1LowGain',
         'nbCells_s1MedGain',
         'nbCells_s1HighGain',
         'nbCells_s2LowGain',
         'nbCells_s2MedGain',
         'nbCells_s2HighGain',
         'nbCells_s3LowGain',
         'nbCells_s3MedGain',
         'nbCells_s3HighGain',
     ])
            

if D3PDMakerFlags.DoTruth:
    truthClassification = \
        ElectronD3PDObject.defineBlock (1, 'TruthClassification',
                                        D3PD.egammaTruthClassificationFillerTool,
                                        DoBkgElecOrigin = True,
                                        Classifier = D3PD.D3PDMCTruthClassifier)
    def _truthClassificationHook (c, flags, acc, *args, **kw):
        from TruthD3PDMaker.MCTruthClassifierConfig \
            import D3PDMCTruthClassifierCfg
        acc.merge (D3PDMCTruthClassifierCfg (flags))
        c.Classifier = acc.getPublicTool ('D3PDMCTruthClassifier')
        return
    truthClassification.defineHook (_truthClassificationHook)
    ElectronTruthPartAssoc = SimpleAssociation \
        (ElectronD3PDObject,
         D3PD.egammaGenParticleAssociationTool,
         prefix = 'truth_',
         matched = 'matched',
         blockname = 'TruthAssoc')
    def _truthClassificationAssocHook (c, flags, acc, *args, **kw):
        from TruthD3PDMaker.MCTruthClassifierConfig \
            import D3PDMCTruthClassifierCfg
        acc.merge (D3PDMCTruthClassifierCfg (flags))
        c.Associator.Classifier = acc.getPublicTool ('D3PDMCTruthClassifier')
        return
    ElectronTruthPartAssoc.defineHook (_truthClassificationAssocHook)
    ElectronTruthPartAssoc.defineBlock (0, 'TruthKin',
                                        D3PD.FourMomFillerTool,
                                        WriteE = True,
                                        WriteM = False)
    ElectronTruthPartAssoc.defineBlock (0, 'Truth',
                                        # TruthD3PDMaker
                                        D3PD.TruthParticleFillerTool,
                                        PDGIDVariable = 'type')
    ElectronTruthPartMotherAssoc = SimpleAssociation \
      (ElectronTruthPartAssoc,
       D3PD.FirstAssociationTool,
       # TruthD3PDMaker
       Associator = D3PD.TruthParticleParentAssociationTool
         ('ElectronTruthPartMotherAssoc2'),
       blockname = 'ElectronTruthPartMotherAssoc',
       prefix = 'mother')
    ElectronTruthPartMotherAssoc.defineBlock (0, 'MotherTruth',
                                              # TruthD3PDMaker
                                              D3PD.TruthParticleFillerTool,
                                              PDGIDVariable = 'type')
    ElectronTruthPartAssoc.defineBlock (0, 'TruthBrem',
                                        # TruthD3PDMaker
                                        D3PD.TruthParticleBremFillerTool)
    ElectronTruthPartAssoc.defineBlock (0, 'TruthAssocIndex',
                                        D3PD.IndexFillerTool,
                                        Target = 'mc_')


ElectronD3PDObject.defineBlock (
    1, 'HadLeakage',
    D3PD.AuxDataFillerTool,
    Vars = ['Ethad = ethad < float: 0',
            'Ethad1 = ethad1 < float: 0'])
ElectronD3PDObject.defineBlock (
    1, 'Layer1Shape',
    D3PD.AuxDataFillerTool,
    Vars = ['f1',
            'f1core',
            'Emins1 = emins1 < float: 0',
            'fside = fracs1',
            'Emax2 = e2tsts1 < float: 0',
            'ws3 = weta1',
            'wstot = wtots1',
            'emaxs1 < float: 0'])
ElectronD3PDObject.defineBlock (1, 'Layer1ShapeExtra',
                                D3PD.egammaLayer1ExtraFillerTool)
ElectronD3PDObject.defineBlock (
    1, 'Layer2Shape',
    D3PD.AuxDataFillerTool,
    Vars = ['E233 = e233 < float: 0',
            'E237 = e237 < float: 0',
            'E277 = e277 < float: 0',
            'weta2'])
ElectronD3PDObject.defineBlock (
    1, 'Layer3Shape',
    D3PD.AuxDataFillerTool,
    Vars = ['f3', 'f3core'])
ElectronD3PDObject.defineBlock (
    1, 'Iso',
    D3PD.AuxDataFillerTool,
    Vars = ['rphiallcalo = r33over37allcalo < float: 0',
            'Etcone20 = etcone20 < float: 0',
            'Etcone30 = etcone30 < float: 0',
            'Etcone40 = etcone40 < float: 0',
            'ptcone20 < float: 0',
            'ptcone30 < float: 0',
            'ptcone40 < float: 0',
        ])
ElectronD3PDObject.defineBlock (
    2, 'IsoPtCorrected',
    D3PD.AuxDataFillerTool,
    Vars = ['Etcone20_pt_corrected = etcone20_ptcorrected  < float: 0 #pt-corrected isolation energy within DR=0.20 cone',
            'Etcone30_pt_corrected = etcone30_ptcorrected  < float: 0 #pt-corrected isolation energy within DR=0.30 cone',
            'Etcone40_pt_corrected = etcone40_ptcorrected  < float: 0 #pt-corrected isolation energy within DR=0.40 cone',
            ])
ElectronD3PDObject.defineBlock (
    1, 'TrkMatch',
    D3PD.AuxDataFillerTool,
    Vars = ['pos7 < float: 0',
            'deltaeta1 = deltaEta1',
            'deltaeta2 = deltaEta2',
            'deltaphi2 = deltaPhi2',
            'deltaphiRescaled = deltaPhiRescaled2',
            ])


ElectronD3PDObject.defineBlock (
    1, 'TopoClusterIsolationCones',
    D3PD.AuxDataFillerTool,
    Vars = ['topoEtcone20 = topoetcone20 < float: 0',
            'topoEtcone30 = topoetcone30 < float: 0',
            'topoEtcone40 = topoetcone40 < float: 0',
            ])

traversedMaterial = \
  ElectronD3PDObject.defineBlock (2, 'TraversedMaterial',
                                  D3PD.egammaTraversedMaterialFillerTool )
def _traversedMaterialExtrapolatorHook (c, flags, acc, *args, **kw):
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    c.Extrapolator = acc.popToolsAndMerge (AtlasExtrapolatorCfg (flags))
    return
traversedMaterial.defineHook (_traversedMaterialExtrapolatorHook)
    
ElectronD3PDObject.defineBlock (
    2, 'PointingShape',
    D3PD.AuxDataFillerTool,
    Vars = ['zvertex < float: 0',
            'errz < float: 0',
            'etap < float: 0',
            'depth < float: 0'])


from egammaD3PDMaker.egammaCluster import egammaCluster
ElectronClusterAssoc = egammaCluster (ElectronD3PDObject,
                                      allSamplings = True,
                                      fwdEVars = True)


############################################################################
# TrackParticle variables
#


ElectronTPAssoc = SimpleAssociation \
    (ElectronD3PDObject,
     D3PD.ElectronTrackParticleAssociationTool,
     matched = 'hastrack',
     blockname = 'TrkInfo')

TrackParticlePerigeeAssoc = SimpleAssociation \
    (ElectronTPAssoc,
     D3PD.TrackParticlePerigeeAtOOAssociationTool, # TrackD3PDMaker
     prefix = 'track')

TrackParticlePerigeeAssoc.defineBlock (1, 'Trk',
                                       D3PD.PerigeeFillerTool, # TrackD3PDMaker
                                       FillMomentum = True)
TrackParticleCovarAssoc = SimpleAssociation (TrackParticlePerigeeAssoc,
                                             # TrackD3PDMaker
                                             D3PD.PerigeeCovarianceAssociationTool)
TrackParticleCovarAssoc.defineBlock (3, 'TrkCovDiag',
                                     # TrackD3PDMaker
                                     D3PD.CovarianceFillerTool,
                                     IsTrackPerigee = True,
                                     Error = False,
                                     DiagCovariance = True)
TrackParticleCovarAssoc.defineBlock (3, 'TrkCovOffDiag',
                                     # TrackD3PDMaker
                                     D3PD.CovarianceFillerTool,
                                     IsTrackPerigee = True,
                                     Error = False,
                                     OffDiagCovariance = True)


ElectronTPAssoc.defineBlock (
    2, 'TrkFitQuality',
    D3PD.AuxDataFillerTool,
    Vars = ['chi2 = chiSquared < float:0',
            'ndof = numberDoF < float:0'],
    prefix = 'trackfit')

xAODTrackSummaryFiller (ElectronTPAssoc, 1, 'IDHits',
                        IDSharedHits = True,
                        IDHits = True,
                        IDOutliers = True,
                        MuonHits = False,
                        HitSum = False,
                        HoleSum = False,
                        SCTInfoPlus = True,
                        PixelInfoPlus = True)

ElectronTPAssoc.defineBlock (
    1, 'TrackSummaryPID',
    D3PD.AuxDataFillerTool,
    Vars = ['eProbabilityComb < float:0',
            'eProbabilityHT < float:0',
            'eProbabilityToT < float:0',
            'eProbabilityBrem < float:0'])


ElectronVertAssoc = SimpleAssociation \
                    (ElectronTPAssoc,
                     D3PD.TrackParticleVertexAssociationTool, # TrackD3PDMaker
                     Fast = False,
                     prefix = 'vert')
ElectronVertAssoc.defineBlock (
    2, 'Vertex',
    D3PD.AuxDataFillerTool,
    Vars = ['x < float:0 ', 'y < float:0', 'z < float:0'])


from TrackD3PDMaker.TrackParticleImpactParameters \
     import TrackParticleImpactParameters
TrackParticleImpactParameters (ElectronTPAssoc)


###Original Unrefitted track

TPPerigeePairAssoc = SimpleAssociation \
    (ElectronTPAssoc,
     # TrackD3PDMaker
     D3PD.TrackParticlePerigeePairAtOOAssociationTool,
     AssocGetter = D3PD.SGObjGetterTool
     ('GSFAssocGetter',
      SGKey = D3PDMakerFlags.GSFTrackAssocSGKey,
      TypeName = 'TrackParticleAssocs'),
     blockname = 'TPPerigeePairAssoc')

UnRefittedTrackParticlePerigeeAssoc = SimpleAssociation \
    (TPPerigeePairAssoc,
     # TrackD3PDMaker
     D3PD.PerigeePairOldPerigeeAssociationTool,
     blockname = 'UnrefittedTrackPerigeeAssoc',
     prefix = 'Unrefittedtrack_')

UnRefittedTrackParticlePerigeeAssoc.defineBlock (1, 'UnrefitTrk',
                                                 # TrackD3PDMaker
                                                 D3PD.PerigeeFillerTool,
                                                 FillMomentum = True)


############################################################################
# From UserData
#

if D3PDMakerFlags.HaveEgammaUserData or D3PDMakerFlags.MakeEgammaUserData:
    # `target' arg needs to be passed in from the caller;
    # otherwise, we don't make this block.
    def _jetAssocLevel (reqlev, args):
        if reqlev < 2: return False
        if 'target' not in args: return False
        args['Target'] = args['target']
        return True

    EgammaJetDRAssoc = IndexMultiAssociation\
                       (ElectronD3PDObject,
                        D3PD.ElectronJetDRAssociator,
                        '', # Overridden by the level function.
                        blockname='EgammaJetSignedIPAndPTRel',
                        prefix = 'jetcone_',
                        level = _jetAssocLevel,
                        DRCut=0.7,
                        VertexContainerName = D3PDMakerFlags.VertexSGKey)
    def _trackToVertexHook (c, flags, acc, *args, **kw):
        from TrkConfig.TrkVertexFitterUtilsConfig import TrackToVertexIPEstimatorCfg
        c.Associator.TrackToVertexIPEstimator = acc.popToolsAndMerge (TrackToVertexIPEstimatorCfg (flags))
        return
    EgammaJetDRAssoc.defineHook (_trackToVertexHook)
    EgammaJetDRAssoc.defineBlock \
          (2,
           'EgammaJetSignedIPAndPTRelKin',
           D3PD.FourMomFillerTool,
           WriteE = True)


    EgammaTrackJetDRAssoc = IndexMultiAssociation\
                            (ElectronD3PDObject,
                             D3PD.ElectronJetDRAssociator,
                             '', # Overridden by the level function.
                             blockname='EgammaTrackJetSignedIPAndPTRel',
                             prefix = 'jettrack_',
                             level = _jetAssocLevel,
                             DRCut=0.7,
                             VertexContainerName = D3PDMakerFlags.VertexSGKey)
    EgammaTrackJetDRAssoc.defineHook (_trackToVertexHook)
    EgammaTrackJetDRAssoc.defineBlock \
          (2,
           'EgammaTrackJetSignedIPAndPTRelKin',
           D3PD.FourMomFillerTool,
           WriteE = True)


   

############################################################################
# Jet associations
#

EleJetD3PDAssoc = DRAssociation (ElectronD3PDObject,
                                 'DataVector<xAOD::Jet_v1>',
                                 D3PDMakerFlags.JetSGKey,
                                 0.2,
                                 'jet_',
                                 level = 2,
                                 blockname = 'JetMatch')
EleJetD3PDAssoc.defineBlock (2, 'JetKinematics',
                             D3PD.FourMomFillerTool,
                             WriteE = True)


if D3PDMakerFlags.DoTruth and testSGKey (configFlags, D3PDMakerFlags.TruthJetSGKey):
    JetTruthJetD3PDAssoc = DRAssociation (EleJetD3PDAssoc,
                                          'DataVector<xAOD::Jet_v1>',
                                          D3PDMakerFlags.TruthJetSGKey,
                                          0.2,
                                          'truth_',
                                          level = 2,
                                          blockname = 'TrueJetMatch')
    JetTruthJetD3PDAssoc.defineBlock (2, 'TrueJetKinematics',
                                      D3PD.FourMomFillerTool,
                                      WriteE = True)
