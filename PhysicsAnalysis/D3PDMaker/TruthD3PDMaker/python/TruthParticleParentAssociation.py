# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
# @file TruthD3PDMaker/python/TruthParticleParentAssociation.py
# @author Ryan Reece <ryan.reece@cern.ch>
# @date Mar, 2010
# @brief Helper for setting up an association for truth particle 
#   parents by index.
#


from D3PDMakerCoreComps.IndexMultiAssociation import IndexMultiAssociation
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD


def TruthParticleParentAssociation (parent,
                   prefix = '',
                   target = '',
                   level = 0,
                   blockname = None,
                   *args, **kw):
    """Helper for setting up an association for truth particle
parents by index.
"""
    if blockname is None:
        blockname = prefix + 'TruthParticleParentAssociation'

    return IndexMultiAssociation (parent,
                                  D3PD.TruthParticleParentAssociationTool,
                                  target,
                                  prefix,
                                  level,
                                  blockname,
                                  nrowName = '')
