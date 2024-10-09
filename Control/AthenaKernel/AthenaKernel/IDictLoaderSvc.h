///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// IDictLoaderSvc.h 
// Header file for class IDictLoaderSvc
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENAKERNEL_IDICTLOADERSVC_H 
#define ATHENAKERNEL_IDICTLOADERSVC_H 1 

/** @class IDictLoaderSvc
 *  a simple interface to interact with the Reflex
 *  dictionaries and abstract/workaround a few ROOT bugs.
 *  There are 2 methods in this interface:
 *    @c has_type to check if a type is known to the reflex system
 *    @c load_type to retrieve the @c RootType associated with a given
 *                 C++ type (by name or by @c std::type_info)
 */

// STL includes
#include <string>
#include <typeinfo>

// FrameWork includes
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/ClassID.h"

#include "DataModelRoot/RootType.h"
#include "CxxUtils/checker_macros.h"


class IDictLoaderSvc
  : virtual public ::IInterface
{ 
  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 
  DeclareInterfaceID(IDictLoaderSvc, 1, 0);

  /** Destructor: 
   */
  virtual ~IDictLoaderSvc();

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /** @brief check a @c Reflex dictionary exists for a given type
   */
  virtual 
  bool has_type (const std::string& type_name) = 0;

  /** @brief check a @c Reflex dictionary exists for a given type
   */
  virtual 
  bool has_type (const std::type_info& typeinfo) = 0;

  /** @brief check a @c Reflex dictionary exists for a given type
   */
  virtual 
  bool has_type (CLID clid) = 0;

  /** @brief retrieve a @c RootType by name (auto)loading the dictionary
   *         by any necessary means.
   *         If @c recursive is true, then recursively load contained types.
   */
  virtual
  const RootType load_type (const std::string& type_name, bool recursive = false) = 0;

  /** @brief retrieve a @c RootType by @c std::type_info (auto)loading the
   *         dictionary by any necessary means.
   *         This method is preferred over the above one as it is guaranteed to
   *         succeed *IF* the dictionary for that type has been generated.
   *         If @c recursive is true, then recursively load contained types.
   */
  virtual
  const RootType load_type (const std::type_info& typeinfo, bool recursive = false) = 0;

  /** @brief retrieve a @c RootType by name (auto)loading the dictionary
   *         by any necessary means.
   *         If @c recursive is true, then recursively load contained types.
   */
  virtual
  const RootType load_type (CLID clid, bool recursive = false) = 0;

}; 

#endif //> !ATHENAKERNEL_IDICTLOADERSVC_H
