/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/AuxTypeRegistry.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2013
 * @brief Handle mappings between names and auxid_t.
 */


#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/exceptions.h"
#include "AthContainers/normalizedTypeinfoName.h"
#include "AthContainers/tools/AuxTypeVector.h"
#include "AthContainers/JaggedVec.h"
#include "AthContainers/tools/error.h"
#include "AthContainers/tools/threading.h"
#include "AthContainers/tools/concurrent_vector.h"
#include "CxxUtils/ConcurrentStrMap.h"
#include "CxxUtils/SimpleUpdater.h"
#include "CxxUtils/checker_macros.h"
#include <cassert>
#include <sstream>
#include <cstring>


namespace SG {


/**
 * @Brief Implementation class for @c AuxTypeRegistry.
 *
 * We split this off in order to reduce compile-time dependencies
 * on tbb and ConcurrentStrMap.  Since this can get accessed frequently,
 * but is a singleton, we don't use the more commom pimpl idiom.
 * Instead, we put the implementation stuff in a derived class.
 * The function that creates the singleton actually makes an instance
 * of the derived class, and we static_cast to the derived class
 * where needed.  Since instances of this cannot be created anywhere
 * else, this is safe.
 */
class AuxTypeRegistryImpl
  : public AuxTypeRegistry
{
public:
  /**
   * @brief Constructor.
   *
   * Populates the type -> factory mappings for standard C++ types.
   */
  AuxTypeRegistryImpl();


  /**
   * @brief Destructor.
   *
   * Delete factory instances.
   */
  ~AuxTypeRegistryImpl();


  using AuxTypeRegistry::addFactory;
  using AuxTypeRegistry::getFactory;


  /**
   * @brief Look up a name -> @c auxid_t mapping.
   * @param name The name of the aux data item.
   * @param clsname The name of its associated class.  May be blank.
   * @param flags Optional flags qualifying the type.  See above.
   * @param linkedVariable auxid of a linked variable, or null_auxid.
   * @param ti The type of this aux data item.
   * @param ti_alloc The type of the vector allocator.
   * @param alloc_name The name of the vector allocator.
   *                   Used only if ti_alloc is null.
   * @param makeFactory Function to create a factory for this type, if needed.
   *                    May return 0 if the type is unknown.
   *
   *
   * If the aux data item already exists, check to see if the provided
   * type matches the type that was used before.  If so, then set
   * return the auxid; otherwise, throw @c SG::ExcAuxTypeMismatch.
   *
   * If the aux data item does not already exist, then see if we
   * have a factory registered for this @c type_info.  If not, then
   * call @c makeFactory and use what it returns.  If that returns 0,
   * then fail and return null_auxid.  Otherwise, assign a new auxid
   * and return it.
   */
  SG::auxid_t
  findAuxID (const std::string& name,
             const std::string& clsname,
             const Flags flags,
             const SG::auxid_t linkedVariable,
             const std::type_info& ti,
             const std::type_info* ti_alloc,
             const std::string* alloc_name,
             std::unique_ptr<IAuxTypeVectorFactory> (AuxTypeRegistry::*makeFactory) () const);


  /**
   * @brief Add a new type -> factory mapping.  (external locking)
   * @param lock The registry lock.
   * @param ti Type of the vector element.
   * @param ti_alloc The type of the vector allocator
   * @param factory The factory instance.  Ownership is not taken.
   *
   * This records that @c factory can be used to construct vectors with
   * an element type of @c ti.  If a mapping already exists, the new
   * factory is discarded, unless the old one is a dynamic factory and
   * the new one isn't, in which case the new replaces the old one.
   */
  const IAuxTypeVectorFactory*
  addFactory (lock_t& lock,
              const std::type_info& ti,
              const std::type_info& ti_alloc,
              const IAuxTypeVectorFactory* factory);


  /**
   * @brief Add a new type -> factory mapping.  (external locking)
   * @param lock The registry lock.
   * @param ti Type of the vector element.
   * @param ti_alloc_name The name of the vector allocator type.
   * @param factory The factory instance.
   *
   * This records that @c factory can be used to construct vectors with
   * an element type of @c ti.  If a mapping already exists, the new
   * factory is discarded, unless the old one is a dynamic factory and
   * the new one isn't, in which case the new replaces the old one.
   */
  const IAuxTypeVectorFactory*
  addFactory (lock_t& /*lock*/,
              const std::type_info& ti,
              const std::string& ti_alloc_name,
              std::unique_ptr<const IAuxTypeVectorFactory> factory);


  /**
   * @brief Check for valid variable name.
   * @param name Name to check.
   *
   * Require that NAME be not empty, contains only alphanumeric characters plus
   * underscore, and first character is not a digit.
   */
  static bool checkName (const std::string& s);


  /**
   * @brief Return the vector factory for a given vector element type.
   *        (External locking.)
   * @param lock The registry lock.
   * @param ti The type of the vector element.
   * @param ti_alloc The type of the vector allocator
   *
   * Returns nullptr if the type is not known.
   * (Use @c addFactory to add new mappings.)
   */
  const IAuxTypeVectorFactory*  getFactory (lock_t& lock,
                                            const std::type_info& ti,
                                            const std::type_info& ti_alloc);


  /// Hold information about one aux data item.
  struct typeinfo_t
  {
    /// Factory object.
    AthContainers_detail::atomic<const IAuxTypeVectorFactory*> m_factory;

    /// Type of the aux data item.
    const std::type_info* m_ti;

    /// Type of the vector allocator.   May be null for a dynamic type;
    const std::type_info* m_ti_alloc;

    /// Name of the vector allocator.
    std::string m_alloc_name;

    /// Aux data name.
    std::string m_name;

    /// Class name associated with this aux data item.  May be blank.
    std::string m_clsname;

    /// auxid of a linked variable, or null_auxid.
    auxid_t m_linked;

    /// Additional type flags.
    Flags m_flags;

    /// Check that the allocator type for this entry matches
    /// the requested type.
    bool checkAlloc (const std::type_info* ti_alloc,
                     const std::string* alloc_name) const;
  };


  /// Table of aux data items, indexed by @c auxid.
  // A concurrent vector, so we don't need to take a lock to read it.
  AthContainers_detail::concurrent_vector<typeinfo_t> m_types;

  /// Map from name -> auxid.
  using id_map_t = CxxUtils::ConcurrentStrMap<SG::auxid_t, CxxUtils::SimpleUpdater>;
  id_map_t m_auxids;

  /// Map from type_info name + allocator ti name -> IAuxTypeVectorFactory.
  using ti_map_t = CxxUtils::ConcurrentStrMap<const IAuxTypeVectorFactory*, CxxUtils::SimpleUpdater>;
  ti_map_t m_factories;

  /// Hold additional factory instances we need to delete.
  std::vector<const IAuxTypeVectorFactory*> m_oldFactories;

  /// Save the information provided by @c setInputRenameMap.
  /// Each entry is of the form   KEY.DECOR -> DECOR_RENAMED
  typedef std::unordered_map<std::string, std::string> renameMap_t;
  renameMap_t m_renameMap;

  // Map from the TI name for T to the TI for AuxAllocator_t<T>.
  // Filled in by addFactory().  Used in findAuxID() if the allocator
  // type is not specified.
  using allocMap_t = CxxUtils::ConcurrentStrMap<const std::type_info*, CxxUtils::SimpleUpdater>;
  allocMap_t m_allocMap;

  /// Mutex controlling access to the registry.
  // We originally used an upgrading mutex here.
  // But that's relatively slow, and most of the locked sections are short,
  // so it's not really a win.
  // This guards write access to all members, and read access to all members
  // except for m_types.
  mutable mutex_t m_mutex;
};


/**
 * @brief Constructor.
 *
 * Populates the type -> factory mappings for standard C++ types.
 */
AuxTypeRegistryImpl::AuxTypeRegistryImpl()
  : m_auxids (id_map_t::Updater_t()),
    m_factories (ti_map_t::Updater_t()),
    m_allocMap (allocMap_t::Updater_t())
{
  m_types.reserve (auxid_set_size_hint);

  // Make sure we have factories registered for common C++ types.
#define ADD_FACTORY(T) AuxTypeRegistry::addFactory(typeid(T), typeid(AuxAllocator_t<T>), std::make_unique<AuxTypeVectorFactory<T> >())
  ADD_FACTORY (bool);
  ADD_FACTORY (char);
  ADD_FACTORY (unsigned char);
  ADD_FACTORY (short);
  ADD_FACTORY (unsigned short);
  ADD_FACTORY (int);
  ADD_FACTORY (unsigned int);
  ADD_FACTORY (long);
  ADD_FACTORY (unsigned long);
  ADD_FACTORY (long long);
  ADD_FACTORY (unsigned long long);
  ADD_FACTORY (float);
  ADD_FACTORY (double);
  ADD_FACTORY (std::string);

  ADD_FACTORY (std::vector<char>);
  ADD_FACTORY (std::vector<unsigned char>);
  ADD_FACTORY (std::vector<int>);
  ADD_FACTORY (std::vector<unsigned int>);
  ADD_FACTORY (std::vector<float>);
  ADD_FACTORY (std::vector<double>);

  ADD_FACTORY (SG::JaggedVecElt<char>);
  ADD_FACTORY (SG::JaggedVecElt<unsigned char>);
  ADD_FACTORY (SG::JaggedVecElt<short>);
  ADD_FACTORY (SG::JaggedVecElt<unsigned short>);
  ADD_FACTORY (SG::JaggedVecElt<int>);
  ADD_FACTORY (SG::JaggedVecElt<unsigned int>);
  ADD_FACTORY (SG::JaggedVecElt<long>);
  ADD_FACTORY (SG::JaggedVecElt<unsigned long>);
  ADD_FACTORY (SG::JaggedVecElt<long long>);
  ADD_FACTORY (SG::JaggedVecElt<unsigned long long>);
  ADD_FACTORY (SG::JaggedVecElt<float>);
  ADD_FACTORY (SG::JaggedVecElt<double>);
  ADD_FACTORY (SG::JaggedVecElt<std::string>);
#undef ADD_FACTORY
}


/**
 * @brief Destructor.
 *
 * Delete factory instances.
 */
AuxTypeRegistryImpl::~AuxTypeRegistryImpl()
{
  // not using reference, because our iterator doesn't return a reference
  for (auto p : m_factories) {
    if (p.first.find (";TI;") == std::string::npos) {
      delete p.second;
    }
  }
  for (const IAuxTypeVectorFactory* p : m_oldFactories)
    delete p;
}


/**
 * @brief Look up a name -> @c auxid_t mapping.
 * @param name The name of the aux data item.
 * @param clsname The name of its associated class.  May be blank.
 * @param flags Optional flags qualifying the type.  See above.
 * @param linkedVariable auxid of a linked variable, or null_auxid.
 * @param ti The type of this aux data item.
 * @param ti_alloc The type of the vector allocator.
 * @param alloc_name The name of the vector allocator.
 *                   Used only if ti_alloc is null.
 * @param makeFactory Function to create a factory for this type, if needed.
 *                    May return 0 if the type is unknown.
 *
 *
 * If the aux data item already exists, check to see if the provided
 * type matches the type that was used before.  If so, then set
 * return the auxid; otherwise, throw @c SG::ExcAuxTypeMismatch.
 *
 * If the aux data item does not already exist, then see if we
 * have a factory registered for this @c type_info.  If not, then
 * call @c makeFactory and use what it returns.  If that returns 0,
 * then fail and return null_auxid.  Otherwise, assign a new auxid
 * and return it.
 */
SG::auxid_t
AuxTypeRegistryImpl::findAuxID (const std::string& name,
                                const std::string& clsname,
                                const Flags flags,
                                const SG::auxid_t linkedVariable,
                                const std::type_info& ti,
                                const std::type_info* ti_alloc,
                                const std::string* alloc_name,
                                std::unique_ptr<IAuxTypeVectorFactory> (AuxTypeRegistry::*makeFactory) () const)
{
  // The extra test here is to avoid having to copy a string
  // in the common case where clsname is blank.
  const std::string& key = clsname.empty() ? name : makeKey (name, clsname);


  // Fast path --- try without acquiring the lock.
  {
    id_map_t::const_iterator i = m_auxids.find (key);
    if (i != m_auxids.end()) {
      typeinfo_t& m = m_types[i->second];
      if (!(CxxUtils::test (m.m_flags, Flags::Atomic) &&
            !CxxUtils::test (flags, Flags::Atomic)) &&
          (CxxUtils::test (m.m_flags, Flags::Linked) == CxxUtils::test (flags, Flags::Linked)) &&
          (linkedVariable == m.m_linked) &&
          (&ti == m.m_ti || strcmp(ti.name(), m.m_ti->name()) == 0) &&
          m.checkAlloc (ti_alloc, alloc_name) &&
          !(*m.m_factory).isDynamic())
      {
        return i->second;
      }
    }
  }

  // Something went wrong.  Acquire the lock and try again.
  lock_t lock (m_mutex);
  id_map_t::const_iterator i = m_auxids.find (key);
  if (i != m_auxids.end()) {
    typeinfo_t& m = m_types[i->second];

    if ((CxxUtils::test (m.m_flags, Flags::Atomic) &&
         !CxxUtils::test (flags, Flags::Atomic)) ||
        (CxxUtils::test (m.m_flags, Flags::Linked) != CxxUtils::test (flags, Flags::Linked)))
    {
      throw SG::ExcFlagMismatch (i->second, ti, m.m_flags, flags);
    }

    if (linkedVariable != m.m_linked)
    {
      throw SG::ExcLinkMismatch (i->second, ti, m.m_linked, linkedVariable);
    }

    // By all rights, these two tests should be redundant.
    // However, there are cases where we see distinct @c type_info objects
    // for the same type.  This is usually associated with dictionaries
    // being loaded `too early,' during python configuration processing.
    // It's a C++ standard violation for this to ever happen, but it's
    // not clear that it's feasible to actually eliminate the possibility
    // of this happening.  So if the @c type_info instances differ,
    // we still accept the match as long as the names are the same.
    if ((&ti == m.m_ti || strcmp(ti.name(), m.m_ti->name()) == 0) &&
        m.checkAlloc (ti_alloc, alloc_name))
    {
      // Try to upgrade a dynamic factory.
      if ((*m.m_factory).isDynamic()) {
        std::unique_ptr<IAuxTypeVectorFactory> fac2 = (*this.*makeFactory)();
        if (fac2) {
          if (!ti_alloc) {
            ti_alloc = fac2->tiAlloc();
          }
          std::string allocName = fac2->tiAllocName();
          m.m_factory = addFactory (lock, ti, allocName, std::move (fac2));
          if (ti_alloc) {
            m.m_factory = addFactory (lock, ti, *ti_alloc, m.m_factory);
          }
        }
      }
      return i->second;
    }
    if( *m.m_ti != typeid(SG::AuxTypePlaceholder) ) {
      throw SG::ExcAuxTypeMismatch (i->second, ti, *m.m_ti,
                                    ti_alloc ? SG::normalizedTypeinfoName (*ti_alloc) : (alloc_name ? *alloc_name : ""),
                                    m.m_alloc_name);
    }
    // fall through, get a new auxid and real type info
    // new auxid needed so a new data vector is created in the AuxStore
  }

  // Verify now that the variable names are ok.
  if (!(flags & Flags::SkipNameCheck) &&
      (!checkName (name) ||
       (!clsname.empty() && !checkName (clsname))))
  {
    throw ExcBadVarName (key);
  }

  const IAuxTypeVectorFactory* fac = nullptr;
  if (ti_alloc) {
    fac = getFactory (lock, ti, *ti_alloc);
  }
  else if (alloc_name) {
    fac = getFactory (ti, *alloc_name);
  }
  else {
    std::string def_alloc_name = SG::auxAllocatorNamePrefix + SG::normalizedTypeinfoName (ti);
    if (def_alloc_name[def_alloc_name.size()-1] == '>') def_alloc_name += ' ';
    def_alloc_name += '>';
    fac = getFactory (ti,def_alloc_name);
  }

  if (!fac || fac->isDynamic()) {
    std::unique_ptr<IAuxTypeVectorFactory> fac2 = (*this.*makeFactory)();
    if (fac2) {
      if (!ti_alloc) {
        ti_alloc = fac2->tiAlloc();
      }
      std::string allocName = fac2->tiAllocName();
      fac = addFactory (lock, ti, allocName, std::move (fac2));
      if (ti_alloc) {
        fac = addFactory (lock, ti, *ti_alloc, fac);
      }
    }
  }
  if (!fac) return null_auxid;
  if (!ti_alloc) ti_alloc = fac->tiAlloc();
  SG::auxid_t auxid = m_types.size();
  m_types.resize (auxid+1);
  typeinfo_t& t = m_types[auxid];
  t.m_name = name;
  t.m_clsname = clsname;
  t.m_ti = &ti;
  t.m_ti_alloc = ti_alloc;
  t.m_alloc_name = fac->tiAllocName();
  t.m_factory = fac;
  t.m_flags = (flags & ~Flags::SkipNameCheck);
  t.m_linked = linkedVariable;
  if (linkedVariable != SG::null_auxid) {
    if (!isLinked (linkedVariable)) std::abort();
  }
  AthContainers_detail::fence_seq_cst();
  m_auxids.insert_or_assign (key, auxid);

  return auxid;
}


/**
 * @brief Add a new type -> factory mapping.  (external locking)
 * @param lock The registry lock.
 * @param ti Type of the vector element.
 * @param ti_alloc The type of the vector allocator
 * @param factory The factory instance.  Ownership is not taken.
 *
 * This records that @c factory can be used to construct vectors with
 * an element type of @c ti.  If a mapping already exists, the new
 * factory is discarded, unless the old one is a dynamic factory and
 * the new one isn't, in which case the new replaces the old one.
 */
const IAuxTypeVectorFactory*
AuxTypeRegistryImpl::addFactory (lock_t& /*lock*/,
                                 const std::type_info& ti,
                                 const std::type_info& ti_alloc,
                                 const IAuxTypeVectorFactory* factory)
{
  std::string key = std::string (ti.name()) + ";TI;" + ti_alloc.name();
  ti_map_t::const_iterator it = m_factories.find (key);
  if (it != m_factories.end()) {
    if (it->second->isDynamic() && !factory->isDynamic()) {
      // Replacing a dynamic factory with a non-dynamic one.
      // The string version should put it to m_oldFactories.
      m_factories.insert_or_assign (key, factory);
    }
    else {
      factory = it->second;
    }
  }
  else
    m_factories.insert_or_assign (key, factory);

  if (SG::normalizedTypeinfoName (ti_alloc).starts_with(
                          SG::auxAllocatorNamePrefix))
  {
    m_allocMap.insert_or_assign (ti.name(), &ti_alloc);
  }

  return factory;
}


/**
 * @brief Add a new type -> factory mapping.  (external locking)
 * @param lock The registry lock.
 * @param ti Type of the vector element.
 * @param ti_alloc_name The name of the vector allocator type.
 * @param factory The factory instance.
 *
 * This records that @c factory can be used to construct vectors with
 * an element type of @c ti.  If a mapping already exists, the new
 * factory is discarded, unless the old one is a dynamic factory and
 * the new one isn't, in which case the new replaces the old one.
 */
const IAuxTypeVectorFactory*
AuxTypeRegistryImpl::addFactory (lock_t& /*lock*/,
                                 const std::type_info& ti,
                                 const std::string& ti_alloc_name,
                                 std::unique_ptr<const IAuxTypeVectorFactory> factory)
{
  std::string key = ti.name();
  key += ';';
  key += ti_alloc_name;
  ti_map_t::const_iterator it = m_factories.find (key);
  const IAuxTypeVectorFactory* fac = factory.get();
  if (it != m_factories.end()) {
    if (it->second->isDynamic() && !factory->isDynamic()) {
      // Replacing a dynamic factory with a non-dynamic one.
      // But don't delete the old one, since it might still be referenced.
      // Instead, push it on a vector to remember it so we can delete
      // it later.
      m_oldFactories.push_back (it->second);
      m_factories.insert_or_assign (key, factory.release());
    }
    else {
      fac = it->second;
    }
  }
  else
    m_factories.insert_or_assign (key, factory.release());

  return fac;
}


/**
 * @brief Check for valid variable name.
 * @param name Name to check.
 *
 * Require that NAME be not empty, contains only alphanumeric characters plus
 * underscore, and first character is not a digit.
 */
bool AuxTypeRegistryImpl::checkName (const std::string& s)
{
  static const std::string chars1 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
  static const std::string chars2 = chars1 + "0123456789";

  if (s.empty()) return false;
  if (chars1.find (s[0]) == std::string::npos) return false;
  return s.find_first_not_of (chars2, 1) == std::string::npos;
}


/**
 * @brief Return the vector factory for a given vector element type.
 *        (External locking.)
 * @param lock The registry lock.
 * @param ti The type of the vector element.
 * @param ti_alloc The type of the vector allocator
 *
 * Returns nullptr if the type is not known.
 * (Use @c addFactory to add new mappings.)
 */
const IAuxTypeVectorFactory*
AuxTypeRegistryImpl::getFactory (lock_t& lock,
                                 const std::type_info& ti,
                                 const std::type_info& ti_alloc)
{
  std::string key = ti.name();
  key += ";TI;";
  key += ti_alloc.name();
  ti_map_t::const_iterator it = m_factories.find (key);
  if (it != m_factories.end())
    return it->second;

  std::string name = SG::normalizedTypeinfoName (ti_alloc);
  const IAuxTypeVectorFactory* fac = getFactory (ti, name);
  if (fac) {
    // We only really need to be holding the lock here, but doing things
    // otherwise would require some code duplication.
    addFactory (lock, ti, ti_alloc, fac);
  }
  return fac;
}


/**
 * @brief Check that the allocator type for this entry matches
 *        the requested type.
 */
bool AuxTypeRegistryImpl::typeinfo_t::checkAlloc (const std::type_info* ti_alloc,
                                                  const std::string* alloc_name) const
{
  if (ti_alloc && m_ti_alloc) {
    if (ti_alloc == m_ti_alloc) return true;
    return strcmp (ti_alloc->name(), m_ti_alloc->name()) == 0;
  }
  if (alloc_name) {
    return *alloc_name == m_alloc_name;
  }
  if (ti_alloc) {
    return SG::normalizedTypeinfoName (*ti_alloc) == m_alloc_name;
  }
  return true;
}


//***************************************************************************


/**
 * @brief Return the singleton registry instance.
 */
AuxTypeRegistry& AuxTypeRegistry::instance()
{
  static AuxTypeRegistryImpl auxTypeRegistry ATLAS_THREAD_SAFE;
  return auxTypeRegistry;
}


/**
 * @brief Return the total number of registered auxiliary variable.
 *
 * (This will be one more than the current largest auxid.)
 */
size_t AuxTypeRegistry::numVariables() const
{
  auto* impl = static_cast<const AuxTypeRegistryImpl*> (this);
  return impl->m_types.size();
}


/**
 * @brief Look up a name -> @c auxid_t mapping.
 * @param ti Type of the aux data item.
 * @param name The name of the aux data item.
 * @param clsname The name of its associated class.  May be blank.
 * @param flags Optional flags qualifying the type.  See above.
 * @param linkedVariable auxid of a linked variable, or null_auxid.
 *
 * The type of the item is given by @a ti.
 * Return @c null_auxid if we don't know how to make vectors of @a ti.
 * (Use @c addFactory to register additional types.)
 * If an item with the same name was previously requested
 * with a different type, then throw @c SG::ExcAuxTypeMismatch.
 */
SG::auxid_t AuxTypeRegistry::getAuxID (const std::type_info& ti,
                                       const std::string& name,
                                       const std::string& clsname /*= ""*/,
                                       const Flags flags /*= Flags::None*/,
                                       const SG::auxid_t linkedVariable /*= SG::null_auxid*/)
{
  auto impl = static_cast<AuxTypeRegistryImpl*> (this);
  return impl->findAuxID (name, clsname, flags, linkedVariable,
                          ti, nullptr, nullptr,
                          &AuxTypeRegistry::makeFactoryNull);
}


/**
 * @brief Look up a name -> @c auxid_t mapping, specifying allocator.
 * @param ti_alloc Type of the vector allocator.
 * @param ti Type of the aux data item.
 * @param name The name of the aux data item.
 * @param clsname The name of its associated class.  May be blank.
 * @param flags Optional flags qualifying the type.  See above.
 * @param linkedVariable auxid of a linked variable, or null_auxid.
 * @param makeFactory Function to create a factory for this type, if needed.
 *                    May return 0 if the type is unknown.
 *
 * The type of the item is given by @a ti.
 * Return @c null_auxid if we don't know how to make vectors of @a ti.
 * (Use @c addFactory to register additional types.)
 * If an item with the same name was previously requested
 * with a different type, then throw @c SG::ExcAuxTypeMismatch.
 */
SG::auxid_t AuxTypeRegistry::getAuxID (const std::type_info& ti_alloc,
                                       const std::type_info& ti,
                                       const std::string& name,
                                       const std::string& clsname /*= ""*/,
                                       const Flags flags /*= Flags::None*/,
                                       const SG::auxid_t linkedVariable /*= SG::null_auxid*/,
                                       std::unique_ptr<IAuxTypeVectorFactory> (AuxTypeRegistry::*makeFactory) () const /*= &AuxTypeRegistry::makeFactoryNull*/)
{
  auto impl = static_cast<AuxTypeRegistryImpl*> (this);
  return impl->findAuxID (name, clsname, flags, linkedVariable,
                          ti, &ti_alloc, nullptr,
                          makeFactory);
}


/**
 * @brief Look up a name -> @c auxid_t mapping, specifying allocator.
 * @param alloc_name Name of the vector allocator type.
 * @param ti Type of the aux data item.
 * @param name The name of the aux data item.
 * @param clsname The name of its associated class.  May be blank.
 * @param flags Optional flags qualifying the type.  See above.
 * @param linkedVariable auxid of a linked variable, or null_auxid.
 *
 * The type of the item is given by @a ti.
 * Return @c null_auxid if we don't know how to make vectors of @a ti.
 * (Use @c addFactory to register additional types.)
 * If an item with the same name was previously requested
 * with a different type, then throw @c SG::ExcAuxTypeMismatch.
 */
SG::auxid_t AuxTypeRegistry::getAuxID (const std::string& alloc_type,
                                       const std::type_info& ti,
                                       const std::string& name,
                                       const std::string& clsname /*= ""*/,
                                       const Flags flags /*= Flags::None*/,
                                       const SG::auxid_t linkedVariable /*= SG::null_auxid*/)
{
  auto impl = static_cast<AuxTypeRegistryImpl*> (this);
  return impl->findAuxID (name, clsname, flags, linkedVariable,
                          ti, nullptr, &alloc_type,
                          &AuxTypeRegistry::makeFactoryNull);
}


/**
 * @brief Look up a name -> @c auxid_t mapping.
 * @param name The name of the aux data item.
 * @param clsname The name of its associated class.  May be blank.
 *
 * Will only find an existing @c auxid_t; unlike @c getAuxID,
 * this won't make a new one.  If the item isn't found, this
 * returns @c null_auxid.
 */
SG::auxid_t
AuxTypeRegistry::findAuxID (const std::string& name,
                            const std::string& clsname) const
{
  auto impl = static_cast<const AuxTypeRegistryImpl*> (this);

  // No locking needed here.
  // The extra test here is to avoid having to copy a string
  // in the common case where clsname is blank.
  AuxTypeRegistryImpl::id_map_t::const_iterator i =
    impl->m_auxids.find (clsname.empty() ?
                         name :
                         makeKey (name, clsname));
  if (i != impl->m_auxids.end()) {
    return i->second;
  }
  return null_auxid;
}


/**
 * @brief Verify type for an aux variable.
 * @param auxid The ID of the variable to check.
 * @param ti Type of the aux data item.
 * @param ti_alloc Type of the vector allocator.
 * @param flags Optional flags qualifying the type.  See above.
 *
 * If the type of @c auxid is not compatible with the supplied
 * types @c ti / @c ti_alloc, then throw a @c SG::ExcAuxTypeMismatch exception.
 * Also may throw @c SG::ExcFlagMismatch.
 */
void AuxTypeRegistry::checkAuxID (const SG::auxid_t auxid,
                                  const std::type_info& ti,
                                  const std::type_info& ti_alloc,
                                  const Flags flags)
{
  auto impl = static_cast<AuxTypeRegistryImpl*> (this);
  AuxTypeRegistryImpl::typeinfo_t& m = impl->m_types.at (auxid);

  if ( ! ((&ti == m.m_ti || strcmp(ti.name(), m.m_ti->name()) == 0) &&
          m.checkAlloc (&ti_alloc, nullptr)))
  {
    throw SG::ExcAuxTypeMismatch (auxid, ti, *m.m_ti,
                                  SG::normalizedTypeinfoName (ti_alloc),
                                  m.m_alloc_name);
  }
  if ((CxxUtils::test (m.m_flags, Flags::Atomic) &&
       !CxxUtils::test (flags, Flags::Atomic)) ||
      (CxxUtils::test (m.m_flags, Flags::Linked) != CxxUtils::test (flags, Flags::Linked)))
  {
    throw SG::ExcFlagMismatch (auxid, ti, m.m_flags, flags);
  }
}


/**
 * @brief Construct a new vector to hold an aux item.
 * @param auxid The desired aux data item.
 * @param size Initial size of the new vector.
 * @param capacity Initial capacity of the new vector.
 */
std::unique_ptr<IAuxTypeVector>
AuxTypeRegistry::makeVector (SG::auxid_t auxid,
                             size_t size,
                             size_t capacity) const
{
  const SG::IAuxTypeVectorFactory* factory = getFactory (auxid);
  assert (factory != 0);
  return factory->create (auxid, size, capacity, isLinked (auxid));
}


/**
 * @brief Construct an @c IAuxTypeVector object from a vector.
 * @param data The vector object.
 * @param linkedVector The interface for another variable linked to this one,
 *                     or nullptr if there isn't one.
 *                     (We do not take ownership.)
 * @param isPacked If true, @c data is a @c PackedContainer.
 * @param ownFlag If true, the newly-created IAuxTypeVector object
 *                will take ownership of @c data.
 *
 * If the element type is T, then @c data should be a pointer
 * to a std::vector<T> object, which was obtained with @c new.
 * But if @c isPacked is @c true, then @c data
 * should instead point at an object of type @c SG::PackedContainer<T>.
 *
 * Returns a newly-allocated object.
 */
std::unique_ptr<IAuxTypeVector>
AuxTypeRegistry::makeVectorFromData (SG::auxid_t auxid,
                                     void* data,
                                     IAuxTypeVector* linkedVector,
                                     bool isPacked,
                                     bool ownMode) const
{
  const SG::IAuxTypeVectorFactory* factory = getFactory (auxid);
  assert (factory != 0);
  return factory->createFromData (auxid, data, linkedVector,
                                  isPacked, ownMode, isLinked (auxid));
}


/**
 * @brief Return the key used to look up an entry in m_auxids.
 * @param name The name of the aux data item.
 * @param clsname The name of its associated class.  May be blank.
 */
std::string AuxTypeRegistry::makeKey (const std::string& name,
                                      const std::string& clsname)
{
  if (clsname.empty()) {
    return name;
  }
  std::string output = clsname;
  output += "::";
  output += name;
  return output;
}



/**
 * @brief Return the name of an aux data item.
 * @param auxid The desired aux data item.
 */
std::string AuxTypeRegistry::getName (SG::auxid_t auxid) const
{
  auto impl = static_cast<const AuxTypeRegistryImpl*> (this);
  if (auxid >= impl->m_types.size())
    return "";
  return impl->m_types[auxid].m_name;
}


/**
 * @brief Return the class name associated with an aux data item
 *        (may be blank).
 * @param auxid The desired aux data item.
 */
std::string AuxTypeRegistry::getClassName (SG::auxid_t auxid) const
{
  auto impl = static_cast<const AuxTypeRegistryImpl*> (this);
  if (auxid >= impl->m_types.size())
    return "";
  return impl->m_types[auxid].m_clsname;
}


/**
 * @brief Return the type of an aux data item.
 * @param auxid The desired aux data item.
 */
const std::type_info* AuxTypeRegistry::getType (SG::auxid_t auxid) const
{
  auto impl = static_cast<const AuxTypeRegistryImpl*> (this);
  if (auxid >= impl->m_types.size())
    return 0;
  return impl->m_types[auxid].m_ti;
}


/**
 * @brief Return the type name of an aux data item.
 * @param auxid The desired aux data item.
 *
 * Returns an empty string if the type is not known.
 */
std::string AuxTypeRegistry::getTypeName (SG::auxid_t auxid) const
{
  auto impl = static_cast<const AuxTypeRegistryImpl*> (this);
  if (auxid >= impl->m_types.size())
    return "";
  return normalizedTypeinfoName (*impl->m_types[auxid].m_ti);
}


/**
 * @brief Return the type of the STL vector used to hold an aux data item.
 * @param auxid The desired aux data item.
 */
const std::type_info* AuxTypeRegistry::getVecType (SG::auxid_t auxid) const
{
  const SG::IAuxTypeVectorFactory* factory = getFactory (auxid);
  if (factory)
    return factory->tiVec();
  return 0;
}


/**
 * @brief Return the type name of the STL vector used to hold an aux data item.
 * @param auxid The desired aux data item.
 *
 * Returns an empty string if the type is not known.
 */
std::string AuxTypeRegistry::getVecTypeName (SG::auxid_t auxid) const
{
  const SG::IAuxTypeVectorFactory* factory = getFactory (auxid);
  if (factory)
    return normalizedTypeinfoName (*factory->tiVec());
  return "";
}


/**
 * @brief Return the type of the vector allocator.
 * @param auxid The desired aux data item.
 */
const std::type_info* AuxTypeRegistry::getAllocType (SG::auxid_t auxid) const
{
  auto impl = static_cast<const AuxTypeRegistryImpl*> (this);
  if (auxid >= impl->m_types.size())
    return 0;
  return impl->m_types[auxid].m_ti_alloc;
}


/**
 * @brief Return size of an element in the STL vector.
 * @param auxid The desired aux data item.
 */
size_t AuxTypeRegistry::getEltSize (SG::auxid_t auxid) const
{
  const SG::IAuxTypeVectorFactory* factory = getFactory (auxid);
  if (factory)
    return factory->getEltSize();
  return 0;
}


/**
 * @brief Return flags associated with an auxiliary variable.
 * @param auxid The desired aux data item.
 */
AuxVarFlags AuxTypeRegistry::getFlags (SG::auxid_t auxid) const
{
  auto impl = static_cast<const AuxTypeRegistryImpl*> (this);
  if (auxid >= impl->m_types.size())
    return Flags::None;
  return impl->m_types[auxid].m_flags;
}


/**
 * @brief Return the auxid if the linked variable, if there is one.
 * @param auxid The aux data item to test.
 *
 * Returns null_auxid if @c auxid is invalid or it doesn't have
 * a linked variable.
 */
SG::auxid_t AuxTypeRegistry::linkedVariable (SG::auxid_t auxid) const
{
  auto impl = static_cast<const AuxTypeRegistryImpl*> (this);
  if (auxid >= impl->m_types.size())
    return null_auxid;
  return impl->m_types[auxid].m_linked;
}


/**
 * @brief Copy elements between vectors.
 * @param auxid The aux data item being operated on.
 * @param dst Container for the destination vector.
 * @param dst_index Index of the first destination element in the vector.
 * @param src Container for the source vector.
 * @param src_index Index of the first source element in the vector.
 * @param n Number of elements to copy.
 *
 * @c dst and @ src can be either the same or different.
 */
void AuxTypeRegistry::copy (SG::auxid_t auxid,
                            AuxVectorData& dst,       size_t dst_index,
                            const AuxVectorData& src, size_t src_index,
                            size_t n) const
{
  const SG::IAuxTypeVectorFactory* factory = getFactory (auxid);
  if (factory)
    factory->copy (auxid, dst, dst_index, src, src_index, n);
}


/**
 * @brief Copy elements between vectors.
 *        Apply any transformations needed for output.
 * @param auxid The aux data item being operated on.
 * @param dst Container for the destination vector.
 * @param dst_index Index of the first destination element in the vector.
 * @param src Container for the source vector.
 * @param src_index Index of the first source element in the vector.
 * @param n Number of elements to copy.
 *
 * @c dst and @ src can be either the same or different.
 */
void AuxTypeRegistry::copyForOutput (SG::auxid_t auxid,
                                     AuxVectorData& dst,       size_t dst_index,
                                     const AuxVectorData& src, size_t src_index,
                                     size_t n) const
{
  const SG::IAuxTypeVectorFactory* factory = getFactory (auxid);
  if (factory) {
    factory->copyForOutput (auxid, dst, dst_index, src, src_index, n);
  }
}


/**
 * @brief Swap elements between vectors.
 * @param auxid The aux data item being operated on.
 * @param a Container for the first vector.
 * @param aindex Index of the first element in the first vector.
 * @param b Container for the second vector.
 * @param bindex Index of the first element in the second vector.
 * @param n Number of elements to swap.
 *
 * @c a and @ b can be either the same or different.
 * However, the ranges should not overlap.
 */
void AuxTypeRegistry::swap (SG::auxid_t auxid,
                            AuxVectorData& a, size_t aindex,
                            AuxVectorData& b, size_t bindex,
                            size_t n) const
{
  const SG::IAuxTypeVectorFactory* factory = getFactory (auxid);
  if (factory)
    factory->swap (auxid, a, aindex, b, bindex, n);
}


/**
 * @brief Clear a range of elements within a vector.
 * @param auxid The aux data item being operated on.
 * @param dst Container holding the element
 * @param dst_index Index of the first element in the vector.
 * @param n Number of elements to clear.
 */
void AuxTypeRegistry::clear (SG::auxid_t auxid,
                             AuxVectorData& dst, size_t dst_index,
                             size_t n) const
{
  const SG::IAuxTypeVectorFactory* factory = getFactory (auxid);
  if (factory)
    factory->clear (auxid, dst, dst_index, n);
}


/**
 * @brief Return the vector factory for a given vector element type.
 * @param ti The type of the vector element.
 * @param ti_alloc The type of the vector allocator
 *
 * Returns nullptr if the type is not known.
 * (Use @c addFactory to add new mappings.)
 */
const IAuxTypeVectorFactory*
AuxTypeRegistry::getFactory (const std::type_info& ti,
                             const std::type_info& ti_alloc)
{
  auto impl = static_cast<AuxTypeRegistryImpl*> (this);
  lock_t lock (impl->m_mutex);
  return impl->getFactory (lock, ti, ti_alloc);
}


/**
 * @brief Return the vector factory for a given auxid.
 * @param auxid The desired aux data item.
 *
 * Returns nullptr if the type is not known.
 * (Use @c addFactory to add new mappings.)
 */
const IAuxTypeVectorFactory*
AuxTypeRegistry::getFactory (SG::auxid_t auxid) const
{
  auto impl = static_cast<const AuxTypeRegistryImpl*> (this);
  if (auxid >= impl->m_types.size())
    return 0;
  return impl->m_types[auxid].m_factory;
}


/**
 * @brief Return the vector factory for a given vector element type.
 * @param ti The type of the vector element.
 * @param alloc_name The name of the vector allocator type.
 *
 * Returns nullptr if the type is not known.
 * (Use @c addFactory to add new mappings.)
 */
const IAuxTypeVectorFactory*
AuxTypeRegistry::getFactory (const std::type_info& ti,
                             const std::string& alloc_name)
{
  auto impl = static_cast<AuxTypeRegistryImpl*> (this);

  std::string key = std::string (ti.name()) + ";" + alloc_name;
  AuxTypeRegistryImpl::ti_map_t::const_iterator it = impl->m_factories.find (key);
  if (it != impl->m_factories.end()) {
    return it->second;
  }

  return nullptr;
}


/**
 * @brief Add a new type -> factory mapping.
 * @param ti Type of the vector element.
 * @param ti_alloc The type of the vector allocator
 * @param factory The factory instance.
 *
 * This records that @c factory can be used to construct vectors with
 * an element type of @c ti.  If a mapping already exists, the new
 * factory is discarded, unless the old one is a dynamic factory and
 * the new one isn't, in which case the new replaces the old one.
 */
const IAuxTypeVectorFactory*
AuxTypeRegistry::addFactory (const std::type_info& ti,
                             const std::type_info& ti_alloc,
                             std::unique_ptr<const IAuxTypeVectorFactory> factory)
{
  auto impl = static_cast<AuxTypeRegistryImpl*> (this);
  lock_t lock (impl->m_mutex);
  std::string name = SG::normalizedTypeinfoName (ti_alloc);
  const IAuxTypeVectorFactory* fac =
    impl->addFactory (lock, ti, name, std::move(factory));
  return impl->addFactory (lock, ti, ti_alloc, fac);
}


/**
 * @brief Add a new type -> factory mapping.
 * @param ti Type of the vector element.
 * @param ti_alloc_name The name of the vector allocator type.
 * @param factory The factory instance.
 *
 * This records that @c factory can be used to construct vectors with
 * an element type of @c ti.  If a mapping already exists, the new
 * factory is discarded, unless the old one is a dynamic factory and
 * the new one isn't, in which case the new replaces the old one.
 */
const IAuxTypeVectorFactory*
AuxTypeRegistry::addFactory (const std::type_info& ti,
                             const std::string& ti_alloc_name,
                             std::unique_ptr<const IAuxTypeVectorFactory> factory)
{
  auto impl = static_cast<AuxTypeRegistryImpl*> (this);
  lock_t lock (impl->m_mutex);
  return impl->addFactory (lock, ti, ti_alloc_name, std::move (factory));
}


/**
 * @brief Constructor.
 *
 * Populates the type -> factory mappings for standard C++ types.
 */
AuxTypeRegistry::AuxTypeRegistry()
{
}


/**
 * @brief Destructor.
 */
AuxTypeRegistry::~AuxTypeRegistry()
{
}


#ifndef XAOD_STANDALONE
/**
 * @brief Declare input renaming requests.
 * @param map Map of (hashed) sgkey -> sgkey for renaming requests.
 * @param pool String pool in which the hashed keys are defined.
 *
 * This is called by @c AddressRemappingSvc when there is a request
 * to rename input objects.  It saves any requests involving renaming
 * of auxiliary variables and makes that information available via
 * @c inputRename.
 */
void
AuxTypeRegistry::setInputRenameMap (const Athena::IInputRename::InputRenameMap_t* map,
                                    const IStringPool& pool)
{
  auto impl = static_cast<AuxTypeRegistryImpl*> (this);

  lock_t lock (impl->m_mutex);
  impl->m_renameMap.clear();
  if (!map) return;
  for (const auto& p : *map) {
    sgkey_t from_sgkey = p.first;
    sgkey_t to_sgkey = p.second.m_sgkey;

    const std::string* from_str = pool.keyToString (from_sgkey);
    if (!from_str) continue;
    std::string::size_type from_dpos = from_str->find (".");
    if (from_dpos == std::string::npos || from_dpos == from_str->size()-1) continue;

    const std::string* to_str = pool.keyToString (to_sgkey);
    if (!to_str) continue;
    std::string::size_type to_dpos = to_str->find (".");
    if (to_dpos == std::string::npos || to_dpos == to_str->size()-1) continue;

    impl->m_renameMap[*from_str] = to_str->substr (to_dpos+1, std::string::npos);
  }
}
#endif


/**
 * @brief Check for an input renaming of an auxiliary variable.
 * @brief key The SG key of the object to which the variable is attached.
 * @brief name The name of the variable on the input file.
 *
 * @returns The variable name to use in the transient representation.
 * Will usually be @c name, but may be different if there was a renaming request.
 */
const std::string& AuxTypeRegistry::inputRename (const std::string& key,
                                                 const std::string& name) const
{
  auto impl = static_cast<const AuxTypeRegistryImpl*> (this);
  lock_t lock (impl->m_mutex);
  if (impl->m_renameMap.empty())
    return name;

  std::string fullkey = key + "." + name;
  auto it = impl->m_renameMap.find (fullkey);
  if (it != impl->m_renameMap.end())
    return it->second;
  return name;
}


/**
 * @brief Test if a variable name corresponds to a linked variable.
 */
bool AuxTypeRegistry::isLinkedName (const std::string& name)
{
  return name.ends_with ("_linked");
}


/**
 * @brief Given a variable name, return the name of the corresponding
 *        linked variable.
 */
std::string AuxTypeRegistry::linkedName (const std::string& name)
{
  return name + "_linked";
}


/**
 * @brief Test to see if a class name corresponds to a class
 *        with a linked variable.
 */
bool AuxTypeRegistry::classNameHasLink (const std::string& className)
{
  if (className.find ("SG::JaggedVecElt<") != std::string::npos) {
    return true;
  }
  if (className.find ("SG::PackedLink<") != std::string::npos) {
    return true;
  }
  return false;
}


} // namespace SG


