/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMSELECTOR_H
#define GLOBALSIM_EEMSELECTOR_H

#include "IeEmSelector.h"
#include "../../IO/IeEmTOB.h"  // bitset widths

#include <climits>

namespace  GlobalSim {
  /**
   * @brief Implementaton of IeEmSelector. Selects IeEmTOBs following
   * hypo block VHDL code using window cuts on et, eta and phi.
   *
   */

  using GlobalSim::IOBitwise::IeEmTOB;
  
  class eEmSelector : public IeEmSelector {
  public:

    /// Passes all.
    eEmSelector() = default;

    /// window limits from strings, to match the eEmTOB bitsets
    eEmSelector(const std::string& rhad_low,
		   const std::string& rhad_high,
		   const std::string& reta_low,
		   const std::string& reta_high,
		   const std::string& wstot_low,
		   const std::string& wstot_high);

    virtual ~eEmSelector() = default;

    virtual bool select(const IeEmTOB&) const override;

  private:
    ulong m_rhad_low{0};
    ulong m_rhad_high{ULONG_MAX};
    
    ulong m_reta_low{0};
    ulong m_reta_high{ULONG_MAX};
        
    ulong m_wstot_low{0};
    ulong m_wstot_high{ULONG_MAX};

  };
}
#endif
