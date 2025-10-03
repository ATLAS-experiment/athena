/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CAVERNINFRAGEOMODEL_CAVERNINFRADETECTORFACTORY_H
#define CAVERNINFRAGEOMODEL_CAVERNINFRADETECTORFACTORY_H

#include "CavernInfraDetectorManager.h"
#include "GeoModelKernel/GeoVDetectorFactory.h"
#include "StoreGate/StoreGateSvc.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"

#include <string>

class CavernInfraDetectorFactory : public GeoVDetectorFactory
{
 public:
  // Constructor:
  CavernInfraDetectorFactory(const ServiceHandle<StoreGateSvc>& pDetStore,
			     const ServiceHandle<IRDBAccessSvc>& pAccess);

  // Illegal operations:
  const CavernInfraDetectorFactory & operator=(const CavernInfraDetectorFactory &right) = delete;
  CavernInfraDetectorFactory(const CavernInfraDetectorFactory &right) = delete;

  // Destructor:
  ~CavernInfraDetectorFactory() = default;

  // Creation of geometry:
  virtual void create(GeoPhysVol *world) override;

  // Access to the results:
  virtual const CavernInfraDetectorManager* getDetectorManager() const override;

  // Set version Tag and Node
  void setTagNode(const std::string& tag, const std::string& node);

 private:
  // The manager:
  CavernInfraDetectorManager*  m_detectorManager{nullptr};

  ServiceHandle<StoreGateSvc>  m_detectorStore;
  ServiceHandle<IRDBAccessSvc> m_access;
  std::string              m_versionTag;
  std::string              m_versionNode;
};

// Class CavernInfraDetectorFactory
#endif

