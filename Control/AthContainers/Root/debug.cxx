/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file AthContainers/Root/debug.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2015
 * @brief Helper functions intended to be called from the debugger.
 */


#include "AthContainers/debug.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/JaggedVecImpl.h"
#include "AthContainers/PackedLinkImpl.h"
#include "AthContainers/tools/error.h"
#include "AthContainersInterfaces/IConstAuxStore.h"
#include "AthLinks/DataLinkBase.h"
#ifndef XAOD_STANDALONE
# include "SGTools/DataProxy.h"
#endif
#include "CxxUtils/StrFormat.h"
#include <format>
#include <vector>
#include <sstream>
#include <iostream>


namespace SGdebug {


/**
 * @brief Return the name corresponding to a given aux id.
 * @param id The aux id to look up.
 */
std::string aux_var_name (SG::auxid_t id)
{
  SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();
  return reg.getClassName(id) + "::" + reg.getName(id);
}


/**
 * @brief Print the name corresponding to a given aux id.
 * @param id The aux id to print.
 */
void print_aux_var_name (SG::auxid_t id)
{
  std::cout << aux_var_name(id) << "\n";
}


/******************************************************************************
 * Print the list of aux variables given some object.
 */


/**
 * @brief Print the list of aux variables in a set.
 * @param auxids The set to print.
 */
void print_aux_vars (const SG::auxid_set_t& auxids,
                     const SG::auxid_set_t& decors /*= SG::auxid_set_t()*/)
{
  SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();
  std::vector<SG::auxid_t> ids (auxids.begin(), auxids.end());
  std::sort (ids.begin(), ids.end());

  for (SG::auxid_t id : ids) {
    std::cout << id << " "
              << reg.getClassName(id) << "::" << reg.getName(id) << " "
              << "[" << reg.getTypeName(id);

    SG::AuxVarFlags flags = reg.getFlags(id);
    if (flags & SG::AuxVarFlags::Atomic) {
      std::cout << " (atomic)";
    }
    if (flags & SG::AuxVarFlags::Linked) {
      std::cout << " (linked)";
    }
    if (decors.test (id)) {
      std::cout << " (decor)";
    }

    std::cout << "]\n";
  }
}


/**
 * @brief Print the list of aux variables handled by a store.
 * @param store The store to dump.
 */
void print_aux_vars (const SG::IConstAuxStore& store)
{
  print_aux_vars (store.getAuxIDs(), store.getDecorIDs());
}


/**
 * @brief Print the list of aux variables handled by a store.
 * @param store The store to dump.
 */
void print_aux_vars (const SG::IConstAuxStore* store)
{
  print_aux_vars (*store);
}


/**
 * @brief Print the list of aux variables associated with a container.
 * @param vec The container to dump.
 */
void print_aux_vars (const SG::AuxVectorData& vec)
{
  print_aux_vars (vec.getAuxIDs(), vec.getDecorIDs());
}


/**
 * @brief Print the list of aux variables associated with a container.
 * @param vec The container to dump.
 */
void print_aux_vars (const SG::AuxVectorData* vec)
{
  print_aux_vars (*vec);
}


/**
 * @brief Print the list of aux variables associated with an element.
 * @param elt The element to dump.
 */
void print_aux_vars (const SG::AuxElement& elt)
{
  print_aux_vars (elt.getAuxIDs(), elt.getDecorIDs());
}


/**
 * @brief Print the list of aux variables associated with an element.
 * @param elt The element to dump.
 */
void print_aux_vars (const SG::AuxElement* elt)
{
  print_aux_vars (*elt);
}


/******************************************************************************
 * Dump out aux variables.
 */


namespace {


template <class T>
void convert (std::ostream& os, const T& x)
{
  os << x;
}


void convert (std::ostream& os, const float x)
{
  os << CxxUtils::strformat ("%.3f", x);
}


void convert (std::ostream& os, const double x)
{
  os << CxxUtils::strformat ("%.3f", x);
}


void convert (std::ostream& os, const SG::JaggedVecEltBase& x, size_t i)
{
  os << std::format ("[{},{}]", x.begin(i), x.end());
}


void convert (std::ostream& os, const DataLinkBase& x)
{
#ifdef XAOD_STANDALONE
  os << std::format ("DataLink[{}]", x.persKey());
#else
  os << std::format ("DataLink[{}/{}]", x.proxy() ? x.proxy()->clID() : CLID_NULL, x.dataID());
#endif
}


void convert (std::ostream& os, const SG::PackedLinkBase& x)
{
  os << std::format ("PackedLink[{}/{}]", x.collection(), x.index());
}


template <class T>
void convert (std::ostream& os, const std::vector<T>& x)
{
  os << "[";
  bool first = true;
  // using `decltype(auto)` in case T=bool
  // cppcheck-suppress internalAstError
  for (decltype(auto) elt : x) {
    if (first)
      first = false;
    else
      os << ", ";
    convert (os, elt);
  }
  os << "]";
}


struct AuxVarSort
{
  AuxVarSort (SG::auxid_t the_id)
    : id(the_id),
      name(aux_var_name(the_id))
  {}

  SG::auxid_t id;
  std::string name;
};


bool operator< (const AuxVarSort& a, const AuxVarSort& b )
{
  return a.name < b.name;
}


} // anonymous namespace


/**
 * @brief Convert an aux variable to a string.
 * @param auxid The id of the variable.
 * @param p Pointer to the location of the variable.
 * @param i Index of the element being dumped.
 */
std::string aux_var_as_string (SG::auxid_t auxid, const void* p, size_t i)
{
  if (!p) {
    return "(null)";
  }

  std::ostringstream os;

  const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  const std::type_info* ti = r.getType(auxid);
#define CONVERT(T) if (ti == &typeid(T)) convert (os, *reinterpret_cast<const T*>(p)); else
#define CONVERT1(T) CONVERT(T) CONVERT(std::vector<T>)
  CONVERT1 (int)
  CONVERT1 (unsigned int)
  CONVERT1 (short)
  CONVERT1 (unsigned short)
  CONVERT1 (char)
  CONVERT1 (unsigned char)
  CONVERT1 (long)
  CONVERT1 (unsigned long)
  CONVERT1 (long long)
  CONVERT1 (unsigned long long)
  CONVERT1 (float)
  CONVERT1 (double)
  CONVERT1 (bool)
  CONVERT1 (std::string)
  //else
  {
    std::string tiname = AthContainers_detail::typeinfoName(*ti);
    if (tiname.starts_with ("SG::JaggedVecElt<")) {
      convert (os, *reinterpret_cast<const SG::JaggedVecEltBase*>(p), i);
    }
    else if (tiname.starts_with ("DataLink<")) {
      convert (os, *reinterpret_cast<const DataLinkBase*>(p));
    }
    else if (tiname.starts_with ("SG::PackedLink<")) {
      convert (os, *reinterpret_cast<const SG::PackedLinkBase*>(p));
    }
    else if (tiname.starts_with ("std::vector<SG::PackedLink<")) {
      convert (os, *reinterpret_cast<const std::vector<SG::PackedLinkBase>*>(p));
    }
    else {
      os << "<??? " << tiname << ">";
    }
  }
  return os.str();
}


/**
 * @brief Dump aux variables from a store for a single element.
 * @param os The stream to which to write.
 * @param store The store from which to dump.
 * @param i The index of the element to dump.
 */
void dump_aux_vars (std::ostream& os, const SG::IConstAuxStore& store, size_t i)
{
  if (i >= store.size()) return;
  SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();
  const SG::auxid_set_t& ids = store.getAuxIDs();
  std::vector<AuxVarSort> vars (ids.begin(), ids.end());
  std::sort (vars.begin(), vars.end());
  for (const AuxVarSort& v : vars) {
    if (reg.isLinked (v.id)) continue;
    const void* pbeg = store.getData (v.id);
    size_t eltsz = reg.getEltSize (v.id);
    const char* p = reinterpret_cast<const char*>(pbeg) + eltsz*i;
    os << v.name << " " << aux_var_as_string (v.id, p, i) << "\n";
    SG::auxid_t linked_id = reg.linkedVariable (v.id);
    if (linked_id != SG::null_auxid) {
      os << "  linked: " << aux_var_name (linked_id) << " ";
      const SG::IAuxTypeVector* lv = store.linkedVector (v.id);
      if (!lv) {
        os << "(missing linkedVector)\n";
        continue;
      }
      size_t sz = lv->size();
      const char* lbeg = reinterpret_cast<const char*>(lv->toPtr());
      size_t leltsz = reg.getEltSize (linked_id);
      os << "[";
      bool first = true;
      for (size_t j = 0; j < sz; j++) {
        if (first)
          first = false;
        else
          os << ", ";
        const char* p = reinterpret_cast<const char*>(lbeg) + leltsz*j;
        os << aux_var_as_string (linked_id, p, i);
      }
      os << "]\n";
    }
  }
}


/**
 * @brief Dump aux variables from a store for a single element (to cout).
 * @param store The store from which to dump.
 * @param i The index of the element to dump.
 */
void dump_aux_vars (const SG::IConstAuxStore& store, size_t i)
{
  dump_aux_vars (std::cout, store, i);
}


/**
 * @brief Dump aux variables from a store for a single element.
 * @param store The store from which to dump.
 * @param i The index of the element to dump.
 */
void dump_aux_vars (const SG::IConstAuxStore* store, size_t i)
{
  dump_aux_vars (*store, i);
}


/**
 * @brief Dump aux variables from a store for all elements.
 * @param store The store from which to dump.
 */
void dump_aux_vars (const SG::IConstAuxStore& store)
{
  size_t sz = store.size();
  for (size_t i = 0; i < sz; i++) {
    std::cout << "=== Element " << i << "\n";
    dump_aux_vars (store, i);
  }
}


/**
 * @brief Dump aux variables from a store for all elements.
 * @param store The store from which to dump.
 */
void dump_aux_vars (const SG::IConstAuxStore* store)
{
  dump_aux_vars (*store);
}


/**
 * @brief Dump aux variables from a vector for a single element.
 * @param vec The vector from which to dump.
 * @param i The index of the element to dump.
 */
void dump_aux_vars (const SG::AuxVectorData& vec, size_t i)
{
  const SG::IConstAuxStore* store = vec.getConstStore();
  if (store)
    dump_aux_vars (*store, i);
}


/**
 * @brief Dump aux variables from a vector for a single element.
 * @param vec The vector from which to dump.
 * @param i The index of the element to dump.
 */
void dump_aux_vars (const SG::AuxVectorData* vec, size_t i)
{
  dump_aux_vars (*vec, i);
}


/**
 * @brief Dump aux variables from a vector for all elements.
 * @param vec The vector from which to dump.
 */
void dump_aux_vars (const SG::AuxVectorData& vec)
{
  const SG::IConstAuxStore* store = vec.getConstStore();
  if (store)
    dump_aux_vars (*store);
}


/**
 * @brief Dump aux variables from a vector for all elements.
 * @param vec The vector from which to dump.
 */
void dump_aux_vars (const SG::AuxVectorData* vec)
{
  dump_aux_vars (*vec);
}


/**
 * @brief Dump aux variables for an element.
 * @param elt The element from which to dump.
 */
void dump_aux_vars (const SG::AuxElement& elt)
{
  const SG::AuxVectorData* cont = elt.container();
  if (cont)
    dump_aux_vars (*cont, elt.index());
}


/**
 * @brief Dump aux variables for an element.
 * @param elt The element from which to dump.
 */
void dump_aux_vars (const SG::AuxElement* elt)
{
  dump_aux_vars (*elt);
}


} // namespace SG
