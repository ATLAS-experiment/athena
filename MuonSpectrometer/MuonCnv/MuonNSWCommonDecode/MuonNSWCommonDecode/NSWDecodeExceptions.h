/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _MUON_NSW_DECODER_EXCEPTIONS_H_
#define _MUON_NSW_DECODER_EXCEPTIONS_H_
                                                                                
#include "ers/ers.h"

namespace Muon
{
  namespace nsw
  {
    ERS_DECLARE_ISSUE (MuonNSWCommonDecoder, NSWElinkException, , )
    //cid 21227
    //coverity[UNNECESSARY_STRING_COPY]
    ERS_DECLARE_ISSUE_BASE (MuonNSWCommonDecoder,
			    NSWElinkFelixHeaderException,
			    MuonNSWCommonDecoder::NSWElinkException,
			    "" << name,
			    ,
			    ((std::string) name)
			    )
	  //cid 20840
    //coverity[UNNECESSARY_STRING_COPY]
    ERS_DECLARE_ISSUE_BASE (MuonNSWCommonDecoder,
			    NSWElinkROCHeaderException,
			    MuonNSWCommonDecoder::NSWElinkException,
			    "" << name,
			    ,
			    ((std::string) name)
			    )
  }
}

#endif // _MUON_NSW_DECODER_EXCEPTIONS_H_
           
