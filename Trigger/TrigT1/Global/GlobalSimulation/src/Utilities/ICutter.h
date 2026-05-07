/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_ICUTTER_H
#define GLOBALSIM_ICUTTER_H

#include <climits>
#include <memory>

namespace  GlobalSim {
  /**
   * @brief Implementaton of ICutter, for converting text to
   * cut logic in selectors.
   */
  class ICutter {
  public:
    virtual ~ICutter() = default;
    virtual bool cut(const float&) const = 0;
    virtual std::string to_string() const = 0;
  };
  std::unique_ptr<ICutter> make_cutter(const float& cut,
				       const std::string& op);
}
#endif
