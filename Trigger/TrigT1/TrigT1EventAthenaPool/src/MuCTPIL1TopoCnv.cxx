/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


// Local include(s):
#include "MuCTPIL1TopoCnv.h"

/**
 * Function creating a persistent MuCTPIL1Topo_PERS object from a transient
 * MuCTPIL1Topo one.
 */
MuCTPIL1Topo_PERS* MuCTPIL1TopoCnv::createPersistent( LVL1::MuCTPIL1Topo* transObj ) {

  return m_converter.createPersistent( transObj, msg() );

}

/**
 * Function reading a version of MuCTPIL1Topo from POOL and converting it to
 * a transient MuCTPIL1Topo object.
 */
LVL1::MuCTPIL1Topo* MuCTPIL1TopoCnv::createTransient(const Token* token) {

   static const pool::Guid p1_guid( "BC2BAC47-504A-4A8A-89D9-2086B9038E18" );

   if( this->compareClassGuid(token,  p1_guid ) ) {
     std::unique_ptr< MuCTPIL1Topo_p1 > pers_ref( this->poolReadObject< MuCTPIL1Topo_p1 >() );
     return m_converter.createTransient( pers_ref.get(), msg() );

   } else {
     throw std::runtime_error( "Unsupported persistent version of MuCTPIL1Topo" );
     return 0;
   }
}
