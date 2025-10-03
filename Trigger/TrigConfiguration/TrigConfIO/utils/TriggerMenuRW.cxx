/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/



#include "TrigConfIO/JsonFileLoader.h"
#include "TrigConfIO/JsonFileWriterL1.h"
#include "TrigConfIO/JsonFileWriterHLT.h"
#include "TrigConfIO/TrigDBMenuLoader.h"
#include "TrigConfIO/TrigDBJobOptionsLoader.h"
#include "TrigConfIO/TrigDBL1PrescalesSetLoader.h"
#include "TrigConfIO/TrigDBHLTPrescalesSetLoader.h"
#include "TrigConfIO/TrigDBL1BunchGroupSetLoader.h"
#include "TrigConfIO/TrigDBMonitoringLoader.h"
#include "TrigConfIO/TrigDBCTPFilesLoader.h"
#include "TrigConfData/HLTMenu.h"
#include "TrigConfData/L1Menu.h"
#include "TrigConfData/L1PrescalesSet.h"
#include "TrigConfData/HLTPrescalesSet.h"
#include "TrigConfData/L1BunchGroupSet.h"

#include <cstdlib>
#include <vector>
#include <format> //for format error

using namespace std;

struct Config {
public:

   std::vector<std::string> knownParameters { 
      "file", "f", "smk", "l1psk", "hltpsk", "bgsk", "db", "crest-db", "crest-server", "crest-api", "write", "w", 
      "Write", "W", "help", "h", "detail", "d", "ctp", "c"
      };

   // parameters
   // input
   std::vector<std::string> inputFiles {};
   unsigned int smk { 0 };
   unsigned int l1psk { 0 };
   unsigned int hltpsk { 0 };
   unsigned int bgsk { 0 };
   std::string  dbalias { "TRIGGERDB_RUN3" };
   std::string crestDb { "" };
   const std::string crestServerDefault{ "http://crest-04.cern.ch/" };
   std::string  crestServer { crestServerDefault };
   std::string crestApi { "" };
   bool doCtp { false }; // flag to read CTP files

   // output
   bool         write { false }; // flag to enable writing
   bool         writeFromDataStructure { false }; // flag to enable writing of the L1Menu structure (if available)
   std::string  base { "" };

   // other
   bool         help { false };
   bool         detail { false };

   // to keep track of configuration errors
   vector<string> error;

   // parses the commandline 
   void parseProgramOptions(int argc, char* argv[]);

   // help
   void usage();

};


void Config::usage() {

   cout << "The program needs to be run with the following specifications:\n\n";
   cout << "TriggerMenuRW <options>\n";
   cout << "\n";
   cout << "[Input options]\n";
   cout << "  -f|--file             file1 [file2 [file3 ...]]     ... one or multiple json files\n";
   cout << "  --smk                 smk                           ... smk \n";
   cout << "  --l1psk               l1psk                         ... the L1 prescale key \n";
   cout << "  --hltpsk              hltpsk                        ... the HLT prescale key \n";
   cout << "  --bgsk                bgsk                          ... the bunchgroup key \n";
   cout << "  --db                  dbalias                       ... dbalias for oracle/frontier access (default " << dbalias << ") \n";
   cout << "  --crest-db            crestdb                       ... crest_db for Crest access, if specified Crest will be used (default '') \n";
   cout << "                                                          possible values: CONF_DATA_RUN3, CONF_MC_RUN3, CONF_REPR_RUN3\n";
   cout << "  --crest-server        server                        ... crest server (default " << crestServerDefault << ")\n";
   cout << "  --crest-api           version                       ... crest api version (default taken from CrestApi)";
   cout << "  -c|--ctp                                            ... if provided together with the SMK and DB then will read only CTP files from the DB and not the rest of the menu\n";
   cout << "[Output options]\n";
   cout << "  -w|--write            [base]                        ... to write out json files, e.g. L1menu[_<base>].json. base is optional.\n";
   cout << "  -W|--Write            [base]                        ... to write out json files from the internal structure (only for L1Menu), e.g. L1menu[_<base>].json. base is optional.\n";
   cout << "[Other options]\n";
   cout << "  -h|--help                                           ... this help\n";
   cout << "  -d|--detail                                         ... prints detailed job options\n";
   cout << "\n\n";
   cout << "Examples\n";
   cout << "  --file L1menu.json HLTMenu.json                     ... read L1Menu.json and HLTMenu.json and show some basic statistics\n";
   cout << "  --db TRIGGERDB_RUN3 --smk 3205                      ... read the L1 menu, HLT menu, HLT job options, and HLT monitoring groups for SMK 3205\n";
   cout << "  --db TRIGGERDB_RUN3 --smk 3205 -w                   ... read the information and write DB content directly as json files\n";
}

void
Config::parseProgramOptions(int argc, char* argv[]) {

   std::string currentParameter("");
   std::string listofUnknownParameters = "";

   for(int i=1; i<argc; i++) {

      std::string currentWord(argv[i]);
      bool isParam = currentWord[0]=='-'; // string starts with a '-', so it is a parameter name

      // get the parameter name
      int firstChar = currentWord.find_first_not_of('-');
      string paramName = currentWord.substr(firstChar);

      // check if the parameter is known
      if ( isParam && std::find(knownParameters.begin(), knownParameters.end(), paramName) == knownParameters.end() ) {
         listofUnknownParameters += " " + currentWord;
         continue;
      }

      if(isParam) {
         currentParameter = "";
         // check the boolean parameters
         if(paramName == "h" || paramName == "help" ) { help = true; continue; }
         if(paramName == "d" || paramName == "detail" ) { detail = true; continue; }
         if(paramName == "w" || paramName == "write" ) { write = true; }
         if(paramName == "W" || paramName == "Write" ) { writeFromDataStructure = true; }
         if(paramName == "c" || paramName == "ctp" ) { doCtp = true; }
         currentParameter = std::move(paramName);
         continue;
      }

      // now treat the parameter values

      // inputs
      if(currentParameter == "file" || currentParameter == "f") {
         inputFiles.push_back(std::move(currentWord));
         continue; 
      }
      if(currentParameter == "smk") { 
         smk = stoul(currentWord);
         continue; 
      }
      if(currentParameter == "l1psk") { 
         l1psk = stoul(currentWord);
         continue; 
      }
      if(currentParameter == "hltpsk") { 
         hltpsk = stoul(currentWord);
         continue; 
      }
      if(currentParameter == "bgsk") {
         bgsk = stoul(currentWord);
         continue;
      }
      if(currentParameter == "db") { 
         dbalias = std::move(currentWord);
         continue; 
      }
      if(currentParameter == "crest-db") { 
         crestDb = std::move(currentWord);
         continue; 
      }
      if(currentParameter == "crest-server") { 
         crestServer = std::move(currentWord);
         continue; 
      }
      if(currentParameter == "crest-api") { 
         crestApi = std::move(currentWord);
         continue; 
      }
      // output
      if(currentParameter == "write" || currentParameter == "w" || currentParameter == "Write" || currentParameter == "W") {
         base = std::move(currentWord);
         continue; 
      }

   }

   // some sanity checks
   if ( inputFiles.size() == 0 and smk == 0 and l1psk == 0 and hltpsk == 0 and bgsk == 0 ) {
      error.push_back("No input specified! Please provide either one of the following: input file(s), smk, l1psk, hltpsk, or bgsk");
   }

   if ( listofUnknownParameters.size() > 0 ) {
      error.push_back( string("Unknown parameter(s):") + listofUnknownParameters);
   }

}

namespace {
   bool
   writeJsonFile(const TrigConf::DataStructure & ds, const std::string & kind, const Config & cfg) {
      if( cfg.write ) {
         std::string filename = kind;
         if ( cfg.base != "" ) {
            filename += "_" + cfg.base;
         }
         filename += ".json";
         TrigConf::JsonFileLoader fileLoader;
         return fileLoader.saveFile(filename, ds); 
      } else if ( cfg.writeFromDataStructure ) {
         std::string filename = kind;
         if ( cfg.base != "" ) {
            filename += "_" + cfg.base;
         }
         filename += ".fromDS.json";
         if ( kind=="L1Menu" ) {
            TrigConf::JsonFileWriterL1 fileWriter;
            const auto & l1menu = dynamic_cast<const TrigConf::L1Menu &>(ds);
            return fileWriter.writeJsonFile(filename, l1menu); 
         } else if ( kind == "HLTMenu") {
            TrigConf::JsonFileWriterHLT fileWriter;
            const auto & hltmenu = dynamic_cast<const TrigConf::HLTMenu &>(ds);
            return fileWriter.writeJsonFile(filename, hltmenu);
         }
      }
      return true;
   }

   std::string
   outputFileName(const std::string & kind, const Config & cfg) {
      if( ! cfg.write )
         return "";
      std::string filename = kind;
      if ( cfg.base != "" ) {
         filename += "_" + cfg.base;
      }
      filename += ".json";
      return filename;
   }

}

int main(int argc, char** argv) {

   Config cfg;
   cfg.parseProgramOptions(argc, argv);
   if(cfg.help) {
      cfg.usage();
      return 0;
   }

   if(cfg.error.size()!=0) {
      for(const string & e: cfg.error)
         cerr << e << endl;
      cfg.usage();
      return 1;
   }

   if( cfg.inputFiles.size()>0 ) {
      // load config from files
      TrigConf::JsonFileLoader fileLoader;
      for (const std::string & fn : cfg.inputFiles) {
         // check if the file is L1 or HLT
         std::string filetype = fileLoader.getFileType( fn );
         if(filetype == "l1menu") {
            TrigConf::L1Menu l1menu;
            fileLoader.loadFile( fn, l1menu);
            cout << "Loaded L1 menu " << l1menu.name() << " with " << l1menu.size() << " items from " << fn << endl;
            l1menu.printMenu(cfg.detail);
            writeJsonFile(l1menu, "L1Menu", cfg);
         } else if(filetype == "hltmenu" ) {
            TrigConf::HLTMenu hltmenu;
            fileLoader.loadFile( fn, hltmenu);
            cout << "Loaded HLT menu " << hltmenu.name() << " with " << hltmenu.size() << " chains from " << fn << endl;
            hltmenu.printMenu(cfg.detail);
            writeJsonFile(hltmenu, "HLTMenu", cfg);
         } else if(filetype == "l1prescale" ) {
            TrigConf::L1PrescalesSet l1pss;
            fileLoader.loadFile( fn, l1pss);
            cout << "Loaded L1 prescales set file " << fn << " with " << l1pss.size() << " prescales from " << fn << endl;
            writeJsonFile(l1pss, "L1PrescalesSet", cfg);
         } else if(filetype == "hltprescale" ) {
            TrigConf::HLTPrescalesSet hltpss;
            fileLoader.loadFile( fn, hltpss);
            cout << "Loaded HLT prescales set file " << fn << " with " << hltpss.size() << " prescales from " << fn << endl;
            hltpss.printPrescaleSet(cfg.detail);
            writeJsonFile(hltpss, "HLTPrescalesSet", cfg);
         } else if(filetype == "bunchgroupset" ) {
            TrigConf::L1BunchGroupSet bgs;
            fileLoader.loadFile( fn, bgs);
            cout << "Loaded L1 BunchGroup set file " << fn << " with " << bgs.sizeNonEmpty() << " non-empty bunchgroups from " << fn << endl;
            bgs.printSummary(cfg.detail);
            writeJsonFile(bgs, "BunchGroupSet", cfg);
         } else if(filetype == "joboptions" ) {
            TrigConf::DataStructure jo;
            fileLoader.loadFile( fn, jo);
            cout << "Loaded job options with " << jo.getObject("properties").getKeys().size() << " properties from " << fn << endl;
            if( cfg.detail ) {
               TrigConf::DataStructure ds = jo.getObject("properties");
               for( const auto& alg : ds.data()) {
                  std::cout << alg.first << std::endl;
                  for( const auto& prop : alg.second ) {
                     std::cout << "      " << prop.first << " -> " << prop.second.data() << std::endl;
                  }
               }
            }
            writeJsonFile(jo, "HLTJobOptions", cfg);
         } else if(filetype == "hltmonitoringsummary" ) {
            TrigConf::HLTMonitoring mon;
            fileLoader.loadFile( fn, mon);
            cout << "Loaded HLT monnitoring with " << mon.size() << " signatures from " << fn << endl;
            mon.printMonConfig(cfg.detail);
            writeJsonFile(mon, "HLTMonitoring", cfg);
         } else {
            cerr << "File " << fn << " not recognized as being an L1 or HLT menu or prescale set or bunchgroup set" << endl;
         }
      }
   }

   if( cfg.smk != 0 && !cfg.doCtp ) {
      // load config from DB
      {
         // db menu loader
         TrigConf::TrigDBMenuLoader dbloader(cfg.dbalias);
         if(!cfg.crestDb.empty()) {
            dbloader.setCrestTrigDB(cfg.crestDb);
            dbloader.setCrestConnection(cfg.crestServer, cfg.crestApi);
         }
         
         // L1 menu
         {
            TrigConf::L1Menu l1menu;
            try {
               dbloader.loadL1Menu( cfg.smk, l1menu, outputFileName("L1Menu", cfg) );
            }
            catch(TrigConf::IOException & ex) {
               cout << "Could not load L1 menu. An exception occurred: " << ex.what() << endl;
            }
            if(l1menu) {
               cout << "Loaded L1 menu " << l1menu.name() << " with " << l1menu.size() << " items from " << cfg.dbalias << " with SMK " << cfg.smk << endl;
               if( cfg.detail ) {
                  l1menu.printMenu(true);
               }
            }
            cout << endl;
         }

         // HLT menu
         {
            TrigConf::HLTMenu hltmenu;
            try {
               dbloader.loadHLTMenu( cfg.smk, hltmenu, outputFileName("HLTMenu", cfg));
            }
            catch(TrigConf::IOException & ex) {
               cout << "Could not load HLT menu. An exception occurred: " << ex.what() << endl;
            }
            if (hltmenu) {
               cout << "Loaded HLT menu " << hltmenu.name() << " with " << hltmenu.size() << " chains from " << cfg.dbalias << " with SMK " << cfg.smk << endl;
               if( cfg.detail ) {
                  hltmenu.printMenu(true);
               }
            }
            cout << endl;
         }
      }

      // Job options
      {
         TrigConf::TrigDBJobOptionsLoader dbloader(cfg.dbalias);
         if(!cfg.crestDb.empty()) {
            dbloader.setCrestTrigDB(cfg.crestDb);
            dbloader.setCrestConnection(cfg.crestServer, cfg.crestApi);
         }
         TrigConf::DataStructure jo;
         try {
            dbloader.loadJobOptions( cfg.smk, jo, outputFileName("HLTJobOptions", cfg) );
         }
         catch(TrigConf::IOException & ex) {
            cout << "Could not load HLT job options. An exception occurred: " << ex.what() << endl;
         }
         if (jo) {
            cout << "Loaded job options with " << jo.getObject("properties").getKeys().size() << " entries from " << cfg.dbalias << " with SMK " << cfg.smk << endl;
            if( cfg.detail ) {
               TrigConf::DataStructure ds = jo.getObject("properties");
               for( const auto& alg : ds.data()) {
                  std::cout << alg.first << std::endl;
                  for( const auto& prop : alg.second ) {
                     std::cout << "      " << prop.first << " -> " << prop.second.data() << std::endl;
                  }
               }
            }
         }
         cout << endl;
      }

      // HLT monitoring groups
      {
         TrigConf::TrigDBMonitoringLoader dbloader(cfg.dbalias);
         if(!cfg.crestDb.empty()) {
            dbloader.setCrestTrigDB(cfg.crestDb);
            dbloader.setCrestConnection(cfg.crestServer, cfg.crestApi);
         }
         TrigConf::HLTMonitoring hltmon;
         try {
            dbloader.loadHLTMonitoring( cfg.smk, hltmon, outputFileName("HLTMonitoring", cfg));
         }
         catch(TrigConf::IOException & ex) {
            cout << "Could not load HLT monitoring. An exception occurred: " << ex.what() << endl;
         }
         if (hltmon) {
            cout << "Loaded HLT monitoring with " << hltmon.size() << " signatures rom " << cfg.dbalias << " with SMK " << cfg.smk << endl;
            if( cfg.detail ) {
               hltmon.printMonConfig(true);
            }
         }
         cout << endl;
      }

   }

   if( cfg.smk != 0 && cfg.doCtp ) {
      TrigConf::TrigDBCTPFilesLoader dbloader(cfg.dbalias);
      TrigConf::L1CTPFiles ctpfiles;
      try{
        dbloader.loadHardwareFiles(cfg.smk, ctpfiles, 0x0F, outputFileName("CTPFiles", cfg));
      } catch (std::format_error & e){
        cout << " format_error "<<e.what()<<" thrown in TriggerMenuRW.\n";
        return 1;
      }
      ctpfiles.print();
   }

   if( cfg.l1psk != 0 ) {
      // load L1 prescales set from DB
      TrigConf::TrigDBL1PrescalesSetLoader dbloader(cfg.dbalias);
      if(!cfg.crestDb.empty()) {
         dbloader.setCrestTrigDB(cfg.crestDb);
         dbloader.setCrestConnection(cfg.crestServer, cfg.crestApi);
      }
      TrigConf::L1PrescalesSet l1pss;
      try {
         dbloader.loadL1Prescales( cfg.l1psk, l1pss, outputFileName("L1PrescalesSet", cfg) );
      }
      catch(TrigConf::IOException & ex) {
         cout << "Could not load L1 prescales. An exception occurred: " << ex.what() << endl;
      }      
      if (l1pss) {
         cout << "Loaded L1 prescales set " << l1pss.name() << " with " << l1pss.size() << " prescales from " << cfg.dbalias << " with L1 PSK " << cfg.l1psk << endl;
      }
   }

   if( cfg.hltpsk != 0 ) {
      // load L1 prescales set from DB
      TrigConf::TrigDBHLTPrescalesSetLoader dbloader(cfg.dbalias);
      if(!cfg.crestDb.empty()) {
         dbloader.setCrestTrigDB(cfg.crestDb);
         dbloader.setCrestConnection(cfg.crestServer, cfg.crestApi);
      }
      TrigConf::HLTPrescalesSet hltpss;    
      try {
         dbloader.loadHLTPrescales( cfg.hltpsk, hltpss, outputFileName("HLTPrescalesSet", cfg) );
      }
      catch(TrigConf::IOException & ex) {
         cout << "Could not load HLT prescales. An exception occurred: " << ex.what() << endl;
      }      
      if (hltpss) {
         cout << "Loaded HLT prescales set " << hltpss.name() << " with " << hltpss.size() << " prescales from " << cfg.dbalias << " with HLT PSK " << cfg.hltpsk << endl;
      }
   }

   if( cfg.bgsk != 0 ) {
      // load L1 prescales set from DB
      TrigConf::TrigDBL1BunchGroupSetLoader dbloader(cfg.dbalias);
      if(!cfg.crestDb.empty()) {
         dbloader.setCrestTrigDB(cfg.crestDb);
         dbloader.setCrestConnection(cfg.crestServer, cfg.crestApi);
      }
      TrigConf::L1BunchGroupSet bgs;
      try {
         dbloader.loadBunchGroupSet( cfg.bgsk, bgs, outputFileName("BunchGroupSet", cfg) );
      }
      catch(TrigConf::IOException & ex) {
         cout << "Could not load bunchgroup set. An exception occurred: " << ex.what() << endl;
      }      
      if (bgs) {
         cout << "Loaded L1 bunchgroup set " << bgs.name() << " with " << bgs.size() << " bunchgroups from " << cfg.dbalias << " with BGSK " << cfg.bgsk << endl;
         if( cfg.detail ) {
            bgs.printSummary(true);
         }
      }
   }

   return 0;
}
