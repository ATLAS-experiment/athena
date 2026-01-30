/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <iostream>
#include <vector>
#include <memory>
#include <stdint.h>

#include "CxxUtils/checker_macros.h"

#include "TrigConfBase/TrigConfMessaging.h"
#include "TrigConfIO/JsonFileLoader.h"
#include "TrigConfData/L1Menu.h"

#include "L1TopoCoreSim/TopoSteering.h"
#include "L1TopoCoreSim/StandaloneL1TopoHistSvc.h"
#include "L1TopoCoreSim/TopoASCIIReader.h"
#include "L1TopoInterfaces/Decision.h"

#include "L1TopoEvent/CompositeTOB.h"
#include "L1TopoCoreSim/Connector.h"
#include "L1TopoCoreSim/DecisionConnector.h"

#include "L1TopoEvent/MetTOB.h"

#include "TFile.h"
#include "TH1.h"

using namespace std;

int printHelp(const char * exeName) {
   cout << "Please specify menu and data file and optionally the message levels for the framework and the algorithms:" << endl << endl;
   cout << exeName << " <menu.json> <data.txt> [INFO|DEBUG|WARNING] [INFO|DEBUG|WARNING] [filename.root] [optional arguments]" << endl << endl;
   cout << "optional arguments:" << endl
        << "   -o|--outfile <filename.root>" << endl
        << "   -n|--nevt <#events>" << endl
        << "      --msgLvl INFO|DEBUG|WARNING" << endl
        << "      --algMsgLvl INFO|DEBUG|WARNING" << endl << endl;
   return 1;
}

int run ATLAS_NOT_THREAD_SAFE (int argc, const char* argv[]) {

   if(argc<3) {
      return printHelp(argv[0]);
   }

   TrigConf::MSGTC::Level msgLvl = TrigConf::MSGTC::WARNING;
   TrigConf::MSGTC::Level algMsgLvl = TrigConf::MSGTC::WARNING;
   string filename = "L1TopoSimulation.root";
   int nevt=-1;
   bool isLegacy = false;

   int cmsg=0;
   for(int c=0; c<argc; ++c) {
      auto arg = std::string(argv[c]);
      if(arg=="-h" or arg=="--help") {
         return printHelp(argv[0]);
      }
      if( (arg=="-n" or arg=="--nevt") and c<=argc) {
         nevt = std::stoi(std::string(argv[++c]));
      }
      if( arg=="--msgLvl" and c<=argc ) {
         string msgInput(argv[++c]);
         msgLvl = (msgInput=="DEBUG")?TrigConf::MSGTC::DEBUG:(msgInput=="INFO")?TrigConf::MSGTC::INFO:TrigConf::MSGTC::WARNING;
      }
      if( (arg=="-o" or arg=="--outfile") and c<=argc ) {
         filename = std::string(argv[++c]);
      }
      if( (arg=="-l" or arg=="--legacy") and c<=argc ) {
         isLegacy = true;
      }
      if( arg=="--algMsgLvl" and c<=argc ) {
         string msgInput(argv[++c]);
         algMsgLvl = (msgInput=="DEBUG")?TrigConf::MSGTC::DEBUG:(msgInput=="INFO")?TrigConf::MSGTC::INFO:TrigConf::MSGTC::WARNING;
      }
      if( arg=="DEBUG" ) {
         if(cmsg==0) {
            msgLvl = TrigConf::MSGTC::DEBUG; cmsg = 1;
         } else {
            algMsgLvl = TrigConf::MSGTC::DEBUG; cmsg = 1;
         }
      }
      if( arg=="INFO" ) {
         if(cmsg==0) {
            msgLvl = TrigConf::MSGTC::INFO; cmsg = 1;
         } else {
            algMsgLvl = TrigConf::MSGTC::INFO; cmsg = 1;
         }
      }
      if( arg=="WARNING" ) {
         if(cmsg==0) {
            msgLvl = TrigConf::MSGTC::WARNING; cmsg = 1;
         } else {
            algMsgLvl = TrigConf::MSGTC::WARNING; cmsg = 1;
         }
      }
      if( arg.size()>5 and arg.find(".root", arg.size()-5)!=std::string::npos ) {
         filename = std::move(arg);
      }
   }

   TrigConf::MsgStreamTC msg("TopoStandalone");
   msg.setLevel( msgLvl );

   // read the menu
   TrigConf::L1Menu l1menu;
   TrigConf::JsonFileLoader fileLoader;
   fileLoader.loadFile(argv[1], l1menu);


   // instantiate steering
   TCS::TopoSteering steering;
   steering.setUseBitwise(false);
   steering.setLegacyMode(isLegacy);
   steering.setupFromConfiguration(l1menu);

   steering.setMsgLevel( msgLvl );

   steering.setAlgMsgLevel( algMsgLvl );

   std::shared_ptr<IL1TopoHistSvc> topoHistSvc = std::shared_ptr<IL1TopoHistSvc>( new StandaloneL1TopoHistSvc() );
   steering.setHistSvc(std::move(topoHistSvc));


   steering.initializeAlgorithms();

   TCS::TopoASCIIReader reader; // instantiate ascii reader

   reader.setVerbosity(0); // disable print to screen
   // load ascii event file
   reader.loadInput(argv[2]);
   reader.validateInput();
   // instantiate input event
   TCS::TopoInputEvent & inputEvent = steering.inputEvent();
   inputEvent.msg().setLevel( msgLvl );
   reader.setInputEvent(&inputEvent);

   // loop over the events
   while(nevt-- and reader.getNextEvent()) {
      msg << TrigConf::MSGTC::INFO << "=======================================================" << TrigConf::endmsgtc;
      steering.executeEvent();
      steering.reset();
   }
   msg << TrigConf::MSGTC::INFO << "=======================================================" << TrigConf::endmsgtc;
   steering.saveHist();
   reader.printFileSummary();
   return 0;
}


int main ATLAS_NOT_THREAD_SAFE (int argc, const char * argv[]) {
   try {
      return run(argc, argv);
   }
   catch(std::exception & e) {
      cerr << "Caught exception: " << e.what() << endl;
      return 1;
   }
   return 0;
}

