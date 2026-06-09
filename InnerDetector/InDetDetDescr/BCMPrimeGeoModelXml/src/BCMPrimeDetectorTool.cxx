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
 ATH_MSG_INFO("HELLO BCM TEST HELLO BCM TEST HELLO BCM TEST HELLO BCM TEST HELLO BCM TEST HELLO BCM TEST HELLO BCM TEST ");
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
//
// PEDRO PEDRO
//

  //
  // Create the detector manager
  //
  // The * converts a ConstPVLink to a ref to a GeoVPhysVol
  // The & takes the address of the GeoVPhysVol
  GeoPhysVol *world = &*theExpt->getPhysVol();
  auto *manager = new InDetDD::BCMPrimeDetectorManager(m_detectorName);
  InDetDD::BCMPrimeGmxInterface gmxInterface(manager);


// PEDRO Add hardcoded BCM' FOLDER with XML
//  const std::string gmxFilename = "/home/purrejol/Documents/itk/ITKLayouts/ITKLayouts/data/BCM/BCMPrime.gmx";


  // Load the geometry, create the volume, 
  // node,table are the location in the DB to look for the clob
  // empty strings are the (optional) containing detector and envelope names
  // allowed to pass a null sqlreader ptr - it will be used to steer the source of the geometry
  // PEDRO START COMMENT AND ADD NEXT two LINES const GeoVPhysVol* topVolume = createTopVolume(world, gmxInterface, node, table,"ITkPixel","ITkPixelDetector",sqlreader); / PEDRO
  //ATH_MSG_INFO("PEDRO TOPVOLUME("<<world<<", "<<gmxInterface<<", InnerDetector, BCMPrimeXDD, "<<gmxFilename<<", , nullptr)");
  //topVolume = createTopVolume(world, gmxInterface, "InnerDetector", "BCMPrimeXDD", gmxFilename, "", nullptr);
  const GeoVPhysVol* topVolume = createTopVolume(world, gmxInterface,
                            "InnerDetector",
                            "BCMPrimeXDD");
//                            "ITkPixel",
//                            "ITkPixelDetector",
//                            nullptr);
  // PEDRO END
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

