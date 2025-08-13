# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
# @file TruthD3PDMaker/python/TruthParticleD3PDObject.py
# @author scott snyder <snyder@bnl.gov>
# @date Dec, 2009
# @brief Define D3PD object for a collection of TruthParticle's.
#


from D3PDMakerCoreComps.D3PDObject        import make_SGDataVector_D3PDObject
from D3PDMakerCoreComps.SimpleAssociation import SimpleAssociation
from TruthD3PDMaker.TruthParticleChildAssociation     import TruthParticleChildAssociation
from TruthD3PDMaker.TruthParticleParentAssociation    import TruthParticleParentAssociation
from D3PDMakerConfig.D3PDMakerFlags         import D3PDMakerFlags
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD


TruthParticleD3PDObject = make_SGDataVector_D3PDObject \
  ('DataVector<xAOD::TruthParticle_v1>',
   D3PDMakerFlags.TruthParticlesSGKey,
   'mc_',
   'TruthParticleD3PDObject')

TruthParticleD3PDObject.defineBlock (0, 'TruthKin',
                                     D3PD.FourMomFillerTool)

TruthParticleD3PDObject.defineBlock (0, 'TruthInfo',
                                     D3PD.TruthParticleFillerTool)

ProdVertexAssoc = SimpleAssociation \
                  (TruthParticleD3PDObject,
                   D3PD.TruthParticleProdVertexAssociationTool,
                   level = 1,
                   prefix = 'vx_',
                   blockname = 'ProdVert')
ProdVertexAssoc.defineBlock (
    1, 'ProdVertPos',
    D3PD.AuxDataFillerTool,
    Vars = ['x', 'y', 'z', 'uid'])


ChildAssoc = TruthParticleChildAssociation(
                        parent = TruthParticleD3PDObject,
                        prefix = 'child_',
                        # target = '', # filled by hook
                        level = 0 )

def _TruthParticleAssocHook (c, flags, acc, *args, parent_prefix = None, **kw):
    indexer = c.BlockFillers[0]
    indexer.Target = parent_prefix
    return
ChildAssoc.defineHook(_TruthParticleAssocHook)

ParentAssoc = TruthParticleParentAssociation(
                        parent = TruthParticleD3PDObject,
                        prefix = 'parent_',
                        # target = '', # filled by hook
                        level = 0 )
ParentAssoc.defineHook(_TruthParticleAssocHook)

