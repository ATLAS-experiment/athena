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
      while(std::getline(lineStream, cell, ','))
      {
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