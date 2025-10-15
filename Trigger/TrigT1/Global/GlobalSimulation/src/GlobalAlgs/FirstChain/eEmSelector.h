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

  
  class ICutter {
  public:
    virtual bool cut(const ulong&) const = 0;
  };

  class eEmSelector : public IeEmSelector {
  public:

    /// Passes all.
    eEmSelector() = default;

    /// window limits from strings, to match the eEmTOB bitsets
    eEmSelector(ulong rhad_cut,
		const std::string& rhad_op,
		ulong reta_cut,
		const std::string& reta_op,
		ulong wstot_cut,
		const std::string& wstot_op);
    
    virtual ~eEmSelector() = default;
    
    virtual bool select(const IeEmTOB&) const override;

  private:
    ulong m_rhad_cut{0};
    ulong m_eta_cut{0};
    ulong m_wstot_cut{0};
    std::unique_ptr<ICutter> m_rhad_cutter{nullptr};
    std::unique_ptr<ICutter> m_reta_cutter{nullptr};
    std::unique_ptr<ICutter> m_wstot_cutter{nullptr};
  };
}
#endif
