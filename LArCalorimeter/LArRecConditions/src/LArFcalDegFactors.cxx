/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRecConditions/LArFcalDegFactors.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralBase/Blob.h"
#include "GaudiKernel/MsgStream.h"

LArFcalDegFactors::LArFcalDegFactors(const CondAttrListCollection* attrList,  MsgStream &msg ) {

   if (!attrList) return;

   CondAttrListCollection::const_iterator gainIt=attrList->begin();
   
   if (attrList->size() != 2 ) { // channel 0 - payload, channel 1001 - metadata
      msg << MSG::ERROR<<"Wrong size of CondAttrListCollection " << attrList->size() << " !!!"<<endmsg;
      return;
   }

   const unsigned gain=gainIt->first;
   if (gain != 0) {
     msg << MSG::ERROR << "Found unexpected COOL-channel (=gain) number:" << gain << endmsg;
     return;
   }

   const coral::AttributeList& attr=gainIt->second;
   const coral::Blob& Blob = attr["FCALDegradation"].data<coral::Blob>();
   const auto blobSize=Blob.size();
   
   //Sanity checks:
   if (blobSize != nBins * nValues * sizeof(float) ) {
     msg << MSG::ERROR << "Wrong blob size (" << blobSize << ")"  << endmsg;
     return;
   }
   auto pBlob=static_cast<const float*>(Blob.startingAddress());
   for(unsigned short bin=0; bin < nBins; ++bin) {
      m_fcalDegradationMap[bin].assign(pBlob + nValues*bin, pBlob + nValues*(bin+1));
   }

   return;

}
