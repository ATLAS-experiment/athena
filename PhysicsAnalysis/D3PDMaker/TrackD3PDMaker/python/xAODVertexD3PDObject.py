# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.flagTestLOD import flagTestLOD, deferFlag
from D3PDMakerCoreComps.D3PDObject import make_SGDataVector_D3PDObject
from D3PDMakerCoreComps.SimpleAssociation import SimpleAssociation
from D3PDMakerCoreComps.ContainedVectorMultiAssociation import ContainedVectorMultiAssociation
from D3PDMakerConfig.D3PDMakerFlags import D3PDMakerFlags
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD


def DefinexAODVertexD3PDObject(object,
                               trackTarget='trk',
                               trackPrefix='trk_',
                               flags = D3PDMakerFlags.Track):

    # Position
    object.defineBlock (flagTestLOD('vertexPositionLevelOfDetails>=1', flags),
                        'Position',
                        D3PD.AuxDataFillerTool,
                        Vars = ['x', 'y', 'z'])

    # Covariance Matrix
    CovarianceAssoc = SimpleAssociation \
                      (object,
                       D3PD.VertexCovarianceAssociationTool,
                       level = flagTestLOD('vertexPositionLevelOfDetails>=2', flags))


    CovarianceAssoc.defineBlock (
        flagTestLOD('vertexPositionLevelOfDetails>=2', flags),
        'Error',
        D3PD.CovarianceFillerTool,
        Error = deferFlag ('storeDiagonalCovarianceAsErrors', flags),
        DiagCovariance = deferFlag('not storeDiagonalCovarianceAsErrors',flags),
        OffDiagCovariance = False,
        IsPosition = True)

    CovarianceAssoc.defineBlock (flagTestLOD('vertexPositionLevelOfDetails>=3', flags),
                                 'Covariance',
                                 D3PD.CovarianceFillerTool,
                                 Error = False,
                                 DiagCovariance = False,
                                 OffDiagCovariance = True,
                                 IsPosition = True)

    # Type
    object.defineBlock (flagTestLOD('storeVertexType', flags),
                        'Type',
                        D3PD.AuxDataFillerTool,
                        Vars = ['type = vertexType'])
    
    # Fit Quality
    object.defineBlock (flagTestLOD('storeVertexFitQuality', flags),
                        'FitQuality',
                        D3PD.AuxDataFillerTool,
                        Vars = ['chi2 = chiSquared', 'ndof = numberDoF'])

    # Kine
    object.defineBlock (flagTestLOD('storeVertexKinematics', flags),
                        'Kine',
                        D3PD.VertexKineFillerTool)

    # Track Association
    TrackAssoc = ContainedVectorMultiAssociation \
                 (object,
                  D3PD.VertexTrackParticleAssociationTool,
                  trackPrefix,
                  level = flagTestLOD ('storeVertexTrackAssociation or storeVertexTrackIndexAssociation',
                                       flags))

    TrackAssoc.defineBlock (
        flagTestLOD ('storeVertexTrackAssociation or storeVertexTrackIndexAssociation', flags),
        'TrkFitQuality',
        D3PD.AuxDataFillerTool,
        Vars = ['chi2 = chiSquared'])

    PerigeeAssoc = SimpleAssociation \
                   (TrackAssoc,
                    D3PD.TrackParticlePerigeeAtPVAssociationTool)
    def _trackToVertexHook (c, flags, acc, *args, **kw):
        from TrackToVertex.TrackToVertexConfig import InDetTrackToVertexCfg
        c.Associator.TrackToVertexTool = acc.popToolsAndMerge (InDetTrackToVertexCfg (flags))
        return
    PerigeeAssoc.defineHook (_trackToVertexHook)
    PerigeeAssoc.defineBlock (flagTestLOD ('storeVertexTrackAssociation or storeVertexTrackIndexAssociation', flags),
                              'Trk',
                              D3PD.PerigeeFillerTool)

    TrackAssoc.defineBlock (
        flagTestLOD ('storeVertexTrackIndexAssociation', flags),
        'TrackAssocIndex',
        D3PD.IndexFillerTool,
        Target = trackTarget)
    return


def BuildxAODVertexD3PDObject(_prefix='vx_',
                              _label='vx',
                              _sgkey=D3PDMakerFlags.VertexSGKey,
                              _object_name='PrimaryVertexD3PDObject',
                              trackTarget='trk',
                              trackPrefix='trk_'):

    object = make_SGDataVector_D3PDObject (
        'DataVector<xAOD::Vertex_v1>',
        _sgkey,
        _prefix,
        _object_name,
        #default_allowMissing = True,
        allow_args = ['storeVertexTrackAssociation',
                      'storeVertexTrackIndexAssociation',
                      'storeDiagonalCovarianceAsErrors',
                      'storeVertexType',
                      'storeVertexFitQuality',
                      'storeVertexKinematics',
                      'storeVertexPurity',
                      'vertexPositionLevelOfDetails',
                      'doTruth'])

    DefinexAODVertexD3PDObject(object,
                               trackTarget,
                               trackPrefix)

    return object


#
# Additional arguments that can be passed to this D3PDObject:
#   storeVertexAssociation
#   storeVertexTrackIndexAssociation
#
PrimaryxAODVertexD3PDObject = BuildxAODVertexD3PDObject(
    _prefix='vx_',
    _label='vx',
    _sgkey='VxPrimaryCandidate',
    _object_name='PrimaryVertexD3PDObject',
    trackTarget='trk',
    trackPrefix='trk_')
