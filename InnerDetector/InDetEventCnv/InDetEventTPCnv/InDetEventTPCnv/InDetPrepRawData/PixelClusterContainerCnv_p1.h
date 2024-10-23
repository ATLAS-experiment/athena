/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELCLUSTERCONTAINERCNV_P1_H
#define PIXELCLUSTERCONTAINERCNV_P1_H

//-----------------------------------------------------------------------------
//
// file:   PixelClusterContainerCnv_p1.h
//
//-----------------------------------------------------------------------------
 
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
 
#include "InDetPrepRawData/PixelClusterContainer.h"
#include "InDetEventTPCnv/InDetPrepRawData/InDetPRD_Container_p1.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/ReadCondHandleKey.h"

class PixelID;

class StoreGateSvc;
class MsgStream;
namespace InDet{
class PixelClusterContainerCnv_p1 : public T_AthenaPoolTPCnvBase< InDet::PixelClusterContainer, InDet::InDetPRD_Container_p1 >
 
{
 public:
   typedef InDet::InDetPRD_Container_p1 PERS; 
   typedef InDet::PixelClusterContainer TRANS;
   PixelClusterContainerCnv_p1() = default;
   virtual void persToTrans(const PERS* persCont, TRANS* transCont, MsgStream &log); 
   virtual void transToPers(const TRANS* transCont, PERS* persCont, MsgStream &log);
   virtual InDet::PixelClusterContainer* createTransient(const InDet::InDetPRD_Container_p1* persObj, MsgStream& log);
   void setUseDetectorElement(const bool useDetectorElement);
 private:
   const PixelID *m_pixId{};
   ServiceHandle<StoreGateSvc> m_storeGate{"StoreGateSvc", "PixelClusterContainerCnv_p1"};
   SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey{"PixelDetectorElementCollection"};
   bool m_useDetectorElement{true};
   bool m_isInitialized{false};
   StatusCode initialize(MsgStream &log);
};
 
}
#endif
