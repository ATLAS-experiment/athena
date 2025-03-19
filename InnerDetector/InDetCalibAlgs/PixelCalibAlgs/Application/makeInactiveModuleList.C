/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TKey.h"
#include "Riostream.h"

#include <array>
#include <vector>
#include <cmath>
#include <map>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <utility>
#include <filesystem>
#include <memory>
#include <numeric> //for std::accumulate
#include <regex>

#include "CxxUtils/checker_macros.h"


ATLAS_NO_CHECK_FILE_THREAD_SAFETY;  // standalone application

namespace fs = std::filesystem;

//global in this application
using Position = std::array<int,4>;
using LumiBlockVector = std::vector<std::pair<int, int>>;
using ModuleIdVector = std::vector<std::string>;
//
using FrontEndIdVector = std::vector<std::string>;
using FrontEndCodeVector = std::vector<std::string>;
//
static constexpr Position invalidPosition{-100,-100,-100,-100};
std::map<std::string, Position> pixelMapping;
std::map<Position, int> hashMapping;
std::map<int, int> channelMapping;

namespace{
 const std::array<std::string, 6> globalDiskNames{
    "Disk1A",
    "Disk2A",
    "Disk3A",
    "Disk1C",
    "Disk2C",
    "Disk3C"
  };
  //initialiser for layer names
  std::vector<std::string>
  layerNames(bool isIBL){
    std::vector<std::string> layers;
    if(isIBL) layers.push_back("IBL");
    layers.insert(layers.end(), {"B0", "B1", "B2"});
    return layers;
  }
  
  //initialiser for stave numbers
  std::vector<int>
  staveInitialiser(bool isIBL){
    if (isIBL) return std::vector<int>{14, 22, 38, 52};
    return std::vector<int>{22, 38, 52};
  }
  
  //check a certain root directory exists in the file
  bool
  directoryExists(TFile * pFile, const std::string & prefix){
    if (not pFile){
      std::cout<<"TFile pointer is invalid"<<std::endl;
      exit(1);
    }
    auto pDir = pFile->Get(prefix.substr(0,prefix.size()-1).c_str());
    return bool(pDir);
  }

  unsigned
  getNKeys (TFile* pFile, const std::string & path)
  {
    auto pDir = pFile->Get(path.c_str());
    if (not pDir){
      std::cout<<"Directory pointer is invalid; looking for "<<path<<std::endl;
      return 0;
    }
    return (static_cast<TDirectory*>(pDir))->GetListOfKeys()->GetSize();
  }
  
  //get the histoName from a root file and path, variable value ; this will be key in the maps
  std::string 
  getHistoName(TFile* pFile, const std::string & path, const unsigned var ){
    auto pDir = pFile->Get(path.c_str());
    if (not pDir){
      std::cout<<"Directory pointer is invalid; looking for "<<path<<std::endl;
      return std::string();
    }
    return (static_cast<TKey*>((static_cast<TDirectory*>(pDir))->GetListOfKeys()->At(var)))->GetName();
  };
  
  //get the histoPtr from a root file and path, variable value ; this will be value in the maps
  TH1*
  getHistoPtr(TFile* pFile, const std::string & path, const unsigned var ){
    auto h = dynamic_cast<TH1*>((static_cast<TKey*>(((static_cast<TDirectory*>(pFile->Get( path.c_str() ) ))->GetListOfKeys() )->At(var) ))->ReadObj());
    if (!h) {
      std::cout << "Error: Cannot read hist number " << var << " under " << path << " as a TH1\n";
      std::exit(1);
    }
    return h;
  };
  TH2*
  getHistoPtr2D(TFile* pFile, const std::string & path, const unsigned var ){
    auto h = dynamic_cast<TH2*>((static_cast<TKey*>(((static_cast<TDirectory*>(pFile->Get( path.c_str() ) ))->GetListOfKeys() )->At(var) ))->ReadObj());
    if (!h) {
      std::cout << "Error: Cannot read hist number " << var << " under " << path << " as a TH2\n";
      std::exit(1);
    }
    return h;
  };
  
  std::vector<std::string> 
  splitter(const std::string &s, const std::string & delim) {
    static const std::regex reg(delim);
    std::sregex_token_iterator iter(s.begin(), s.end(), reg, -1);
    std::sregex_token_iterator end;
    std::vector<std::string> elems(iter, end);
    return elems;
  }
  
  //find a file from the datapath paths and return its full path+name
  std::string
  findFile(const std::string & datapaths, const std::string & filename){
    std::string fullPath;
    const std::vector<std::string> paths = splitter(datapaths, ":");//paths to search
    auto pathMatches = [&filename](const std::string & p)->bool{
      return fs::exists(p+"/"+filename);
    };
    auto ptr = std::find_if(paths.begin(), paths.end(), pathMatches);
    if (ptr!=paths.end()) return *ptr +"/"+filename;
    //file not found if you reach here
    std::cout <<"File "<<filename<<" not found in any datapath searched."<<std::endl;
    return fullPath;
  }
  
  Position 
  getPositionFromDCSID (const std::string & module_name){
    const static std::regex r("(.I.{11}[^78]).{2}");
    std::smatch m;
    bool matched = std::regex_match(module_name, m, r);
    auto  ptr = pixelMapping.find(matched ? m[1] : module_name);
    if (ptr != pixelMapping.end()) return ptr->second;
    return invalidPosition;
  }
  
  void setPixelMapping(bool isIBL){
    const std::string cmtpath = std::getenv("DATAPATH");
    const std::string mappingFilename = isIBL ? ("PixelMapping_Run2.dat") : ("PixelMapping_May08.dat");
    const std::string & mappingFullPath = findFile(cmtpath, mappingFilename);
    if (mappingFullPath.empty()) exit(1);
    std::cout<<"Mapping file path: "<<mappingFullPath<<std::endl; 
    std::ifstream ifs(mappingFullPath);
    Position position{};
    std::string module_name;
    //tmp_position[0] = barrel_ec; tmp_position[1] = layer; tmp_position[2] = module_phi;tmp_position[3] = module_eta;
    while(ifs >> position[0] >> position[1] >> position[2] >> position[3] >> module_name) {
      const auto & [ptr, inserted] = pixelMapping.insert({module_name, position});
      if (not inserted) std::cout<<"Duplicate name inserted for module_name "<<module_name<<std::endl;
    }
    //
    if (isIBL){
      const std::string hashDCSIdPath = findFile(cmtpath, "HashVSdcsID.dat");
      if (hashDCSIdPath.empty()) exit(1);
      std::cout<<"hashDCSIdPath file path: "<<hashDCSIdPath<<std::endl;
      std::ifstream ifs2(hashDCSIdPath);
      int hash{}; 
      int ignoreInt{};
      std::string ignoreString;
      while(ifs2 >> ignoreInt >> hash >> ignoreInt >> ignoreInt >> position[0] >> position[1] >> position[2] >> position[3] >> ignoreInt >> ignoreInt >> ignoreString) {
        const auto & [ptr, inserted] = hashMapping.insert({position,hash});
        if (not inserted) std::cout<<"Duplicate position inserted for hash "<<hash<<std::endl;
      }
    }
    //
    const std::string run2TableFullPath = findFile(cmtpath,"table_Run2.txt");
    if (run2TableFullPath.empty()) exit(1);
    std::cout<<"run2TableFullPath file path: "<<run2TableFullPath<<std::endl;
    std::ifstream ifs3(run2TableFullPath);
    /* in table_Run2.txt the lines are like
        PixMapOverlayWr...   INFO     0        20480  [2.1.-4.0.0.0.0.0] LI_S15_C_34_M3_C1_2 
        PixMapOverlayWr...   INFO   135     26955776  [2.1.-2.2.27.0.0.0]D3C-B03-S2-M2   
    */
    const static std::regex re(R"(^PixMapOverlayWr...   INFO\s+([0-9]*)\s+([0-9]*)\s+\[([-.0-9]*)\]\s*([-_A-Z0-9]*)\s*$)");  
    std::string str;
    while(getline(ifs3, str)){
      std::smatch sm;
      if (!std::regex_match (str, sm,re)) continue;
      const auto& [ptr, inserted] = channelMapping.insert({std::stoi(sm[2]), std::stoi(sm[1])}); //{hash, channel} pair
      if (not inserted) std::cout<<"Duplicate chanHash at module "<<sm[4]<<std::endl;
    }// while(getline(ifs3, str))
    std::cout<<"Channel mapping size "<<channelMapping.size()<<std::endl;
  }
  
  //-----------------------------
  // Function to read info from Lumi-block information (IBL for each FE, rest for each module)
  //-----------------------------
  std::pair< ModuleIdVector, LumiBlockVector > 
  checkInactiveModule(bool isIBL, TFile* hitMapFile){
    ModuleIdVector moduleIds;
    LumiBlockVector lumiblocks;
    //-----------------------------
    // Everything adapted to new noise maps
    //-----------------------------
    TH1* nEventsLBHistogram = 0;
    hitMapFile->GetObject("NEventsLB", nEventsLBHistogram);
  
    //-------------------------------
    // Initialization
    //-------------------------------

    std::map<std::string, TH1*> lbdep;
    //------------------------------------
    // read hit maps from input file
    //
    // endcaps
    // 48 modules each disk
    // phi_index [0,327]
    // eta_index [0,143]
    //------------------------------------
    std::string prefix("All/OccupancyLb/");//might modify this, so not const
    //check this directory exists, modify the prefix if not
    if (not directoryExists(hitMapFile, prefix)){
      std::cout <<"Directory "<<prefix<<" does not exist in the chosen hit map file; trying alternative name."<<std::endl;
      prefix = "OccupancyLb/";
      if (not directoryExists(hitMapFile, prefix)){
        std::cout <<"Directory "<<prefix<<" also does not exist in the chosen hit map file. Aborting."<<std::endl;
        exit(1);
      }
    }
    for(const auto & d: globalDiskNames){
      std::string lbdepPath =  prefix + d;
      unsigned nkeys = getNKeys(hitMapFile,lbdepPath);
      for(unsigned phi = 0; phi < nkeys; phi++){
        const std::string & name = getHistoName(hitMapFile,lbdepPath, phi);
        char invalidCharacter = name[0]; //has to be L or D to be a proper module ID
        if(invalidCharacter=='O'){continue;} //Sometimes the function grabs the directory name, propagating this needs to be avoided
        lbdep[name] = getHistoPtr(hitMapFile,lbdepPath, phi);
        lbdep[name]->SetName(name.c_str());
      }
    }

    //-----------------------------------------------------------
    // read hit maps from input file
    //
    // barrel
    // 22, 38, 52 slaves for layer 0, 1 and 2
    // 13 modules per stave
    // phi_index [0,319] for layer 0, [0,327] for layer 1 & 2
    // eta_index [0,143] for layer 0, 1 & 2
    //-----------------------------------------------------------
    const std::vector<int> & staves = staveInitialiser(isIBL);
    const std::vector<std::string> & layers = layerNames(isIBL);
  
    for(unsigned int layer = 0; layer < staves.size(); layer++){
      std::string lbdepPath = prefix + layers[layer];
      //int nModulesPerStave = 13;
      //if (isIBL && layer == 0) nModulesPerStave = 32; // --- IBL --- //
      //const int nModulesTotal = staves[layer] * nModulesPerStave;
      unsigned nkeys = getNKeys(hitMapFile,lbdepPath);
      for(unsigned module = 0; module < nkeys; module++){ // loop on modules
        const std::string & name =  getHistoName(hitMapFile,lbdepPath, module);
        char invalidCharacter = name[0]; //has to be L or D to be a proper module ID
        if(invalidCharacter=='O'){continue;} //Sometimes the function grabs the directory name, propagating this needs to be avoided
        lbdep[name] = getHistoPtr(hitMapFile,lbdepPath, module);
        lbdep[name]->SetName(name.c_str());
      }
    }
  
    const int nbin = nEventsLBHistogram->GetNbinsX();
    for(const auto & [moduleID, pHisto] : lbdep){
      int LB_start = 0;
      int LB_end = 0;
      int flag_start = 0;
    
      //-----------------------------
      // Writing list of modules (or FEs in IBL modules) that do not work during a LB
      //-----------------------------
      for (int LB = 0; LB <= nbin; LB++) {
        if(nEventsLBHistogram->GetBinContent(LB) < 80.) {
          continue; // #to ensure that the LB contains at least 80 events
        }
        if(pHisto->GetBinContent(LB) == 0) { // (#module hits in LB) / (#events in LB) < 1
          if(flag_start == 0){
            flag_start = 1;
            LB_start = LB-1;
          }
          LB_end = LB-1;
        }else{// the module have hits
          if(flag_start == 1){
            flag_start = 0;
            moduleIds.push_back(moduleID);
            lumiblocks.push_back({LB_start, LB_end});
          }
        }
      }
      if(flag_start == 1){
        moduleIds.push_back(moduleID);
        lumiblocks.push_back({LB_start, LB_end});
      }
    }//for .. lbdep
    return std::pair(moduleIds, lumiblocks);
  }
  
  
  
  
  //---------------------------------------
  // Make a txt file out of masked modules/FEs
  //---------------------------------------
  void
  make_txt(const std::string& srun,
           int npush_back,
           int npush_backFE,
           const std::vector<std::string>& vsFE,
           const ModuleIdVector& vsmodule,
           const LumiBlockVector& vLB,
           const std::vector<std::string>& FEcode)
  {
    std::string spyfilename = "./PixelModuleFeMask_run" + srun;
    spyfilename += ".txt";
    std::ofstream txtFile;
    txtFile.open(spyfilename.c_str(),std::ofstream::out);
    std::cout <<"spyfilename: "<<spyfilename << std::endl;
  
    //---------------------------------------
    // List of masked FE
    //---------------------------------------
    std::vector<std::vector<int>> channel_FEcode(npush_backFE+1, std::vector<int>(2));
    std::cout<<"Number  of FE Codes " << npush_backFE<<std::endl;
    if(npush_backFE > 0){
      for(int i=1; i<=npush_backFE; i++){
        const auto & position = getPositionFromDCSID(vsFE[i-1]);
        if (position == invalidPosition) continue;
        auto pairPtr = hashMapping.find(position);
        if (pairPtr == hashMapping.end()) continue;
        int module_hash = pairPtr->second;
        const auto  ptr = channelMapping.find(module_hash);
        if (ptr == channelMapping.end()) continue;
        const int channel = ptr->second;
        const int FECode = std::stoi(FEcode[i-1]);
        channel_FEcode[i][0]=channel;
        channel_FEcode[i][1]=FECode;
        for(int l=1; l<i;l++){// Calculating/getting values for list
          if (channel_FEcode[i][0]>channel_FEcode[l][0]){
            continue;
          }
          else{
            for(int k=0; k<i-l; k++){
              channel_FEcode[i-k][0]=channel_FEcode[i-1-k][0];
              channel_FEcode[i-k][1]=channel_FEcode[i-1-k][1];
            }
          }
          channel_FEcode[l][0]=channel;
          channel_FEcode[l][1]=FECode;
          break;
        }
      }
    }
  
    auto it_smodule = vsmodule.begin();
    auto it_LB = vLB.begin();
    std::cout<<"Number to push back "<<npush_back<<std::endl;
    static constexpr int defaultCode=65535;
    //---------------------------------------
    //List of masked modules and FEs(IBL) per LB
    //---------------------------------------
    std::vector<std::vector<unsigned long>> channel_Modulecode(npush_back+1, std::vector<unsigned long>(4));
 
    if(npush_back > 0){
      using RX = std::regex;
      /*
        module names are like D3C-B03-S2-M2, LI_S01_C_M4_C8_1, LI_S05_A_M2_A3
      */
      static const std::array<RX, 5> arr={RX(".I.{11}(7|8).*"), RX(".I.{5}A.{5}[^78]_1"), RX(".I.{5}C.{5}[^78]_1"), RX(".I.{5}A.{5}[^78]_2"), RX(".I.{5}C.{5}[^78]_2")  };
      static constexpr std::array<int, 5> codes{0,1,2,2,1};//module codes corresponding to the module name matches
      for(int i=1; i<=npush_back; i++){
        const auto & position = getPositionFromDCSID(*it_smodule);
        if (position == invalidPosition) continue;
        std::string module_name = *it_smodule;
        auto pairPtr = hashMapping.find(position);
        if (pairPtr == hashMapping.end()) continue;
        int module_hash = pairPtr->second;
        const auto & ptr = channelMapping.find(module_hash);
        if (ptr == channelMapping.end()) continue;
        unsigned long channel = static_cast<unsigned long>(ptr->second);       
        unsigned long sRun = std::stoul (srun);
        unsigned long iov_start = (sRun << 32) + it_LB->first;
        unsigned long iov_end = (sRun << 32) + it_LB->second;
        channel_Modulecode[i]={channel, defaultCode, iov_start, iov_end};
        //
        auto findMatch = [&module_name](const RX & rx)->bool{
          return std::regex_match(module_name, rx);
        };
      
        for(int l=1; l<=i;l++){//Calculating/getting values for list
          if (channel_Modulecode[i][0]>channel_Modulecode[l][0]){
            continue;
          }
          for(int k=0; k<i-l; k++){
            channel_Modulecode[i-k]=channel_Modulecode[i-1-k];
          }
          channel_Modulecode[l][0]=channel;
          channel_Modulecode[l][1]=defaultCode;
          auto pMatch = std::find_if(arr.begin(),arr.end(),findMatch);
          if (pMatch != arr.end()){
            const auto dist = std::distance(arr.begin(), pMatch);
            channel_Modulecode[l][1]=codes[dist];
          }
          channel_Modulecode[l][2]=iov_start;
          channel_Modulecode[l][3]=iov_end;
          break;
        }
        ++it_smodule;
        ++it_LB;
      }  
    }
    //-----------------------------------
    // Combine both lists
    //-----------------------------------
    std::vector<std::vector<unsigned long>> combine(npush_backFE+npush_back+1, std::vector<unsigned long>(4));
    std::string module_info;
    for(int i=1; i<=npush_backFE+npush_back; i++){
      if (i<=npush_backFE){
        combine[i][0]=channel_FEcode[i][0];
        combine[i][1]=channel_FEcode[i][1];
        combine[i][2]=0;
        combine[i][3]=0;
        if(combine[i][1]==65535)
          continue;     
      } else {
        combine[i]=channel_Modulecode[i-npush_backFE];
      }
      if(combine[i][1]==65535){
       combine[i][1]=0;
      }
      module_info =std::to_string(combine[i][0])+" "+std::to_string(combine[i][1])+" "+std::to_string(combine[i][2])+" "+std::to_string(combine[i][3])+"\n";
      txtFile << module_info;
    }
    txtFile.close();
  }
  
  std::pair<FrontEndIdVector, FrontEndCodeVector> 
  checkInactiveFEs(bool isIBL, TFile* hitMapFile){//, int &npush_backFE, 
                        //std::vector<std::string> &vsFE, std::vector<std::string> &FEcode){ //Function for FE masking
    FrontEndIdVector frontEndIds;
    FrontEndCodeVector frontEndCodes;
    //-------------------------------
    // Everything adapted to new noise maps
    // Initialization
    //-------------------------------

    std::map<std::string, TH2*> hitMaps;
    //------------------------------------
    // read hit maps from input file
    //
    // endcaps
    // 48 modules each disk
    // phi_index [0,327]
    // eta_index [0,143]
    //------------------------------------
 
    std::string hitMapsDirName{"All/Occupancy2d/"};
    if (not directoryExists(hitMapFile, hitMapsDirName)){
      std::cout << "Directory "<<hitMapsDirName <<" does not exist in the hits map file; trying alternative."<<std::endl;
      hitMapsDirName = "Occupancy2d/";
      if (not directoryExists(hitMapFile, hitMapsDirName)){
        std::cout << "Directory "<<hitMapsDirName <<" also does not exist. Aborting."<<std::endl;
        exit(1);
      }
    }
  
    for(const auto & component: globalDiskNames){
      const std::string hitMapsPath = hitMapsDirName + component;
      unsigned nkeys = getNKeys(hitMapFile,hitMapsPath);
      for(unsigned phi = 0; phi < nkeys; phi++){
        const std::string & name = getHistoName(hitMapFile, hitMapsPath, phi);
        char invalidCharacter = name[0]; //has to be L or D to be a proper module ID
        if(invalidCharacter=='O'){continue;} //Sometimes the function grabs the directory name, propagating this needs to be avoided
        hitMaps[name] = getHistoPtr2D(hitMapFile, hitMapsPath, phi);
      }
    } // loop over k
  

    //-----------------------------------------------------------
    // read hit maps from input file
    //
    // barrel
    // 22, 38, 52 slaves for layer 0, 1 and 2
    // 13 modules per stave
    // phi_index [0,319] for layer 0, [0,327] for layer 1 & 2
    // eta_index [0,143] for layer 0, 1 & 2
    //-----------------------------------------------------------
    // E.G
    // D1A_B01_S1_M2
    const std::vector<int> & staves = staveInitialiser(isIBL);
    const std::vector<std::string> & layers = layerNames(isIBL);

    for(unsigned int layer = 0; layer < staves.size(); layer++){
      const std::string hitMapsPath = hitMapsDirName + layers[layer];
      //int nModulesPerStave = 13;
      //if (isIBL && layer == 0) nModulesPerStave = 32; // --- IBL --- //
      if (layer !=0){						//IBL ignored,because treated in function for LB information
        unsigned nkeys = getNKeys(hitMapFile,hitMapsPath);
        for(unsigned module = 0; module < nkeys; module++){ // loop on modules
          const std::string & name = getHistoName(hitMapFile, hitMapsPath, module);
          char invalidCharacter = name[0]; //has to be L or D to be a proper module ID
          if(invalidCharacter=='O'){continue;} //Sometimes the function grabs the directory name, propagating this needs to be avoided
          hitMaps[name] = getHistoPtr2D(hitMapFile, hitMapsPath, module);
          hitMaps[name]->SetName(name.c_str());
        }
      }
    }
    //---------------------------------------
    // Writing list of FE that shall be masked
    //---------------------------------------
    std::cout<<"hitMaps.size(): "<<hitMaps.size()<<std::endl;
    int modulesWithNoHits = 0;
    //int a=0; //counting variable
    for(const auto & [moduleID, pHisto] : hitMaps){
      //---------------------------------------
      // Further variables for counting
      //---------------------------------------
      int nbinx = pHisto->GetNbinsX();
      int nbiny = pHisto->GetNbinsY();
      std::array<double, 16> FE={};
   
      //---------------------------------------
      // Variables to identify modules
      //---------------------------------------
      std::size_t disk1A = moduleID.find("D1A");
      std::size_t disk2A = moduleID.find("D2A");
      std::size_t disk3A = moduleID.find("D3A");
      std::size_t disk1C = moduleID.find("D1C");
      std::size_t disk2C = moduleID.find("D2C");
      std::size_t disk3C = moduleID.find("D3C");
      std::size_t M1_find = moduleID.find("M1");
      std::size_t M2_find = moduleID.find("M2");
      std::size_t M3_find = moduleID.find("M3");
      std::size_t M4_find = moduleID.find("M4");
      std::size_t M5_find = moduleID.find("M5");
      std::size_t M6_find = moduleID.find("M6");
      //---------------------------------------
      // Check for hits in FEs
      //---------------------------------------
      bool inDiskA1to3 = (!disk1A or !disk2A or !disk3A);
      bool isM1M2orM3 =  (M1_find==11 or M2_find==11 or M3_find==11);
      bool inDiskC1to3 = (!disk1C or !disk2C or !disk3C);
      bool isM4M5orM6 =  (M4_find==11 or M5_find==11 or M6_find==11);
      bool found = (inDiskA1to3 and isM1M2orM3) or  (inDiskC1to3 and isM4M5orM6);
      //limits in original code; last one is dummy large number
      constexpr std::array<int, 8> kBounds{18, 36, 54, 72, 90, 108, 126, 1000000};
      //FE indices swap around for lval>164
      constexpr std::array<std::pair<int, int>, 8> indices{
        {{8,7},{9,6}, {10,5}, {11,4}, {12,3}, {13,2}, {14, 1}, {15,0}}
      };
      auto findIndices=[&kBounds, &indices](int kVal)->std::pair<int, int>{
        auto place = std::lower_bound(kBounds.begin(), kBounds.end(), kVal);
        auto i = std::distance(kBounds.begin(), place);
        return indices[i];
      };
      auto returnIndex=[](const std::pair<int, int> indices, bool found, int lVal)->int{
        if (lVal>164) return found ? indices.first : indices.second;
        return found ? indices.second : indices.first;
      };
      for(int k=1; k<=nbinx; k++){
        const auto & relevantPair = findIndices(k);
        for(int l=1; l<=nbiny; l++){
          const bool hitExists = pHisto->GetBinContent(k,l)!=0;
          if (not hitExists) continue;
          const int i = returnIndex(relevantPair, found, l);
          ++FE[i];
        }
      }// Hits in FEs checked 
    
      //---------------------------------------
      // Variables for threshold
      //---------------------------------------
      int sum=0;
      //how many zeroes in the array?
      const int numZeroes = std::count(FE.begin(), FE.end(), 0.);
      int denom = FE.size() - numZeroes;
      //---------------------------------------
      // Making list of masked FE
      //---------------------------------------
      int average_hit=0;
      if(denom!=0){ 
       average_hit = std::accumulate(FE.begin(), FE.end(), 0.)/denom;
      } else {
        ++modulesWithNoHits;
      }
      const auto ninetyPercent = 0.9*average_hit;
      for (int i(0); i!= FE.size();++i){
        if(FE[i]<=ninetyPercent){
          sum=sum+std::pow(2,i);//calculating number to know which FEs in the module shall be masked
        }
      }
      //
      if (sum!=0){
        frontEndCodes.push_back(std::to_string(sum));
        frontEndIds.push_back(moduleID);
      }
    }
    std::cout<<modulesWithNoHits<<" modules had no hits out of "<<hitMaps.size()<<" modules in the hit maps."<<std::endl;
    return std::pair(frontEndIds, frontEndCodes);
  }
  //
 
}

int main(int argc, char* argv[]){
  const bool isIBL = true;
  //-----------------------------------
  //Usage configuration of the program
  //------------------------------------
  std::string runNumber = "0";
  std::string finname = "";
  std::string sflag_first = "";

  bool flag_first = false;
  if( argc == 2 ){
    sflag_first = argv[1];
    if( sflag_first.find("first") != std::string::npos ) flag_first = true;
  }else if( argc == 3 ){
    runNumber = argv[1];
    finname = argv[2];
  }else{
    std::cout << "To make inactive module list: " << std::endl;
    std::cout << ">makeInactiveModuleList.exe [run #] [ROOT file]" << std::endl;
    std::cout << "Then, PixelModuleFeMask_run[run#].txt is created." << std::endl;
    return 0;
  }

  //-----------------------------
  //Read pixel mapping
  //------------------------------
  setPixelMapping(isIBL);
  //-----------------------------
  //Open input and output files; define variables
  //------------------------------
  int npush_back = 0;
  int npush_backFE = 0;
  FrontEndIdVector frontEnds;
  FrontEndCodeVector codes;
  ModuleIdVector moduleIds;
  LumiBlockVector lumiblocks;
  //
  if(flag_first != true){
    auto hitMapFile = std::make_unique<TFile>(finname.c_str(), "READ");
    if( !hitMapFile || !hitMapFile->IsOpen() ){
      std::cerr << "Error: Unable to open input file." << std::endl;
      return 1;
    
    }
    std::tie(frontEnds, codes) = checkInactiveFEs(isIBL, hitMapFile.get());
    npush_backFE += codes.size();
    std::tie(moduleIds, lumiblocks) = checkInactiveModule(isIBL, hitMapFile.get());
    npush_back += lumiblocks.size();
    hitMapFile->Close();
  }
  //-----------------------------
  // Open file to save txt fragment with disabled modules
  //-----------------------------
  std::cout<<"create txt file"<<std::endl;
  make_txt(runNumber, npush_back, npush_backFE, frontEnds,  moduleIds, lumiblocks, codes);
  return 0;
}


