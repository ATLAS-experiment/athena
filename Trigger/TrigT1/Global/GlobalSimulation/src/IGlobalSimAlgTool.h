/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_IGLOBALSIMALGTOOL_H
#define GLOBALSIM_IGLOBALSIMALGTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

#include <string>
#include <bitset>

// provide an pure abstract interface to AlgTools implementing
// GlobalSim Algs.

namespace GlobalSim {
  class IGlobalSimAlgTool : virtual public ::IAlgTool {

  public:
    
    /// Number of bits for the TIP word. The TIP word gathers
    /// results from Global, and sends to the the CTP.
    static constexpr std::size_t s_nbits_TIP{1024};
    
    DeclareInterfaceID(IGlobalSimAlgTool, 1, 0);
    virtual ~IGlobalSimAlgTool() = default;

    virtual StatusCode run(const EventContext& ctx) const = 0;

    virtual StatusCode updateTIP(std::bitset<s_nbits_TIP>&,
				 const EventContext& ) const = 0;


    virtual std::string toString() const = 0;
  };
  
}
#endif
