/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/*
  This Algorithm simulates both the input bitsream which would be received by the MUX from the LASP based on
  the input GlobalLArCellContainer, as well as the output bitsream which each MUX would send to the GEPs. All
  bitstreams are stored as textfiles in the running directory
*/

#include "LArCellMuxAlg.h"

#include "PathResolver/PathResolver.h"
#include "xAODEventInfo/EventInfo.h"

#include <fstream>
#include <fmt/core.h>

namespace GlobalSim {

  // Initializing LArCellMuxAlg
  StatusCode LArCellMuxAlg::initialize() {
    ATH_MSG_INFO ("Initializing " << name());

    CHECK(m_eventInfo.initialize());
    CHECK(m_gblLArCellContainerKey.initialize());

    return StatusCode::SUCCESS;
  }


  // Execute function which retrieves containers from StoreGate and then calls the two
  // functions which write the input and output bitstream of the MUX
  StatusCode LArCellMuxAlg::execute(const EventContext& ctx) const {

    ATH_MSG_DEBUG ("Executing LArCellMuxAlg algorithm");

    // Get EventInfo
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfo, ctx);
    CHECK(eventInfo.isValid());

    // Read in GlobalLArCellContainer
    auto h_gblLArCells = SG::makeHandle(m_gblLArCellContainerKey, ctx);
    CHECK(h_gblLArCells.isValid());
    const auto & gblLArCells = *h_gblLArCells;

    ATH_MSG_DEBUG("Reading " << std::to_string(h_gblLArCells->size()) << " cells in input GlobalLArCellContainer");

    Feb2BitsetMap feb2Bitsets;

    // Compile and write the input bitstreams which are sent from LASP to MUX
    CHECK(writeMuxInputBitstream(feb2Bitsets, *eventInfo, gblLArCells));

    // Compile and write the output bitstreams which are sent from MUX to GEP
    CHECK(writeMuxOutputBitstream(feb2Bitsets, *eventInfo, gblLArCells));

    return StatusCode::SUCCESS;
  }


  // Function that takes the GlobalLArCellContainer as input and writes the input bitstreams
  // to the MUX for all FEB2 to file (if requested in job options). All bistreams are stored
  // in a map for later use
  StatusCode LArCellMuxAlg::writeMuxInputBitstream(Feb2BitsetMap& feb2Bitsets, 
                                                   const xAOD::EventInfo& eventInfo, 
                                                   const GlobalSim::GlobalLArCellContainer& gblLArCells) const {
    
    std::ofstream out;
    if (m_writeMuxInputBitstreamToFile) {
        std::string filename = fmt::format("MuxInputBitstream/{}_MuxInput_{}_{}.dat", name(), eventInfo.runNumber(), eventInfo.eventNumber());
        out.open(filename);
        if (!out) {
            ATH_MSG_ERROR("Failed to open file: " << filename);
            return StatusCode::FAILURE;
        }
    }

    for (const auto& feb2Key : gblLArCells.getFeb2Keys()) {

        const std::vector<GlobalLArCell*>& cells = gblLArCells.getCellsForFeb2(feb2Key);
        std::bitset<FEB2_BITSTREAM_SIZE> input_bitstream = assembleBitsetForFeb2(cells, 
            gblLArCells.getMaxCellsPerFeb2(), 
            gblLArCells.feb2InOverflow(feb2Key), 
            gblLArCells.feb2InError(feb2Key), 
            eventInfo.bcid());

        feb2Bitsets[feb2Key] = input_bitstream;

        if (m_writeMuxInputBitstreamToFile) {
            out << "====== " << feb2Key << " is sending to " << gblLArCells.getMuxForFeb2(feb2Key) << ":\n";

            for (std::size_t row = 0; row < FEB2_BITSTREAM_SIZE / BITWIDTH_MUX_INPUT; ++row) {
                for (int col = BITWIDTH_MUX_INPUT-1; col >= 0; --col) {
                    int bitIndex = row * BITWIDTH_MUX_INPUT + col;
                    out << input_bitstream[bitIndex];
                }
                out << "\n";
            }
            out << "\n";
        }
    }

    if (m_writeMuxInputBitstreamToFile) out.close();

    return StatusCode::SUCCESS;
  }


  // Function that assembles the bitstream from each FEB2
  std::bitset<LArCellMuxAlg::FEB2_BITSTREAM_SIZE> LArCellMuxAlg::assembleBitsetForFeb2(const std::vector<GlobalSim::GlobalLArCell*>& cells, std::size_t maxCells,  bool inOverflow, bool inError, uint32_t bcid) const {

      // Sort cells based on channel number
      std::vector<const GlobalSim::GlobalLArCell*> sortedCells(cells.begin(), cells.end());
      std::sort(sortedCells.begin(), sortedCells.end(), [](const auto* a, const auto* b) {
          return a->getChannel() < b->getChannel();
      });

      // Declare bitsets for 2sigma and 4sigma mask, as well as energies
      const std::size_t bitwidth_4SigmaMask = maxCells;
      const std::size_t bitwidth_energyBlock = FEB2_BITSTREAM_SIZE - BITWIDTH_2SIGMAMASK - bitwidth_4SigmaMask - 2 - 8;

      std::vector<bool> mask_twoSigma(BITWIDTH_2SIGMAMASK, false);
      std::vector<bool> mask_fourSigma(bitwidth_4SigmaMask, false);
      std::vector<bool> energyBits(bitwidth_energyBlock, false);

      std::size_t bitPosEnergy = 0;

      for (std::size_t i = 0; i < sortedCells.size(); ++i) {

          const auto* cell = sortedCells[i];

          // 2sigma mask by channel
          mask_twoSigma[cell->getChannel()] = true;

          // 4sigma mask by order of 2sigma cells
          mask_fourSigma[i] = (cell->getSigma() >= 4.0);

          // Energy bits
          const boost::dynamic_bitset<>& enBits = cell->getEnergyBitstring();
          for (std::size_t j = 0; j < enBits.size() && bitPosEnergy < bitwidth_energyBlock; ++j, ++bitPosEnergy) {
              energyBits[bitPosEnergy]= enBits[j];
          }
      }

      // 8b bunch crossing number
      std::bitset<8> bitsBCN(bcid & 0xFF);

      // Assemble final bitstream
      std::bitset<FEB2_BITSTREAM_SIZE> bitstream;
      std::size_t pos = 0;

      pos = appendBits(bitstream, mask_twoSigma, pos);
      pos = appendBits(bitstream, mask_fourSigma, pos);

      if (inOverflow) bitstream.set(pos);
      ++pos;
      if (inError) bitstream.set(pos);
      ++pos;

      pos = appendBits(bitstream, energyBits, pos);
      pos = appendBits(bitstream, bitsBCN, pos);

      return bitstream;
  }


  // Function which writes the output bitstream of each MUX to file (if requested in job options)
  // Each bitstream consists of a header, which is assembled here, the body, which is taken from
  // the map previously filled when assembling the input bitsets, and a footer, which is seven
  // words of zeros
  StatusCode LArCellMuxAlg::writeMuxOutputBitstream(const Feb2BitsetMap& feb2Bitsets, 
                                                    const xAOD::EventInfo& eventInfo, 
                                                    const GlobalSim::GlobalLArCellContainer& gblLArCells) const {

    if (!m_writeMuxOutputBitstreamToFile) {
        return StatusCode::SUCCESS;
    }

    std::string filename = fmt::format("MuxOutputBitstream/{}_MuxOutput_AllMuxes_{}_{}.dat", name(), eventInfo.runNumber(), eventInfo.eventNumber());
    std::ofstream out(filename);
    if (!out) {
        ATH_MSG_ERROR("Failed to open file: " << filename);
        return StatusCode::FAILURE;
    }
    
    // Assemble the header for each MUX
    const int nBitsMarker = BITWIDTH_MUX_OUTPUT / 8;

    // MUX marker line has the pattern ABBA ABBA
    unsigned long long markerPattern = 
        (0xAULL << nBitsMarker*7) | 
        (0xBULL << nBitsMarker*6) | 
        (0xBULL << nBitsMarker*5) | 
        (0xAULL << nBitsMarker*4) | 
        (0xAULL << nBitsMarker*3) | 
        (0xBULL << nBitsMarker*2) | 
        (0xBULL << nBitsMarker) | 
        (0xAULL);

    std::bitset<BITWIDTH_MUX_OUTPUT> markerBitset(markerPattern);

    // 12b bunch crossing number
    std::bitset<12> bitsBCN(eventInfo.bcid() & 0xFFF);

    // Loop over all MUX IDs
    for (int muxID = 0; muxID < 32; ++muxID) {

        std::string muxKey = fmt::format("apl-gbl-mux-lasp-{:02d}", muxID);
        out << "====== " << muxKey << " is sending to GEP:\n";

        std::bitset<6> bitsetMuxID(muxID);

        // Bitset for the entire header block
        const int bitwidthMuxHeader = BITWIDTH_MUX_OUTPUT * N_WORDS_MUX_HEADER;
        std::bitset<bitwidthMuxHeader> fullMuxHeader;

        // Assemble the header block
        std::size_t pos = 0;
        pos = appendBits(fullMuxHeader, markerBitset, pos);
        pos = appendBits(fullMuxHeader, bitsBCN, pos);
        pos = appendBits(fullMuxHeader, bitsetMuxID, pos);

        // Overflow and error bits
        if (gblLArCells.muxInOverflow(muxKey)) fullMuxHeader.set(pos);
        ++pos;
        if (gblLArCells.muxInError(muxKey)) fullMuxHeader.set(pos);
        ++pos;

        // Write header in lines of BITWIDTH_MUX_OUTPUT
        for (std::size_t row = 0; row < N_WORDS_MUX_HEADER; ++row) {
            for (int col = BITWIDTH_MUX_OUTPUT-1; col >= 0; --col) {
                int bitIndex = row * BITWIDTH_MUX_OUTPUT + col;
                out << fullMuxHeader[bitIndex];
            }
            out << "\n";
        }

        // Get and loop over FEB2s to write the FEB2 block
        const std::vector<std::string>& feb2s = gblLArCells.getOrderedFeb2sForMux(muxKey);
        for (const auto& feb2 : feb2s) {

            auto it = feb2Bitsets.find(feb2);
            if (it == feb2Bitsets.end()) {
                ATH_MSG_ERROR("No bitset for FEB2 " << feb2);
                return StatusCode::FAILURE;
            }
            const std::bitset<FEB2_BITSTREAM_SIZE>& feb2Bitset = it->second;

            for (std::size_t row = 0; row < FEB2_BITSTREAM_SIZE / BITWIDTH_MUX_OUTPUT; ++row) {
                for (int col = BITWIDTH_MUX_OUTPUT-1; col >= 0; --col) {
                    int bitIndex = row * BITWIDTH_MUX_OUTPUT + col;
                    out << feb2Bitset[bitIndex];
                }
                out << "\n";
            }
        }

        // Close by writing the footer of the MUX (N_WORDS_MUX_FOOTER empty words)
        for (std::size_t row = 0; row < N_WORDS_MUX_FOOTER; ++row) {
            for (std::size_t col = 0; col < BITWIDTH_MUX_OUTPUT; ++col) {
                out << '0';
            }
            out << "\n";
        }
        out << "\n";

    } // Loop over all MUX IDs

    out.close();
    return StatusCode::SUCCESS;
  }

  template <std::size_t N, typename T> std::size_t LArCellMuxAlg::appendBits(std::bitset<N>& target, const T& src, std::size_t pos) {
      for (std::size_t i = 0; i < src.size(); ++i, ++pos)
          if (src[i]) target.set(pos);
      return pos;
  }

} // namespace GlobalSim

