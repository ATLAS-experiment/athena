/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_ICOMMONSELECTOR_H
#define	GLOBALSIM_ICOMMONSELECTOR_H

#include <string>

namespace GlobalSim {
  namespace IOBitwise{
    class ICommonTOB;
  }
}

namespace  GlobalSim {
  /**
   * @brief PABC to selector class for ICommonTOBs.
   *
   */
  
  using GlobalSim::IOBitwise::ICommonTOB;

  class ICommonSelector {
  public:
    
    virtual ~ICommonSelector() = default;
    virtual bool select(const ICommonTOB&) const = 0;
    virtual std::string to_string() const = 0;

  };
}
#endif
