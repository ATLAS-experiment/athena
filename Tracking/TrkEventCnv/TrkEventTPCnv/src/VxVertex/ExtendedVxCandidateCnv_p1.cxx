/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------------
//
// file:   ExtendedVxCandidate_p1.cxx
//
//-----------------------------------------------------------------------------

#include "VxVertex/ExtendedVxCandidate.h"
#include "TrkEventTPCnv/VxVertex/ExtendedVxCandidateCnv_p1.h"
#include "TrkEventTPCnv/helpers/EigenHelpers.h"


void  ExtendedVxCandidateCnv_p1::persToTrans(const Trk::ExtendedVxCandidate_p1 * persObj, Trk::ExtendedVxCandidate * transObj, MsgStream &log)
{
 fillTransFromPStore( &m_vxCandidateConverter, persObj->m_vxCandidate,transObj, log);
 // transObj->m_fullCovariance = createTransFromPStore( &m_errorMatrixConverter, persObj->m_fullCovariance, log );
 
 Trk::ErrorMatrix dummy;
 fillTransFromPStore( &m_errorMatrixConverter, persObj->m_fullCovariance, &dummy, log );
 Amg::MatrixX* temp = new Amg::MatrixX;
 EigenHelpers::vectorToEigenMatrix(dummy.values, *temp, "ExtendedVxCandidateCnv_p1");
 transObj->m_fullCovariance = temp;
}

void  ExtendedVxCandidateCnv_p1::transToPers(const Trk::ExtendedVxCandidate * transObj, Trk::ExtendedVxCandidate_p1 * persObj, MsgStream &log)
{
 persObj->m_vxCandidate  = baseToPersistent(&m_vxCandidateConverter,  transObj, log );
 Trk::ErrorMatrix pMat;
 if (auto p = transObj->m_fullCovariance; p!= nullptr)[[likely]]{
   EigenHelpers::eigenMatrixToVector(pMat.values, *p, "ExtendedVxCandidateCnv_p1");
   persObj->m_fullCovariance = toPersistent( &m_errorMatrixConverter, &pMat, log );
  }
 
}
