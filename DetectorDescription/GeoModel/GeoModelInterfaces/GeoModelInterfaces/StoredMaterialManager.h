/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELINTERFACES_STOREDMATERIALMANAGER_H
#define GEOMODELINTERFACES_STOREDMATERIALMANAGER_H

/**
 * @class StoredMaterialManager
 *
 * @brief This class holds one or more material managers and makes 
 *        them storeable, under StoreGate
 *
 * @author Joe Boudreau, March 2003
 */


#include "GeoModelKernel/GeoIntrusivePtr.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoElement.h"
#include "AthenaKernel/CLASS_DEF.h"

#include <iosfwd>
#include <map>
#include <string>



class StoredMaterialManager
{
 public:
  using MaterialMap = std::map<std::string, GeoIntrusivePtr<GeoMaterial>, std::less<>>;
  using MaterialMapIterator = MaterialMap::const_iterator;

  // Constructor:
  StoredMaterialManager() = default;
  
  // Destructor:
  virtual ~StoredMaterialManager() = default;

  // Query the material:
  virtual const GeoMaterial* getMaterial(std::string_view name) = 0;

  // Query the elements:
  virtual const GeoElement* getElement(const std::string& name) = 0;

  // Query the elements (by atomic number):
  virtual const GeoElement* getElement(unsigned int atomicNumber) = 0;

  // Add new material
  virtual void addMaterial(const std::string& space, GeoMaterial* material) = 0;

  // Return iterators
  virtual MaterialMapIterator begin() const = 0;
  virtual MaterialMapIterator end() const = 0;

  // Number of materials in the manager
  virtual size_t size() = 0;

  // Dump the contents
  virtual std::ostream& printAll(std::ostream & o) const = 0;
  //default stream to std::cout
  virtual std::ostream& printAll() const = 0;
};

CLASS_DEF(StoredMaterialManager, 9896,1)

#endif
