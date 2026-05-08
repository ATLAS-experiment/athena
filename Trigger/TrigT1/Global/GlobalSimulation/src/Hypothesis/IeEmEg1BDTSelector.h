/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_IEEMEG1BDTSELECTOR_H
#define GLOBALSIM_IEEMEG1BDTSELECTOR_H


#include <string>

namespace GlobalSim {
  namespace IOBitwise{
    class eEmEg1BDTTOB;
  }
}

namespace  GlobalSim {

  /**
   * @brief PABC to selector classes for eEmEg1BDTTOBs.
   *
   */

  using IOBitwise::eEmEg1BDTTOB;
  
  class IeEmEg1BDTSelector {
  public:
    virtual ~IeEmEg1BDTSelector() = default;
    virtual bool select(const eEmEg1BDTTOB&) const = 0;
    virtual std::string to_string() const = 0;

  };
}
#endif
