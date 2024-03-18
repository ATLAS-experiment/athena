/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetSimData/InDetSimData.h"
#include "InDetSimData/InDetSimDataCollection.h"
#include "InDetEventAthenaPool/InDetSimDataCollection_p4.h"
#include "InDetSimDataCollectionCnv_p4.h"
#include "InDetSimDataCnv_p3.h"
#include "Identifier/Identifier.h"
#include "SGTools/CurrentEventStore.h"
#include "MsgUtil.h"

void InDetSimDataCollectionCnv_p4::transToPers(const InDetSimDataCollection* transCont, InDetSimDataCollection_p4* persCont, MsgStream &log)
{
  InDetSimDataCnv_p3  simDataCnv;
  persCont->m_simdata.resize(transCont->size());
  MSG_DEBUG(log," Preparing " << persCont->m_simdata.size() << "Collections");
  unsigned int collIndex(0);
  for (const auto& transSimDataPair : *transCont) {
    // Add in new collection
    (persCont->m_simdata[collIndex]).first = transSimDataPair.first.get_compact();
    const InDetSimData& simData = transSimDataPair.second;
    InDetSimData_p3& psimData = persCont->m_simdata[collIndex].second;
    simDataCnv.transToPers(&simData,&psimData,log);
    ++collIndex;
  }
  MSG_DEBUG(log," ***  Writing InDetSimdataCollection");
}

void  InDetSimDataCollectionCnv_p4::persToTrans(const InDetSimDataCollection_p4* persCont, InDetSimDataCollection* transCont, MsgStream &log)
{
  InDetSimDataCnv_p3  simDataCnv;
  MSG_DEBUG(log," Preparing " << persCont->m_simdata.size() << "Collections");
  simDataCnv.setCurrentStore (SG::CurrentEventStore::store());
  for (const auto& persSimDataPair : persCont->m_simdata) {
    // Add in new collection
    const InDetSimData_p3& psimData = persSimDataPair.second;
    InDetSimData simData;
    simDataCnv.persToTrans(&psimData,&simData,log);
    transCont->insert( transCont->end(), std :: make_pair( Identifier( persSimDataPair.first ), simData ) );
  }
  MSG_DEBUG(log," ***  Reading InDetSimdataCollection");
}


