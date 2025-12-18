/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_COMMONMULTSELECTOR_H
#define GLOBALSIM_COMMONMULTSELECTOR_H

#include "ICommonSelector.h"
#include "../IO/CommonTOB.h"  // bitset widths

#include <string>
#include <climits>

namespace  GlobalSim {
  /**
   * @brief Implementaton of ICommonSelector. Selects ICommonTOBs following
   * hypo block VHDL code using window cuts on et, eta and phi.
   *
   */

  using GlobalSim::IOBitwise::CommonTOB;
  
  class CommonSelector : public ICommonSelector {
  public:

    /// Passes all.
    CommonSelector() = default;

    /// window limits from strings, to match the CommonTOB bitsets
    CommonSelector(const std::string& et_low,
		   const std::string& et_high,
		   const std::string& eta_low,
		   const std::string& eta_high,
		   const std::string& phi_low,
		   const std::string& phi_high);

    virtual ~CommonSelector() = default;

    virtual bool select(const CommonTOB&) const override;

    virtual std::string to_string() const override;

  private:
    ulong m_et_low{0};
    ulong m_et_high{ULONG_MAX};
    
    ulong m_eta_low{0};
    ulong m_eta_high{ULONG_MAX};

    
    ulong m_phi_low{0};
    ulong m_phi_high{ULONG_MAX};
  };
}
#endif
