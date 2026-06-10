// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODCORECNV_ELEMENTLINKRESETALG_H
#define XAODCORECNV_ELEMENTLINKRESETALG_H

// System include(s):
#include <vector>
#include <string>

// Gaudi/Athena include(s):
#include "AthenaBaseComps/AthAlgorithm.h"
#include "CxxUtils/checker_macros.h"

// Forward declaration(s):
namespace SG {
   class IConstAuxStore;
}

namespace xAODMaker {

   /**
    *  @short Algorithm reseting ElementLink objects to their default state
    *
    *         This algorithm can be used to make sure that during/after an
    *         AODFix-like configuration EDM objects are not left with cached
    *         pointers to no longer existing objects.
    *
    *         The algorithm can make sure that every ElementLink in some
    *         selected container(s), or all containers that are in StoreGate,
    *         are cleared of all cached information.
    *
    * @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
    *
    */
   class ATLAS_NOT_THREAD_SAFE ElementLinkResetAlg : public AthAlgorithm {

   public:
      /// Regular Algorithm constructor
      ElementLinkResetAlg( const std::string& name, ISvcLocator* svcLoc );

      /// Function initialising the algorithm
      virtual StatusCode initialize();
      /// Function executing the algorithm
      virtual StatusCode execute(const EventContext& ctx);

   private:
      /// Function reseting all the ElementLinks in one specific container
      StatusCode reset( const SG::IConstAuxStore& store,
                        const std::string& key );

      /// StoreGate keys of the auxiliary objects to be processed
      std::vector< std::string > m_keys;

      /// Helper class for caching auxiliary ID types
      class AuxIDType {
      public:
         AuxIDType();
         bool isSet; ///< Flag for whether this type was already set up
         bool isEL; ///< True if the type is an ElementLink
         bool isELVec; ///< True of the type is an ElementLink vector
      };

      /// Cached types of the auxiliary IDs
      std::vector< AuxIDType > m_typeCache;

   }; // class ElementLinkResetAlg

} // namespace xAODMaker

#endif // XAODCORECNV_ELEMENTLINKRESETALG_H
