/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


///////////////////////////////////////////////////////////////////////
//                                                                   //
//  Implementation of class GeoExporter                              //
//                                                                   //
//  Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>  //
//                                                                   //
//  Initial version: Sep 2017                                        
//
//  Main updates:
//  - 2024, Feb -- Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
//                 Migrated to CA, added new CLI options, 
//                 added new filter over DetectorManagers
//
///////////////////////////////////////////////////////////////////////

#include "GeoExporter/GeoExporter.h"

#include "VP1Utils/VP1JobConfigInfo.h"
#include "VP1Utils/VP1SGAccessHelper.h"

#include "GeoModelKernel/GeoVolumeCursor.h"
#include "GeoModelKernel/GeoVDetectorManager.h"
#include "GeoModelKernel/GeoBox.h"

#include "GeoModelUtilities/GeoModelExperiment.h"

#include "GeoModelDBManager/GMDBManager.h"

#include "GeoModelWrite/WriteGeoModel.h"
// #include "GeoModelHelpers/defineWorld.h" //TODO: Use this as soon as we have the latest GeoModel in Athena main

#include <cassert>
#include <iostream>
#include <filesystem>

// define terminal colors and the reset code
#define RESET   "\033[0m"
#define RED     "\033[31m"  /* Red */

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
  std::cout << "Creating the 'world' volume, i.e. the root volume of the GeoModel tree..." << std::endl;
  const GeoBox* worldBox = new GeoBox(1000*UNITS::cm, 1000*UNITS::cm, 1000*UNITS::cm);
  const GeoLogVol* worldLog = new GeoLogVol("WorldLog", worldBox, air);
  GeoPhysVol* world = new GeoPhysVol(worldLog);
  return world;
}


std::vector<std::string> splitCommaSepStringIntoVector(std::string str) {
  std::stringstream sst(str);
  std::vector<std::string> vv;
  while (sst.good()) {
        std::string substr;
        getline(sst, substr, ',');
        vv.push_back(substr);
    }
 
    for (size_t i = 0; i < vv.size(); i++)
        std::cout << vv[i] << std::endl;
  return vv;
}

std::string joinVectorOfStringWithSep(std::vector<std::string> vv, std::string sep){
  std::string str;
    for (unsigned int i=0; i<vv.size(); i++) {
        str += vv[i];
        if (i >= (vv.size() - 1)) {
            break; // escaping in the last iteration
        }
        str += sep; // concatenating string
    }
    return str;
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
class GeoExporter::Imp {
public:
  Imp() {};
  //We hold the arguments here until init is called:
  StoreGateSvc* sg = nullptr;
  StoreGateSvc* detstore = nullptr;
  ISvcLocator* svclocator = nullptr;
  IToolSvc*toolSvc = nullptr;
};


//____________________________________________________________________
GeoExporter::GeoExporter(StoreGateSvc* sg,StoreGateSvc* detstore,
	       ISvcLocator* svclocator,IToolSvc*toolSvc)
 : m_d(new Imp)
{
  m_d->sg = sg;
  m_d->detstore = detstore;
  m_d->svclocator = svclocator;
  m_d->toolSvc = toolSvc;
}

//____________________________________________________________________
GeoExporter::~GeoExporter()
{
  delete m_d; m_d=0;
}

//____________________________________________________________________
bool GeoExporter::argumentsAreValid() const
{
  //Athena pointers:
  if (!m_d->sg) {
     std::cout <<"ERROR: Null pointer to event store." << std::endl;
    return false;
  }
  if (!m_d->detstore) {
     std::cout <<"ERROR: Null pointer to detector store." << std::endl;
    return false;
  }

  return true;
}


//____________________________________________________________________
void GeoExporter::init()
{
  std::cout << "Start of GeoExporter::init()...\n"; // TODO: move to MSG_DEBUG

   std::cout << "\n===================================================\n";
   std::cout <<"\t\tLaunching the GeoExporter\n";
   std::cout << "===================================================\n";

   std::cout <<"Accessing the ATLAS geometry..." << std::endl;
  StoreGateSvc* detstore = m_d->detstore;
 //Get the world volume:
  const GeoModelExperiment * theExpt = nullptr;
  if (!VP1SGAccessHelper(detstore).retrieve(theExpt,"ATLAS")) {
    std::cout << "Error: Could not retrieve the ATLAS GeoModelExperiment from detector store" << std::endl; // TODO: move to ATH_MSG_
    //ATH_MSG_FATAL ("Error: Could not retrieve the ATLAS GeoModelExperiment from detector store");
    //return StatusCode::FAILURE;
    return; // TODO: move to Return statuscode
  }
  // GET ATLAS GEOMETRY
  PVConstLink world(theExpt->getPhysVol());

//   // -- get Detector Description tag
  char const* tmpDD = getenv( "DUMPGEODETDESCRTAG" );
  std::string detDescrTag{""};
  if ( tmpDD != NULL ) {
  detDescrTag = std::string(tmpDD);
} else {
  std::cout << RED << "ERROR! The environmental variable 'DUMPGEODETDESCRTAG' is not set! Exiting..." << std::endl;
  throw "env var 'DUMPGEODETDESCRTAG' not set";
} 
     std::cout <<"GeoExporter -- We're dumping DetDescrTag: '" + detDescrTag << "'..." << std::endl;
  

  //TODO: move to CA configuration
  // // -- get sub-systems settings
  // bool user_noid = environment.value("DUMPGEO_NOID").toInt();
  // bool user_nocalo = environment.value("DUMPGEO_NOCALO").toInt();
  // bool user_nomuon = environment.value("DUMPGEO_NOMUON").toInt();
  
  // ### Get user's settings ###
  // -- get GeoModel Treetop filter // FIXME: check and update this!!
  char const* tmpTT = getenv("DUMPGEOFILTERTREETOPS");
  std::string user_filterTreeTops{""};
  if ( tmpTT != NULL ) {
    user_filterTreeTops = std::string(tmpTT);
    std::cout <<"User's settings - GeoModel TreeTops filter: " << user_filterTreeTops << std::endl;
  }
  // -- get GeoModel Detector Managers filter // FIXME: check and update this!!
  char const* tmpDM = getenv("DUMPGEOFILTERDETMANAGERS");
  std::string user_filterDetManagers{""};
  if ( tmpDM != NULL ) {
    user_filterDetManagers = std::string(tmpDM);
    std::cout <<"User's settings - GeoModel DetectorManagers filter: " + user_filterDetManagers << std::endl;
  }
  //TODO: move to Python Config
  // QString user_subsystems_filters = "" + QString(((user_noid) ? "-noID" : "")) + QString(((user_nocalo) ? "-noCalo" : "")) + QString(((user_nomuon) ? "-noMuon" : ""));

  // Get list of TreeTops from the TREETOPFILTER
  std::vector<std::string> user_treetopslist;
  if ( ! user_filterTreeTops.empty() ) {
    // user_treetopslist = user_filterTreeTops.split(',');
    user_treetopslist = splitCommaSepStringIntoVector(user_filterTreeTops);
  }
  // Get list of DetectorManagers from the TREETOPFILTER
  std::vector<std::string> user_detmanagerslist;
  if ( ! user_filterDetManagers.empty() ) {
    // user_detmanagerslist = user_filterDetManagers.split(','); 
    user_detmanagerslist = splitCommaSepStringIntoVector(user_filterDetManagers); 
  }

  GeoPhysVol* volTop = createTheWorld();

if ( !(user_detmanagerslist.empty()) ) {
  // Get list of managers
  std::cout << "\nList of GeoModel Detector Managers: " << std::endl;
  std::vector<std::string> managersList = theExpt->getListOfManagers();
  if ( !(managersList.empty()) ) {
   for (auto const& mm : managersList)
    {
        // get the DetectorManager
        const GeoVDetectorManager* manager = theExpt->getManager(mm);

        // get the name of the DetectorManager
        std::string detManName = manager->getName();
        std::cout << "\n\tDetectorManager: " << detManName << std::endl;

        // get the DetManager's TreeTops
        unsigned int nTreetops = manager->getNumTreeTops();
        std::cout << "\t" << mm << " - # TreeTops: " << nTreetops << std::endl;

        if ( nTreetops > 0 &&  isStringInVector(user_detmanagerslist, detManName) ) {
            
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

                // DEBUG: dive into the Treetop
                if ("BeamPipe"==detManName) {
                GeoVolumeCursor av(treetop);
                while (!av.atEnd()) {
                    std::cout << "\t\ttreetop n." << i << " - child name: "  << av.getName() << "\n";
                    av.next(); // increment volume cursor.
                } // end while
                }
            } // end for
        } // end if
    } // end for
  }
} 
// if ( !(user_treetopslist.empty()) ) {
  std::cout << "\nLooping over top volumes in the GeoModel tree (children of the 'World' volume)..." << std::endl;
  GeoVolumeCursor av(world);
  while (!av.atEnd()) {

	  std::string volname = av.getName();
    std::cout << "\t* relevant NameTag:" << volname << std::endl ;
    
    av.next(); // increment volume cursor.
    }
// }

  std::cout << "Creating the SQLite DB file..." << std::endl;
  char const* tmpOF = getenv("DUMPGEOOUTFILENAME");
  std::string fileName{""};
   if ( tmpOF != NULL ) {
  fileName = std::string(tmpOF);
} else {
  std::cout << RED << "ERROR! The environmental variable 'DUMPGEOOUTFILENAME' is not set! Exiting..." << std::endl;
  throw "env var 'DUMPGEOOUTFILENAME' not set";
} 
  std::cout <<"Output file name: " << fileName << std::endl;

  // open the DB connection
  GMDBManager db(fileName);

  // check the DB connection
  if (db.checkIsDBOpen())
      std::cout << "OK! Database is open!" << std::endl;
  else {
      std::cout << "Database ERROR!! Exiting..." << std::endl;
      return;
  }

   std::cout << "Dumping the GeoModel geometry to the DB file..." << std::endl;
  // Dump the tree volumes into a DB
  GeoModelIO::WriteGeoModel dumpGeoModelGraph(db); // init the GeoModel node action
  // visit all GeoModel nodes  
  if (!(user_detmanagerslist.empty()) || !(user_treetopslist.empty())) {
    volTop->exec(&dumpGeoModelGraph); 
  } else {
    world->exec(&dumpGeoModelGraph); 
  }
  std::cout << "Saving the GeoModel tree to the DB." << std::endl;
  dumpGeoModelGraph.saveToDB(); // save to the SQlite DB file
  std::cout << "DONE. Geometry saved." <<std::endl;

  std::cout << "\nTest - list of all the GeoMaterial nodes in the persistified geometry:" << std::endl;
  db.printAllMaterials();
  std::cout << "\nTest - list of all the GeoElement nodes in the persistified geometry:" << std::endl;
  db.printAllElements();

  std::cout << "end of GeoExporter::init()." << std::endl; // TODO: move to MSG_DEBUG
}
