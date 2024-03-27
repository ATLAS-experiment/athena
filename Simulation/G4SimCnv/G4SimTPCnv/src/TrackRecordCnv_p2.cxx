/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "G4SimTPCnv/TrackRecordCnv_p2.h"
#include "G4SimTPCnv/TrackRecord_p2.h"
#include "TrackRecord/TrackRecord.h"

void
TrackRecordCnv_p2::persToTrans(const TrackRecord_p2* persObj, TrackRecord* transObj, MsgStream &log) const
{
  log << MSG::DEBUG << "TrackRecordCnv_p2::persToTrans called " << endmsg;

  transObj->SetPDGCode (persObj->PDG_code());
  transObj->SetStatus (persObj->status());
  transObj->SetEnergy ((double) persObj->energy());
  transObj->SetMomentum (CLHEP::Hep3Vector( persObj->momentumX(), persObj->momentumY(), persObj->momentumZ() ));
  transObj->SetPosition (CLHEP::Hep3Vector( persObj->positionX(), persObj->positionY(), persObj->positionZ() ));
  transObj->SetTime ((double) persObj->time());
  transObj->SetID (persObj->uniqueID());
  transObj->SetVolName (persObj->volName());
}


void
TrackRecordCnv_p2::transToPers(const TrackRecord* transObj, TrackRecord_p2* persObj, MsgStream &log) const
{
  log << MSG::DEBUG << "TrackRecordCnv_p2::transToPers called " << endmsg;
  persObj->m_PDG_code = transObj->GetPDGCode();
  persObj->m_status = transObj->status();
  persObj->m_energy = (float) transObj->GetEnergy();
  CLHEP::Hep3Vector mom = transObj->GetMomentum();
  persObj->m_momentumX = (float) mom.x();
  persObj->m_momentumY = (float) mom.y();
  persObj->m_momentumZ = (float) mom.z();
  CLHEP::Hep3Vector pos = transObj->GetPosition();
  persObj->m_positionX = (float) pos.x();
  persObj->m_positionY = (float) pos.y();
  persObj->m_positionZ = (float) pos.z();
  persObj->m_time = (float) transObj->GetTime();
  persObj->m_uniqueID = transObj->id();
  persObj->m_volName = transObj->GetVolName();
}
