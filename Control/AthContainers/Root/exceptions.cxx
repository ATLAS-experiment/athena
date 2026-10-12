/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/src/exceptions.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2013
 * @brief Exceptions that can be thrown from AthContainers.
 */


#include "AthContainers/exceptions.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/tools/error.h"
#include <format>


namespace SG {


/// Helper: format an aux data item name.
std::string excFormatName (SG::auxid_t auxid)
{
  if (auxid != null_auxid) {
    SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
    return std::format ("`{}::{}' ({})",
                        r.getClassName (auxid),
                        r.getName (auxid),
                        auxid);
  }
  return "(null)";
}
  

//*************************************************************************


/// Helper: format exception error string.
std::string excNoAuxStore_format (SG::auxid_t auxid)
{
  return std::format ("SG::ExcNoAuxStore: "
                      "Requested aux data item {} but there is no associated aux data store.",
                      excFormatName (auxid));
}


/// Helper: format exception error string.
std::string excNoAuxStore_format (const char* op)
{
  return std::format ("SG::ExcNoAuxStore: "
                      "Operation attempted on container with no associated aux data store: {}", op);;
}


/**
 * @brief Constructor.
 * @param auxid The ID of the requested aux data item.
 */
ExcNoAuxStore::ExcNoAuxStore (SG::auxid_t auxid)
  : std::runtime_error (excNoAuxStore_format (auxid))
{
}


/**
 * @brief Constructor.
 * @param op The attempted operation.
 */
ExcNoAuxStore::ExcNoAuxStore (const char* op)
  : std::runtime_error (excNoAuxStore_format (op))
{
}


//*************************************************************************


/// Helper: format exception error string.
std::string excBadAuxVar_format (SG::auxid_t auxid)
{
  return std::format ("SG::ExcBadAuxVar: "
                      "Attempt to retrieve nonexistent aux data item {}.",
                      excFormatName (auxid));
}


/**
 * @brief Constructor.
 * @param auxid The ID of the requested aux data item.
 */
ExcBadAuxVar::ExcBadAuxVar (SG::auxid_t auxid)
  : std::runtime_error (excBadAuxVar_format (auxid))
{
}


//*************************************************************************


/// Helper: format exception error string.
std::string excConstAuxData_format (const std::string& op, SG::auxid_t auxid)
{
  return std::format ("SG::ExcConstAuxData: "
                      "Non-const operation `{}' performed on const aux data {}.",
                      op, excFormatName (auxid));
}


/**
 * @brief Constructor.
 * @param op The attempted operation.
 * @param auxid The ID of the requested aux data item, if any.
 */
ExcConstAuxData::ExcConstAuxData (const std::string& op, SG::auxid_t auxid)
  : std::runtime_error (excConstAuxData_format (op, auxid))
{
}


//*************************************************************************


/**
 * @brief Constructor.
 */
ExcUntrackedSetStore::ExcUntrackedSetStore()
  : std::runtime_error
    ("SG::ExcUntrackedSetStore: "
     "Attempt to set aux data store on container that doesn't track indices, "
     "or disable index tracking for a container with aux data.")
{
}


//*************************************************************************


/**
 * @brief Constructor.
 * @param op The attempted operation.
 */
ExcBadPrivateStore::ExcBadPrivateStore (const std::string& op)
  : std::runtime_error ("SG::ExcBadPrivateStore: "
                        "Bad use of private store: " + op)
{
}


//*************************************************************************


/// Helper: format exception error string.
std::string excAuxTypeMismatch_format (SG::auxid_t auxid,
                                       const std::type_info& new_type,
                                       const std::type_info& old_type,
                                       const std::string& new_alloc_type,
                                       const std::string& old_alloc_type)
{
  std::string s =
    std::format ("SG::ExcAuxTypeMismatch: "
                 "Type mismatch for aux variable {}; old type is {} new type is {}",
                 excFormatName (auxid),
                 AthContainers_detail::typeinfoName (old_type),
                 AthContainers_detail::typeinfoName (new_type));
  if (old_alloc_type != new_alloc_type)
  {
    s += std::format (".  Old allocator type is {} new allocator type is {}",
                      old_alloc_type, new_alloc_type);
  }
  return s;
}


/**
 * @brief Constructor.
 * @param auxid ID of the requested aux data item.
 * @param new_type New type requested for the item.
 * @param old_type Previous type associated with the item.
 * @param new_alloc_type New allocator type requested for this item.
 * @param old_alloc_type Previous allocator type associated with this item.
 */
ExcAuxTypeMismatch::ExcAuxTypeMismatch (SG::auxid_t auxid,
                                        const std::type_info& new_type,
                                        const std::type_info& old_type,
                                        const std::string& new_alloc_type,
                                        const std::string& old_alloc_type)
  : std::runtime_error (excAuxTypeMismatch_format (auxid,
                                                   new_type, old_type,
                                                   new_alloc_type,
                                                   old_alloc_type))
{
}


//*************************************************************************


/// Helper: format exception error string.
std::string excInsertionInBaseClass_format (const char* op,
                                            const std::type_info& base_type,
                                            const std::type_info& complete_type)
{
  return std::format ("SG::ExcInsertionInBaseClass: "
                      "Attempted to do {} on a {} base class of {}; can only be done on the most-derived class.",
                      op,
                      AthContainers_detail::typeinfoName (base_type),
                      AthContainers_detail::typeinfoName (complete_type));
}


/**
 * @brief Constructor.
 * @param op The attempted operation.
 * @param base_type The type on which the operation was attempted.
 * @param complete_type The most-derived type of the object.
 */
ExcInsertionInBaseClass::ExcInsertionInBaseClass
   (const char* op,
    const std::type_info& base_type,
    const std::type_info& complete_type)
     : std::runtime_error (excInsertionInBaseClass_format (op,
                                                           base_type,
                                                           complete_type))
{
}


//*************************************************************************


/// Helper: format exception error string.
std::string excStoreLocked_format (SG::auxid_t auxid)
{
  return std::format ("SG::ExcStoreLocked: "
                      "Attempted to modify auxiliary data in a locked store: {}",
                      excFormatName (auxid));
}


/// Helper: format exception error string.
std::string excStoreLocked_format (const char* op)
{
  return std::format ("SG::ExcStoreLocked: "
                      "Attempted to modify auxiliary data in a locked store: {}", op);
}


/**
 * @brief Constructor.
 * @param op The attempted operation.
 */
ExcStoreLocked::ExcStoreLocked (SG::auxid_t auxid)
  : std::runtime_error (excStoreLocked_format (auxid))
{
}


/**
 * @brief Constructor.
 * @param op The attempted operation.
 */
ExcStoreLocked::ExcStoreLocked (const char* op)
  : std::runtime_error (excStoreLocked_format (op))
{
}


//*************************************************************************


/**
 * @brief Constructor.
 */
ExcNonowningContainer::ExcNonowningContainer()
  : std::runtime_error ("SG::ExcNonowningContainer: Attempted to insert a unique_ptr to a non-owning container.")
{
}


/**
 * @brief Throw a SG::ExcNonowningContainer exception.
 */
void throwExcNonowningContainer()
{
  throw ExcNonowningContainer();
}


//*************************************************************************


/// Helper: format exception error string.
std::string excUnknownAuxItem_format (const std::string& name,
                                      const std::string& clsname,
                                      const std::type_info* typ)
{
  std::string s = std::format ("SG::ExcUnknownAuxItem: "
                               "Unknown aux data item {}{}{}",
                               clsname, (clsname.empty() ? "" : "::"), name);
  if (typ)
    s += std::format (" (of type {})",
                      AthContainers_detail::typeinfoName (*typ));
  return s;
}


/**
 * @brief Constructor.
 * @param name Name of the aux data item.
 * @param clsname Class name of the aux data item, or an empty string.
 * @param typ Type of the item, if provided.
 */
ExcUnknownAuxItem::ExcUnknownAuxItem (const std::string& name,
                                      const std::string& clsname /*= ""*/,
                                      const std::type_info* typ /*= 0*/)
  : std::runtime_error (excUnknownAuxItem_format (name, clsname, typ))
{
}


/**
 * @brief Throw a SG::ExcUnknownAuxItem exception.
 * @param name Name of the aux data item.
 * @param clsname Class name of the aux data item, or an empty string.
 * @param typ Type of the item, if provided.
 */
void throwExcUnknownAuxItem (const std::string& name,
                             const std::string& clsname /*= ""*/,
                             const std::type_info* typ /*= 0*/)
{
  throw ExcUnknownAuxItem (name, clsname, typ);
}


//*************************************************************************


/// Helper: format exception error string.
std::string excDVToELV_format (const std::string& detail)
{
  return std::format ("SG::ExcDVToELV: "
                      "Can't convert DataVector to vector of ElementLinks: {}",
                      detail);
}


/**
 * @brief Constructor.
 * @param detail More information about the error.
 */
ExcDVToELV::ExcDVToELV (const std::string& detail)
  : std::runtime_error (excDVToELV_format (detail))
{
}


//*************************************************************************


/**
 * @brief Constructor.
 */
ExcViewVectorNotView::ExcViewVectorNotView()
  : std::runtime_error ("ViewVector not in view mode.")
{
}


/**
 * @brief Throw a SG::ExcViewVectorNotView exception.
 */
void throwExcViewVectorNotView()
{
  throw ExcViewVectorNotView();
}


//*************************************************************************


/**
 * @brief Constructor.
 */
ExcMissingViewVectorCLID::ExcMissingViewVectorCLID (const std::type_info& ti)
  : std::runtime_error ("ViewVector " + AthContainers_detail::typeinfoName(ti) +
                        "was used in a context that requires a CLID, "
                        "but no CLID was available.  Make sure a "
                        "VIEWVECTOR_CLASS_DEF declaration exists for the class "
                        "in a library that has been loaded.")
{
}


/**
 * @brief Throw a SG::ExcViewVectorCLID exception.
 */
void throwExcMissingViewVectorCLID (const std::type_info& ti)
{
  throw ExcMissingViewVectorCLID (ti);
}


//*************************************************************************


/**
 * @brief Constructor.
 */
ExcMissingBaseInfo::ExcMissingBaseInfo (const std::type_info& ti)
  : std::runtime_error ("Missing BaseInfo for " +
                        AthContainers_detail::typeinfoName(ti))
{
}


/**
 * @brief Throw a SG::ExcMissingBaseInfo exception.
 */
void throwExcMissingBaseInfo (const std::type_info& ti)
{
  throw ExcMissingBaseInfo (ti);
}


//*************************************************************************


/**
 * @brief Constructor.
 */
ExcInsertMoveOwnershipMismatch::ExcInsertMoveOwnershipMismatch()
  : std::runtime_error ("Ownership mismatch for insertMove.")
{
}


//*************************************************************************


/// Helper: format flags.
std::string excFormatFlags (SG::AuxVarFlags flags)
{
  std::string out;
  if (flags & SG::AuxVarFlags::Atomic)
    out = "atomic";
  if (flags & SG::AuxVarFlags::Linked) {
    if (!out.empty()) out += " ";
    out = "linked";
  }

  return "[" + out + "]";
}


/// Helper: format exception error string.
std::string excFlagMismatch_format (SG::auxid_t auxid,
                                    const std::type_info& type,
                                    SG::AuxVarFlags existing_flags,
                                    SG::AuxVarFlags requested_flags)
{
  return std::format ("SG::ExcFlagMismatch: "
                      "Flag mismatch for aux variable {} of type {} between existing {} and requested {} flags. "
                      "This may occur as a result of not using AtomicDecorator to access an atomic variable.",
                      excFormatName (auxid),
                      AthContainers_detail::typeinfoName (type),
                      excFormatFlags (existing_flags),
                      excFormatFlags (requested_flags));
}


/**
 * @brief Constructor.
 * @param auxid ID of the requested aux data item.
 * @param new_type Type of the item.
 * @param existing_flags Existing flags for the variable.
 * @param requested_flags Requested flags for the variable.
 */
ExcFlagMismatch::ExcFlagMismatch (SG::auxid_t auxid,
                                  const std::type_info& type,
                                  SG::AuxVarFlags existing_flags,
                                  SG::AuxVarFlags requested_flags)
  : std::runtime_error (excFlagMismatch_format (auxid, type, existing_flags, requested_flags))
{
}


//*************************************************************************


/// Helper: format exception error string.
std::string excLinkMismatch_format (SG::auxid_t auxid,
                                    const std::type_info& type,
                                    SG::auxid_t existing_linked_id,
                                    SG::auxid_t requested_linked_id)
{
  return std::format ("SG::ExcLinkMismatch: "
                      "Linked variable mismatch for aux variable {} of type {} between existing {} and requested {} linked variables.",
                      excFormatName (auxid),
                      AthContainers_detail::typeinfoName (type),
                      excFormatName (existing_linked_id),
                      excFormatName (requested_linked_id));
}


/**
 * @brief Constructor.
 * @param auxid ID of the requested aux data item.
 * @param type Type of the item.
 * @param existing_linked_id ID of existing linked variable.
 * @param requested_linked_id ID of requested linked variable.
 */
ExcLinkMismatch::ExcLinkMismatch (SG::auxid_t auxid,
                                  const std::type_info& type,
                                  SG::auxid_t existing_linked_id,
                                  SG::auxid_t requested_linked_id)
  : std::runtime_error (excLinkMismatch_format (auxid, type,
                                                existing_linked_id, requested_linked_id))
{
}


//*************************************************************************


/// Helper: format exception error string.
std::string excNoLinkedVar_format (SG::auxid_t auxid,
                                   const std::type_info& type)
{
  return std::format ("SG::ExcNoLinkedVar: "
                      "Linked variable not found for aux variable {} of type {}",
                      excFormatName (auxid), AthContainers_detail::typeinfoName (type));
}


/**
 * @brief Constructor.
 * @param auxid ID of the requested aux data item.
 * @param type Type of the item.
 */
ExcNoLinkedVar::ExcNoLinkedVar (SG::auxid_t auxid,
                                const std::type_info& type)
  : std::runtime_error (excNoLinkedVar_format (auxid, type))
{
}


//*************************************************************************


/// Helper: format exception error string.
std::string excInvalidThinningTarget_format (unsigned int clid,
                                             const std::string& key)
{
  return std::format ("SG::ExcInvalidThinningTarget: "
                      "ThinningHandle target does not exist: {}/{}",
                      clid, key);
}


/**
 * @brief Constructor.
 * @param clid The CLID of the requested container.
 * @param key The StoreGate key of the requested container.
 */
ExcInvalidThinningTarget::ExcInvalidThinningTarget (unsigned int clid,
                                                    const std::string& key)
  : std::runtime_error (excInvalidThinningTarget_format (clid, key))
{
}


//*************************************************************************


/**
 * @brief Constructor.
 */
ExcBadIterSwap::ExcBadIterSwap()
  : std::runtime_error ("SG::ExcBadIterSwap: Attempt to swap between containers with inconsistent ownership.")
{
}


//*************************************************************************


/**
 * @brief Constructor.
 */
ExcAllocOwnership::ExcAllocOwnership()
  : std::runtime_error ("SG::ExcAllocOwnership: Bad allocation ownership.")
{
}


/**
 * @brief Throw a SG::ExcAllocOwnership exception.
 */
void throwExcAllocOwnership()
{
  throw ExcAllocOwnership();
}


//*************************************************************************


/// Helper: format exception error string.
std::string excBadVarName_format (const std::string& name)
{
  return std::format ("SG::ExcBadVarName: "
                      "Bad name for auxiliary variable `{}'. "
                      "Variable and class names must consist of only digits, letters, "
                      "and underscores, and cannot start with a digit.  "
                      "Variable names may not be empty, but class names may be.",
                      name);
}


/**
 * @brief Constructor.
 * @param name Name of the variable.
 */
ExcBadVarName::ExcBadVarName (const std::string& name)
  : std::runtime_error (excBadVarName_format (name))
{
}


//*************************************************************************


/// Helper: format exception error string.
std::string excJaggedVecOverlappingCopy_format()
{
  return "SG::ExcJaggedVecOverlappingCopy: Overlapping copies not implemented for jagged vectors.  Talk to core software if this is an issue.";
}


/**
 * @brief Constructor.
 */
ExcJaggedVecOverlappingCopy::ExcJaggedVecOverlappingCopy()
  : std::runtime_error (excJaggedVecOverlappingCopy_format())
{
}


/**
 * @brief Throw a SG::ExcJaggedVecOverlappingCopy exception.
 */
void throwJaggedVecOverlappingCopy()
{
  throw ExcJaggedVecOverlappingCopy();
}


} // namespace SG
