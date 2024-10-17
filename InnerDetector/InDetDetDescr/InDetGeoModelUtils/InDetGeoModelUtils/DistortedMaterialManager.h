/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETGEOMODELUTILS_DISTORTEDMATERIALMANAGER_H
#define INDETGEOMODELUTILS_DISTORTEDMATERIALMANAGER_H

class StoredMaterialManager;
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "AthenaBaseComps/AthMessaging.h"

namespace InDetDD {

class DistortedMaterialManager : public AthMessaging
{
public:
  DistortedMaterialManager();  
  IRDBRecordset_ptr  extraMaterialTable() const {return  m_xMatTable;}
  StoredMaterialManager * materialManager() {return  m_materialManager;}

private:
  StatusCode initialize();

  StoredMaterialManager * m_materialManager{nullptr};
  IRDBRecordset_ptr  m_xMatTable;
};


} // endnamespace

#endif // InDetGeoModelUtils_DistortedMaterialManager_h
