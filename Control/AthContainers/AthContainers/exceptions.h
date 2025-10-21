// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/exceptions.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2013
 * @brief Exceptions that can be thrown from AthContainers.
 */


#ifndef ATHCONTAINERS_EXCEPTIONS_H
#define ATHCONTAINERS_EXCEPTIONS_H


#include "AthContainersInterfaces/AuxTypes.h"
#include <stdexcept>
#include <typeinfo>
#include <string>


namespace SG {


/**
 * @brief Exception --- Aux data requested from object with no store.
 *
 * You attempted to retrieve auxiliary data from an object with no
 * associated auxiliary data store.
 */
class ExcNoAuxStore
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param auxid The ID of the requested aux data item.
   */
  ExcNoAuxStore (SG::auxid_t auxid);


  /**
   * @brief Constructor.
   * @param op The attempted operation.
   */
  ExcNoAuxStore (const char* op);
};


/**
 * @brief Exception --- Attempt to retrieve nonexistent aux data item.
 *
 * You attempted to retrieve auxiliary data from an object, but the
 * requested item does not exist.
 */
class ExcBadAuxVar
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param auxid The ID of the requested aux data item.
   */
  ExcBadAuxVar (SG::auxid_t auxid);
};


/**
 * @brief Exception --- Non-const operation performed on const aux data.
 *
 * You attempted to perform some non-const operation on an object
 * with const auxiliary data.
 */
class ExcConstAuxData
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param op The attempted operation.
   * @param auxid The ID of the requested aux data item, if any.
   */
  ExcConstAuxData (const std::string& op, SG::auxid_t auxid = null_auxid);
};


/**
 * @brief Exception --- Attempt to set aux data store on container
 *                      that doesn't track indices, or disable index tracking
 *                      for a container with aux data.
 */
class ExcUntrackedSetStore
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   */
  ExcUntrackedSetStore();
};



/**
 * @brief Exception --- Bad use of private store.
 *
 * You called @c makePrivateStore when there was already one,
 * or @c releasePrivateStore when there wasn't one.
 */
class ExcBadPrivateStore
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param op The attempted operation.
   */
  ExcBadPrivateStore (const std::string& op);
};



/**
 * @brief Exception --- Type mismatch for aux variable
 *
 * Thrown when there's a mismatch between the type requested for
 * an aux data item and a type previously requested for the same item.
 */
class ExcAuxTypeMismatch
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param auxid ID of the requested aux data item.
   * @param new_type New type requested for the item.
   * @param old_type Previous type associated with the item.
   * @param new_alloc_type New allocator type requested for this item.
   * @param old_alloc_type Previous allocator type associated with this item.
   */
  ExcAuxTypeMismatch (SG::auxid_t auxid,
                      const std::type_info& new_type,
                      const std::type_info& old_type,
                      const std::string& new_alloc_type,
                      const std::string& old_alloc_type);
};



/**
 * @brief Exception --- Attempted to do OP on a BASE base class of COMPLETE;
 *                      can only be done on the most-derived class.
 *
 * Thrown when there's a mismatch between the type requested for
 * an aux data item and a type previously requested for the same item.
 */
class ExcInsertionInBaseClass
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param op The attempted operation.
   * @param base_type The type on which the operation was attempted.
   * @param complete_type The most-derived type of the object.
   */
  ExcInsertionInBaseClass (const char* op,
                           const std::type_info& base_type,
                           const std::type_info& complete_type);
};


/**
 * @brief Exception --- Attempted to modify auxiliary data in a locked store.
 *
 * Thrown when there was an attempt to access a non-decorator auxiliary
 * item in a locked store, or when some other operation that would
 * change the store is attempted.
 */
class ExcStoreLocked
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param op The attempted operation.
   */
  ExcStoreLocked (SG::auxid_t auxid);


  /**
   * @brief Constructor.
   * @param op The attempted operation.
   */
  ExcStoreLocked (const char* op);
};


/**
 * @brief Exception --- Attempted to insert a unique_ptr to a non-owning container.
 *
 * The @c unique_ptr interfaces can be used only on containers
 * that own their elements.
 */
class ExcNonowningContainer
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   */
  ExcNonowningContainer();
};


/**
 * @brief Throw a SG::ExcNonowningContainer exception.
 */
[[noreturn]]
void throwExcNonowningContainer();


/**
 * @brief Exception --- Unknown aux data item.
 *
 * This can be thrown by @c TypelessConstAccessor if the requested
 * item is not known to the registry, or (if the @c type_info is supplied)
 * we don't know how to make a vector factory for the given @c type_info.
 */
class ExcUnknownAuxItem
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param name Name of the aux data item.
   * @param clsname Class name of the aux data item, or an empty string.
   * @param typ Type of the item, if provided.
   */
  ExcUnknownAuxItem (const std::string& name,
                     const std::string& clsname = "",
                     const std::type_info* typ = 0);
};


/**
 * @brief Throw a SG::ExcUnknownAuxItem exception.
 * @param name Name of the aux data item.
 * @param clsname Class name of the aux data item, or an empty string.
 * @param typ Type of the item, if provided.
 */
[[noreturn]]
void throwExcUnknownAuxItem (const std::string& name,
                             const std::string& clsname = "",
                             const std::type_info* typ = 0);


/**
 * @brief Exception --- Can't convert DataVector to vector of ElementLinks.
 *
 * Thrown by @c dataVectorAsELV when trying to convert a view vector
 * and the contained objects either do not derive from @c AuxElement
 * or are not part of a container.
 */
class ExcDVToELV
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param detail More information about the error.
   */
  ExcDVToELV (const std::string& detail);
};


/**
 * @brief Exception --- ViewVector not in view mode.
 *
 * A ViewVector may only be created with an ownership mode of SG::VIEW_ELEMENTS.
 */
class ExcViewVectorNotView
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   */
  ExcViewVectorNotView();
};


/**
 * @brief Throw a SG::ExcViewVectorNotView exception.
 */
[[noreturn]]
void throwExcViewVectorNotView();


/**
 * @brief Exception --- Missing CLID for @c ViewVector.
 *
 * A @c ViewVector was used in a context that requires a CLID, but no CLID
 * was available.  Make sure a VIEWVECTOR_CLASS_DEF declaration exists
 * for the class in a library that has been loaded.
 */
class ExcMissingViewVectorCLID
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param ti @c type_info for the class with the missing CLID.
   */
  ExcMissingViewVectorCLID (const std::type_info& ti);
};


/**
 * @brief Throw a SG::ExcMissingViewVectorCLID exception.
 * @param ti @c type_info for the class with the missing CLID.
 */
[[noreturn]]
void throwExcMissingViewVectorCLID (const std::type_info& ti);


/**
 * @brief Exception --- Missing BaseInfo.
 *
 * Can't find BaseInfo for CLASS.
 */
class ExcMissingBaseInfo
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param ti @c type_info for the class with the missing base info.
   */
  ExcMissingBaseInfo (const std::type_info& ti);
};


/**
 * @brief Throw a SG::ExcMissingBaseInfo exception.
 * @param ti @c type_info for the class with the missing base info.
 */
[[noreturn]]
void throwExcMissingBaseInfo (const std::type_info& ti);


/**
 * @brief Exception --- Ownership mismatch for insertMove.
 *
 * For insertMove, both vectors must have the same ownership mode.
 */
class ExcInsertMoveOwnershipMismatch
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   */
  ExcInsertMoveOwnershipMismatch();
};


/**
 * @brief Exception --- Flag mismatch for aux variable.
 *
 * Could occur due to not using AtomicDecorator to access an atomic variable.
 */
class ExcFlagMismatch
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param auxid ID of the requested aux data item.
   * @param new_type Type of the item.
   * @param existing_flags Existing flags for the variable.
   * @param requested_flags Requested flags for the variable.
   */
  ExcFlagMismatch (SG::auxid_t auxid,
                   const std::type_info& type,
                   SG::AuxVarFlags existing_flags,
                   SG::AuxVarFlags requested_flags);
};


/**
 * @brief Exception --- Linked variable mismatch.
 */
class ExcLinkMismatch
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param auxid ID of the requested aux data item.
   * @param type Type of the item.
   * @param existing_linked_id ID of existing linked variable.
   * @param requested_linked_id ID of requested linked variable.
   */
  ExcLinkMismatch (SG::auxid_t auxid,
                   const std::type_info& type,
                   SG::auxid_t existing_linked_id,
                   SG::auxid_t requested_linked_id);
};


/**
 * @brief Exception --- Linked variable not found.
 */
class ExcNoLinkedVar
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param auxid ID of the requested aux data item.
   * @param type Type of the item.
   */
  ExcNoLinkedVar (SG::auxid_t auxid,
                  const std::type_info& type);
};


/**
 * @brief Exception --- ThinningHandle target does not exist.
 *
 * The target container referenced by a ThinningHandle does not exist
 * in the event store.
 */
class ExcInvalidThinningTarget
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param clid The CLID of the requested container.
   * @param key The StoreGate key of the requested container.
   */
  ExcInvalidThinningTarget (unsigned int clid,
                            const std::string& key);
};


/**
 * @brief Exception --- Bad iter_swap.
 *
 * Attempt to swap between containers with inconsistent ownership.
 */
class ExcBadIterSwap
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   */
  ExcBadIterSwap();
};


/**
 * @brief Exception --- Bad allocation ownership.
 *
 * Attempt to add a locally-allocated element to a container when it was
 * allocated from a different container.
 */
class ExcAllocOwnership
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   */
  ExcAllocOwnership();
};


/**
 * @brief Throw a SG::ExcAllocOwnership exception.
 */
[[noreturn]]
void throwExcAllocOwnership();


/**
 * @brief Exception --- Bad name for auxiliary variable.
 *
 * Variable and class name must be identifiers, and variable name
 * must not be empty.
 */
class ExcBadVarName
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   * @param name Name of the variable.
   */
  ExcBadVarName (const std::string& name);
};


/**
 * @brief Exception --- Overlapping copies not implemented for jagged vectors.
 *
 * Talk to core software if this is an issue.
 */
class ExcJaggedVecOverlappingCopy
  : public std::runtime_error
{
public:
  /**
   * @brief Constructor.
   */
  ExcJaggedVecOverlappingCopy();
};


/**
 * @brief Throw a SG::ExcJaggedVecOverlappingCopy exception.
 */
[[noreturn]]
void throwJaggedVecOverlappingCopy();


} // namespace SG


#endif // not ATHCONTAINERS_EXCEPTIONS_H
