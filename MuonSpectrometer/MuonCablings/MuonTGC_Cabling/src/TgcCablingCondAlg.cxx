/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcCablingCondAlg.h"

#include "AthenaKernel/IOVInfiniteRange.h"
#include "PathResolver/PathResolver.h"
#include "StoreGate/WriteCondHandle.h"

namespace Muon {
StatusCode TgcCablingCondAlg::initialize() {
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}
StatusCode TgcCablingCondAlg::execute(const EventContext& ctx) const {
    SG::WriteCondHandle writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle "
                      << writeHandle.fullKey() << " is already valid."
                      << ". In theory this should not be called, but may happen"
                      << " if multiple concurrent events are being processed "
                         "out of order.");
        return StatusCode::SUCCESS;
    }
    writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteRunLB()));

    ATH_CHECK(m_idHelperSvc.retrieve());

    auto findCalibFile = [this](const std::string& db,
                                std::string& fileName) -> StatusCode {
        fileName = PathResolver::find_file(db, "DATAPATH");
        if (fileName.empty() || !std::filesystem::exists(fileName)) {
            ATH_MSG_ERROR("Cannot resolve database '" << db << "'");
            return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    };

    Muon::TgcCablingMap::Config cfg{};
    cfg.idHelperSvc = m_idHelperSvc.get();
    cfg.AsideId = m_AsideId;
    cfg.CsideId = m_CsideId;

    if (m_isRun4) {
      ATH_MSG_DEBUG("TGC Cabling map is being prepared for Run 4+");
      ATH_CHECK(findCalibFile(std::string("R4_") + std::string(m_databaseASDToPP), cfg.fileNameASDtoPP));
      ATH_CHECK(findCalibFile(std::string("R4_") + std::string(m_databaseInPP), cfg.fileNameInPP));
      ATH_CHECK(findCalibFile(std::string("R4_") + std::string(m_databasePPToSL), cfg.fileNamePPtoSL));
      ATH_CHECK(findCalibFile(std::string("R4_") + std::string(m_databaseSLBToROD), cfg.fileNameSLBtoROD));
    } else {
      ATH_MSG_DEBUG("TGC Cabling map is being prepared for Run 1-3");
      ATH_CHECK(findCalibFile(m_databaseASDToPP, cfg.fileNameASDtoPP));
      ATH_CHECK(findCalibFile(m_databaseInPP, cfg.fileNameInPP));
      ATH_CHECK(findCalibFile(m_databasePPToSL, cfg.fileNamePPtoSL));
      ATH_CHECK(findCalibFile(m_databaseSLBToROD, cfg.fileNameSLBtoROD));
    }
    ATH_CHECK(findCalibFile(m_databaseASDToPP, cfg.fileNameASDtoPPdiff));

    // instantiate TGC cabling manager
    auto cabling = std::make_unique<Muon::TgcCablingMap>(cfg);
    ATH_CHECK(writeHandle.record(std::move(cabling)));
    return StatusCode::SUCCESS;
}

}  // namespace Muon
