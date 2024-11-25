# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# File: CaloRec/python/CaloBCIDAvgAlgConfig.py
# Created: Mar 2019, sss
# Purpose: Configure CaloBCIDAvgAlg.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep

def CaloBCIDAvgAlgCfg (flags, **kwargs):
    CaloBCIDAvgAlg=CompFactory.CaloBCIDAvgAlg

    result = ComponentAccumulator()

    from LArRecUtils.LArRecUtilsConfig import LArMCSymCondAlgCfg
    result.merge (LArMCSymCondAlgCfg (flags))

    from CaloRec.CaloBCIDLumiCondAlgConfig import CaloBCIDLumiCondAlgCfg
    result.merge (CaloBCIDLumiCondAlgCfg (flags))
    if flags.Common.ProductionStep is ProductionStep.PileUpPretracking:
         kwargs.setdefault("EventInfoKey", flags.Overlay.BkgPrefix + "EventInfo")
    result.addEventAlgo (CaloBCIDAvgAlg(**kwargs))
    return result



if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags

    only = ['CaloBCIDAvgAlg',
            'CaloBCIDCoeffsCondAlg',
            'CaloBCIDLumiCondAlg',
            'CondInputLoader',
            'LuminosityCondAlg-',
            'LArPileUpAvgSymCondAlg',
            'LArPileUpShapeSymCondAlg',
            'LArADC2MeVCondAlg-',
            'LArOFCCondAlg-',
            'LArAutoCorrTotalCondAlg-',
            'BunchCrossingCondAlg',
            ]
    print ('--- data')
    flags1 = initConfigFlags()
    flags1.Input.Files = []
    flags1.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags1.lock()
    acc1 = CaloBCIDAvgAlgCfg (flags1)
    acc1.printConfig(summariseProps=True, onlyComponents=only)
    acc1.wasMerged()

    print ('--- mc')
    flags2 = initConfigFlags()
    flags2.Input.Files = []
    flags2.Input.isMC = True
    flags2.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags2.lock()
    acc2 = CaloBCIDAvgAlgCfg (flags2)
    acc2.printConfig(summariseProps=True, onlyComponents=only)
    acc2.wasMerged()

    print ('--- online')
    flags3 = initConfigFlags()
    flags3.Input.Files = []
    flags3.Common.isOnline = True
    flags3.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags3.lock()
    acc3 = CaloBCIDAvgAlgCfg (flags3)
    acc3.printConfig(summariseProps=True, onlyComponents=only)
    acc3.wasMerged()
