/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PU1SuppAlgTool.h
 * @brief Athena AlgTool for running the PU1 suppression logic on input TOBs.
 *
 * Reads a FIFO of TOBs and rho values from StoreGate, applies the suppression 
 * algorithm, and writes the filtered output TOBs back into the event store.
 */

#ifndef GLOBALSIM_PU1SUPPALGTOOL_H
#define GLOBALSIM_PU1SUPPALGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "../GlobalSimComponents/IGlobalSimAlgTool.h"

#include "PU1SuppPortsIn.h"
#include "PU1SuppPortsOut.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

namespace GlobalSim {

/**
 * @class PU1SuppAlgTool
 * @brief Implements the IGlobalSimAlgTool interface to run PU1 suppression.
 */
class PU1SuppAlgTool : public extends<AthAlgTool, IGlobalSimAlgTool> {
public:
    /// Constructor
    PU1SuppAlgTool(const std::string& type,
                   const std::string& name,
                   const IInterface* parent);
  
    /// Destructor
    virtual ~PU1SuppAlgTool() = default;
  
    /// Initialization of keys and setup
    virtual StatusCode initialize() override;
  
    /// Run suppression on input TOBs and write output TOBs
    virtual StatusCode run(const EventContext& ctx) const override;
  
    /// Returns a string representation for diagnostics
    virtual std::string toString() const override;
  
private:
    /// Key to read TOB FIFO from the event store
    SG::ReadHandleKey<GepAlgoPU1SuppFIFO> m_HypoFIFOReadKey{
      this, "HypoFIFOReadKey", "SuppFIFO", "Input key for PU1 TOBs"
    };
  
    /// Key to write suppressed TOBs to the event store
    SG::WriteHandleKey<GepAlgoPU1SuppPortsOutFIFO> m_portsOutWriteKey{
      this, "PortsOutWriteKey", "PU1SuppOut", "Output key for suppressed TOBs"
    };
};

} // namespace GlobalSim

#endif // GLOBALSIM_PU1SUPPALGTOOL_H

