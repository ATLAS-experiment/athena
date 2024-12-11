/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "GNNTrackReaderTool.h"

// Framework include(s).
#include "PathResolver/PathResolver.h"
#include <cmath>
#include <fstream>

InDet::GNNTrackReaderTool::GNNTrackReaderTool(
  const std::string& type, const std::string& name, const IInterface* parent):
    base_class(type, name, parent)
{
    declareInterface<IGNNTrackReaderTool>(this);
}

MsgStream&  InDet::GNNTrackReaderTool::dump( MsgStream& out ) const
{
  out<<std::endl;
  return dumpevent(out);
}

std::ostream& InDet::GNNTrackReaderTool::dump( std::ostream& out ) const
{
  return out;
}

MsgStream& InDet::GNNTrackReaderTool::dumpevent( MsgStream& out ) const
{
  out<<"|---------------------------------------------------------------------|"
       <<std::endl;
  out<<"| Number output tracks    | "<<std::setw(12)  
     <<"                              |"<<std::endl;
  out<<"|---------------------------------------------------------------------|"
     <<std::endl;
  return out;
}

void InDet::GNNTrackReaderTool::getTracks(uint32_t runNumber, uint32_t eventNumber,
  std::vector<std::vector<uint32_t> >& trackCandidates) const
{
  std::string fileName = m_inputTracksDir +  "/" + m_csvPrefix + "_" \
                         + std::to_string(runNumber) + "_" + std::to_string(eventNumber) + ".csv";

  trackCandidates.clear();
  std::ifstream csvFile(fileName);

  if (!csvFile.is_open()) {
    ATH_MSG_ERROR("Cannot open file " << fileName);
    return;
  } else {
    ATH_MSG_INFO("File " << fileName << " is opened.");
  }

  std::string line;
  while(std::getline(csvFile, line)){
      std::stringstream lineStream(line);
      std::string cell;
      std::vector<uint32_t> trackCandidate;
      // allow both "," and " " as delimiter
      char delimiter = ',';
      if (line.find(delimiter) == std::string::npos) {
        delimiter = ' ';
      }
      while (std::getline(lineStream, cell, delimiter)) {
          uint32_t cellId = 0;
          try {
              cellId = std::stoi(cell);
          } catch (const std::invalid_argument& ia) {
              std::cout << "Invalid argument: " << ia.what() << " for cell " << cell << std::endl;
              continue;
          }

          if (std::find(trackCandidate.begin(), trackCandidate.end(), cellId) == trackCandidate.end()) {
              trackCandidate.push_back(cellId);
          }
      }
      trackCandidates.push_back(std::move(trackCandidate));
  }
}

// this function reads each track candidate as 2 lists, a list of sp and 
// a list of clusters, the trackCandidates vector will be the clusters, 
// the seeds vector will be the SPs
void InDet::GNNTrackReaderTool::getTracks(
    uint32_t runNumber, uint32_t eventNumber,
    std::vector<std::vector<uint32_t>>& trackCandidates,
    std::vector<std::vector<uint32_t>>& seeds) const {
  std::string fileName = m_inputTracksDir + "/" + m_csvPrefix + "_" +
                         std::to_string(runNumber) + "_" +
                         std::to_string(eventNumber) + ".csv";

  trackCandidates.clear();
  std::ifstream csvFile(fileName);

  if (!csvFile.is_open()) {
    ATH_MSG_ERROR("Cannot open file " << fileName);
    return;
  } else {
    ATH_MSG_INFO("File " << fileName << " is opened.");
  }

  std::string line;
  while (std::getline(csvFile, line)) {
    std::istringstream lineStream(line);
    std::string CLString, SPString;
    char delimiter = ',';
    if (line.find(delimiter) == std::string::npos) {
      delimiter = ' ';
    }

    if (std::getline(lineStream, CLString, '|') &&
        std::getline(lineStream, SPString)) {
      std::istringstream CLStream(CLString), SPStream(SPString);
      std::vector<uint32_t> cls, sps;
      std::string number;

      while (std::getline(CLStream, number, delimiter)) {
        cls.push_back(std::stoi(number));
      }

      while (std::getline(SPStream, number, delimiter)) {
        sps.push_back(std::stoi(number));
      }

      trackCandidates.push_back(cls);
      seeds.push_back(sps);
    }
  }
  csvFile.close();

  ATH_MSG_DEBUG("Length of track list " << trackCandidates.size());
}