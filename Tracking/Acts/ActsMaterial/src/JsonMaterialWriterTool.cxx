/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "JsonMaterialWriterTool.h"
#include "ActsInterop/Logger.h"
#include "ActsPlugins/Json/MaterialMapJsonConverter.hpp"

#include <fstream>
#include <iomanip>
#include <ios>
#include <vector>

#include <nlohmann/json.hpp>

void ActsTrk::JsonMaterialWriterTool::writeMaterial(const ActsTrk::GeometryContext& gctx,
                                                     const Acts::TrackingGeometryMaterial& detMaterial) const
{
    // Set up the Athena-backed Acts logger, and take its level for the converter
    std::unique_ptr<const Acts::Logger> logger = makeActsAthenaLogger(this, "JsonMaterialWriterTool");

    // Configure the Json converter
    Acts::MaterialMapJsonConverter::Config converterCfg;
    converterCfg.context = gctx.context();

    Acts::MaterialMapJsonConverter converter(converterCfg, logger->level());

    // Convert the surface and volume material maps to Json
    nlohmann::json jOut = converter.materialMapsToJson(detMaterial);

    if (m_writeJson.value()) {
        const std::string fileName = m_fileName.value() + ".json";
        ATH_MSG_INFO("Writing Json material map to '" << fileName << "'");
        std::ofstream ofj(fileName);
        ofj << std::setw(4) << jOut << std::endl;
    }

    if (m_writeCbor.value()) {
        const std::vector<std::uint8_t> cborOut = nlohmann::json::to_cbor(jOut);
        const std::string fileName = m_fileName.value() + ".cbor";
        ATH_MSG_INFO("Writing Cbor material map to '" << fileName << "'");
        std::ofstream ofj(fileName, std::ios::out | std::ios::binary);
        ofj.write(reinterpret_cast<const char*>(cborOut.data()), cborOut.size());
    }
}
