/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPrepRawData/sTgcPrepData.h"
#include "GaudiKernel/MsgStream.h"

namespace Muon {

sTgcPrepData::sTgcPrepData(const Identifier& RDOId,
                           const IdentifierHash& idDE,
                           Amg::Vector2D&& locpos,
                           std::vector<Identifier>&& rdoList,
                           Amg::MatrixX&& locErrMat,
                           const MuonGM::sTgcReadoutElement* detEl,
                           const int charge,
                           const short int time,
                           std::vector<uint16_t>&& stripNumbers,
                           std::vector<short int>&& stripTimes,
                           std::vector<int>&& stripCharges)
  : MuonCluster(RDOId, idDE, locpos, std::move(rdoList),std::move(locErrMat))
  , m_detEl(detEl)
  , m_charge(charge)
  , m_time(time)
  , m_stripNumbers(std::move(stripNumbers))
  , m_stripTimes(std::move(stripTimes))
  , m_stripCharges(std::move(stripCharges)) {}

sTgcPrepData::sTgcPrepData(const Identifier& RDOId,
                           const IdentifierHash& idDE,
                           Amg::Vector2D&& locpos,
                           std::vector<Identifier>&& rdoList,
                           Amg::MatrixX&& locErrMat,
                           const MuonGM::sTgcReadoutElement* detEl,
                           const int charge,
                           const short int time)
  : MuonCluster(RDOId, idDE, locpos, std::move(rdoList), std::move(locErrMat))
  , m_detEl(detEl)
  , m_charge(charge)
  , m_time(time){}

  MsgStream& sTgcPrepData::dump( MsgStream& stream) const {
      stream << MSG::INFO<<"sTgcPrepData {"<<std::endl;    
      MuonCluster::dump(stream); 
      stream<<"}"<<endmsg;
      return stream;
  }

  std::ostream& sTgcPrepData::dump( std::ostream& stream) const {
    stream << "sTgcPrepData {"<<std::endl;    
    MuonCluster::dump(stream);
    stream<<"}"<<std::endl;
    return stream;
  }
  //end of classdef
}//end of ns

