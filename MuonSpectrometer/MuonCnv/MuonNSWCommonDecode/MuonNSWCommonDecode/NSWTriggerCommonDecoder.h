/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONNSWCMOMONDECODE_NSWTRIGGERCOMMONDECODER_H
#define MUONNSWCMOMONDECODE_NSWTRIGGERCOMMONDECODER_H

#include "eformat/eformat.h"
#include <vector>
#include <string>
#include <memory>

namespace Muon
{
  namespace nsw
  {
    class NSWTriggerElink;

    class NSWTriggerCommonDecoder
    {
     public:
      explicit NSWTriggerCommonDecoder (const eformat::read::ROBFragment &rob, const std::string& triggerType);
      //will use trigger type for stgc/mm/pad specialization?
      virtual ~NSWTriggerCommonDecoder () = default;

      //it's up to the user dynamic casting to needed elink type
      //or we can make this a template but then loosing the ability to change trigger type at run time
      const std::vector<std::shared_ptr<Muon::nsw::NSWTriggerElink>>& get_elinks () const {return m_elinks;};

      //keep in mind that the decoder is stopping at first exception: 
      //no data recovery implemented on purpose, since there are infinite ways to have broken TP packets
      bool has_error () const {return m_has_error;};
      int error_id () const {return m_error_id;};

     private:
      bool m_has_error{};
      int  m_error_id{};
      std::string m_triggerType;
      std::vector<std::shared_ptr<Muon::nsw::NSWTriggerElink>> m_elinks;
    };

  }
}

#endif // not MUONNSWCMOMONDECODE_NSWTRIGGERCOMMONDECODER_H
