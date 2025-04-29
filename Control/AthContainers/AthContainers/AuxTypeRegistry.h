// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/AuxTypeRegistry.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2013
 * @brief Handle mappings between names and auxid_t.
 */


#ifndef ATHCONTAINERS_AUXTYPEREGISTRY_H
#define ATHCONTAINERS_AUXTYPEREGISTRY_H


#include "AthContainersInterfaces/AuxTypes.h"
#include "AthContainersInterfaces/IAuxTypeVector.h"
#include "AthContainersInterfaces/IAuxTypeVectorFactory.h"
#include "AthContainers/tools/AuxTypeVectorFactory.h"
#ifndef XAOD_STANDALONE
#include "AthenaKernel/InputRenameMap.h"
#include "AthenaKernel/IStringPool.h"
#endif
#include <cstddef>
#include <typeinfo>
#include <vector>
#include <unordered_map>
#include <functional>


namespace SG {


class AuxTypeRegistryImpl;


/**
 * @brief Handle mappings between names and auxid_t.
 *
 * Each auxiliary data item associated with a container has a name.
 * Internally, they are identified by small integers of type @c auxid_t.
 * This class handles the mapping between names and @c auxid_t's.
 * It also keeps track of the type of each aux data item, and provides
 * some generic methods for operating on this data.
 *
 * The @c auxid_t namespace is global, shared among all classes.
 * It's no problem for two classes to use aux data with the same
 * @c auxid_t, as long as the type is the same.  If they want to define
 * them as different types, though, that's a problem.  To help with this,
 * an optional class name may be supplied; this qualifies the aux data
 * name to make it unique across classes.
 *
 * This class is meant to be used as a singleton.  Use the @c instance
 * method to get the singleton instance.
 *
 * Methods are locked internally as needed, so access is thread-safe.
 * Thread-safety is however not supported in standalone builds.
 */
class AuxTypeRegistry
{
public:
  /// Additional flags to qualify an auxiliary variable.
  // Now in AuxTypes.h to avoid cyclic header dependencies.
  using Flags = SG::AuxVarFlags;


  /**
   * @brief Return the singleton registry instance.
   */
  static AuxTypeRegistry& instance();


  /**
   * @brief Return the total number of registered auxiliary variable.
   *
   * (This will be one more than the current largest auxid.)
   */
  size_t numVariables() const;


  /**
   * @brief Look up a name -> @c auxid_t mapping.
   * @param name The name of the aux data item.
   * @param clsname The name of its associated class.  May be blank.
   * @param flags Optional flags qualifying the type.  See above.
   * @param linkedVariable auxid of a linked variable, or null_auxid.
   *
   * The type of the item is given by the template parameter @c T,
   * and the @c ALLOC gives the type of the vector allocator.
   * If an item with the same name was previously requested
   * with a different type, then throw @c SG::ExcAuxTypeMismatch.
   */
  template <class T, class ALLOC = AuxAllocator_t<T> >
  SG::auxid_t getAuxID (const std::string& name,
                        const std::string& clsname = "",
                        const Flags flags = Flags::None,
                        const SG::auxid_t linkedVariable = SG::null_auxid);


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
  SG::auxid_t getAuxID (const std::type_info& ti,
                        const std::string& name,
                        const std::string& clsname = "",
                        const Flags flags = Flags::None,
                        const SG::auxid_t linkedVariable = SG::null_auxid);


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
  SG::auxid_t getAuxID (const std::type_info& ti_alloc,
                        const std::type_info& ti,
                        const std::string& name,
                        const std::string& clsname = "",
                        const Flags flags = Flags::None,
                        const SG::auxid_t linkedVariable = SG::null_auxid,
                        std::unique_ptr<IAuxTypeVectorFactory> (AuxTypeRegistry::*makeFactory) () const = &AuxTypeRegistry::makeFactoryNull);


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
  SG::auxid_t getAuxID (const std::string& alloc_type,
                        const std::type_info& ti,
                        const std::string& name,
                        const std::string& clsname = "",
                        const Flags flags = Flags::None,
                        const SG::auxid_t linkedVariable = SG::null_auxid);


  /**
   * @brief Look up a name -> @c auxid_t mapping.
   * @param name The name of the aux data item.
   * @param clsname The name of its associated class.  May be blank.
   *
   * Will only find an existing @c auxid_t; unlike @c getAuxID,
   * this won't make a new one.  If the item isn't found, this
   * returns @c null_auxid.
   */
  SG::auxid_t findAuxID (const std::string& name,
                         const std::string& clsname = "") const;


  /**
   * @brief Verify type for an aux variable.
   * @param auxid The ID of the variable to check.
   * @param flags Optional flags qualifying the type.  See above.
   *
   * If the type of @c auxid is not compatible with the supplied
   * types @c T / @c ALLOC, then throw a @c SG::ExcAuxTypeMismatch exception.
   * Also may throw @c SG::ExcFlagMismatch.
   */
  template <class T, class ALLOC = AuxAllocator_t<T> >
  void checkAuxID (const SG::auxid_t auxid,
                   const Flags flags = Flags::None);


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
  void checkAuxID (const SG::auxid_t auxid,
                   const std::type_info& ti,
                   const std::type_info& ti_alloc,
                   const Flags flags);


  /**
   * @brief Construct a new vector to hold an aux item.
   * @param auxid The desired aux data item.
   * @param size Initial size of the new vector.
   * @param capacity Initial capacity of the new vector.
   *
   * Returns a newly-allocated object.
   */
  std::unique_ptr<IAuxTypeVector> makeVector (SG::auxid_t auxid,
                                              size_t size,
                                              size_t capacity) const;


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
  std::unique_ptr<IAuxTypeVector> makeVectorFromData (SG::auxid_t auxid,
                                                      void* data,
                                                      IAuxTypeVector* linkedVector,
                                                      bool isPacked,
                                                      bool ownFlag) const;


  /**
   * @brief Return the name of an aux data item.
   * @param auxid The desired aux data item.
   */
  std::string getName (SG::auxid_t auxid) const;


  /**
   * @brief Return the class name associated with an aux data item
   *        (may be blank). 
   * @param auxid The desired aux data item.
   */
  std::string getClassName (SG::auxid_t auxid) const;


  /**
   * @brief Return the type of an aux data item.
   * @param auxid The desired aux data item.
   */
  const std::type_info* getType (SG::auxid_t auxid) const;


  /**
   * @brief Return the type name of an aux data item.
   * @param auxid The desired aux data item.
   *
   * Returns an empty string if the type is not known.
   */
  std::string getTypeName (SG::auxid_t auxid) const;


  /**
   * @brief Return the type of the STL vector used to hold an aux data item.
   * @param auxid The desired aux data item.
   */
  const std::type_info* getVecType (SG::auxid_t auxid) const;


  /**
   * @brief Return the type of the STL vector used to hold an aux data item.
   * @param auxid The desired aux data item.
   *
   * Returns an empty string if the type is not known.
   */
  std::string getVecTypeName (SG::auxid_t auxid) const;


  /**
   * @brief Return the type of the vector allocator.
   * @param auxid The desired aux data item.
   */
  const std::type_info* getAllocType (SG::auxid_t auxid) const;


  /**
   * @brief Return size of an element in the STL vector.
   * @param auxid The desired aux data item.
   */
  size_t getEltSize (SG::auxid_t auxid) const;


  /**
   * @brief Return flags associated with an auxiliary variable.
   * @param auxid The desired aux data item.
   */
  Flags getFlags (SG::auxid_t auxid) const;


  /**
   * @brief Test whether this is a linked variable.
   * @param auxid The aux data item to test.
   */
  bool isLinked (SG::auxid_t auxid) const;


  /**
   * @brief Return the auxid if the linked variable, if there is one.
   * @param auxid The aux data item to test.
   *
   * Returns null_auxid if @c auxid is invalid or it doesn't have
   * a linked variable.
   */
  SG::auxid_t linkedVariable (SG::auxid_t auxid) const;


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
  void copy (SG::auxid_t auxid,
             AuxVectorData& dst,       size_t dst_index,
             const AuxVectorData& src, size_t src_index,
             size_t n) const;


  /**
   * @brief Copy elements between vectors.
   * @param auxid The aux data item being operated on.
   * @param dst Container for the destination vector.
   *            Declared as a rvalue reference to allow passing a temporary
   *            here (such as from AuxVectorInterface).
   * @param dst_index Index of the first destination element in the vector.
   * @param src Container for the source vector.
   * @param src_index Index of the first source element in the vector.
   * @param n Number of elements to copy.
   *
   * @c dst and @ src can be either the same or different.
   */
  void copy (SG::auxid_t auxid,
             AuxVectorData&& dst,      size_t dst_index,
             const AuxVectorData& src, size_t src_index,
             size_t n) const;


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
  void copyForOutput (SG::auxid_t auxid,
                      AuxVectorData& dst,       size_t dst_index,
                      const AuxVectorData& src, size_t src_index,
                      size_t n) const;


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
  void swap (SG::auxid_t auxid,
             AuxVectorData& a, size_t aindex,
             AuxVectorData& b, size_t bindex,
             size_t n) const;


  /**
   * @brief Clear a range of elements within a vector.
   * @param auxid The aux data item being operated on.
   * @param dst Container holding the element
   * @param dst_index Index of the first element in the vector.
   * @param n Number of elements to clear.
   */
  void clear (SG::auxid_t auxid,
              AuxVectorData& dst, size_t dst_index,
              size_t n) const;


  /**
   * @brief Return the vector factory for a given vector element type.
   * @param ti The type of the vector element.
   * @param ti_alloc The type of the vector allocator.
   *
   * Returns nullptr if the type is not known.
   * (Use @c addFactory to add new mappings.)
   */
  const IAuxTypeVectorFactory* getFactory (const std::type_info& ti,
                                           const std::type_info& ti_alloc);


  /**
   * @brief Return the vector factory for a given vector element type.
   * @param ti The type of the vector element.
   * @param alloc_name The name of the vector allocator type.
   *
   * Returns nullptr if the type is not known.
   * (Use @c addFactory to add new mappings.)
   */
  const IAuxTypeVectorFactory* getFactory (const std::type_info& ti,
                                           const std::string& alloc_name);


  /**
   * @brief Return the vector factory for a given auxid.
   * @param auxid The desired aux data item.
   *
   * Returns nullptr if the type is not known.
   * (Use @c addFactory to add new mappings.)
   */
  const IAuxTypeVectorFactory* getFactory (SG::auxid_t auxid) const;


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
  addFactory (const std::type_info& ti,
              const std::type_info& ti_alloc,
              std::unique_ptr<const IAuxTypeVectorFactory> factory);
  

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
  addFactory (const std::type_info& ti,
              const std::string& ti_alloc_name,
              std::unique_ptr<const IAuxTypeVectorFactory> factory);


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
  void setInputRenameMap (const Athena::InputRenameMap_t* map,
                          const IStringPool& pool);
#endif


  /**
   * @brief Check for an input renaming of an auxiliary variable.
   * @brief key The SG key of the object to which the variable is attached.
   * @brief name The name of the variable on the input file.
   *
   * @returns The variable name to use in the transient representation.
   * Will usually be @c name, but may be different if there was a renaming request.
   */
  const std::string& inputRename (const std::string& key,
                                  const std::string& name) const;


  /**
   * @brief Test if a variable name corresponds to a linked variable.
   */
  static bool isLinkedName (const std::string& name);


  /**
   * @brief Given a variable name, return the name of the corresponding
   *        linked variable.
   */
  static std::string linkedName (const std::string& name);


  /**
   * @brief Test to see if a class name corresponds to a class
   *        with a linked variable.
   */
  static bool classNameHasLink (const std::string& className);


private:
  friend class AuxTypeRegistryImpl;

  typedef AthContainers_detail::mutex mutex_t;
  typedef AthContainers_detail::lock_guard<mutex_t> lock_t;

  /**
   * @brief Constructor.
   *
   * Populates the type -> factory mappings for standard C++ types.
   */
  AuxTypeRegistry();


  /**
   * @brief Destructor.
   */
  ~AuxTypeRegistry();


  /// Disallow copy construction and assignment.
  AuxTypeRegistry (const AuxTypeRegistry&);
  AuxTypeRegistry& operator= (const AuxTypeRegistry&);


  /**
   * @brief Create an @c AuxTypeVectorFactory instance.
   *
   * This is passed to @c findAuxID when we're looking up an item
   * for which we know the type at compile-time.
   *
   * The ALLOC template parameter is the allocator to use
   * for the resulting vector.
   */
  template <class T, class ALLOC = AuxAllocator_t<T> >
  std::unique_ptr<IAuxTypeVectorFactory> makeFactory() const;


  /**
   * @brief @c makeFactory implementation that always returns nullptr.
   *
   * This is passed to @c findAuxID when we're looking up an item
   * for which we do not know the type at compile-time.
   */
  std::unique_ptr<IAuxTypeVectorFactory> makeFactoryNull() const;


  /**
   * @brief Return the key used to look up an entry in m_auxids.
   * @param name The name of the aux data item.
   * @param clsname The name of its associated class.  May be blank.
   */
  static std::string makeKey (const std::string& name,
                              const std::string& clsname);
};


} // namespace SG


#include "AthContainers/AuxTypeRegistry.icc"


#endif // not ATHCONTAINERS_AUXTYPEREGISTRY_H
