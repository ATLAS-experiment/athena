# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @file D3PDMakerConfig/python/egammaD3PDConfig.py
# @author scott snyder <snyder@bnl.gov>
# Date: Dec 2023, from old config
# @brief Construct an egamma D3PD.
#

from D3PDMakerConfig.D3PDMakerFlags           import D3PDMakerFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


# Make a container merging xAOD central and forward electrons.
import AthenaPython.PyAthena as PyAthena
from AthenaPython.PyAthena import StatusCode
class MergeElectrons (PyAthena.Alg):
    def __init__ (self, name = 'MergeElectrons', **kw):
        super (MergeElectrons, self).__init__ (name = name, **kw)
        return
    def execute (self):
        import ROOT
        sg=PyAthena.py_svc('StoreGateSvc')
        e1 = sg['Electrons']
        e2 = sg['ForwardElectrons']
        enew = ROOT.DataVector(ROOT.xAOD.Electron_v1) (1) #VIEW_ELEMENTS
        for e in e1: enew.push_back(e)
        for e in e2: enew.push_back(e)
        ROOT.SetOwnership (enew, False)
        sg.record (enew, 'AllElectrons')

        # Make sure these aux variables are defined at this point.
        ROOT.xAOD.ElectronAuxContainer()
        ROOT.xAOD.CaloClusterAuxContainer()
               
        return StatusCode.Success
        


def _args (level, name, kwin, **kw):
    kw = kw.copy()
    kw['level'] = level
    for (k, v) in kwin.items():
        if k.startswith (name + '_'):
            kw[k[len(name)+1:]] = v
    return kw


def egammaD3PDCfg (flags, stream = 'egamma', file = 'egamma.root',
                   level = 10,
                   clevel = 6,
                   autoflush = -1,
                   **kw):
    acc = ComponentAccumulator()

    acc.addEventAlgo (MergeElectrons())

    from D3PDMakerCoreComps.MakerAlgConfig import MakerAlgConfig
    alg = MakerAlgConfig (flags, acc, stream, file,
                          clevel = clevel,
                          autoflush = autoflush)

    from EventCommonD3PDMaker.EventInfoD3PDObject import EventInfoD3PDObject
    alg += EventInfoD3PDObject        (**_args (level, 'EventInfo', kw))

    from CaloD3PDMaker.LArCollisionTimeD3PDObject import LArCollisionTimeD3PDObject
    alg += LArCollisionTimeD3PDObject (**_args (level, 'LArCollisionTime', kw))

    from egammaD3PDMaker.ElectronD3PDObject       import ElectronD3PDObject
    alg += ElectronD3PDObject         (**_args (level, 'Electron', kw,
                                                EgammaJetSignedIPAndPTRel_target='jet_'
                                                ))

    from egammaD3PDMaker.PhotonD3PDObject         import PhotonD3PDObject
    alg += PhotonD3PDObject           (**_args (level, 'Photon', kw))

    from MuonD3PDMaker.MuonD3PDObject             import MuonD3PDObject
    alg += MuonD3PDObject             (**_args (10, 'Muons', kw,
                                                sgkey='StacoMuonCollection,Muons', prefix='mu_',
                                                include = ["EFCBInfoIndex", "EFMGInfoIndex",
                                                           "EFMEInfoIndex",
                                                           "L2CBInfoIndex", "L1InfoIndex",
                                                           "MuonScatteringAngleSignificance"],
                                                exclude = ["EFCBInfo", "EFMGInfo", "EFMEInfo",
                                                           "L2CBInfo", "L1Info"],
                                                allowMissing = True ))

    from JetD3PDMaker.JetD3PDObject import JetD3PDObject
    alg += JetD3PDObject              (**_args (0, 'Jet', kw,
                                                include=['JetQual',
                                                         'DQMoments']))

    from CaloD3PDMaker.MBTSD3PDObject import MBTSD3PDObject
    alg += MBTSD3PDObject             (**_args (level, 'MBTS', kw))

    from CaloD3PDMaker.MBTSTimeD3PDObject import MBTSTimeD3PDObject
    alg += MBTSTimeD3PDObject         (**_args (level, 'MBTSTime', kw))

    # May be missing in single-beam data.
    from TrackD3PDMaker.xAODVertexD3PDObject import PrimaryxAODVertexD3PDObject
    alg += PrimaryxAODVertexD3PDObject (**_args (
        1, 'PrimaryVertex', kw,
        #allowMissing = True,
        sgkey = D3PDMakerFlags.VertexSGKey,
        prefix = 'vxp_',
        storeVertexTrackIndexAssociation = False,
        storeVertexTrackAssociation = True,
        storeDiagonalCovarianceAsErrors = True))

    from TrackD3PDMaker.xAODTrackD3PDObject import xAODTrackParticleD3PDObject
    alg += xAODTrackParticleD3PDObject    (**_args (
        3, 'TrackParticleCandidate', kw,
        trackParametersAtGlobalPerigeeLevelOfDetails = 3,
        storeDiagonalCovarianceAsErrors = True))

    if D3PDMakerFlags.DoTruth:
        from MuonD3PDMaker.TruthMuonD3PDObject    import TruthMuonD3PDObject
        alg += TruthMuonD3PDObject    (**_args (level, 'TruthMuon', kw))

        from TruthD3PDMaker.TruthEventD3PDObject  import TruthEventD3PDObject
        alg += TruthEventD3PDObject     (**_args (1, 'TruthEvent', kw))

        from TruthD3PDMaker.TruthParticleD3PDObject  import TruthParticleD3PDObject
        alg += TruthParticleD3PDObject(**_args (level, 'TruthParticle', kw))

    acc.addEventAlgo (alg.alg)

    return acc
