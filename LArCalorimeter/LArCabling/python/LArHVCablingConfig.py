# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from IOVDbSvc.IOVDbSvcConfig import addFolders

def LArHVCablingCfg(flags):
    result=ComponentAccumulator()
    if flags.Input.isMC:
        result.merge(addFolders(flags,["/LAR/Identifier/HVLineToElectrodeMap<tag>LARHVLineToElectrodeMap-001</tag>"], "LAR_OFL",
                                className="AthenaAttributeList"))
        
        larHVIdMappingAlg=CompFactory.LArHVIdMappingAlg(ReadKey="/LAR/Identifier/HVLineToElectrodeMap",WriteKey="LArHVIdMap")
        result.addCondAlgo(larHVIdMappingAlg)

    else:
        #Data case
        result.merge(addFolders(flags,["/LAR/IdentifierOfl/HVLineToElectrodeMap"], "LAR_OFL", className="AthenaAttributeList"))
        hvmapalg = CompFactory.LArHVIdMappingAlg(ReadKey="/LAR/IdentifierOfl/HVLineToElectrodeMap",WriteKey="LArHVIdMap")
        result.addCondAlgo(hvmapalg)

    return result
