/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MdtPrepData.cxx
//   Implementation file for class MdtPrepData
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Version 1.0 03/30/2005 Ketevi A. Assamagan
///////////////////////////////////////////////////////////////////

#include "MuonPrepRawData/MdtPrepData.h"
#include "GaudiKernel/MsgStream.h"

namespace Muon
{
    
  // Constructor with parameters:
MdtPrepData::MdtPrepData(const Identifier &id,                        
                         const Amg::Vector2D& driftRadius,
                         const Amg::MatrixX& errDriftRadius,
                         const MuonGM::MdtReadoutElement* detEl,
                         const int tdc,
                         const int adc,
                         const MdtDriftCircleStatus status) :
    PrepRawData(id, driftRadius, errDriftRadius), //call base class constructor
    m_detEl(detEl),
    m_tdc(tdc), 
    m_adc(adc), 
    m_status(status)
   
{
//    assert(rdoList.size()==1); 
//    assert( rdoList[0]==id);
}

MdtPrepData::~MdtPrepData() = default;
MdtPrepData::MdtPrepData() = default;


//Copy constructor:
MdtPrepData::MdtPrepData(const MdtPrepData & RIO) :
    PrepRawData( RIO ),
    m_detEl(RIO.m_detEl),
    m_tdc(RIO.m_tdc),
    m_adc(RIO.m_adc),
    m_status(RIO.m_status) {}

//Move constructor:
MdtPrepData::MdtPrepData(MdtPrepData && RIO) noexcept :
    PrepRawData( std::move(RIO) ),
    m_detEl(RIO.m_detEl),
    m_tdc(RIO.m_tdc),
    m_adc(RIO.m_adc),
    m_status(RIO.m_status) {}

//assignment operator
MdtPrepData& MdtPrepData::operator=(const MdtPrepData& RIO) {
    if (&RIO !=this) {
      Trk::PrepRawData::operator=(RIO);
        m_detEl           = RIO.m_detEl;
        m_tdc             = RIO.m_tdc;
        m_adc             = RIO.m_adc;
        m_status          = RIO.m_status;
        if (m_globalPosition) m_globalPosition.release();
    }
    return *this;
}

//assignment operator
MdtPrepData& MdtPrepData::operator=(MdtPrepData&& RIO) noexcept {
    if (&RIO !=this) {
      m_detEl = RIO.m_detEl;
      m_tdc = RIO.m_tdc;
      m_adc = RIO.m_adc;
      m_status = RIO.m_status;
      Trk::PrepRawData::operator=(std::move(RIO));
      if (m_globalPosition) {
          m_globalPosition.release();
      }        
    }
    return *this;
}

MsgStream& MdtPrepData::dump( MsgStream&    stream) const
  {
    stream << MSG::INFO<<"MdtPrepData {"<<std::endl;

    Trk::PrepRawData::dump(stream);

    //MdtPrepData methods
    stream <<"TDC = "<<tdc()<<", ";
    stream <<"ADC = "<<adc()<<", ";
    stream <<"status = "<<status()<<", ";
    stream<<"} End MdtPrepData"<<endmsg;

    return stream;
  }

  std::ostream& MdtPrepData::dump( std::ostream&    stream) const
  {
    stream << "MdtPrepData {"<<std::endl;

    Trk::PrepRawData::dump(stream);

    //MdtPrepData methods
    stream <<"TDC = "<<tdc()<<", ";
    stream <<"ADC = "<<adc()<<", ";
    stream <<"status = "<<status()<<", ";
    stream<<"} End MdtPrepData"<<std::endl;
    return stream;
  }
  
  

}//end of ns


