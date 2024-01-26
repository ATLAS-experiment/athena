# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.D3PDObject      import make_SGDataVector_D3PDObject
from D3PDMakerConfig.D3PDMakerFlags     import D3PDMakerFlags
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD

TruthJetD3PDObject = make_SGDataVector_D3PDObject ('DataVector<xAOD::Jet_v1>',
                                                   D3PDMakerFlags.TruthJetSGKey,
                                                   'truthjet_',
                                                   'TruthJetD3PDObject')

TruthJetD3PDObject.defineBlock(0, 'Kinematics',
                               D3PD.FourMomFillerTool,
                               WriteE  = True)

