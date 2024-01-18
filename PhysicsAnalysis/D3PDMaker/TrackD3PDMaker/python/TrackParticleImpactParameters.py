# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
# @file TrackD3PDMaker/python/TrackParticleImpactParameters.py
# @author scott snyder <snyder@bnl.gov>
# @date 2009
# @brief Add to a D3PD object blocks filling in impact parameter information.
#


from D3PDMakerCoreComps.SimpleAssociation   import SimpleAssociation
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD


def TrackParticleImpactParameters (TPD3PDObject,
                                   prefix = 'track',
                                   beam_suffix = 'beam',
                                   pv_suffix = 'pv',
                                   pvunbiased_suffix = 'pvunbiased'):
    """Add to a D3PD object blocks filling in impact parameter information.

TPD3PDObject should be a D3PD object for a TrackParticle
(could be an association).  This will add information for impact
parameters with respect to the beam spot and the primary vertex,
and the unbiased impact parameter with respect to the primary vertex.
The variable names are constructed using the prefix and *_suffix
arguments."""


    #
    # Beamspot
    #
    BSPerigeeAssoc = SimpleAssociation \
                     (TPD3PDObject,
                      D3PD.TrackParticlePerigeeAtBSAssociationTool,
                      blockname = prefix + 'BSPerigeeAssoc',
                      prefix = prefix)
    def _trackToVertexHook (c, flags, acc, prefix, *args, **kw):
        from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
        acc.merge(BeamSpotCondAlgCfg(flags))
        from TrackToVertex.TrackToVertexConfig import InDetTrackToVertexCfg
        c.Associator.TrackToVertexTool = acc.popToolsAndMerge (InDetTrackToVertexCfg (flags))
        return
    BSPerigeeAssoc.defineHook (_trackToVertexHook)
    BSPerigeeAssoc.defineBlock (1, prefix + 'Impact' + beam_suffix,
                                D3PD.PerigeeFillerTool,
                                FillThetaAndQoverP = False,
                                FillPhi = False,
                                Suffix = beam_suffix,
                                DefaultValue = -9999)
    BSCovarAssoc = SimpleAssociation \
                   (BSPerigeeAssoc,
                    D3PD.PerigeeCovarianceAssociationTool,
                    blockname = prefix + 'BSPerigeeCovarAssoc')
    BSCovarAssoc.defineBlock (1, prefix + 'ImpactSig' + beam_suffix,
                              D3PD.ImpactSigmaFillerTool,
                              Suffix = beam_suffix)

    #
    # Primary vertex
    #
    PVPerigeeAssoc = SimpleAssociation \
                     (TPD3PDObject,
                      D3PD.TrackParticlePerigeeAtPVAssociationTool,
                      blockname = prefix + 'PVPerigeeAssoc',
                      prefix = prefix)
    PVPerigeeAssoc.defineHook (_trackToVertexHook)
    PVPerigeeAssoc.defineBlock (1, prefix + 'Impact' + pv_suffix,
                                D3PD.PerigeeFillerTool,
                                FillThetaAndQoverP = False,
                                FillPhi = False,
                                Suffix = pv_suffix,
                                DefaultValue = -9999)
    PVCovarAssoc = SimpleAssociation \
                   (PVPerigeeAssoc,
                    D3PD.PerigeeCovarianceAssociationTool,
                    blockname = prefix + 'PVPerigeeCovarAssoc')
    PVCovarAssoc.defineBlock (1, prefix + 'ImpactSig' + pv_suffix,
                              D3PD.ImpactSigmaFillerTool,
                              Suffix = pv_suffix)

