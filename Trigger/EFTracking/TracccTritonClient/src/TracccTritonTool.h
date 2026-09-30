/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TracccTritonTool_H
#define TracccTritonTool_H

#include "AthOnnxInterfaces/IAthInferenceTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "ITracccTritonTool.h"

/**
 * @class TracccTritonTool
 * @brief TracccTritonTool sends the cells of an event to a Triton server
 * running traccc and returns the serialized traccc EDM collections it
 * reconstructed.
 */

class TracccTritonTool : public extends<AthAlgTool, ITracccTritonTool> {
public:
    TracccTritonTool(const std::string& type, const std::string& name, const IInterface* parent);
    virtual StatusCode initialize() override;

    ///////////////////////////////////////////////////////////////////
    // Main methods for remote track finding asked by the ITracccTritonTool
    ///////////////////////////////////////////////////////////////////

    /**
     * @brief Reconstruct the tracks of one event on the server.
     * @param cellBytes serialized traccc silicon_cell_collection, forwarded to
     * the server as a single UINT8 tensor.
     * @param result the serialized collections the server returned
     */
    virtual StatusCode getTracks(
        std::vector<uint8_t>& cellBytes,
        TracccTritonResult& result) const override;


private:
    ToolHandle<AthInfer::IAthInferenceTool> m_TracccTritonTool{
        this, "TritonTool", "AthInfer::TritonTool"};

};

#endif  // TracccTritonTool_H
