// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file xAODCore/tools/JaggedVecPersVector.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief Specialization of @c AuxPersVector for jagged vector types.
 *
 * For jagged vector variables, the corresponding @c AuxPersVector class
 * should derive from @c JaggedVecVectorHolder in order to get the
 * proper implementations for the @c IAuxTypeVector methods.
 */


#ifndef XAODCORE_JAGGEDVECPERSVECTOR_H
#define XAODCORE_JAGGEDVECPERSVECTOR_H

#include "xAODCore/tools/AuxPersVector.h"
#include "AthContainers/JaggedVec.h"


namespace xAOD {

  template< class T, class VEC >
   class AuxPersVector<SG::JaggedVecElt<T>, VEC>
    : public SG::JaggedVecVectorHolder<T, typename VEC::allocator_type> {

   public:
      /// Convenience type definition
      typedef VEC& vector_type;

      /// Constructor
      AuxPersVector( SG::auxid_t auxid, vector_type vec, [[maybe_unused]] bool isLinked, SG::IAuxStore* store )
        : SG::JaggedVecVectorHolder<T, typename VEC::allocator_type>
          (auxid, &vec, store->linkedVector( auxid ), false)
      {
         assert( !isLinked );
      }

      virtual std::unique_ptr<SG::IAuxTypeVector> clone() const override {
        return std::make_unique<AuxPersVector>(*this);
      }
   }; // class AuxPersVector


} // namespace xAOD


#endif // not XAODCORE_JAGGEDVECPERSVECTOR_H
