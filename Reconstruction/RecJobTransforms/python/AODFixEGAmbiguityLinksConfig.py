# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from RecJobTransforms.AODFixHelper import releaseInRange


def AODFixEGAmbiguityLinksCfg(flags):

   #first check if we need to apply this AODFix
   if not releaseInRange(flags,"Athena-24.0.0","Athena-24.0.83"): 
      return None

   result=ComponentAccumulator()

   #Use the AddressRemappingSvc to rename the input object
   from SGComps.AddressRemappingConfig import InputRenameCfg
   result.merge(InputRenameCfg("xAOD::ElectronContainer", "Electrons", "old_Electrons"))
   result.merge(InputRenameCfg("xAOD::PhotonContainer", "Photons", "old_Photons"))
   result.merge(InputRenameCfg("xAOD::ElectronAuxContainer", "ElectronsAux.", "old_ElectronsAux."))
   result.merge(InputRenameCfg("xAOD::PhotonAuxContainer", "PhotonsAux.", "old_PhotonsAux."))

   #Re-run the required algorithms
   result.addEventAlgo(CompFactory.egammaAmbiguityRelinker(ElectronInputName="old_Electrons", 
                                                           ElectronOutputName="Electrons",
                                                           PhotonInputName="old_Photons", 
                                                           PhotonOutputName="Photons"))
                                                     
   return result


   
