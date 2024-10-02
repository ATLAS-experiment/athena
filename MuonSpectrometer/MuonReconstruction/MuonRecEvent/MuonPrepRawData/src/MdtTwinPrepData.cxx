/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/



#include "MuonPrepRawData/MdtTwinPrepData.h"
#include "GaudiKernel/MsgStream.h"

namespace Muon
{
    

  // Constructor with parameters:
  MdtTwinPrepData::MdtTwinPrepData(const Identifier &id,
                                   const Amg::Vector2D& driftRadiusXTwin,
                                   const Amg::MatrixX& errDriftRadiusXTwin,
                                   const MuonGM::MdtReadoutElement* detEl,
                                   const int tdc, const int adc,
                                   const int tdcTwin, const int adcTwin,
                                   const MdtDriftCircleStatus status ):
    MdtPrepData(id, driftRadiusXTwin, errDriftRadiusXTwin, detEl, tdc, adc, status), //call base class constructor
    m_tdcTwin(tdcTwin), 
    m_adcTwin(adcTwin) {}


  MsgStream& MdtTwinPrepData::dump( MsgStream&    stream) const
  {
    stream << MSG::INFO<<"MdtTwinPrepData {"<<std::endl;

    MdtPrepData::dump(stream);

    //MdtTwinPrepData methods
    stream <<"TDC TWIN = "<<tdcTwin()<<", ";
    stream <<"ADC TWIN= "<<adcTwin()<<", ";
    stream<<"} End MdtTwinPrepData"<<endmsg;

    return stream;
  }

  std::ostream& MdtTwinPrepData::dump( std::ostream&    stream) const
  {
    stream << "MdtTwinPrepData {"<<std::endl;

    MdtPrepData::dump(stream);

    //MdtTwinPrepData methods
    stream <<"TDC TWIN = "<<tdcTwin()<<", ";
    stream <<"ADC TWIN = "<<adcTwin()<<", ";
    stream<<"} End MdtTwinPrepData"<<std::endl;
    return stream;
  }
  
  

}//end of ns


