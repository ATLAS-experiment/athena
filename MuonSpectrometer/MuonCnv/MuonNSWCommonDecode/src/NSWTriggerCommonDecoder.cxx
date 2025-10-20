/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonNSWCommonDecode/NSWTriggerCommonDecoder.h"
#include "MuonNSWCommonDecode/NSWTriggerElink.h"
#include "MuonNSWCommonDecode/NSWTriggerMML1AElink.h"
#include "MuonNSWCommonDecode/NSWTriggerMMMonElink.h"
#include "MuonNSWCommonDecode/NSWTriggerSTGL1AElink.h"
#include "MuonNSWCommonDecode/NSWPadTriggerL1a.h"


Muon::nsw::NSWTriggerCommonDecoder::NSWTriggerCommonDecoder (const eformat::read::ROBFragment &robFrag, const std::string& triggerType)
  : m_has_error (false),
    m_triggerType (triggerType)
{

  static const uint32_t min_packet_size = 2; //2w felix header

  robFrag.check ();

  uint32_t nWords = robFrag.rod_ndata (); //total number of words (32-bit word)
  const uint32_t *bs = robFrag.rod_data (); //point directly to the first data element
  const uint32_t *pp = bs; //pointer moving
  uint32_t remaining = nWords; // data-element (32-bit word) counter; it will decrement according to each elink output

  while (remaining >= min_packet_size)
  {
    try
    {
      std::shared_ptr<Muon::nsw::NSWTriggerElink> elink;

      if ( m_triggerType== "MML1A" ){
	std::shared_ptr<Muon::nsw::NSWTriggerMML1AElink> tmplink = std::make_shared<Muon::nsw::NSWTriggerMML1AElink>(pp, remaining); 
	elink = tmplink;
      } else if ( m_triggerType== "MMMon" ) {
	std::shared_ptr<Muon::nsw::NSWTriggerMMMonElink> tmplink = std::make_shared<Muon::nsw::NSWTriggerMMMonElink>(pp, remaining);
	elink = tmplink;
      } else if ( m_triggerType== "PadL1A" ) {
	std::shared_ptr<Muon::nsw::NSWPadTriggerL1a> tmplink = std::make_shared<Muon::nsw::NSWPadTriggerL1a>(pp, remaining);
	elink = tmplink;
      } else if ( m_triggerType== "STGL1A" ) {
        std::shared_ptr<Muon::nsw::NSWTriggerSTGL1AElink> tmplink = std::make_shared<Muon::nsw::NSWTriggerSTGL1AElink>(pp, remaining);
        elink = tmplink;
      } else {
	std::shared_ptr<Muon::nsw::NSWTriggerElink> tmplink = std::make_shared<Muon::nsw::NSWTriggerElink>(pp, remaining);
	elink = std::move(tmplink);
      }

      m_elinks.push_back(elink);
      pp += elink->nwords();
      remaining -= elink->nwords();;

    }
    catch (Muon::nsw::NSWTriggerException &e) {
      //known expections, with ID
      //could think of an error msg print in case needed
      m_has_error = true;
      ERS_DEBUG(1, "Following exception found");
      ERS_DEBUG(1, e.what());
      m_error_id = e.id();
      break;
    }    
    catch (std::exception &e) {
      //better to be ready to capture generic ones as well
      m_has_error = true;
      ERS_DEBUG(1, "Following exception found");
      ERS_DEBUG(1, e.what());
      m_error_id = -1; //negative for unknown exceptions
      break;
    }
  }

  if ( remaining > 0 ) {
    m_has_error = true;
    ERS_DEBUG(1, Muon::nsw::format("There are remaining words ({}) after decoding {} elink(s)", remaining, m_elinks.size()));
    m_error_id = 0;
  }

}

