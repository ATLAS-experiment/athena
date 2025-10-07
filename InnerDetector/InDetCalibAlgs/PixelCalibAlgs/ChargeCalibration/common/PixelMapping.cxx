/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//  PixelMapping.cxx
//  PixelMapping
//
//  Created by sroe on 13/03/2023.
//

#include "PixelMapping.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>
namespace pix{

  PixelMapping::PixelMapping(const std::string & csvFilename){
    //read in csv file and fill data structures
    //lines in file are of format 'D2A_B02_S1_M3, 1950, 2, 1, 10, 0'
    //'#' at start of line denotes a comment or header
    std::ifstream fs;
    fs.open(csvFilename.c_str());
    if (not fs.is_open()){
      const std::string msg = "Failed to open the mapping file: " + csvFilename;
      throw std::runtime_error(msg);
    }
    std::string line;
    //string, digits, signed digits, digits, digits, signed digits
    std::regex re(R"delim(^(.*), (\d+), (-?\d+), (\d+), (\d+), (-?\d+)$)delim");
    auto dissectMatch=[](const std::smatch & m)->std::pair<std::string, Coordinates>{
      static constexpr size_t expectedSize(7);
      static constexpr size_t coordStart(2);
      std::pair<std::string, PixelMapping::Coordinates> result{};
      if (m.size()!=expectedSize) return result;
      const std::string name = m[1];
      Coordinates coords{};
      
      for (size_t i(0); i!=coords.size();++i){
        try{
          coords[i] = std::stoi(m.str(i+coordStart));
        } catch (std::invalid_argument &e){
          const std::string msg = "PixelMapping: Line of csv file in unexpected format, \n"+ std::string(m[0]);
          throw std::runtime_error(msg);
        }
      }
      return {name, coords};
    };
    while (std::getline(fs, line)){
      //hash at start indicates comment, do nothing
      if (line[0] == '#') continue;
      std::smatch m;
      bool matched = std::regex_match(line, m, re);
      if (not matched){
        std::cout<<"line in unexpected format:\n"<<line<<std::endl;
        continue;
      }
      const auto& [ptr, Ok]=m_internalMap.insert(dissectMatch(m));
      if (not Ok){
        std::cout<<"repeated entry:\n"<<line<<std::endl;
      }
    }
    fs.close();
  }
  
  
  void 
  PixelMapping::mapping(const std::string & geographicalID, int *hashID, int *bec, int *layer, int *phimod, int *etamod) const{
    auto pName = m_internalMap.find(geographicalID);
    if (pName == m_internalMap.end()){
      std::cout<<"id "<<geographicalID<<" not found in mapping"<<std::endl;
      return;
    }
    const auto & coordinates = pName->second;
    *hashID = coordinates[0];
    *bec = coordinates[1];
    *layer = coordinates[2];
    *phimod = coordinates[3];
    *etamod = coordinates[4];
    return;
  }
  
  
int PixelMapping::getID(const std::string & geographicalID) const {
    auto pName = m_internalMap.find(geographicalID);
    if (pName == m_internalMap.end()) {
        std::cout<<"id "<<geographicalID<<" not found in mapping"<<std::endl;
        return -1;
    }
    const auto & coordinates = pName->second;
    return coordinates[0];
}
  
  bool
  PixelMapping::contains(const std::string & geographicalID) const{
    return (m_internalMap.find(geographicalID) != m_internalMap.end());
  }
  
  int 
  PixelMapping::nModules() const{
    return m_internalMap.size();
  }


}
