/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////
// CDIReader.h, (c) ATLAS Detector software
//////////////////////////////////////////////////////////////////////

#ifndef ANALYSISCDIREADER_H
#define ANALYSISCDIREADER_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <unordered_map>
#include <algorithm>

#include "TFile.h"
#include "TDirectoryFile.h"
#include "TMath.h"
#include "TEnv.h"
#include "TObjString.h"

#include "CalibrationDataInterface/CalibrationDataVariables.h"
#include "CalibrationDataInterface/CalibrationDataInterfaceBase.h"
#include "CalibrationDataInterface/CalibrationDataContainer.h"

#include <filesystem>
#include <fstream>

class TH1;
class TFile;
class TMap;

namespace Analysis
{
  class CDIReader {
  
  typedef std::vector<std::string> Labels;
  typedef std::map<std::string, Labels> Data; // {"labels": ["B", "C", ...], "systematics" : ["syst1", "syst2", ...], "B_syst" : ["syst1", "syst3", ...], "C_syst" : ["syst2", "syst3", ...], "DSID" : ["1", "2", "3", ...]}
  typedef std::map<std::string, Data> WPoint;
  typedef std::map<std::string, WPoint> JetColl;
  typedef std::map<std::string, JetColl> Meta;

  public:

    /** normal constructor. The second argument, if true, will attempt to retrieve a 'recommended' set of uncertainties to be excluded from EV decomposition */
    CDIReader(const std::string& cdipath, bool verbose = false);
    ~CDIReader() = default;

    void printTaggers(){
      std::cout << "number of taggers: " << m_taggers.size() << std::endl;
      for(const std::string& el : m_taggers){
        std::cout << el << ", ";
      } std::cout << std::endl;
    }

    void printJetCollections(){
      std::cout << "number of jet collections: " << m_jetcollections.size() << std::endl;
      for(const std::string& el : m_jetcollections){
        std::cout << el << ", ";
      } std::cout << std::endl;
    }

    void printWorkingPoints(){
      std::cout << "number of working points: " << m_workingpoints.size() << std::endl;
      for(const std::string& el : m_workingpoints){
        std::cout << el << ", ";
      } std::cout << std::endl;
    }

    void printLabels(){
      std::cout << "number of labels: " << m_labels.size() << std::endl;
      for(const std::string& el : m_labels){
        std::cout << el << ", ";
      } std::cout << std::endl;
    }

    void printDSIDs(){
      Labels DSIDs = getDSIDs(); // without arguments, will return the vector of DSIDs
      std::cout << "number of DSIDs: " << DSIDs.size() << std::endl;
      for(const std::string& el : DSIDs){
        std::cout << el << ", ";
      } std::cout << std::endl;
    };

    bool checkConfig(const std::string& tagger, const std::string& jetcoll, const std::string& wp, bool verbose = false);
    Labels getDSIDs(const std::string& tagger = "", const std::string& jetcollection = "", const std::string& workingpoint = "");
    Labels getLabels(const std::string& tagger = "", const std::string& jetcollection = "", const std::string& workingpoint = "");
    Labels getWorkingPoints(const std::string& tagger, const std::string& jetcollection);
    Labels getJetCollections(const std::string& tagger);
    Labels getTaggers();

  private:
    // utility function for printing out the configuration options
    void printMetadata(int tagger = -1, int jetcoll = -1, int wpoint = -1, int label = -1);
    /** flag whether the initialization has been carried out */
    bool m_initialized = false;
    bool m_use_json = false;

    std::string m_cdipath = "";
    std::unique_ptr<TFile> m_CDIFile;
    std::set<std::string> m_taggers {};
    std::set<std::string> m_jetcollections {};
    std::set<std::string> m_workingpoints {};
    std::set<std::string> m_labels {};
    std::set<std::string> m_DSIDs {};

    void crawlCDI(TDirectoryFile* parentDir, int depth = 0, const std::string& metamap = "");
    
    void record_metadata(const std::string& datum, int depth = 0){
      switch(depth){
        case 0:
          m_taggers.insert(datum);
          break;
        case 1:
          m_jetcollections.insert(datum);
          break;
        case 2:
          m_workingpoints.insert(datum);
          break;
        case 3:
          m_labels.insert(datum);
          break;
        default:
          std::cout << " record_metadata :: depth is " << depth << std::endl;
      }
    }
    
    void record_metadata_map(const Data& data, const std::string& path){
      Labels metamap_path = split(path); // <"tagger", "jetcoll", "wp", "label"> 
      int size = metamap_path.size();
      if(size == 3){
        m_metadata[metamap_path.at(0)][metamap_path.at(1)][metamap_path.at(2)] = data;
      } else {
        std::cout << " record_metadata_map :: the directory structure doesn't match what we expect!" << std::endl;
      }
    }
    
    // this function defines how we identify the working point directory
    bool isWPdirectory(TList* list){
      // check the directory (TList) contents
      for(const auto label : *list){
        const char* labelname = label->GetName();
        if(strcmp(labelname, "B") == 0 || strcmp(labelname, "C") == 0
        || strcmp(labelname, "Light") == 0 || strcmp(labelname, "T") == 0
        || strcmp(labelname, "Z_BB") == 0 || strcmp(labelname, "QCD_BB") == 0
        || strcmp(labelname, "Top_BX") == 0) return true;
      }
      return false;
    }

    // local utility function: trim leading and trailing whitespace in the property strings
    std::string trim(const std::string& str, const std::string& whitespace = " \t") {
      const auto strBegin = str.find_first_not_of(whitespace);
      if (strBegin == std::string::npos){
        return ""; // no content
      }
      const auto strEnd = str.find_last_not_of(whitespace);
      const auto strRange = strEnd - strBegin + 1;
      return str.substr(strBegin, strRange);
    }
 
    // local utility function: split string into a vector of substrings separated by a specified separator
    std::vector<std::string> split(const std::string& str, char token = ';') {
      std::vector<std::string> result;
      if (str.size() > 0) {
        std::string::size_type end;
        std::string tmp(str);
        do {
          end = tmp.find(token);
          std::string entry = trim(tmp.substr(0,end));
          if (!entry.empty()) result.push_back(std::move(entry)); 
          if (end != std::string::npos) tmp = tmp.substr(end+1);
        } while (end != std::string::npos);
      }
      return result;
    }

    Labels m_label_vec;
    Meta m_metadata;

  };


}

#endif
