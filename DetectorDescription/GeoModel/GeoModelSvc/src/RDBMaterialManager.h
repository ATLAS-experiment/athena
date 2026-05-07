/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEOMODELSVC_RDBMATERIALMANAGER_H
#define GEOMODELSVC_RDBMATERIALMANAGER_H

/**
 *  @class  RDBMaterialManager
 *  @brief  This is a material manager which gets material definitions from
 *          the Geometry DB (Oracle)
 *  @author Joe Boudreau
 */

#include "GeoModelInterfaces/StoredMaterialManager.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "GeoModelKernel/GeoIntrusivePtr.h"
#include "GeoModelKernel/GeoElement.h"
#include "AthenaBaseComps/AthMessaging.h"

#include <string>
#include <string_view>
#include <vector>
#include <iosfwd>

class GeoMaterial;
class ISvcLocator;

class RDBMaterialManager final : public StoredMaterialManager, public AthMessaging {

 public:

  // Constructor:
  RDBMaterialManager(ISvcLocator* pSvcLocator);
  
  // Destructor:
  virtual ~RDBMaterialManager();

  // Query the material:
  virtual const GeoMaterial *getMaterial(std::string_view name) override;
 
  // Query the elements:
  virtual const GeoElement *getElement(const std::string & name) override;

  // Query the elements (by atomic number):
  virtual const GeoElement *getElement(unsigned int atomicNumber) override;

  // Add new material
  virtual void addMaterial(const std::string& space, GeoMaterial* material) override;

  virtual StoredMaterialManager::MaterialMapIterator begin() const override;
  virtual StoredMaterialManager::MaterialMapIterator end() const override;

  // Number of materials in the manager
  virtual size_t size() override;

  virtual std::ostream & printAll(std::ostream & o) const override;
  //default std::cout output
  virtual std::ostream & printAll() const override;
 private:

  StatusCode readMaterialsFromDB(ISvcLocator* pSvcLocator);

  void buildSpecialMaterials();

  GeoElement *searchElementVector (const std::string & name) const;
  GeoElement *searchElementVector (const unsigned int atomicNumber) const;
  GeoMaterial *searchMaterialMap (std::string_view name) const;
  
  IRDBRecordset_ptr m_elements;

  using GeoEleVec = std::vector<GeoIntrusivePtr<GeoElement>>;
  GeoEleVec m_elementVector;
  StoredMaterialManager::MaterialMap m_materialMap;

  struct DetectorAuxData
  {
    DetectorAuxData(const std::string& prim_key
		    , IRDBRecordset_ptr materials
		    , IRDBRecordset_ptr matcomponents)
      : m_prim_key(prim_key)
      , m_materials(std::move(materials))
      , m_matcomponents(std::move(matcomponents))
    {}

    std::string m_prim_key{};
    IRDBRecordset_ptr m_materials{};
    IRDBRecordset_ptr m_matcomponents{};
  };

  std::map<std::string,DetectorAuxData> m_detData;
};


#endif
