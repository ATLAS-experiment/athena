/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Include files
#include "LArCalibUtils/LArHVCorrMaker.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "CoralBase/Blob.h"
#include "LArCOOLConditions/LArHVScaleCorrFlat.h"
#include "LArRecConditions/LArHVCorr.h"
#include "StoreGate/ReadCondHandle.h"

#include <cmath>
#include <unistd.h>




//---------------------------------------------------------------------------
LArHVCorrMaker::~LArHVCorrMaker()
= default;

//---------------------------------------------------------------------------
StatusCode LArHVCorrMaker::initialize()
{
  ATH_MSG_INFO ( "  in initialize " );
  ATH_CHECK( m_scaleCorrKey.initialize() );
  ATH_CHECK( m_cablingKey.initialize() );
  return StatusCode::SUCCESS;
}


//---------------------------------------------------------------------------
StatusCode LArHVCorrMaker::execute()
{
  return StatusCode::SUCCESS;
}


//---------------------------------------------------------------------------
StatusCode LArHVCorrMaker::stop()
{
  ATH_MSG_INFO ( " in stop" );

  const EventContext& ctx = Gaudi::Hive::currentContext();

  SG::ReadCondHandle<LArOnOffIdMapping> cabling (m_cablingKey, ctx);

  SG::ReadCondHandle<ILArHVScaleCorr> scaleCorr (m_scaleCorrKey, ctx);

  const LArOnlineID_Base*  lar_on_id = nullptr;
  if(m_isSC) {
     const LArOnline_SuperCellID* scid=nullptr;
     ATH_CHECK( detStore()->retrieve(scid,"LArOnline_SuperCellID") );
     lar_on_id=scid;
  } else {
     const LArOnlineID* lid=nullptr;
     ATH_CHECK( detStore()->retrieve(lid,"LArOnlineID") );
     lar_on_id=lid;
  }

  const unsigned hashMax=lar_on_id->channelHashMax();
  coral::AttributeListSpecification* spec = new coral::AttributeListSpecification;
  spec->extend("HVScaleCorr", "blob");
  spec->extend<unsigned>("version");
  auto coll = std::make_unique<CondAttrListCollection>(true);
  coral::AttributeList attrList(*spec);
  spec->release();
  attrList["version"].setValue(0U);
  coral::Blob& blob=attrList["HVScaleCorr"].data<coral::Blob>();
  blob.resize(hashMax*sizeof(float));
  float* pblob=static_cast<float*>(blob.startingAddress());
  //Loop over online hash (to make sure that *every* field of the blob gets filled
  for (unsigned hs=0;hs<hashMax;++hs) {
    float value=1.0;
    if (cabling->isOnlineConnectedFromHash(hs)) {
      const Identifier id=cabling->cnvToIdentifierFromHash(hs);
      const HWIdentifier hwid = cabling->createSignalChannelID (id);
      ATH_MSG_VERBOSE("Filling value for id " << id.get_identifier32().get_compact());
      value=scaleCorr->HVScaleCorr(hwid);
    }
    pblob[hs]=value;
  }

  if(m_isSC) {
     coll->add(0,attrList); //Add as channel 0 to AttrListCollection
  } else {
     coll->add(1,attrList); //Add as channel 1 to AttrListCollection
  }

  ATH_CHECK( detStore()->record(std::move(coll), m_folderName) );  
  return StatusCode::SUCCESS;
}
