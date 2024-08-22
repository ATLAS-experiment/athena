/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelGeoModelXml/PixelDetectorTool.h"
#include "PixelGeoModelXml/PixelGmxInterface.h"

#include <PixelReadoutGeometry/PixelDetectorManager.h>
#include <PixelReadoutGeometry/PixelModuleDesign.h>

#include <DetDescrConditions/AlignableTransformContainer.h>
#include <GeoModelKernel/GeoPhysVol.h>
#include <GeoModelUtilities/DecodeVersionKey.h>
#include <GeoModelUtilities/GeoModelExperiment.h>
#include <SGTools/DataProxy.h>
#include <StoreGate/StoreGateSvc.h>


namespace ITk
{

PixelDetectorTool::PixelDetectorTool(const std::string &type,
                                     const std::string &name,
                                     const IInterface *parent)
 : GeoModelXmlTool(type, name, parent)
{
}


StatusCode PixelDetectorTool::create()
{
  // retrieve the common stuff
  ATH_CHECK(createBaseTool());

  GeoModelExperiment *theExpt = nullptr;
  ATH_CHECK(detStore()->retrieve(theExpt, "ATLAS"));
  const PixelID *idHelper = nullptr;
  ATH_CHECK(detStore()->retrieve(idHelper, "PixelID"));

  m_commonItems = std::make_unique<InDetDD::SiCommonItems>(idHelper);
   
  GeoModelIO::ReadGeoModel* sqlreader = getSqliteReader();
  
  // If we are not taking the geo from sqlite, check the availability of tables 
  // (or that we have a local geometry)
  std::string node{"Pixel"};
  std::string table{"PIXXDD"};
  
   if(!sqlreader){
      if (!isAvailable(node, table)) {
       ATH_MSG_INFO("Trying new " << m_detectorName.value() << " database location.");
       node = "InnerDetector";
       table = "PixelXDD";
       if (!isAvailable(node, table)) {
           ATH_MSG_ERROR("No ITk Pixel geometry found. ITk Pixel can not be built.");
           return StatusCode::FAILURE;
        }
     }
   }
  //
  // Create the detector manager
  //
  // The * converts a ConstPVLink to a ref to a GeoVPhysVol
  // The & takes the address of the GeoVPhysVol
  GeoPhysVol *world = &*theExpt->getPhysVol();
  auto *manager = new InDetDD::PixelDetectorManager(&*detStore(), m_detectorName, "PixelID");
  const std::string topFolder(m_alignmentFolderName);
  manager->addFolder(topFolder);
  if (m_alignable) {
    InDetDD::AlignFolderType alignFolderType = InDetDD::static_run1 ;
    manager->addAlignFolderType(alignFolderType);
    manager->addChannel(topFolder +"/ID",     2, InDetDD::global);
    manager->addChannel(topFolder +"/PIX",    1, InDetDD::global);
    manager->addChannel(topFolder +"/PIXB1",  0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXB2",  0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXB3",  0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXB4",  0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXB5",  0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEA1", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEA2", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEA3", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEA4", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEA5", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEA6", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEA7", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEA8", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEA9", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEC1", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEC2", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEC3", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEC4", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEC5", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEC6", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEC7", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEC8", 0, InDetDD::local);
    manager->addChannel(topFolder +"/PIXEC9", 0, InDetDD::local);
  }
  InDetDD::ITk::PixelGmxInterface gmxInterface(manager, m_commonItems.get(), &m_moduleTree);

  // Load the geometry, create the volume, 
  // node,table are the location in the DB to look for the clob
  // empty strings are the (optional) containing detector and envelope names
  // allowed to pass a null sqlreader ptr - it will be used to steer the source of the geometry
  const GeoVPhysVol* topVolume = createTopVolume(world, gmxInterface, node, table,"","",sqlreader);
  if(sqlreader){
        ATH_MSG_INFO("Building Pixel Readout Geometry from SQLite using "<<m_geoDbTagSvc->getParamSvcName());
        gmxInterface.buildReadoutGeometryFromSqlite(m_sqliteReadSvc.operator->(),sqlreader);
  }
  if (topVolume) { //see that a valid pointer is returned
    manager->addTreeTop(topVolume);
    doNumerology(manager);
    manager->initNeighbours();
  } else {
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


StatusCode PixelDetectorTool::clear()
{
  SG::DataProxy* proxy = detStore()->proxy(ClassID_traits<InDetDD::PixelDetectorManager>::ID(),m_detManager->getName());
  if (proxy) {
    proxy->reset();
    m_detManager = nullptr;
  }
  return StatusCode::SUCCESS;
}

void PixelDetectorTool::doNumerology(InDetDD::PixelDetectorManager * manager)
{
  ATH_MSG_INFO( "\n\nPixel Numerology:\n===============\n\nNumber of parts is " << m_moduleTree.nParts() );
  InDetDD::SiNumerology n;

  bool barrelDone = false;
  for (int b = -1; b <= 1; ++b) {
      if (m_moduleTree.count(b)) {
          msg(MSG::INFO) << "    Found barrel with index " << b << std::endl;
          n.addBarrel(b);
          if (!barrelDone) {
              n.setNumLayers(m_moduleTree[b].nLayers());
              msg(MSG::INFO) << "        Number of barrel layers = " << n.numLayers() << std::endl;
              for (LayerDisk::iterator l = m_moduleTree[b].begin(); l != m_moduleTree[b].end(); ++l) {
                  n.setNumEtaModulesForLayer(l->first, l->second.nEtaModules());
                  // All staves within a layer are assumed identical, so we can just look at the first eta
                  n.setNumPhiModulesForLayer(l->first, l->second.begin()->second.nPhiModules());
                  msg(MSG::INFO) << "        layer = " << l->first << " has " << n.numEtaModulesForLayer(l->first) <<
                                    " etaModules each with " <<  n.numPhiModulesForLayer(l->first) << " phi modules" << std::endl;
              }
              barrelDone = true;
          }
      }

  }

  bool endcapDone = false;
  for (int ec = -2; ec <= 2; ec += 4) {
      if (m_moduleTree.count(ec)) {
          msg(MSG::INFO) << "    Found endcap with index " << ec << std::endl;
          n.addEndcap(ec);
          if (!endcapDone) {
              n.setNumDiskLayers(m_moduleTree[ec].nLayers());
              msg(MSG::INFO) << "        Number of endcap layers = " << n.numDiskLayers() << std::endl;
              for (LayerDisk::iterator l = m_moduleTree[ec].begin(); l != m_moduleTree[ec].end(); ++l) {
                  n.setNumDisksForLayer(l->first, l->second.nEtaModules());
                  msg(MSG::INFO) << "        Layer " << l->first << " has " << n.numDisksForLayer(l->first) << " disks" << std::endl;
                  for (EtaModule::iterator eta = l->second.begin(); eta != l->second.end(); ++eta) {
                      n.setNumPhiModulesForLayerDisk(l->first, eta->first, eta->second.nPhiModules());
                      msg(MSG::DEBUG) << "            Disk " << eta->first << " has " <<
                                          n.numPhiModulesForLayerDisk(l->first, eta->first) << " phi modules" << std::endl;
                  }
              }
              endcapDone = true;
          }
      }
  }

  msg(MSG::INFO) << endmsg;

  int totalWafers = 0;
  for (BarrelEndcap::iterator bec = m_moduleTree.begin(); bec != m_moduleTree.end(); ++bec) {
      for (LayerDisk::iterator ld = bec->second.begin(); ld != bec->second.end(); ++ld) {
          for (EtaModule::iterator eta = ld->second.begin(); eta != ld->second.end(); ++eta) {
              for (PhiModule::iterator phi = eta->second.begin(); phi != eta->second.end(); ++phi) {
                  for (Side::iterator side =phi->second.begin(); side != phi->second.end(); ++side) {
                      totalWafers++;
                  }
              }
          }
      }
  }
  ATH_MSG_INFO("Total number of wafers added is " << totalWafers);
  const PixelID *pixelIdHelper = dynamic_cast<const PixelID *> (m_commonItems->getIdHelper());
  ATH_MSG_INFO("Total number of wafer identifiers is " << pixelIdHelper->wafer_hash_max());

  //    Used in digitization to create one vector big enough to hold all pixels
  n.setMaxNumEtaCells(1);
  for (int d = 0; d < manager->numDesigns(); ++d) {
    n.setMaxNumPhiCells(manager->getPixelDesign(d)->rows());
    n.setMaxNumEtaCells(manager->getPixelDesign(d)->columns());
  }
  ATH_MSG_INFO("Max. eta cells is " << n.maxNumEtaCells());
  ATH_MSG_INFO("Max. phi cells is " << n.maxNumPhiCells());

  manager->numerology() = n;

  ATH_MSG_INFO("End of numerology\n");
}

} // namespace ITk

