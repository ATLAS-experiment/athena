/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPrepRawData/MMPrepData.h"
#include "GaudiKernel/MsgStream.h"

namespace Muon
{

MMPrepData::MMPrepData(const Identifier& RDOId,
                       const IdentifierHash& idDE,
                       Amg::Vector2D&& locpos,
                       std::vector<Identifier>&& rdoList,
                       Amg::MatrixX&& locErrMat,
                       const MuonGM::MMReadoutElement* detEl,
                       const short int time,
                       const int charge,
                       const float driftDist,
                       std::vector<uint16_t>&& stripNumbers,
                       std::vector<short int>&& stripTimes,
                       std::vector<int>&& stripCharges)
  : MuonCluster(RDOId, idDE,locpos, std::move(rdoList), std::move(locErrMat))
  , m_detEl(detEl)
  , m_time(time)
  , m_charge(charge)
  , m_driftDist(driftDist)
  , m_stripNumbers(std::move(stripNumbers))
  , m_stripTimes(std::move(stripTimes))
  , m_stripCharges(std::move(stripCharges)) {}

MMPrepData::MMPrepData(const Identifier& RDOId,
                       const IdentifierHash& idDE,
                       Amg::Vector2D&& locpos,
                       std::vector<Identifier>&& rdoList,
                       Amg::MatrixX&& locErrMat,
                       const MuonGM::MMReadoutElement* detEl,
                       const short int time,
                       const int charge)
  : MuonCluster(RDOId, idDE, locpos, std::move(rdoList), std::move(locErrMat))
  , m_detEl(detEl)
  , m_time(time)
  , m_charge(charge){}

MMPrepData::MMPrepData(const Identifier& RDOId,
                       const IdentifierHash& idDE,
                       Amg::Vector2D&& locpos,
                       std::vector<Identifier>&& rdoList,
                       Amg::MatrixX&& locErrMat,
                       const MuonGM::MMReadoutElement* detEl,
                       const short int time,
                       const int charge,
                       const float driftDist)
  : MuonCluster(RDOId, idDE, locpos, std::move(rdoList), std::move(locErrMat))
  , m_detEl(detEl)
  , m_time(time)
  , m_charge(charge)
  , m_driftDist(driftDist) {}

MMPrepData::MMPrepData(const Identifier& RDOId,
                       const IdentifierHash& idDE,
                       Amg::Vector2D&& locpos,
                       std::vector<Identifier>&& rdoList,
                       Amg::MatrixX&& locErrMat,
                       const MuonGM::MMReadoutElement* detEl)
  : MuonCluster(RDOId, idDE, locpos, std::move(rdoList), std::move(locErrMat))
  , m_detEl(detEl) {}


/// set the micro-tpc quantities
void
MMPrepData::setMicroTPC(float angle, float chisqProb) {
    m_angle = angle;
    m_chisqProb = chisqProb;
}

  /// set drift distances and errors
void MMPrepData::setDriftDist(std::vector<float>&& driftDist, 
                              std::vector<AmgVector(2)>&& driftDistErrors) {
    m_stripDriftDist = std::move(driftDist);
    m_stripDriftErrors = std::move(driftDistErrors);
}
void MMPrepData::setDriftDist(std::vector<float>&& driftDist, 
                              std::vector<float>&& stripDriftErrors_0_0, 
                              std::vector<float>&& stripDriftErrors_1_1) {
    m_stripDriftDist = std::move(driftDist);
    m_stripDriftErrors.reserve(stripDriftErrors_1_1.size());
    for(uint i_strip = 0; i_strip < stripDriftErrors_1_1.size(); i_strip++){
      AmgVector(2) tmp{AmgVector(2)::Zero()};
      tmp[0] = stripDriftErrors_0_0.at(i_strip);
      tmp[1] = stripDriftErrors_1_1.at(i_strip);
      m_stripDriftErrors.push_back(std::move(tmp));
    }
  }

  void MMPrepData::setAuthor(MMPrepData::Author author){
    m_author = author;
  }

  

MsgStream& MMPrepData::dump( MsgStream& stream) const {
    stream << MSG::INFO<<"MMPrepData {"<<std::endl;    
    MuonCluster::dump(stream); 
    stream<<"}"<<endmsg;
    return stream;
}

std::ostream& MMPrepData::dump( std::ostream& stream) const {
    stream << "MMPrepData {"<<std::endl;    
    MuonCluster::dump(stream);
    stream<<"}"<<std::endl;
    return stream;
}

std::vector<float> MMPrepData::stripDriftErrors_0_0 () const {
    std::vector<float> ret;
    ret.reserve(m_stripDriftErrors.size());
    for (const AmgVector(2)& mat: m_stripDriftErrors) {
      ret.push_back(mat[0]);
    }
    return ret;
}
  
std::vector<float> MMPrepData::stripDriftErrors_1_1 () const {
  std::vector<float> ret;
  ret.reserve(m_stripDriftErrors.size());
  for (const AmgVector(2)& mat: m_stripDriftErrors) {
      ret.push_back(mat[1]);
  }
  return ret;
}


  //end of classdef
}//end of ns

