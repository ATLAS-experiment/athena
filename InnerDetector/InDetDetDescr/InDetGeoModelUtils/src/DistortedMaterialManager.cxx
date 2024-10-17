/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#include "InDetGeoModelUtils/DistortedMaterialManager.h"
#include "GeoModelInterfaces/StoredMaterialManager.h"
#include "GeoModelUtilities/DecodeVersionKey.h"
#include "StoreGate/StoreGateSvc.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GeoModelInterfaces/IGeoDbTagSvc.h"
#include "AthenaBaseComps/AthCheckMacros.h"

namespace InDetDD {
  DistortedMaterialManager::DistortedMaterialManager()
    : AthMessaging("ExtraMaterialManager")
  {
    if(initialize().isFailure())
      throw std::runtime_error("Failed to initialize DistortedMaterialManager!");
  }

  StatusCode DistortedMaterialManager::initialize() {
    ISvcLocator* svcLocator = Gaudi::svcLocator();

    SmartIF<StoreGateSvc> detStore{svcLocator->service("DetectorStore")};
    ATH_CHECK(detStore.isValid());

    SmartIF<IGeoDbTagSvc> geoDbTag{svcLocator->service("GeoDbTagSvc")};
    ATH_CHECK(geoDbTag.isValid());

    SmartIF<IRDBAccessSvc> rdbSvc{svcLocator->service(geoDbTag->getParamSvcName())};
    ATH_CHECK(rdbSvc.isValid());

    // Get version tag and node for InDet.
    DecodeVersionKey versionKey("InnerDetector");
    const std::string& detectorKey = versionKey.tag();
    const std::string& detectorNode = versionKey.node();

    ATH_MSG_DEBUG("Retrieving Record Sets from database ...");
    ATH_MSG_DEBUG("Key = " << detectorKey << " Node = " << detectorNode);

    m_xMatTable = rdbSvc->getRecordsetPtr("InDetExtraMaterial", detectorKey, detectorNode);
    m_materialManager = detStore->tryRetrieve<StoredMaterialManager>("MATERIALS");

    ATH_MSG_DEBUG("Initialized InDet Distorted Material Manager");

    return StatusCode::SUCCESS;
  }

} // end namespace
