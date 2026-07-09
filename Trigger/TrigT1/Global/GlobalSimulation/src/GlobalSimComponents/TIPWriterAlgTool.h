/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_TIPWRITERALGTOOL_H
#define GLOBALSIM_TIPWRITERALGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "../GlobalSimComponents/ITIPWriterAlgTool.h"

#include <string>

namespace GlobalSim {

  /**
   * @brief AlgTool base class for all TIP writers
   * Implements common TIP-handling operations and checks
   */


  class TIPWriterAlgTool: public extends<AthAlgTool, ITIPWriterAlgTool> {

  public:
    TIPWriterAlgTool(const std::string& type,
		   const std::string& name,
		   const IInterface* parent);

    virtual ~TIPWriterAlgTool() override = default;

    /** @brief Initialize function running before first event */
    virtual StatusCode initialize() override;

    virtual StatusCode updateTIP(std::bitset<s_nbits_TIP>&,
				 const EventContext&) const override;

    virtual StatusCode countPassingTOBs(const EventContext&, unsigned int&) const = 0;

    // For TIP alg initialize to check for overlaps
    TIPword getFullTIPWord() const;

  protected:
  
    Gaudi::Property<unsigned int> m_TIP_position {
      this,
      "TIPposition",
      0,
      "start position to write into the TIP"};

    Gaudi::Property<unsigned int> m_TIP_width {
      this,
      "TIPwidth",
      1,
      "number of bits to write into the TIP"};    

    ulong m_maxtob{0};

  };
}
#endif
