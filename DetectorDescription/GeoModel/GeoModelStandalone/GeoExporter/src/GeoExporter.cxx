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
#include "VP1Base/VP1Msg.h"

#include "VP1Utils/VP1JobConfigInfo.h"
#include "VP1Utils/VP1SGAccessHelper.h"

#include "GeoModelKernel/GeoVolumeCursor.h"
#include "GeoModelKernel/GeoVDetectorManager.h"
#include "GeoModelKernel/GeoBox.h"

#include "GeoModelUtilities/GeoModelExperiment.h"

#include "GeoModelDBManager/GMDBManager.h"

#include "GeoModelWrite/WriteGeoModel.h"
// #include "GeoModelHelpers/defineWorld.h" //TODO: Use this as soon as we have the latest GeoModel in Athena main

#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtNetwork/QSslSocket>
#include <QProcessEnvironment>
#include <QDebug>

#include <cassert>
#include <iostream>

#include <boost/range/irange.hpp>

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
    VP1Msg::message("ERROR: Null pointer to event store.");
    return false;
  }
  if (!m_d->detstore) {
    VP1Msg::message("ERROR: Null pointer to detector store.");
    return false;
  }

  return true;
}


//____________________________________________________________________
void GeoExporter::init()
{
  VP1Msg::messageDebug("Start of GeoExporter::init()...");

  VP1Msg::message("");
  VP1Msg::message("===================================================");
  VP1Msg::message("               Launching the GeoExporter");
  VP1Msg::message("===================================================");
  VP1Msg::message("");

  VP1Msg::message("Accessing the ATLAS geometry...");
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

  // ### Get user's settings ###
  QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
  // -- get Detector Description tag
  QString default_detdescrtag = environment.value("DUMPGEODETDESCRTAGDEFAULT");
  QString user_detdescrtag = environment.value("DUMPGEODETDESCRTAG");
  if ("1"==default_detdescrtag) {
    VP1Msg::message("The user did not specify a DetDescrTag - Using the default one: " + user_detdescrtag);
    }
  else {
    VP1Msg::message("User's settings - DetDescrTag: " + user_detdescrtag);
    }
  // -- get 'forceOverwrite' option
  bool user_forceOverwrite = ( environment.value("DUMPGEOFORCEOVERWRITE")=="1" ? true : false );
  VP1Msg::message("User's settings - forceOverwrite option: " + QString::number(user_forceOverwrite) );
  // -- get sub-systems settings
  bool user_noid = environment.value("DUMPGEO_NOID").toInt();
  bool user_nocalo = environment.value("DUMPGEO_NOCALO").toInt();
  bool user_nomuon = environment.value("DUMPGEO_NOMUON").toInt();
  QString user_subsystems_filters = "" + QString(((user_noid) ? "-noID" : "")) + QString(((user_nocalo) ? "-noCalo" : "")) + QString(((user_nomuon) ? "-noMuon" : ""));
  // -- get GeoModel Treetop filter // FIXME: check and update this!!
  QString user_filterTreeTops = environment.value("DUMPGEOFILTERTREETOPS");
  VP1Msg::message("User's settings - GeoModel TreeTops filter: " + user_filterTreeTops);
  // -- get GeoModel Detector Managers filter // FIXME: check and update this!!
  QString user_filterDetManagers = environment.value("DUMPGEOFILTERDETMANAGERS");
  VP1Msg::message("User's settings - GeoModel DetectorManagers filter: " + user_filterDetManagers);


  // Get list of TreeTops from the TREETOPFILTER
  QStringList user_treetopslist;
  if ( ! user_filterTreeTops.isEmpty() ) {
    user_treetopslist = user_filterTreeTops.split(',');
  }
  // Get list of DetectorManagers from the TREETOPFILTER
  QStringList user_detmanagerslist;
  if ( ! user_filterDetManagers.isEmpty() ) {
    user_detmanagerslist = user_filterDetManagers.split(','); 
  }

  GeoPhysVol* volTop = createTheWorld();

if ( !(user_detmanagerslist.empty()) ) {
  // Get list of managers
  std::cout << "\nList of GeoModel managers: " << std::endl;
  std::vector<std::string> managersList = theExpt->getListOfManagers();
  if ( !(managersList.empty()) ) {
   for (auto const& mm : managersList)
    {
        // get the DetectorManager
        const GeoVDetectorManager* manager = theExpt->getManager(mm);

        // get the name of the DetectorManager
        std::string detManName = manager->getName();
        std::cout << "\n\t DetectorManager: " << detManName << std::endl;

        // get the DetManager's TreeTops
        unsigned int nTreetops = manager->getNumTreeTops();
        std::cout << mm << "\t - n.Treetops: " << nTreetops << std::endl;

        if ( nTreetops > 0 && user_detmanagerslist.contains(QString::fromStdString(detManName)) ) {
            
            for(unsigned int i=0; i < nTreetops; ++i) {

                PVConstLink treetop(manager->getTreeTop(i));

                // get treetop's volume
                const GeoVPhysVol* vol = treetop;
                
                // get volume's transform
                // NOTE: we use getDefX() to get the transform without any alignment
                GeoTransform* volXf = new GeoTransform( vol->getDefX() );
                
                // get volume's logvol's name
                std::string volName = vol->getLogVol()->getName();
                std::cout << "\t\t treetop: " << volName << std::endl;


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
  QString fileName = "geometry";
  if ( !(user_detdescrtag.isEmpty()) ) {
    fileName = "geometry-" + user_detdescrtag;
  }
  if ( !(user_treetopslist.isEmpty()) ) {
        fileName = fileName + "-" + user_treetopslist.join("-");
  }
  if ( !(user_detmanagerslist.isEmpty()) ) {
        fileName = fileName + "-" + user_detmanagerslist.join("-");
  }
  if ( !(user_subsystems_filters.isEmpty()) ) {
        fileName = fileName + "-" + user_subsystems_filters + ".db";
  }
  fileName = fileName + ".db";

  // check if fileName exists and if yes: Is it a file and no directory?
   bool fileExists = QFileInfo::exists(fileName) && QFileInfo(fileName).isFile();
  if (fileExists) {
    if (user_forceOverwrite) {
        VP1Msg::message("Removing the existing dump file ("+fileName+")...");
        QFile file (fileName);
        file.remove();
    } else if ( !user_forceOverwrite ) {
        VP1Msg::messageWarningAllRed("The output file ("+fileName+") is already present in the current folder, but you don't use the '-f' flag to overwrite it. The program will be stopped. Please remove or move the existing file to another folder, or use the '-f' flag to replace it.");
        throw "existing output file";
    }
  }
  // open the DB connection
  GMDBManager db(fileName.toStdString());

  // check the DB connection
  if (db.checkIsDBOpen())
      qDebug() << "OK! Database is open!";
  else {
      qDebug() << "Database ERROR!! Exiting...";
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

  VP1Msg::messageDebug("end of GeoExporter::init().");
}
