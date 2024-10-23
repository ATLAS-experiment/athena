/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SCT_CLUSTERCONTAINERCNV_P1_H
#define SCT_CLUSTERCONTAINERCNV_P1_H

//-----------------------------------------------------------------------------
//
// file:   SCT_ClusterContainerCnv_p1.h
//
//-----------------------------------------------------------------------------
 
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
 
#include "InDetPrepRawData/SCT_ClusterContainer.h"
#include "InDetEventTPCnv/InDetPrepRawData/InDetPRD_Container_p1.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/ReadCondHandleKey.h"

class MsgStream;
class SCT_ID;
class StoreGateSvc;

namespace InDet{
class SCT_ClusterContainerCnv_p1 : public T_AthenaPoolTPCnvBase< InDet::SCT_ClusterContainer, InDet::InDetPRD_Container_p1 >
 
{
 public:
   typedef InDet::InDetPRD_Container_p1 PERS;
   typedef InDet::SCT_ClusterContainer TRANS;
   SCT_ClusterContainerCnv_p1() = default;
   virtual void transToPers(const TRANS* transCont, PERS* persCont, MsgStream &log);
   virtual void persToTrans(const PERS* persCont, TRANS* transCont, MsgStream &log);
   virtual InDet::SCT_ClusterContainer* createTransient(const InDet::InDetPRD_Container_p1* persObj, MsgStream& log);
   // Method for test/SCT_ClusterContainerCnv_p1_test.cxx
   void setUseDetectorElement(const bool useDetectorElement);
 private:
   const SCT_ID *m_sctId{nullptr};
   ServiceHandle<StoreGateSvc> m_storeGate{"StoreGateSvc", "SCT_ClusterContainerCnv_p1"};
   SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_SCTDetEleCollKey{"SCT_DetectorElementCollection"};
   // Declaration of ReadCondHandleKey in SCT_ClusterContainerCnv_p1 triggers memory leak in SCT_ClusterCnv_p1_test.
   bool m_useDetectorElement{true};
   bool m_isInitialized{false};
   StatusCode initialize(MsgStream &log);
};
 
}
#endif 
 
