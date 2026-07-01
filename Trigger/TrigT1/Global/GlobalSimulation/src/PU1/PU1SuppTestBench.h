/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PU1SuppTestBench.h
 * @brief Declares the PU1SuppTestBenchAlg, an Athena algorithm for testing PU1 suppression logic.
 *
 * This algorithm reads 256-bit TOB and rho inputs from files and writes them into the Event Store.
 * It is intended for standalone testing and debugging of the PU1 suppression algorithm.
 */

#ifndef GLOBALSIM_PU1SUPPTESTBENCHALG_H
#define GLOBALSIM_PU1SUPPTESTBENCHALG_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"

#include "PU1SuppPortsIn.h"
#include "PU1SuppExpectations.h"

#include <string>
#include <memory>
#include <vector>

namespace GlobalSim {

/**
 * @class PU1SuppTestBenchAlg
 * @brief Athena algorithm that feeds TOB and rho data into the PU1 suppression chain.
 *
 * Reads hex-encoded 256-bit TOBs and associated rho values from file, converts them into
 * PU1SuppPortsIn structs, and pushes them into the StoreGate FIFO.
 */
class PU1SuppTestBenchAlg : public AthAlgorithm {
public:
    /// Constructor
    PU1SuppTestBenchAlg(const std::string& name, ISvcLocator* pSvcLocator);

    /// Athena initialize hook
    virtual StatusCode initialize() override;

    /// Athena execute hook (called once per event)
    virtual StatusCode execute(const EventContext& ctx) override;

private:
    /// Path to input file containing 256-bit TOBs as 64-character hex strings
    Gaudi::Property<std::string> m_testsFileName{
      this, "TestsFileName", "", "Path to file containing 256-bit TOB hex strings"
    };

    /// Path to input file containing rho values, one per line
    Gaudi::Property<std::string> m_rhoFileName{
      this, "RhoFileName", "", "Path to file containing rho values"
    };

    /// Write handle key for TOB FIFO
    SG::WriteHandleKey<GepAlgoPU1SuppFIFO> m_suppFIFO_WriteKey{
      this, "SuppFIFOKey", "SuppFIFO", "Key to write FIFO of TOBs to SG"
    };

    /// Write handle key for dummy expectations
    SG::WriteHandleKey<PU1SuppExpectations> m_PU1SuppExpectations_WriteKey{
      this, "ExpectationsKey", "ExpectedTOBs", "Key to write dummy expectations (optional)"
    };

    /// Preloaded list of TOB FIFOs from file
    std::vector<std::unique_ptr<GepAlgoPU1SuppFIFO>> m_fifos;

    /// Index of next FIFO to write during execution
    std::size_t m_fifo_ptr{0};

    /// Populate `m_fifos` from file
    StatusCode init_from_file();

    /// Strip whitespace from both ends of a string
    std::string trim(const std::string& s);
};

} // namespace GlobalSim

#endif // GLOBALSIM_PU1SUPPTESTBENCHALG_H

