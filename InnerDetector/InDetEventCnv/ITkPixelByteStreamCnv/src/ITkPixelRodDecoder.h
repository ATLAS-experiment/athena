/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITkPixelRodDecoder_h
#define ITkPixelRodDecoder_h


#include "ITkPixelByteStreamCnv/IITkPixelRodDecoder.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamData/RawEvent.h" //ROBFragment typedef
#include "InDetRawData/PixelRDO_Container.h"// typedef

#include <string>
#include <vector>

class IInterface;
class StatusCode;
class EventContext;
class IdentifierHash;

class ITkPixelRodDecoder final: virtual public IITkPixelRodDecoder, public AthAlgTool {

  public:
    ITkPixelRodDecoder(const std::string& type, const std::string& name,
      const IInterface* parent ) ;

    // destructor
    ~ITkPixelRodDecoder() = default;

    StatusCode initialize() override;
    StatusCode finalize() override;

    StatusCode fillCollection (const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment *robFrag,
		               IPixelRDO_Container* rdoIdc,
			       std::vector<IdentifierHash>* vecHash, const EventContext& ctx) const override;

  private:
   
};

#endif
