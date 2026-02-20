/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_LARCELLMUXALG_H
#define GLOBALSIM_LARCELLMUXALG_H

/*
  This Algorithm simulates both the input bitsream which would be received by the MUX from the LASP based on
  the input GlobalLArCellContainer, as well as the output bitsream which each MUX would send to the GEPs. All
  bitstreams are stored as textfiles in the running directory
*/

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODEventInfo/EventInfo.h"
#include "GaudiKernel/ToolHandle.h"

#include "GlobalLArCell.h"
#include "GlobalLArCellContainer.h"

#include <vector>
#include <bitset>

namespace GlobalSim {

  class LArCellMuxAlg : public AthReentrantAlgorithm { 
  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    static constexpr std::size_t FEB2_BITSTREAM_SIZE = 576;
    static constexpr std::size_t BITWIDTH_2SIGMAMASK = 128;
    static constexpr std::size_t BITWIDTH_MUX_INPUT  = 32;
    static constexpr std::size_t BITWIDTH_MUX_OUTPUT = 64; // Needs to be multiple of 8
    static constexpr std::size_t N_WORDS_MUX_HEADER  = 2;
    static constexpr std::size_t N_WORDS_MUX_FOOTER  = 7;

    // Define a short-hand alias for the FEB2 bitset map
    using Feb2BitsetMap = std::map<std::string, std::bitset<FEB2_BITSTREAM_SIZE>>;

    /** @brief initialize function running before first event */
    virtual StatusCode  initialize() override;   
    /** @brief execute function running for every event */
    virtual StatusCode  execute(const EventContext& ) const override;

  private:

    /** @brief Function that compiles the LASP to MUX bitstream and writes it to file */
    StatusCode writeMuxInputBitstream(Feb2BitsetMap& feb2Bitsets, const xAOD::EventInfo& eventInfo, const GlobalSim::GlobalLArCellContainer& gblLArCells) const;
    /** @brief Function that compiles the MUX to GEP bitstream and writes it to file */
    StatusCode writeMuxOutputBitstream(const Feb2BitsetMap& feb2Bitsets, const xAOD::EventInfo& eventInfo, const GlobalSim::GlobalLArCellContainer& gblLArCells) const;
    /** @brief Function which compiles the bitset for one particular FEB2 */
    std::bitset<FEB2_BITSTREAM_SIZE> assembleBitsetForFeb2(const std::vector<GlobalSim::GlobalLArCell*>& cells, std::size_t maxCells, bool inOverflow, bool inError, uint32_t bcid) const;
    /** Helper function to append a bitset or vector of bools at a specific position in another bitset */
    template <std::size_t N, typename T> static std::size_t appendBits(std::bitset<N>& target, const T& src, std::size_t pos);


    /** @brief Key for the EventInfo object */
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfo", "EventInfo", "Key for the EventInfo container"};

    /** @brief Key to the GlobalLArCellContainer */
    SG::ReadHandleKey<GlobalSim::GlobalLArCellContainer> m_gblLArCellContainerKey {this, "GlobalLArCellsKey", "GlobalLArCells", "Key for the output container of the LAr cells sent to Global"};

    /** @brief Flag to enable writing of MUX input bitstreams (FEB2/LASP -> MUX) to file */
    Gaudi::Property<bool> m_writeMuxInputBitstreamToFile {this, "WriteMuxInputBitstreamToFile", true, "Flag to enable the writing of MUX input bitstreams to file"};

    /** @brief Flag to enable writing ofMUX output bitstreams (MUX -> GEP) to file */
    Gaudi::Property<bool> m_writeMuxOutputBitstreamToFile {this, "WriteMuxOutputBitstreamToFile", true, "Flag to enable the writing of MUX output bitstreams to file"};

  };

}
#endif




