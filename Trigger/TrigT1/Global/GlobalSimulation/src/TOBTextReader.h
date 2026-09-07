/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
#ifndef GLOBALSIM_TOBTEXTREADER_H
#define GLOBALSIM_TOBTEXTREADER_H

/*
  Reads a hex text file of TOB test vectors and produces a container of
  objects carrying the decoded fields: one line per event, one token per TOB,
  each token the whole word in hex.

  The mirror of TOBTextWriter, and it works the same way: the bit layout is
  chosen by name through the BitSpec property and looked up in SpecRegistry, so
  the reader holds no knowledge of field positions and adding a TOB type needs
  no change here at all.

  Tokens are kept as text until they are used, rather than being parsed into
  integers up front, so that the reader is not limited to words that fit in
  64 bits.
*/

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODCore/BaseContainer.h"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace GlobalSim {

  class TOBTextReader : public AthReentrantAlgorithm {
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    Gaudi::Property<std::string> m_inputFile{
        this, "InputFile", "",
        "Test-vector file: one event per line, space-separated hex words"};

    Gaudi::Property<std::string> m_specName{
        this, "BitSpec", "",
        "Name of the BitSpec giving the bit layout, e.g. topoc_pu_type"};

    SG::WriteHandleKey<xAOD::BaseContainer> m_outKey{
        this, "Output", "",
        "Container of decoded TOBs to write to StoreGate"};

    // Taken from the registry in initialize().
    std::function<void(SG::AuxElement&, const std::string&)> m_unpack;
    std::size_t m_width{0};

    // The whole file, read once: one entry per event, each a list of tokens.
    // Read-only after initialize(), which is what lets execute() stay const.
    std::vector<std::vector<std::string>> m_events;
  };

} // namespace GlobalSim

#endif // GLOBALSIM_TOBTEXTREADER_H
