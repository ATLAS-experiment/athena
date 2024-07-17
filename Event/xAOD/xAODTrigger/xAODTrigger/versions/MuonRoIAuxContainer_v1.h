// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGGER_VERSIONS_MUONROIAUXCONTAINER_V1_H
#define XAODTRIGGER_VERSIONS_MUONROIAUXCONTAINER_V1_H

// System include(s):
extern "C" {
#   include <stdint.h>
}
#include <vector>
#include <string>

// EDM include(s):
#include "xAODCore/AuxContainerBase.h"

namespace xAOD{

   /// Auxiliary store for the LVL1 muon RoI container
   ///
   /// This auxiliary container can describe the properties of a container
   /// of LVL1 muon RoIs.
   ///
   /// @author Lukas Heinrich <Lukas.Heinrich@cern.ch>
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class MuonRoIAuxContainer_v1 : public AuxContainerBase{

   public:
      /// Default constuctor
      MuonRoIAuxContainer_v1();

   private:
      std::vector< float > eta;
      std::vector< float > phi;
      std::vector< uint32_t > roiWord;
      std::vector< float > thrValue;
      std::vector< std::string > thrName;
      std::vector< uint32_t > roiExtraWord;

   }; // class MuonRoIAuxContainer_v1

} // namespace xAOD

// Declare the inheritance of the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::MuonRoIAuxContainer_v1, xAOD::AuxContainerBase );

#endif // XAODTRIGGER_VERSIONS_MUONROIAUXCONTAINER_V1_H
