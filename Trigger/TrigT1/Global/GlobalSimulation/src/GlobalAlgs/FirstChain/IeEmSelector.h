/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_IEEMSELECTOR_H
#define GLOBALSIM_IEEMSELECTOR_H


namespace GlobalSim {
  namespace IOBitwise{
    class IeEmTOB;
  }
}

namespace  GlobalSim {

  /**
   * @brief PABC to selector classes for eEmTOBs.
   *
   */

  using GlobalSim::IOBitwise::IeEmTOB;
  
  class IeEmSelector {
  public:
    virtual ~IeEmSelector() = default;
    virtual bool select(const IeEmTOB&) const = 0; 
  };
}
#endif
