/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/MsgStream.h"

#include "MuonMeasurementsCnv.h"

#include <stdexcept>

MuonMeasurementsCnv::MuonMeasurementsCnv(ISvcLocator* svcloc):
    MuonMeasurementsCnvBase(svcloc)
{
}


void 
MuonMeasurementsCnv::readObjectFromPool( const Token* token )
{
  static const pool::Guid p1_guid( "C4979DA5-4193-410B-9476-A51708C01CF7" );
  static const pool::Guid p2_guid( "87FC613F-390A-4AB0-9BBF-28CE788867D5" );

   // select the object type based on its GUID 
   if( compareClassGuid(token,  p2_guid ) )     {
      // read MuonMeasurements_PERS object from POOL using given TLP converter
      poolReadObject< TPCnv::MuonMeasurements_tlp2 >( m_TPConverter_p2, token );
   }else  if( compareClassGuid(token,  p1_guid ) )    {
      poolReadObject< TPCnv::MuonMeasurements_tlp1 >( m_TPConverter_p1, token );   }
   else
      throw std::runtime_error( "Unsupported version of MuonMeasurements_PERS (unknown GUID)" );
}
