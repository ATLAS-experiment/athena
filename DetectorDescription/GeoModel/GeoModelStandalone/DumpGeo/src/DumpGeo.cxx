/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//  Initial version:                                       
//  - 2017, Sep -- Riccardo Maria BIANCHI                  
//                 <riccardo.maria.bianchi@cern.ch>        
//  Main updates:                                          
//  - 2024, Feb -- Riccardo Maria BIANCHI                  
//                 <riccardo.maria.bianchi@cern.ch>   
//                 Moved configuration to ComponentAccumulator (CA)   
//  - 2024, Mar -- Riccardo Maria BIANCHI                  
//                 <riccardo.maria.bianchi@cern.ch> 
//                 Removed GeoExporter, moved all to DumpGeo     

#include "DumpGeo/DumpGeo.h"

// Athena includes
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "GeoModelUtilities/GeoModelExperiment.h"
#include "GeoModelInterfaces/IGeoDbTagSvc.h"

// From GeoModel in Externals
#include "GeoModelKernel/GeoVolumeCursor.h"
#include "GeoModelKernel/GeoVDetectorManager.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelWrite/WriteGeoModel.h"

// C++ includes
#include <vector>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <cstdlib> //For setenv

// Units
#include "GeoModelKernel/Units.h"
#define UNITS GeoModelKernelUnits  // so we can use, e.g., 'UNITS::cm'


//TODO: replace this with GeoModelHelpers/defineWorld.h
//_____________________________________________________________________________________
GeoPhysVol* createTheWorld()
{
  // Define the units
  #define gr   UNITS::gram
  #define mole UNITS::mole
  #define cm3  UNITS::cm3

  // Define the chemical elements
  GeoElement*  Nitrogen = new GeoElement ("Nitrogen" ,"N"  ,  7.0 ,  14.0067 *gr/mole);
  GeoElement*  Oxygen   = new GeoElement ("Oxygen"   ,"O"  ,  8.0 ,  15.9995 *gr/mole);
  GeoElement*  Argon    = new GeoElement ("Argon"    ,"Ar" , 18.0 ,  39.948  *gr/mole);
  GeoElement*  Hydrogen = new GeoElement ("Hydrogen" ,"H"  ,  1.0 ,  1.00797 *gr/mole);

  // Define the materials
  double densityOfAir=0.001214 *gr/cm3;
  GeoMaterial *air = new GeoMaterial("Air", densityOfAir);
  air->add(Nitrogen  , 0.7494);
  air->add(Oxygen, 0.2369);
  air->add(Argon, 0.0129);
  air->add(Hydrogen, 0.0008);
  air->lock();
 
  //-----------------------------------------------------------------------------------//
  // create the world volume container and
  // get the 'world' volume, i.e. the root volume of the GeoModel tree
  const GeoBox* worldBox = new GeoBox(1000*UNITS::cm, 1000*UNITS::cm, 1000*UNITS::cm);
  const GeoLogVol* worldLog = new GeoLogVol("WorldLog", worldBox, air);
  GeoPhysVol* world = new GeoPhysVol(worldLog);
  return world;
}


bool isStringInVector(
        const std::vector<std::string>& vec,
        const std::string& str) {

    // Use std::find() algorithm to search
    // for the string in the vector
    auto it = std::find(
                    vec.begin(),
                    vec.end(),
                    str);
    return it != vec.end();
}


//____________________________________________________________________
DumpGeo::DumpGeo(const std::string& name, ISvcLocator* svcLocator):
  AthAlgorithm(name, svcLocator),
  m_toolSvc(0)
  // ,m_geoExporter(0)
{
  // ::setenv("LCGPATCH_COINMULTISELECT","1",1);
}

//____________________________________________________________________
StatusCode DumpGeo::initialize()
{
  ATH_MSG_DEBUG("in initialize()");

  ATH_MSG_INFO("===================================================");
  ATH_MSG_INFO("\t\tLaunching DumpGeo");
  ATH_MSG_INFO("===================================================");

  ATH_MSG_INFO("Accessing the ATLAS geometry...");
  // StoreGateSvc* detStore = m_d->detstore;
  const GeoModelExperiment * theExpt = nullptr;
  ATH_CHECK(detStore()->retrieve(theExpt,"ATLAS"));

  // Get the ATLAS GEOMETRY 'World' volume
  PVConstLink world(theExpt->getPhysVol());

  ATH_MSG_INFO("User filter DetectorManagers: " << m_user_filterDetManagersList);

  ServiceHandle<IGeoDbTagSvc> geoDbTag("GeoDbTagSvc", name());
  ATH_CHECK(geoDbTag.retrieve());
  ATH_MSG_INFO("This is the Geometry TAG we are dumping: " << geoDbTag->atlasVersion());


  GeoPhysVol* volTop = createTheWorld();


  if ( !(m_user_filterDetManagersList.empty()) ) {
    // Get list of managers
    ATH_MSG_INFO("List of GeoModel Detector Managers: ");
    std::vector<std::string> managersList = theExpt->getListOfManagers();
    if ( !(managersList.empty()) ) {
    for (auto const& mm : managersList)
      {
          // get the DetectorManager
          const GeoVDetectorManager* manager = theExpt->getManager(mm);

          // get the name of the DetectorManager
          std::string detManName = manager->getName();
          ATH_MSG_INFO("\tDetectorManager: " << detManName);

          // get the DetManager's TreeTops
          unsigned int nTreetops = manager->getNumTreeTops();
          ATH_MSG_INFO("\t" << mm << " - # TreeTops: " << nTreetops);

          if ( nTreetops > 0 &&  isStringInVector(m_user_filterDetManagersList, detManName) ) {
              
              for(unsigned int i=0; i < nTreetops; ++i) {

                  PVConstLink treetop(manager->getTreeTop(i));

                  // get treetop's volume
                  const GeoVPhysVol* vol = treetop;
                  
                  // get volume's transform
                  // NOTE: we use getDefX() to get the transform without any alignment
                  GeoTransform* volXf = new GeoTransform( vol->getDefX() );
                  
                  // get volume's logvol's name
                  std::string volName = vol->getLogVol()->getName();
                  //std::cout << "\t\t treetop: " << volName << std::endl; // debug msg


                  // Add to the main volume a GeoNameTag with the name of the DetectorManager 
                  volTop->add(new GeoNameTag(detManName));
                  // add Transform and Volume to the main PhysVol
                  volTop->add(volXf);
                  volTop->add(const_cast<GeoVPhysVol*>(vol));

                  if (msgLvl (MSG::DEBUG)) {
                    // DEBUG: dive into the Treetop
                    if ("BeamPipe"==detManName) {
                      GeoVolumeCursor av(treetop);
                      while (!av.atEnd()) {
                          ATH_MSG_DEBUG("\t\ttreetop n." << i << " - child name: "  << av.getName());
                          av.next(); // increment volume cursor.
                      } // end while
                    }
                  }
              } // end for
          } // end if
      } // end for
    }
  } 

  // DEBUG inspection
  if (msgLvl (MSG::DEBUG)) {
    ATH_MSG_DEBUG("Looping over top volumes in the GeoModel tree (children of the 'World' volume)...");
    GeoVolumeCursor av(world);
    while (!av.atEnd()) {
      std::string volname = av.getName();
      ATH_MSG_DEBUG("\t* relevant NameTag:" << volname);
      av.next(); // increment volume cursor.
    }
  }

  ATH_MSG_INFO("Creating the SQLite DB file...");
  if ( m_outFileName.empty()) {
    ATH_MSG_FATAL("The name of the output SQLite file is not set!");
  }
  ATH_MSG_INFO("Output file name: " << m_outFileName);

  // open the DB connection
  GMDBManager db(m_outFileName);

  // check the DB connection
  if (db.checkIsDBOpen())
      ATH_MSG_INFO("OK! Database is open!");
  else {
      ATH_MSG_ERROR(" ***** Database ERROR!! Exiting..." );      
              throw GaudiException("The GeoModel SQLite .db file could not be opened successfully.", 
                                    "DumpGeo", StatusCode::FAILURE);
  }

  ATH_MSG_INFO("Traversing the GeoModel tree...");
    // Dump the tree volumes into a DB
    GeoModelIO::WriteGeoModel dumpGeoModelGraph(db); // init the GeoModel node action
    // visit all GeoModel nodes  
    // if (!(user_detmanagerslist.empty()) || !(user_treetopslist.empty())) { // TODO: add back TreeTopList
    if ( !(m_user_filterDetManagersList.empty()) ) {
      volTop->exec(&dumpGeoModelGraph); 
    } else {
      world->exec(&dumpGeoModelGraph); 
    }
  ATH_MSG_INFO("Saving the GeoModel tree to the DB...");
    dumpGeoModelGraph.saveToDB(); // save to the SQlite DB file
    ATH_MSG_ALWAYS("DONE. Geometry saved to: " << m_outFileName);

  // Quick test if DEBUG
  if (msgLvl (MSG::DEBUG)) {
    ATH_MSG_DEBUG("Test - list of all the GeoMaterial nodes in the persistified geometry:");
    db.printAllMaterials();
    ATH_MSG_DEBUG("Test - list of all the GeoElement nodes in the persistified geometry:");
    db.printAllElements();
  }

  ATH_MSG_DEBUG("End of DumpGeo::init().");
  return StatusCode::SUCCESS;
}

//____________________________________________________________________
StatusCode DumpGeo::execute()
{
  return StatusCode::SUCCESS;
}

//____________________________________________________________________
void DumpGeo::handle(const Incident& inc)
{
  ATH_MSG_INFO("Handling incident '" << inc.type() << "'");
}

