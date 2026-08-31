/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
#ifndef GLOBALSIM_SPECREGISTRY_H
#define GLOBALSIM_SPECREGISTRY_H

/*
  The BitSpecs that can be read from and written to hex text, looked up by name.

  TOBTextReader and TOBTextWriter both take a BitSpec property and find their
  entry here, so a TOB type is listed once and becomes usable in both
  directions at the same time. Adding one is a single line in SpecRegistry.cxx.

  Keeping the layout behind a std::function means neither algorithm is a class
  template: there is one component per algorithm however many specs exist, and
  the spec is chosen at configuration time rather than at compile time.
*/

#include "AthContainers/AuxElement.h"

#include <cstddef>
#include <functional>
#include <map>
#include <string>

namespace GlobalSim {

  struct SpecEntry {
    /// Total width of the packed word, in bits. Always a multiple of four,
    /// so a token is width/4 hex characters.
    std::size_t width{0};

    /// Pack one object into its hex token.
    std::function<std::string(const SG::AuxElement&)> pack;

    /// Fill one object from its hex token. The caller must pass exactly
    /// width/4 hex characters -- a longer token would be silently truncated
    /// by std::bitset, so the length is checked where the file is read.
    std::function<void(SG::AuxElement&, const std::string&)> unpack;
  };

  /// The known specs, by name. Built on first use.
  const std::map<std::string, SpecEntry>& specRegistry();

  /// Comma-separated list of the known names, for error messages.
  std::string knownSpecNames();

}

#endif // GLOBALSIM_SPECREGISTRY_H
