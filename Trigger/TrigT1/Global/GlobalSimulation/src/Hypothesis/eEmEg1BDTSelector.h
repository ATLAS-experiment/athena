/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMEG1BDTSELECTOR_H
#define GLOBALSIM_EEMEG1BDTSELECTOR_H

#include "IeEmEg1BDTSelector.h"
#include "../Utilities/ICutter.h"

#include <climits>
#include <memory>

namespace  GlobalSim {
  /**
   * @brief Implementaton of IeEmEg1BDTSelector. Selects eEmEg1BDTTOBs following
   * hypo block VHDL code using logical cuts on the eGamma1BDT result.
   *
   */

  using GlobalSim::IOBitwise::eEmEg1BDTTOB;

  class eEmEg1BDTSelector : public IeEmEg1BDTSelector {
  public:

    /// Passes all.
    eEmEg1BDTSelector() = default;

    /// window limits from strings, to match the eEmEg1BDTTOB bitsets
    eEmEg1BDTSelector(ulong Eg1BDT_cut,
		      const std::string& Eg1BDT_op);
    
    virtual ~eEmEg1BDTSelector() = default;
    
    virtual bool select(const eEmEg1BDTTOB&) const override;

    virtual std::string to_string() const override;

  private:
    std::unique_ptr<ICutter> m_Eg1BDT_cutter{nullptr};
  };
}
#endif
