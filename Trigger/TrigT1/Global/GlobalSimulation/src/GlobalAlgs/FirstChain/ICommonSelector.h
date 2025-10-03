/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_ICOMMONSELECTOR_H
#define	GLOBALSIM_ICOMMONSELECTOR_H

namespace  GlobalSim {
  /**
   * @brief PABC to selector classes for eEmTOBs.
   *
   */

  using IOBitwise::ICommonTOB;
  
  class ICommonSelector {
  public:
    virtual ~ICommonSelector() = default;
    virtual bool select(const ICommonTOB&) const = 0; 
  };
}
#endif
