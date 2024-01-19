# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# @file egammaD3PDAnalysis/python/egamaMaxECellAlgConfig.py
# @author Nikiforos K. Nikiforou <nikiforo@cern.ch> modified from egammaIsIsoConfig.py by scott snyder <snyder@bnl.gov>
# @date Jun, 2012
# @brief Configure egamaMaxECellAlg to fill aux data.
#


from D3PDMakerConfig.D3PDMakerFlags          import D3PDMakerFlags
from D3PDMakerCoreComps.resolveSGKey         import resolveSGKey
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

D3PD = CompFactory.D3PD


def egammaMaxECellAlgCfg \
    (flags,
     prefix = '',
     sgkey = D3PDMakerFlags.ElectronSGKey,
     typeName = 'ElectronContainer',
     allowMissing = False
     ):
    """Configure egammaMaxECellAlg for D3PD making.

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

    algName = 'egammaMaxECellAlg' + resolved_sgkey
        
    myAlg = D3PD.egammaMaxECellAlg \
        (algName,
         Getter = DVGetter
         (prefix + 'egammaMaxECellAlgGetter',
          TypeName = typeName,
          SGKey = sgkey),
         AllowMissing = allowMissing,
         AuxPrefix = auxprefix,)

    acc.addEventAlgo (myAlg)

    return acc

