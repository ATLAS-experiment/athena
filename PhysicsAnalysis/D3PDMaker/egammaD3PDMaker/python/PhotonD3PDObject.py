# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#
# @file egammaD3PDMaker/python/PhotonD3PDObject.py
# @author Haifeng Li <Haifeng.Li@cern.ch>, scott snyder <snyder@bnl.gov>
# @date 13 Aug. 2009
# @brief Configure photon D3PD object.
#

from egammaD3PDMaker.defineBlockAndAlg       import defineBlockAndAlg
from EventCommonD3PDMaker.DRAssociation      import DRAssociation
from D3PDMakerCoreComps.D3PDObject           import make_SGDataVector_D3PDObject
from D3PDMakerCoreComps.D3PDObject           import DeferArg
from D3PDMakerCoreComps.SimpleAssociation    import SimpleAssociation
from D3PDMakerConfig.D3PDMakerFlags          import D3PDMakerFlags
from D3PDMakerConfig.D3PDMakerFlags          import configFlags # noqa: F401
from D3PDMakerCoreComps.resolveSGKey         import testSGKey
from D3PDMakerCoreComps.resolveSGKey         import resolveSGKey # noqa: F401
from TrackD3PDMaker.xAODTrackSummaryFiller   import xAODTrackSummaryFiller
from D3PDMakerCoreComps.ContainedVectorMultiAssociation import ContainedVectorMultiAssociation
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD



PhotonD3PDObject = \
           make_SGDataVector_D3PDObject ('xAOD::PhotonContainer',
                                         D3PDMakerFlags.PhotonSGKey,
                                         'ph_', 'PhotonD3PDObject')

# AuxPrefix args need to be deferred in order to add in the sgkey.
auxprefix = DeferArg ('D3PDMakerFlags.EgammaUserDataPrefix + "_" +'
                     'resolveSGKey (configFlags, sgkey) + "_"',
                     globals())


PhotonD3PDObject.defineBlock (0, 'Kinematics',
                              D3PD.FourMomFillerTool,
                              WriteE  = True,
                              WriteEt = True,
                              WriteRect = True)
PhotonD3PDObject.defineBlock (
    0, 'Author',
    D3PD.AuxDataFillerTool,
    Vars = ['author'])
PhotonD3PDObject.defineBlock (
    0, 'Pass',
    D3PD.AuxDataFillerTool,
    Vars = ['loose = Loose',
            'tight = Tight'
            ])
PhotonD3PDObject.defineBlock (
    0, 'OQ',
    D3PD.AuxDataFillerTool,
    Vars = ['OQ'])

PhotonD3PDObject.defineBlock (0, 'Conversion0',
                              D3PD.egammaConversion0FillerTool,
                              )

defineBlockAndAlg \
    (PhotonD3PDObject,
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
    (PhotonD3PDObject,
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
    (PhotonD3PDObject,
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
        PhotonD3PDObject.defineBlock (1, 'TruthClassification',
                                      D3PD.egammaTruthClassificationFillerTool,
                                      Classifier = D3PD.D3PDMCTruthClassifier())
    def _truthClassificationHook (c, flags, acc, *args, **kw):
        from TruthD3PDMaker.MCTruthClassifierConfig \
            import D3PDMCTruthClassifierCfg
        acc.merge (D3PDMCTruthClassifierCfg (flags))
        c.Classifier = acc.getPublicTool ('D3PDMCTruthClassifier')
        return
    truthClassification.defineHook (_truthClassificationHook)
    PhotonTruthPartAssoc = SimpleAssociation \
        (PhotonD3PDObject,
         D3PD.egammaGenParticleAssociationTool,
         prefix = 'truth_',
         matched = 'matched',
         blockname = 'TruthAssoc',
         DRVar = 'deltaRRecPhoton')
    def _truthClassificationAssocHook (c, flags, acc, *args, **kw):
        from TruthD3PDMaker.MCTruthClassifierConfig \
            import D3PDMCTruthClassifierCfg
        acc.merge (D3PDMCTruthClassifierCfg (flags))
        c.Associator.Classifier = acc.getPublicTool ('D3PDMCTruthClassifier')
        return
    PhotonTruthPartAssoc.defineHook (_truthClassificationAssocHook)
    PhotonTruthPartAssoc.defineBlock (0, 'TruthKin',
                                      D3PD.FourMomFillerTool,
                                      WriteE = True,
                                      WriteM = False)
    PhotonTruthPartAssoc.defineBlock (0, 'Truth',
                                      # TruthD3PDMaker
                                      D3PD.TruthParticleFillerTool,
                                      PDGIDVariable = 'type')
    PhotonTruthPartMotherAssoc = SimpleAssociation \
      (PhotonTruthPartAssoc,
       D3PD.FirstAssociationTool,
       # TruthD3PDMaker
       Associator = D3PD.TruthParticleParentAssociationTool
         ('PhotonTruthPartMotherAssoc2'),
       blockname = 'PhotonTruthPartMotherAssoc',
       prefix = 'mother')
    PhotonTruthPartMotherAssoc.defineBlock (0, 'MotherTruth',
                                            # TruthD3PDMaker
                                            D3PD.TruthParticleFillerTool,
                                            PDGIDVariable = 'type')
    PhotonTruthPartAssoc.defineBlock (0, 'TruthAssocIndex',
                                      D3PD.IndexFillerTool,
                                      Target = 'mc_')



PhotonD3PDObject.defineBlock (
    1, 'HadLeakage',
    D3PD.AuxDataFillerTool,
    Vars = ['Ethad = ethad',
            'Ethad1 = ethad1'])
PhotonD3PDObject.defineBlock (
    1, 'Layer0Shape',
    D3PD.AuxDataFillerTool,
    Vars = ['E033 = e033 < float : 0'])
PhotonD3PDObject.defineBlock (
    1, 'Layer1Shape',
    D3PD.AuxDataFillerTool,
    Vars = ['f1',
            'f1core',
            'Emins1 = emins1',
            'fside = fracs1',
            'Emax2 = e2tsts1',
            'ws3 = weta1',
            'wstot = wtots1',
            'E132 = e132 < float : 0',
            'E1152 = e1152 < float : 0',
            'emaxs1'])
PhotonD3PDObject.defineBlock (1, 'Layer1ShapeExtra',
                              D3PD.egammaLayer1ExtraFillerTool)
PhotonD3PDObject.defineBlock (
    1, 'Layer2Shape',
    D3PD.AuxDataFillerTool,
    Vars = ['E233 = e233',
            'E237 = e237',
            'E277 = e277',
            'weta2'])
PhotonD3PDObject.defineBlock (
    1, 'Layer3Shape',
    D3PD.AuxDataFillerTool,
    Vars = ['f3', 'f3core'])
PhotonD3PDObject.defineBlock (
    1, 'Iso',
    D3PD.AuxDataFillerTool,
    Vars = ['rphiallcalo = r33over37allcalo',
            'Etcone20 = etcone20 < float: 0',
            'Etcone30 = etcone30 < float: 0',
            'Etcone40 = etcone40 < float: 0',
            'ptcone20 < float: 0',
            'ptcone30 < float: 0',
            'ptcone40 < float: 0',
        ])
PhotonD3PDObject.defineBlock (
    2, 'IsoPtCorrected',
    D3PD.AuxDataFillerTool,
    Vars = ['Etcone20_pt_corrected = etcone20_ptcorrected < float: 0 #pt-corrected isolation energy within DR=0.20 cone',
            'Etcone30_pt_corrected = etcone30_ptcorrected < float: 0 #pt-corrected isolation energy within DR=0.30 cone',
            'Etcone40_pt_corrected = etcone40_ptcorrected < float: 0 #pt-corrected isolation energy within DR=0.40 cone',
            ])
PhotonD3PDObject.defineBlock (1, 'Conversion',
                              D3PD.egammaConversionFillerTool)
PhotonD3PDObject.defineBlock (1, 'Retaphi',
                              D3PD.egammaRetaphiFillerTool)
                                           


PhotonD3PDObject.defineBlock (
    1, 'TopoClusterIsolationCones',
    D3PD.AuxDataFillerTool,
    Vars = ['topoEtcone20 = topoetcone20_core57cells,topoetcone20',
            'topoEtcone30 = topoetcone30_core57cells,topoetcone30',
            'topoEtcone40 = topoetcone40_core57cells,topoetcone40',
            ])

traversedMaterial = \
    PhotonD3PDObject.defineBlock (2, 'TraversedMaterial',
                                  D3PD.egammaTraversedMaterialFillerTool )
def _traversedMaterialExtrapolatorHook (c, flags, acc, prefix, *args, **kw):
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    c.Extrapolator = acc.popToolsAndMerge (AtlasExtrapolatorCfg (flags))
    return
traversedMaterial.defineHook (_traversedMaterialExtrapolatorHook)


PhotonD3PDObject.defineBlock (
    2, 'Pointing',
    D3PD.AuxDataFillerTool,
    Vars = ['zvertex < float:0 ',
            'errz < float:0 ',
            'etap < float: 0',
            'depth < float: 0'])

from egammaD3PDMaker.egammaCluster import egammaCluster
PhotonClusterAssoc = egammaCluster (PhotonD3PDObject)


PhotonD3PDObject.defineBlock (
    1, 'ConvDeltaEtaPhi',
    D3PD.AuxDataFillerTool,
    Vars = ['convMatchDeltaEta1',
            'convMatchDeltaEta2',
            'convMatchDeltaPhi1',
            'convMatchDeltaPhi2',
        ])


#----------------------------------------------
# This part gets all conversion vertices
# associated with the photon.  So it makes vector<vector<vector<T>>>'s, again with the
# index pointing towards the trk_ block.
#

# Associate the photon with secondary vertices
ConversionVertexAssociation = ContainedVectorMultiAssociation (
    PhotonD3PDObject,
    D3PD.ConversionVertexAssociationTool,
    prefix='vx_')
ConversionVertexAssociation.defineBlock (
    10, 'ConversionVertexPosition',
    D3PD.AuxDataFillerTool,
    Vars = ['x < float:0 ', 'y < float:0', 'z < float:0'])
ConversionVertexAssociation.defineBlock (
    10, 'ConversionVertexKinematics',
    # TrackD3PDMaker
    D3PD.VertexKineFillerTool)


ConversionVertexTrackParticleAssociation = \
   ContainedVectorMultiAssociation (ConversionVertexAssociation,
                                    # TrackD3PDMaker
                                    D3PD.VertexTrackParticleAssociationTool,
                                    prefix = 'convTrk_')


# `target' arg needs to be passed in from the caller;
# otherwise, we don't make this block.
def _cvAssocLevel (reqlev, args):
    if reqlev < 10: return False
    if 'target' not in args: return False
    args['Target'] = args['target']
    del args['target']
    return True
ConversionVertexTrackParticleAssociation.defineBlock (
    _cvAssocLevel, "ConversionVertexTrackIndex",
    D3PD.IndexFillerTool,
    # Target will be filled in by level function.
    Target = '')

ConversionVertexTrackParticleAssociation.defineBlock (
    10, 'ConversionVertexTrackInfo',
    D3PD.AuxDataFillerTool,
    Vars = ['fitter = trackFitter < unsigned char: 0',
            'patternReco = patternRecoInfo < unsigned long: 0',
            'trackProperties < unsigned char: 0',
            'particleHypothesis < unsigned char: 0',
            ])
     
xAODTrackSummaryFiller (ConversionVertexTrackParticleAssociation,
                        10, 'CoversionTrackSummary',
                        IDHits = True,
                        IDHoles = True,
                        IDSharedHits = True,
                        IDOutliers = True,
                        PixelInfoPlus = True,
                        SCTInfoPlus = True,
                        TRTInfoPlus = True,
                        InfoPlus = True,
                        MuonHits = False,
                        MuonHoles = False,
                        ExpectBLayer = True,
                        HitSum = False,
                        HoleSum = False,
                        )
ConversionVertexTrackParticleAssociation.defineBlock (
    10, 'ConversionTrackSummaryPID',
    D3PD.AuxDataFillerTool,
    Vars = ['eProbabilityComb',
            'eProbabilityHT',
            'eProbabilityToT < float:0',
            'eProbabilityBrem < float:0'])
ConversionVertexTrackParticleAssociation.defineBlock (
    10, 'ConversionTrackFitQuality',
    D3PD.AuxDataFillerTool,
    Vars = ['chi2 = chiSquared',
            'ndof = numberDoF'])
    


############################################################################
# From UserData
#

if D3PDMakerFlags.HaveEgammaUserData or D3PDMakerFlags.MakeEgammaUserData:
    if D3PDMakerFlags.DoTruth:
        defineBlockAndAlg \
          (PhotonD3PDObject,
           0, 'UDTruthFlags',
           D3PD.AuxDataFillerTool,
           'PhotonTruth',
           AuxPrefix = auxprefix,
           Vars = ['truth_isConv',               
                   'truth_isBrem',               
                   'truth_isFromHardProc',       
                   'truth_isPhotonFromHardProc', 
                   ])

        defineBlockAndAlg \
          (PhotonD3PDObject,
           1, 'UDTruthConv',
           D3PD.AuxDataFillerTool,
           'PhotonTruth',
           AuxPrefix = auxprefix,
           Vars = ['truth_Rconv', 
                   'truth_zconv', 
                   ])

    defineBlockAndAlg (PhotonD3PDObject, 
                       1, 'UDLayer1Shape',
                       D3PD.AuxDataFillerTool,
                       'egammaDeltaEmax2',
                       AuxPrefix = auxprefix,
                       Vars = ['deltaEmax2',
                               ])

        

############################################################################
# Jet associations
#

PhotonJetD3PDAssoc = DRAssociation (PhotonD3PDObject,
                                    'DataVector<xAOD::Jet_v1>',
                                    D3PDMakerFlags.JetSGKey,
                                    0.2,
                                    'jet_',
                                    level = 2,
                                    blockname = 'JetMatch')
PhotonJetD3PDAssoc.defineBlock (2, 'JetKinematics',
                                D3PD.FourMomFillerTool,
                                WriteE = True)


if D3PDMakerFlags.DoTruth and testSGKey (configFlags, D3PDMakerFlags.TruthJetSGKey):
    JetTruthJetD3PDAssoc = DRAssociation (PhotonJetD3PDAssoc,
                                          'DataVector<xAOD::Jet_v1>',
                                          D3PDMakerFlags.TruthJetSGKey,
                                          0.2,
                                          'truth_',
                                          level = 2,
                                          blockname = 'TrueJetMatch')
    JetTruthJetD3PDAssoc.defineBlock (2, 'TrueJetKinematics',
                                      D3PD.FourMomFillerTool,
                                      WriteE = True)




############################################################################
# Topo cluster associations.
#


PhotonTopoD3PDAssoc = DRAssociation (PhotonD3PDObject,
                                     'DataVector<xAOD::CaloCluster_v1>',
                                     D3PDMakerFlags.ClusterSGKey,
                                     0.1,
                                     'topo',
                                     level = 2,
                                     blockname = 'TopoMatch')
PhotonTopoD3PDAssoc.defineBlock (2, 'TopoKinematics',
                                 D3PD.FourMomFillerTool,
                                 WriteM = False)
