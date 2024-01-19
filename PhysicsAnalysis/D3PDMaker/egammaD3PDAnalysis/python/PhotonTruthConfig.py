# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#
# @file egammaD3PDAnalysis/python/PhotonTruthConfig.py
# @author scott snyder <snyder@bnl.gov>
# @date Nov, 2011
# @brief Configure PhotonTruthAlg to fill UserData.
#


from D3PDMakerConfig.D3PDMakerFlags           import D3PDMakerFlags
from D3PDMakerCoreComps.resolveSGKey          import resolveSGKey
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory     import CompFactory

D3PD = CompFactory.D3PD


def PhotonTruthCfg \
        (flags,
         prefix = '',
         sgkey = D3PDMakerFlags.ElectronSGKey,
         typeName = 'ElectronContainer',
         allowMissing = False):
    """Configure PhotonTruthAlg for D3PD making.

    FLAGS are the configuration flags.

    PREFIX is a prefix to add to the name of the algorithm scheduled.

    SGKEY/TYPENAME is the StoreGate key of the input electron container
    and the name of its type.

    If ALLOWMISSING is true, don't fail if the SG key doesn't exist.
"""

    acc = ComponentAccumulator()

    if not D3PDMakerFlags.DoTruth:
        return acc

    if (not D3PDMakerFlags.MakeEgammaUserData or
        D3PDMakerFlags.HaveEgammaUserData):
        return acc

    DVGetter = D3PD.SGDataVectorGetterTool
    resolved_sgkey = resolveSGKey (flags, sgkey)
    auxprefix = (D3PDMakerFlags.EgammaUserDataPrefix + '_' +
                 resolved_sgkey + '_')

    from TruthD3PDMaker.MCTruthClassifierConfig \
        import D3PDMCTruthClassifierCfg
    acc.merge (D3PDMCTruthClassifierCfg (flags))
    ptaname = 'PhotonTruthAlg_' + resolved_sgkey
    acc.addEventAlgo (D3PD.PhotonTruthAlg \
                      (ptaname,
                       PhotonGetter = DVGetter
                       (prefix + 'PhotonTruthAlgGetter',
                        TypeName = typeName,
                        SGKey = sgkey),
                       AllowMissing = allowMissing,
                       AuxPrefix = auxprefix,
                       TruthTool = D3PD.PhotonTruthTool
                       ('D3PD__PhotonTruthTool',
                        Classifier = acc.getPublicTool ('D3PDMCTruthClassifier'))))

    return acc
