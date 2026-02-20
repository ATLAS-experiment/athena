/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TgcCoinData.cxx
//   Implementation file for class TgcCoinData
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include <new>
#include "MuonTrigCoinData/RpcCoinData.h"
#include "GaudiKernel/MsgStream.h" 

namespace Muon
{
  // Constructor 
  RpcCoinData::RpcCoinData(  const Identifier& stripId,
			     const IdentifierHash &idDE,
			     const Amg::Vector2D& locpos,
			     const std::vector<Identifier>& stripList,
			     const Amg::MatrixX& locErrMat,
			     const MuonGM::RpcReadoutElement* detEl,
			     const float time,
			     const unsigned short ambiguityFlag,
			     const unsigned short ijk,
			     const unsigned short threshold,
			     const unsigned short overlap,
			     const unsigned short parent_cmId,
			     const unsigned short parent_padId,
			     const unsigned short parent_sectorId,
                             bool lowPtCm):
    RpcPrepData(stripId, idDE, locpos, stripList, locErrMat, detEl, time, ambiguityFlag), 
    m_ijk(ijk),
    m_threshold(threshold),
    m_overlap(overlap),
    m_parentCmId(parent_cmId),
    m_parentPadId(parent_padId), 
    m_parentSectorId(parent_sectorId),
    m_lowPtCm(lowPtCm)
{ }

// Destructor:
RpcCoinData::~RpcCoinData()
= default;


// << operator

MsgStream& RpcCoinData::dump( MsgStream&    stream) const
{
  stream << MSG::INFO<<"RpcCoinData {"<<std::endl;
    
  RpcPrepData::dump(stream);
  
  stream<<"ijk                  = "<<ijk()<<", ";
  stream<<"threshold            = "<<threshold()<<", ";
  stream<<"overlap              = "<<overlap()<<", ";
  stream<<"parentCmId           = "<<parentCmId()<<", ";
  stream<<"parentPadId          = "<<parentPadId()<<", ";
  stream<<"parentSectorId       = "<<parentSectorId()<<", ";
  stream<<"lowPtCm              = "<<isLowPtCoin()<<", ";
  stream<<"lowPtInputToHighPtCm = "<<isLowPtInputToHighPtCm()<<", ";
  stream<<"}"<<endmsg;
  
  return stream;
}

std::ostream& RpcCoinData::dump( std::ostream&    stream) const
{
  stream <<"RpcCoinData {"<<std::endl;
    
  RpcPrepData::dump(stream);
  
  stream<<"ijk                  = "<<ijk()<<", ";
  stream<<"threshold            = "<<threshold()<<", ";
  stream<<"overlap              = "<<overlap()<<", ";
  stream<<"parentCmId           = "<<parentCmId()<<", ";
  stream<<"parentPadId          = "<<parentPadId()<<", ";
  stream<<"parentSectorId       = "<<parentSectorId()<<", ";
  stream<<"lowPtCm              = "<<isLowPtCoin()<<", ";
  stream<<"lowPtInputToHighPtCm = "<<isLowPtInputToHighPtCm()<<", ";
  stream<<"}"<<std::endl;

  return stream;
}
bool RpcCoinData::isLowPtCoin() const
{
    return m_lowPtCm && m_ijk == 6;
}
bool RpcCoinData::isHighPtCoin() const
{
    return (!m_lowPtCm) && m_ijk == 6;
}
bool RpcCoinData::isLowPtInputToHighPtCm() const
{
    return m_ijk == 0;
}


}//end of namespace

