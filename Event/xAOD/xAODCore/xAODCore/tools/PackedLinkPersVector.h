// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file xAODCore/PackedLinkPersVector.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2024
 * @brief Specialization of @c AuxPersVector for @c PackedLink types.
 *
 * For @c PackedLink variables, the corresponding @c AuxPersVector class
 * should derive from @c PackedLinkVectorHolder in order to get the
 * proper implementations for the @c IAuxTypeVector methods.
 */


#ifndef XAODCORE_PACKEDLINKPERSVECTOR_H
#define XAODCORE_PACKEDLINKPERSVECTOR_H


#include "xAODCore/tools/AuxPersVector.h"
#include "AthContainers/PackedLink.h"


namespace xAOD {

  /**
   * @brief Specialization of @c AuxPersVector for @c PackedLink types.
   */
  template< class TARG, class VEC >
   class AuxPersVector<SG::PackedLink<TARG>, VEC>
    : public SG::PackedLinkVectorHolder<TARG, typename VEC::allocator_type> {

   public:
      /// Convenience type definition
      typedef VEC& vector_type;

      /// Constructor
      AuxPersVector( SG::auxid_t auxid, vector_type vec, [[maybe_unused]] bool isLinked, SG::IAuxStore* store )
        : SG::PackedLinkVectorHolder<TARG, typename VEC::allocator_type>
          (auxid, &vec, store->linkedVector( auxid ), false)
      {
         assert( !isLinked );
      }

      virtual std::unique_ptr<SG::IAuxTypeVector> clone() const override {
        return std::make_unique<AuxPersVector>(*this);
      }
   }; // class AuxPersVector


  /**
   * @brief Specialization of @c AuxPersVector for @c std::vector<PackedLink> types.
   */
  template< class TARG, class VALLOC, class VEC >
  class AuxPersVector<std::vector<SG::PackedLink<TARG>, VALLOC>, VEC> :
    public SG::PackedLinkVVectorHolder<TARG, VALLOC,
                                       std::vector<SG::PackedLink<TARG>, VALLOC>,
                                       typename VEC::allocator_type> {

   public:
      /// Convenience type definition
      typedef VEC& vector_type;

      /// Constructor
      AuxPersVector( SG::auxid_t auxid, vector_type vec, [[maybe_unused]] bool isLinked, SG::IAuxStore* store )
        : SG::PackedLinkVVectorHolder<TARG, VALLOC,
                                      std::vector<SG::PackedLink<TARG>, VALLOC>,
                                      typename VEC::allocator_type>
          (auxid, &vec, store->linkedVector( auxid ), false)
      {
         assert( !isLinked );
      }

      virtual std::unique_ptr<SG::IAuxTypeVector> clone() const override {
        return std::make_unique<AuxPersVector>(*this);
      }
   }; // class AuxPersVector


} // namespace xAOD


#endif // not XAODCORE_PACKEDLINKPERSVECTOR_H
