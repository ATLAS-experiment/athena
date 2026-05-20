/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonPrepRawData/MuonCluster.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuonReadoutGeometry/MuonReadoutElement.h"
#include "GaudiKernel/MsgStream.h"

namespace Muon
{

  // Constructor 
  MuonCluster::MuonCluster( const Identifier& RDOId,
                const IdentifierHash & /**collectionHash*/, //FIXME! Should be removed.
                const Amg::Vector2D& locpos,
                const std::vector<Identifier>& rdoList,
                const Amg::MatrixX& locErrMat
                ) :
      PrepRawData(RDOId, locpos, rdoList, locErrMat), //call base class constructor
      m_globalPosition()
  { 
  }

MuonCluster::MuonCluster( const Identifier& RDOId,
                const IdentifierHash & /**collectionHash*/, //FIXME! Should be removed.
                const Amg::Vector2D& locpos,
                std::vector<Identifier>&& rdoList,
                Amg::MatrixX&& locErrMat
                ) :
      PrepRawData(RDOId, locpos, std::move(rdoList), std::move(locErrMat)), //call base class constructor
      m_globalPosition()
  { 
  }


  // Destructor:
  MuonCluster::~MuonCluster()
  = default;

  // Default constructor:
  MuonCluster::MuonCluster():
      PrepRawData(), 
      m_globalPosition()
  { }

  //copy constructor:
  MuonCluster::MuonCluster(const MuonCluster& RIO):
      PrepRawData(RIO),
      m_globalPosition()
  { 
    // copy only if it exists

    if (RIO.m_globalPosition) m_globalPosition.store(std::make_unique<const Amg::Vector3D>(*RIO.m_globalPosition));
  }


  //assignment operator
  MuonCluster& MuonCluster::operator=(const MuonCluster& RIO)
  {
    if (&RIO !=this)
    {
      Trk::PrepRawData::operator=(RIO);
      if (RIO.m_globalPosition) m_globalPosition.store(std::make_unique<const Amg::Vector3D>(*RIO.m_globalPosition));
      else if (m_globalPosition) m_globalPosition.release().reset();
    }
    return *this;
  }
  
  MsgStream& MuonCluster::dump( MsgStream&    stream) const
  {
    stream << MSG::INFO<<"MuonCluster {"<<std::endl;
    Trk::PrepRawData::dump(stream);
    stream << "Global Coordinates (x,y,z) = (";
    stream<< Amg::toString(globalPosition())<<std::endl;
    if (const auto* re = dynamic_cast<const MuonGM::MuonReadoutElement*>(detectorElement())) {
      stream<<"Id - "<<re->idHelperSvc()->toString(identify())<<std::endl;
    } 
    stream<<"} End MuonCluster"<<endmsg;
    return stream;
  }

  std::ostream& MuonCluster::dump( std::ostream&    stream) const
  {
    stream << "MuonCluster {"<<std::endl;
    Trk::PrepRawData::dump(stream); 
    stream << "Global Coordinates (x,y,z) = (";
    stream<< Amg::toString(globalPosition())<<std::endl;
    if (const auto* re = dynamic_cast<const MuonGM::MuonReadoutElement*>(detectorElement())) {
      stream<<"Id - "<<re->idHelperSvc()->toString(identify())<<std::endl;
    }  
    stream<<"} End MuonCluster"<<std::endl;
    return stream;
  }
  //end of classdef

}//end of ns
