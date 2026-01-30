/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PLRDetectorTool.h"
#include "PLRGmxInterface.h"

#include <PixelReadoutGeometry/PixelDetectorManager.h>

#include <DetDescrConditions/AlignableTransformContainer.h>
#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelUtilities/GeoModelExperiment.h>
#include <InDetIdentifier/PLR_ID.h>
#include <SGTools/DataProxy.h>


PLRDetectorTool::PLRDetectorTool(const std::string &type,
                                 const std::string &name,
                                 const IInterface *parent)
 : GeoModelXmlTool(type, name, parent)
{
}


StatusCode PLRDetectorTool::create()
{
  // retrieve the common stuff
  ATH_CHECK(createBaseTool());

  GeoModelExperiment *theExpt = nullptr;
  ATH_CHECK(detStore()->retrieve(theExpt, "ATLAS"));
  const PLR_ID *idHelper = nullptr;
  ATH_CHECK(detStore()->retrieve(idHelper, "PLR_ID"));

  m_commonItems = std::make_unique<InDetDD::SiCommonItems>(idHelper);

  const GeoModelIO::ReadGeoModel* sqlreader = getSqliteReader();

  std::string node{"InnerDetector"};
  std::string table{"PLRXDD"};

  //
  // Check the availability (if not running from SQLite)
  //
  if(!sqlreader){
      if (!isAvailable(node, table)) {
        ATH_MSG_ERROR("No PLR geometry found. PLR can not be built.");
        return StatusCode::FAILURE;
      }
  }
  //
  // Create the detector manager
  //
  // The * converts a ConstPVLink to a ref to a GeoVPhysVol
  // The & takes the address of the GeoVPhysVol
  GeoPhysVol *world = &*theExpt->getPhysVol();
  auto *manager = new InDetDD::PixelDetectorManager(&*detStore(), m_detectorName, "PLR_ID");
  manager->addFolder(m_alignmentFolderName);
  // Load the geometry, create the volume,
  // node,table are the location in the DB to look for the clob
  // empty strings are the (optional) containing detector and envelope names
  // allowed to pass a null sqlreader ptr - it will be used to steer the source of the geometry
  InDetDD::PLRGmxInterface gmxInterface(manager, m_commonItems.get(), &m_moduleTree);

  const GeoVPhysVol * topVol = createTopVolume(world, gmxInterface, node, table, m_containingDetectorName, m_envelopeVolumeName,sqlreader);
  if(topVol){
    manager->addTreeTop(topVol);
    manager->initNeighbours();
    }
    else{
        ATH_MSG_FATAL("Could not find the Top Volume!!!");
        return StatusCode::FAILURE;
   }

  // set the manager
  m_detManager = manager;

  ATH_CHECK(detStore()->record(m_detManager, m_detManager->getName()));
  theExpt->addManager(m_detManager);

  // Create a symLink to the SiDetectorManager base class so it can be accessed as either SiDetectorManager or
  // PixelDetectorManager
  const InDetDD::SiDetectorManager *siDetManager = m_detManager;
  ATH_CHECK(detStore()->symLink(m_detManager, siDetManager));

  return StatusCode::SUCCESS;
}


StatusCode PLRDetectorTool::clear()
{
  SG::DataProxy* proxy = detStore()->proxy(ClassID_traits<InDetDD::PixelDetectorManager>::ID(),m_detManager->getName());
  if (proxy) {
    proxy->reset();
    m_detManager = nullptr;
  }
  return StatusCode::SUCCESS;

}

void PLRDetectorTool::doNumerology()
{
  InDetDD::SiNumerology n;

  ATH_MSG_INFO("\n\nPLR Numerology:\n===============\n\nNumber of parts is " << m_moduleTree.nParts());
}
