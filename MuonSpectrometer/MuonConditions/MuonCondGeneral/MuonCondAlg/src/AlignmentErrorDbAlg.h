/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCONDSVC_MUONALIGNMENTERRORDBALG_H
#define MUONCONDSVC_MUONALIGNMENTERRORDBALG_H

#include <GaudiKernel/EventIDRange.h>
#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonAlignmentData/MuonAlignmentErrorData.h"
#include <Identifier/Identifier.h>
#include <unordered_map>
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonIdHelpers/MuonIdHelper.h"
#include "MuonCalibITools/IIdToFixedIdTool.h"
/** @brief MuonAlignmentErrorDbAlg reads raw condition data and writes derived    
 *         condition data (MuonAlignmentErrorData) to the condition store
*/
namespace Muon{
class AlignmentErrorDbAlg : public AthCondAlgorithm {
public:

    using AthCondAlgorithm::AthCondAlgorithm;
    ~AlignmentErrorDbAlg() override = default;

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

private:
    std::tuple<std::string, EventIDRange> getDbClobContent(const EventContext& ctx) const;
    std::tuple<std::string, EventIDRange> getFileClobContent() const;

    SG::ReadCondHandleKey<CondAttrListCollection> m_readKey{this, "ReadKey", "/MUONALIGN/ERRS",
                                                            "Key of input muon alignment error condition data"};
    SG::WriteCondHandleKey<MuonAlignmentErrorData> m_writeKey{this, "WriteKey", "MuonAlignmentErrorData",
                                                              "Key of output muon alignment error condition data"};
    Gaudi::Property<std::string> m_clobFileOverride{this, "clobFileOverride", "",
        "Set this to the location of a CLOB file to override the DB setting"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    ToolHandle<MuonCalib::IIdToFixedIdTool> m_idTool{this, "idTool", "MuonCalib::IdToFixedIdTool"};
    // SOME USEFUL METHODS //
    // GET STATION EXACT NAME, FROM:
    // https://gitlab.cern.ch/Asap/AsapModules/Track/MuonAlignTrk/-/blob/master/MuonAlignTrk/MuonFixedLongId.h?ref_type=heads
    std::string hardwareName(MuonCalib::MuonFixedLongId calibId) const;
    std::string_view side(MuonCalib::MuonFixedLongId calibId) const;
    std::string sectorString(MuonCalib::MuonFixedLongId calibId) const;
    int sector(MuonCalib::MuonFixedLongId calibId) const;
    int hardwareEta(MuonCalib::MuonFixedLongId calibId) const;
    bool isSmallSector(MuonCalib::MuonFixedLongId calibId) const;
    void generateMap(const auto & helper_obj, const auto & idTool, MuonAlignmentErrorData::MuonAlignmentErrorRuleCache& adev_new, 
        std::vector<MuonAlignmentErrorData::MuonAlignmentErrorRule>& devVec) const;
};
}
#endif
