/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IITkPixelRodDecoder_h
#define IITkPixelRodDecoder_h

#include "GaudiKernel/IAlgTool.h"

#include "InDetRawData/PixelRDO_Container.h"
#include "ByteStreamData/RawEvent.h" //ROBFragment typedef
#include "InDetRawData/PixelRDO_Container.h"// typedef


#include <string>
#include <vector>

class InterfaceID;
class StatusCode;
class EventContext;
class IdentifierHash;


class IITkPixelRodDecoder : virtual public IAlgTool{

  public: 
    static const InterfaceID& interfaceID( ) ;
    virtual ~IITkPixelRodDecoder() = default; 
    virtual StatusCode fillCollection  (const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment *robFrag,
					IPixelRDO_Container* rdoIdc,
					std::vector<IdentifierHash>* vecHash, const EventContext& ctx) const = 0;

};

inline const InterfaceID& IITkPixelRodDecoder::interfaceID(){
  static const InterfaceID IID_IITkPixelRodDecoder ("IITkPixelRodDecoder", 1, 0);
  return IID_IITkPixelRodDecoder;
}


#endif
