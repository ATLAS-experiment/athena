/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRT_RAWDATABYTESTREAMCNV_TRTRAWDATAPROVIDERTOOL_H
#define TRT_RAWDATABYTESTREAMCNV_TRTRAWDATAPROVIDERTOOL_H

#include "TRT_RawDataByteStreamCnv/ITRTRawDataProviderTool.h"

#include "AthenaKernel/SlotSpecificObj.h"

#include "ByteStreamData/RawEvent.h"
#include "InDetRawData/InDetTimeCollection.h"
#include "TRT_RawDataByteStreamCnv/ITRT_RodDecoder.h"
#include "StoreGate/WriteHandleKey.h"

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include <set>
#include <string>

// the tool to decode a ROB frament

class TRT_BSErrorContainer;

class TRTRawDataProviderTool : public extends<AthAlgTool, ITRTRawDataProviderTool>
{
 public:
  //! constructor
  TRTRawDataProviderTool( const std::string& type, const std::string& name,
			  const IInterface* parent ) ;

  //! initialize
  virtual StatusCode initialize() override;

  //! this is the main decoding method
  virtual StatusCode convert(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs,
			     TRT_RDO_Container* rdoIdc,
			     TRT_BSErrContainer* bsErrCont,
           DataPool<TRT_LoLumRawData>* dataItemsPool,
           const EventContext& ctx
			     ) const override;

private:
  ToolHandle<ITRT_RodDecoder>  m_decoder;

  SG::WriteHandleKey<InDetTimeCollection> m_lvl1idkey{this,"LVL1IDKey","TRT_LVL1ID","TRT_LVL1ID out-key"};
  SG::WriteHandleKey<InDetTimeCollection> m_bcidkey{this,"BCIDKey","TRT_BCID","TRT_BCID out-key"};
  Gaudi::Property<bool> m_storeInDetTimeColls{this, "StoreInDetTimeCollections", true, "Store LVL1ID and BCID"};
};

#endif



