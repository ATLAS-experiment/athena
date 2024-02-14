/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IITkPixelRawDataProviderTool_h
#define IITkPixelRawDataProviderTool_h

#include "GaudiKernel/IAlgTool.h"
#include "ByteStreamData/RawEvent.h" //ROBFragment typedef
#include "InDetRawData/PixelRDO_Container.h"// typedef


#include <string>
#include <vector>

class InterfaceID;
class EventContext;
class StatusCode;


// the tool to decode a ROB frament
class IITkPixelRawDataProviderTool : virtual public IAlgTool{

 public:
   
  //! AlgTool InterfaceID
  static const InterfaceID& interfaceID( );
  
  virtual ~IITkPixelRawDataProviderTool() = default;

  //! this is the main decoding method
  virtual StatusCode convert( std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vecRobs,
	  IPixelRDO_Container* rdoIdc,
		const EventContext& ctx) const = 0;

};

inline const InterfaceID& IITkPixelRawDataProviderTool::interfaceID(){
  static const InterfaceID IID_IITkPixelRawDataProviderTool("IITkPixelRawDataProviderTool", 1, 0);
  return IID_IITkPixelRawDataProviderTool;
}

#endif
