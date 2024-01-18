# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# @file  MuonD3PDMaker/python/TruthMuonD3PDObject.py
# $author  Srivas Prasad <srivas.prasad@cern.ch>
# @date    March 2010
# @brief   dump true muons - modeled on Tau code


from D3PDMakerCoreComps.D3PDObject         import make_SGDataVector_D3PDObject
from AthenaConfiguration.ComponentFactory  import CompFactory

D3PD = CompFactory.D3PD


TruthMuonD3PDObject = make_SGDataVector_D3PDObject ('DataVector<xAOD::TruthParticle_v1>',
                                                    'D3PDTruthMuons',
                                                    'muonTruth_',
                                                    'TruthMuonD3PDObject')
def _truthMuonAlgHook (c, flags, acc, *args,
                       TruthContainer = 'TruthParticles',
                       sgkey = None,
                       prefix = None,
                       **kw):
    algname = prefix + 'TruthMuonsToSG'

    from TruthD3PDMaker.MCTruthClassifierConfig \
        import D3PDMCTruthClassifierCfg
    acc.merge (D3PDMCTruthClassifierCfg (flags))

    acc.addEventAlgo (D3PD.TruthMuonsToSG \
                      (algname,
                       TruthMuonContainer = sgkey,
                       TruthContainer = TruthContainer,
                       Classifier = acc.getPublicTool ('D3PDMCTruthClassifier')))
    return
TruthMuonD3PDObject.defineHook (_truthMuonAlgHook)


#-----------------------------------------------------------------------------
# Blocks
#-----------------------------------------------------------------------------
TruthMuonD3PDObject.defineBlock (0, 'Kinematics',
                                 D3PD.FourMomFillerTool,
                                 WriteEt = False,
                                 WritePt = True,
                                 WriteEtaPhi = True )
TruthMuonD3PDObject.defineBlock (0, 'Info',
                                 # TruthD3PDMaker
                                 D3PD.TruthParticleFillerTool,
                                 PDGIDVariable = 'PDGID')
truthMuon = \
    TruthMuonD3PDObject.defineBlock (0, 'Classification',
                                     # TruthD3PDMaker
                                     D3PD.TruthParticleClassificationFillerTool)
def _truthClassifierHook (c, flags, acc, *args, **kw):
    from TruthD3PDMaker.MCTruthClassifierConfig \
        import D3PDMCTruthClassifierCfg
    acc.merge (D3PDMCTruthClassifierCfg (flags))
    c.Classifier = acc.getPublicTool ('D3PDMCTruthClassifier')
    return
truthMuon.defineHook (_truthClassifierHook)

TruthMuonD3PDObject.defineBlock(99, "TruthHits",
                                D3PD.MuonTruthHitsFillerTool )

