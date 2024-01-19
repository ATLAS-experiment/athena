# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# @file egammaD3PDAnalysis/python/egammaDeltaEmax2Config.py
# @author scott snyder <snyder@bnl.gov>
# @date Nov, 2011
# @brief Configure egammaDeltaEmax2Alg to fill UserData.
#


from D3PDMakerConfig.D3PDMakerFlags           import D3PDMakerFlags
from D3PDMakerCoreComps.resolveSGKey          import resolveSGKey
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory     import CompFactory

D3PD = CompFactory.D3PD


def egammaDeltaEmax2Cfg \
        (flags,
         prefix = '',
         sgkey = D3PDMakerFlags.ElectronSGKey,
         typeName = 'DataVector<xAOD::Electron_v1>',
         allowMissing = False):
    """Configure egammaDeltaEmax2Alg for D3PD making.

    FLAGS are the configuration flags.

    PREFIX is a prefix to add to the name of the algorithm scheduled.

    SGKEY/TYPENAME is the StoreGate key of the input electron container
    and the name of its type.

    If ALLOWMISSING is true, don't fail if the SG key doesn't exist.
"""

    acc = ComponentAccumulator()

    if (not D3PDMakerFlags.MakeEgammaUserData or
        D3PDMakerFlags.HaveEgammaUserData):
        return acc

    DVGetter = D3PD.SGDataVectorGetterTool
    resolved_sgkey = resolveSGKey (flags, sgkey)
    auxprefix = (D3PDMakerFlags.EgammaUserDataPrefix + '_' +
                 resolved_sgkey + '_')

    emax2name = 'DeltaEmax2Alg_' + resolved_sgkey
    highlum = False
    if typeName == 'ElectronContainer':
        if flags.Beam.numberOfCollisions >= 20 :
            highlum = True

    acc.addEventAlgo (D3PD.egammaDeltaEmax2Alg \
                      (emax2name,
                       Getter = DVGetter 
                       (prefix + 'DeltaEmax2Getter',
                        TypeName = typeName,
                        SGKey = sgkey),
                       AllowMissing = allowMissing,
                       HighLum = highlum,
                       AuxPrefix = auxprefix))

    return acc
