/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
#ifndef GLOBALSIM_SPECREGISTRY_H
#define GLOBALSIM_SPECREGISTRY_H

/*
  Turns the name of a BitSpec into the pair of functions that pack and unpack
  it. TOBTextReader and TOBTextWriter both take the name as a property and ask
  for it here, so a TOB type becomes usable in both directions at once.

  Which specs exist is not stated in this package's C++ any more. A spec becomes
  usable by being listed in GlobalSimulation/selection.xml, which is also what
  the dictionary is generated from; the functions are then built from the type
  name through the ROOT interpreter the first time that name is asked for, and
  kept. Adding a TOB type touches the header that defines it and the two files
  next to it, and nothing here.

  Keeping the layout behind a std::function means neither algorithm is a class
  template: there is one component per algorithm however many specs exist, and
  the spec is chosen at configuration time rather than at compile time.
*/

#include "AthContainers/AuxElement.h"

#include <cstddef>
#include <functional>
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

  /// The entry for one spec, or nullptr if there is not one. Built on the first
  /// call for a given name and kept, so this is cheap to call again but should
  /// still be called from initialize(): building one is not thread safe.
  ///
  /// On failure `error` is set to the reason, which is either that the name is
  /// not among those listed in selection.xml, or that the interpreter would not
  /// compile against it.
  const SpecEntry* specEntry(const std::string& name, std::string& error);

  /// Comma-separated list of the names in the dictionary, for error messages.
  std::string knownSpecNames();

}

#endif // GLOBALSIM_SPECREGISTRY_H
