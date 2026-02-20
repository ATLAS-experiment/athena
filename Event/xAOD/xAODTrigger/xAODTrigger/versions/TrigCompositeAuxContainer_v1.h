// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGGER_VERSIONS_TRIGCOMPOSITEAUXCONTAINER_V1_H
#define XAODTRIGGER_VERSIONS_TRIGCOMPOSITEAUXCONTAINER_V1_H

// System include(s):
extern "C" {
#   include <stdint.h>
}
#include <vector>
#include <string>

// Local include(s):
#include "xAODTrigger/versions/ByteStreamAuxContainer_v1.h"
#include "SGCore/sgkey_t.h"

namespace xAOD {

   /// Auxiliary store for TrigComposite_v1 containers
   ///
   /// This is an auxiliary container that can be streamed with ROOT without
   /// any special code necessary, handling all the variable types that we
   /// want to provide for the HLT algorithms.
   ///
   /// @author Tomasz Bold <Tomasz.Bold@cern.ch>
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class TrigCompositeAuxContainer_v1 : public ByteStreamAuxContainer_v1 {

   public:
      using sgkey_t = SG::sgkey_t;
      using index_type = uint32_t;

      /// Default constuctor
      TrigCompositeAuxContainer_v1();

   private:
      std::vector< std::string > name;

      std::vector< std::vector< std::string > > linkColNames;
      std::vector< std::vector< sgkey_t > >     linkColKeys;
      std::vector< std::vector< index_type > >  linkColIndices;
      std::vector< std::vector< uint32_t > >    linkColClids;

   }; // class TrigCompositeAuxContainer_v1

} // namespace xAOD

// Declare the inheritance of the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::TrigCompositeAuxContainer_v1, xAOD::ByteStreamAuxContainer_v1 );

#endif // XAODTRIGGER_VERSIONS_TRIGCOMPOSITEAUXCONTAINER_V1_H
