/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthLinks/src/exceptions.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2013
 * @brief Exceptions that can be thrown from AthLinks.
 */


#include "AthLinks/exceptions.h"
#include <sstream>
#include <string>


namespace SG {


/// For setting debugger breakpoints
void AthLinks_error() {}


//*************************************************************************


/// Helper: format exception error string.
std::string excPointerNotInSG_format (const void* pointer)
{
  return std::format ("SG::ExcPointerNotInSG: "
                      "The object referenced by a DataLink / ElementLink is not registered "
                      "in StoreGate: {}.", pointer);
}


/**
 * @brief Constructor.
 * @param pointer Pointer to the object that the link is referencing.
 */
ExcPointerNotInSG::ExcPointerNotInSG (const void* pointer)
  : std::runtime_error (excPointerNotInSG_format (pointer))
{
  AthLinks_error();
}


//*************************************************************************


/// Helper: format exception error string.
std::string excCLIDMismatch_format (CLID obj_clid, CLID link_clid)
{
  return std::format ("SG::ExcCLIDMismatch: "
                      "Attempt to set DataLink / ElementLink with CLID {} to object with CLID {}",
                      link_clid, obj_clid);
}


/**
 * @brief Constructor.
 * @param obj_clid The CLID of the object being assigned to the link.
 * @param link_clid The declared CLID of the link.
 */
ExcCLIDMismatch::ExcCLIDMismatch (CLID obj_clid, CLID link_clid)
  : std::runtime_error (excCLIDMismatch_format (obj_clid, link_clid))
{
  AthLinks_error();
}


//*************************************************************************


/// Helper: format exception error string.
std::string
excInvalidLink_format (CLID clid, const std::string& key, SG::sgkey_t sgkey)
{
  return std::format ("SG::ExcInvalidLink: "
                      "Attempt to dereference invalid DataLink / ElementLink "
                      "[{}/{}] ({})",
                      clid, key, sgkey);
}


/**
 * @brief Constructor.
 * @param clid CLID of the link.
 * @param key String key of the link.
 * @param sgkey Hashed key of the link.
 */
ExcInvalidLink::ExcInvalidLink (CLID clid,
                                const std::string& key,
                                SG::sgkey_t sgkey)
  : std::runtime_error (excInvalidLink_format (clid, key, sgkey))
{
  AthLinks_error();
}


/**
 * @brief Throw a SG::ExcInvalidLink exception.
 * @param clid CLID of the link.
 * @param key String key of the link.
 * @param sgkey Hashed key of the link.
 */
void throwExcInvalidLink (CLID clid, const std::string& key, SG::sgkey_t sgkey)
{
  throw ExcInvalidLink (clid, key, sgkey);
}


//*************************************************************************


/// Helper: format exception error string.
std::string
excBadForwardLink_format (size_t index, size_t size, const std::string& name)
{
  std::string s = std::format ("SG::ExcBadForwardLink: "
                               "ForwardIndexingPolicy: internal link state of '{}' is invalid", name);
  if (index != static_cast<size_t>(-1)) {
    s += std::format (": m_index = {} is >= data container size = {}",
                      index, size);
  } 
  return s;
}


/**
 * @brief Constructor.
 * @param index Index in the link.
 * @param size Size of the referenced container.
 * @param name Type name of the container.
 */
ExcBadForwardLink::ExcBadForwardLink (size_t index, size_t size, const std::string& name)
  : std::runtime_error (excBadForwardLink_format (index, size, name))
{
  AthLinks_error();
}


/**
 * @brief Throw a SG::ExcBadForwardLink exception.
 * @param index Index in the link.
 * @param size Size of the referenced container.
 * @param name Type name of the container.
 */
void throwExcBadForwardLink (size_t index, size_t size, const std::string& name)
{
  throw ExcBadForwardLink (index, size, name);
}


//*************************************************************************


/**
 * @brief Constructor.
 * @param where The operation being attempted.
 */
ExcElementNotFound::ExcElementNotFound (const std::string& where)
  : std::runtime_error ("SG::ExcElementNotFound: " + where +
                        ": element not found")
{
  AthLinks_error();
}


/**
 * @brief Throw a SG::ExcElementNotFound exception.
 * @param where The operation being attempted.
 */
void throwExcElementNotFound (const char* where)
{
  throw ExcElementNotFound (where);
}


//*************************************************************************


/**
 * @brief Constructor.
 * @param where The operation being attempted.
 */
ExcInvalidIndex::ExcInvalidIndex (const std::string& where)
  : std::runtime_error ("SG::ExcInvalidIndex: " + where + ": invalid index")
{
  AthLinks_error();
}


/**
 * @brief Throw a SG::ExcInvalidIndex exception.
 * @param where The operation being attempted.
 */
void throwExcInvalidIndex (const char* where)
{
  throw ExcInvalidIndex (where);
}


//*************************************************************************


/**
 * @brief Constructor.
 * @param where The operation being attempted.
 */
ExcIndexNotFound::ExcIndexNotFound (const std::string& where)
  : std::runtime_error ("SG::ExcIndexNotFound: " + where + ": index not found")
{
  AthLinks_error();
}


/**
 * @brief Throw a SG::ExcIndexNotFound exception.
 * @param where The operation being attempted.
 */
void throwExcIndexNotFound (const char* where)
{
  throw ExcIndexNotFound (where);
}


//*************************************************************************


/**
 * @brief Constructor.
 */
ExcIncomparableEL::ExcIncomparableEL()
  : std::runtime_error ("SG::ExcIncomparableEL: Attempt to compare an ElementLink that does not have a SG key or index.")
{
  AthLinks_error();
}


/**
 * @brief Throw a SG::ExcIncomparableSG exception.
 */
void throwExcIncomparableEL()
{
  throw ExcIncomparableEL();
}


//*************************************************************************


/**
 * @brief Constructor.
 */
ExcBadToTransient::ExcBadToTransient()
  : std::runtime_error ("SG::ExcBadToTransient: toTransient() called on an already-initialized link.")
{
  AthLinks_error();
}


/**
 * @brief Throw a SG::ExcBadToTransient exception.
 */
void throwExcBadToTransient()
{
  throw ExcBadToTransient();
}


//*************************************************************************


/// Helper: format exception error string.
std::string
excConstStorable_format (CLID clid, const std::string& key, SG::sgkey_t sgkey)
{
  return std::format ("SG::ExcConstStorable: "
                      "Tried to retrieve const storable as a non-const pointer "
                      "[{}/{}] ({})",
                      clid, key, sgkey);
}


/**
 * @brief Constructor.
 * @param clid CLID of the link.
 * @param key String key of the link.
 * @param sgkey Hashed key of the link.
 */
ExcConstStorable::ExcConstStorable (CLID clid,
                                    const std::string& key,
                                    SG::sgkey_t sgkey)
  : std::runtime_error (excConstStorable_format (clid, key, sgkey))
{
  AthLinks_error();
}


//*************************************************************************


/// Helper: format exception error string.
std::string
excBadThinning_format (CLID clid, const std::string& key, SG::sgkey_t sgkey)
{
  return std::format ("SG::ExcBadThinning: Bad thinning request [{}/{}] ({})",
                      clid, key, sgkey);
}


/**
 * @brief Constructor.
 * @param clid CLID of the link.
 * @param key String key of the link.
 * @param sgkey Hashed key of the link.
 */
ExcBadThinning::ExcBadThinning (CLID clid,
                                const std::string& key,
                                SG::sgkey_t sgkey)
  : std::runtime_error (excBadThinning_format (clid, key, sgkey))
{
  AthLinks_error();
}


/**
 * @brief Throw a SG::ExcBadThinning exception.
 * @param clid CLID of the link.
 * @param key String key of the link.
 * @param sgkey Hashed key of the link.
 */
void throwExcBadThinning (CLID clid,
                          const std::string& key,
                          SG::sgkey_t sgkey)
{
  throw ExcBadThinning (clid, key, sgkey);
}


} // namespace SG
