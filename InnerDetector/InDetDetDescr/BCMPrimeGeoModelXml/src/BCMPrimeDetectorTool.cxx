/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BCMPrimeDetectorTool.h"
#include "BCMPrimeGmxInterface.h"

#include <BCMPrimeReadoutGeometry/BCMPrimeDetectorManager.h>

#include <DetDescrConditions/AlignableTransformContainer.h>
#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelUtilities/GeoModelExperiment.h>


BCMPrimeDetectorTool::BCMPrimeDetectorTool(const std::string &type,
                                           const std::string &name,
                                           const IInterface *parent)
  : GeoModelXmlTool(type, name, parent)
{
}


StatusCode BCMPrimeDetectorTool::create()
{
  // retrieve the common stuff
  ATH_CHECK(createBaseTool());

  ATH_MSG_INFO("BCMPrime GmxFilename = '" << m_gmxFilename << "'");

  GeoModelExperiment *theExpt = nullptr;
  ATH_CHECK(detStore()->retrieve(theExpt, "ATLAS"));

  //
  // Check the availability
  //
  std::string node{"InnerDetector"};
  std::string table{"BCMPrimeXDD"};


// PEDRO PEDRO REMOVE SQLITE FOR THE MOMENT
//  const GeoModelIO::ReadGeoModel* sqlreader = getSqliteReader();
//  if(!sqlreader){
//      if (!isAvailable(node, table)) {
//        ATH_MSG_ERROR("No BCMPrime geometry found. BCMPrime can not be built.");
//        return StatusCode::FAILURE;
//      }
//  }

  // Create the detector manager
  // The * converts a ConstPVLink to a ref to a GeoVPhysVol
  // The & takes the address of the GeoVPhysVol
  GeoPhysVol *world = &*theExpt->getPhysVol();
  auto *manager = new InDetDD::BCMPrimeDetectorManager(m_detectorName);
  InDetDD::BCMPrimeGmxInterface gmxInterface(manager);


  const GeoVPhysVol* topVolume = createTopVolume(world, gmxInterface,
                            "InnerDetector",
                            "BCMPrimeXDD");

  if (topVolume) { //see that a valid pointer is returned
    manager->addTreeTop(topVolume);
    ATH_MSG_INFO("BCMPrime topVolume ptr = " << topVolume);
    ATH_MSG_INFO("BCMPrime num tree tops = " << manager->getNumTreeTops());
    ATH_MSG_INFO("BCMPrime num detector elements = " << manager->getNumDetectorElements());
    ATH_MSG_INFO("BCMPrime topVolume name = " << topVolume->getLogVol()->getName());
  } else {
    ATH_MSG_FATAL("Could not find the BCMPrime Top Volume!!!");
    return StatusCode::FAILURE;
  }

  // set the manager
  m_detManager = manager;

  ATH_CHECK(detStore()->record(m_detManager, m_detManager->getName()));
  theExpt->addManager(m_detManager);

  return StatusCode::SUCCESS;
}


StatusCode BCMPrimeDetectorTool::clear()
{
  SG::DataProxy* proxy = detStore()->proxy(ClassID_traits<InDetDD::BCMPrimeDetectorManager>::ID(), m_detManager->getName());
  if (proxy) {
    proxy->reset();
    m_detManager = nullptr;
  }
  return StatusCode::SUCCESS;
}

