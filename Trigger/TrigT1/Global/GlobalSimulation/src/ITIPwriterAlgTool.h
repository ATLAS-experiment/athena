/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_ITIPWRITERALGTOOL_H
#define GLOBALSIM_ITIPWRITERALGTOOL_H

#include "GaudiKernel/IAlgTool.h"

#include <string>
#include <bitset>

// provide an pure abstract interface to AlgTools implementing
// AlgTools which write to the TIP word.

namespace GlobalSim {
  class ITIPwriterAlgTool : virtual public ::IAlgTool {

  public:
    
    /// Number of bits for the TIP word. The TIP word gathers
    /// results from Global, and sends to the the CTP.
    static constexpr std::size_t s_nbits_TIP{1024};
    
    DeclareInterfaceID(ITIPwriterAlgTool, 1, 0);
    virtual ~ITIPwriterAlgTool() = default;

    virtual StatusCode updateTIP(std::bitset<s_nbits_TIP>&,
				 const EventContext& ) const = 0;


    virtual std::string toString() const = 0;
  };
  
}
#endif
