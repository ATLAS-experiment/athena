/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
#ifndef GLOBALSIM_TOBTEXTWRITER_H
#define GLOBALSIM_TOBTEXTWRITER_H

/*
  Writes a container of TOBs back out as a hex text file, in the same shape as
  the firmware test vectors: one line per event, one token per TOB, each token
  the whole word in hex (spec width / 4 characters).

  The bit layout is chosen by name at configuration time through the BitSpec
  property, and looked up in a registry of packing functions kept in the .cxx.
  The writer itself holds no knowledge of field positions, and no knowledge of
  which specs exist: adding a TOB type is one line in that registry, with no
  new component and no new class.

  Lines are written as the events arrive, so the algorithm is non-reentrant and
  refuses to run if an event reaches it out of order: with more than one event
  in flight the correspondence between line N and event N no longer holds, and
  nothing else about the output would look wrong. Run these jobs with a single
  concurrent event.
*/

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODCore/BaseContainer.h"

#include <cstdint>
#include <fstream>
#include <functional>
#include <string>

namespace GlobalSim {

  class TOBTextWriter : public AthAlgorithm {
  public:
    using AthAlgorithm::AthAlgorithm;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;
    virtual StatusCode finalize() override;

  private:
    SG::ReadHandleKey<xAOD::BaseContainer> m_inKey{
        this, "Input", "", "Container of TOBs to write out"};

    Gaudi::Property<std::string> m_specName{
        this, "BitSpec", "",
        "Name of the BitSpec giving the bit layout, e.g. topoc_pu_type"};

    Gaudi::Property<std::string> m_outFile{
        this, "OutputFile", "globalsim_tobs.hex.txt",
        "Output file: one line per event, space-separated hex words"};

    // Taken from the registry in initialize(). The width is kept only so that
    // the startup message reports the token size, which is the quickest way to
    // spot an instance pointed at the wrong spec.
    std::function<std::string(const SG::AuxElement&)> m_pack;
    std::size_t m_width{0};

    std::ofstream m_out;

    // Guards line N == event N. The first event seen sets the start, so a job
    // that skips events is still accepted; any gap after that is an error.
    uint64_t m_nextEvt{0};
    uint64_t m_nWritten{0};
  };

} // namespace GlobalSim

#endif // GLOBALSIM_TOBTEXTWRITER_H
