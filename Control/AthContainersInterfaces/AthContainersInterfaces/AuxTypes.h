// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainersInterfaces/AuxTypes.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2013
 * @brief Basic definitions for auxiliary types.
 *
 * This defines types used to identify pieces of aux data.
 */


#ifndef ATHCONTAINERSINTERFACES_AUXTYPES_H
#define ATHCONTAINERSINTERFACES_AUXTYPES_H


#include <cstddef>
#include "CxxUtils/ConcurrentBitset.h"
#include "CxxUtils/bitmask.h"


namespace SG {

/// Identifier for a particular aux data item.
typedef size_t auxid_t;

/// To signal no aux data item.
static const auxid_t null_auxid = static_cast<auxid_t> (-1);

/// Used as type_info when we read an aux data item but it doesn't
/// exist in the registry.
class AuxTypePlaceholder {};

/// A hint for how large we should make the auxid bitsets.  It's best if we don't
/// need to reallocate them later; but on the other hand, we don't want this to be
/// larger than it really needs to be.  The largest auxid i saw in a reco job
/// was about 1800, so this is probably good for now.
static const auxid_t auxid_set_size_hint = 2048;


/// A set of aux data identifiers.
/// This is a ConcurrentBitset, with the default size set to auxid_set_size_hint.
class auxid_set_t
  : public CxxUtils::ConcurrentBitset
{
public:
  using CxxUtils::ConcurrentBitset::ConcurrentBitset;
  auxid_set_t (size_t nbits = auxid_set_size_hint)
    : CxxUtils::ConcurrentBitset (nbits)
  {
  }
};


/// Additional flags to qualify an auxiliary variable.
enum AuxVarFlags {
  /// No special flags set.
  None   = 0x00,

  /// Mark that this variable should only be accessed atomically.
  /// If this is set, then the variable can be accessed only via
  /// @c AtomicDecorator.
  /// Be aware that this is not completely safe; it is still possible
  /// to have non-atomic accesses to the variable via either the implicit
  /// operations provided by this class or by explicit access to the
  /// value vector.  Therefore, this is not recommended for general use.
  /// Contact core software before using this for new code.
  Atomic = 0x01,

  // These flags control the behavior of findAuxID() but are not
  // stored with the variable.
  SkipNameCheck = 0x80,

  /// Enable bitwise functions on this enum; see bitmask.h.
  ATH_BITMASK
};


} // namespace SG



#endif // not ATHCONTAINERSINTERFACES_AUXTYPES_H
