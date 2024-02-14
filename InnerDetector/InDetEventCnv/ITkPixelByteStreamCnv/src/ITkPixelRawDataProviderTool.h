/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITkPixelRawDataProviderTool_h
#define ITkPixelRawDataProviderTool_h

#include "ITkPixelByteStreamCnv/IITkPixelRawDataProviderTool.h"
#include "ByteStreamData/RawEvent.h" //ROBFragment typedef
#include "InDetRawData/PixelRDO_Container.h"// typedef
#include "ITkPixelByteStreamCnv/IITkPixelRodDecoder.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include <string>
#include <vector>

class IInterface;
class EventContext;
class StatusCode;

// the tool to decode a ROB frament
class ITkPixelRawDataProviderTool : 
  virtual public IITkPixelRawDataProviderTool, public AthAlgTool{

 public:
  ITkPixelRawDataProviderTool( const std::string& type, const std::string& name,
			    const IInterface* parent ) ;

  ~ITkPixelRawDataProviderTool() final = default;

  StatusCode initialize() final;

  StatusCode finalize() final;
  
  //! this is the main decoding method
  StatusCode convert( std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs,
		      IPixelRDO_Container* rdoIdc, const EventContext& ctx) const final;


private: 
  ToolHandle<IITkPixelRodDecoder>  m_decoder {this, "Decoder", "ITkPixelRodDecoder", 
    "Tool for ITkPixelRodDecoder"};

};

#endif
