/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_MDTCONDDBALG_H
#define MUONCONDALG_MDTCONDDBALG_H


// Athena includes
#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCondData/MdtCondDbData.h"
#include "CxxUtils/StringUtils.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"


class MdtCondDbAlg : public AthCondAlgorithm {
public:
    MdtCondDbAlg(const std::string& name, ISvcLocator* svc);
    virtual ~MdtCondDbAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext&) const override;

private:
    using writeHandle_t = SG::WriteCondHandle<MdtCondDbData>;
    using dataBaseKey_t = SG::ReadCondHandleKey<CondAttrListCollection>;

    StatusCode loadDependencies(const EventContext& ctx, writeHandle_t& wh) const;
    StatusCode addDHDependency(const EventContext& ctx, const dataBaseKey_t& key,  writeHandle_t& wh) const;
    
    StatusCode loadDataPsHv(const EventContext& ctx, MdtCondDbData& dataOut) const;
    StatusCode loadDataPsLv(const EventContext& ctx, MdtCondDbData& dataOut) const;
    StatusCode loadDataHv(const EventContext& ctx, MdtCondDbData& dataOut) const;
    StatusCode loadDataLv(const EventContext& ctx, MdtCondDbData& dataOut) const;
    StatusCode loadDroppedChambers(const EventContext& ctx, MdtCondDbData& dataOut, bool isMC) const;
    StatusCode loadMcDeadElements(const EventContext& ctx, MdtCondDbData& dataOut) const;
    StatusCode loadMcDeadTubes(const EventContext& ctx, MdtCondDbData& dataOut) const;
    StatusCode loadMcNoisyChannels(const EventContext& ctx, MdtCondDbData& dataOut) const;

    Gaudi::Property<bool> m_isOnline{this, "isOnline", false};
    Gaudi::Property<bool> m_isData{this, "isData", false};
    Gaudi::Property<bool> m_isRun1{this, "isRun1", false};
    Gaudi::Property<bool> m_checkOnSetPoint{this, "useRun1SetPoints", false};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    SG::WriteCondHandleKey<MdtCondDbData> m_writeKey{this, "WriteKey", "MdtCondDbData", "Key of output MDT condition data"};

    dataBaseKey_t m_readKey_folder_da_pshv{this, "ReadKey_DataR1_HV", "/MDT/DCS/PSHVMLSTATE",
                                                                           "Key of input MDT condition data for Run 1 data HV"};
    dataBaseKey_t m_readKey_folder_da_psv0{this, "ReadKey_DataR1_V0", "/MDT/DCS/PSV0SETPOINTS",
                                                                           "Key of input MDT condition data for Run 1 data V0"};
    dataBaseKey_t m_readKey_folder_da_psv1{this, "ReadKey_DataR1_V1", "/MDT/DCS/PSV1SETPOINTS",
                                                                           "Key of input MDT condition data for Run 1 data V1"};
    dataBaseKey_t m_readKey_folder_da_pslv{this, "ReadKey_DataR1_LV", "/MDT/DCS/PSLVCHSTATE",
                                                                           "Key of input MDT condition data for Run 1 data LV"};
    dataBaseKey_t m_readKey_folder_da_droppedChambers{
        this, "ReadKey_DataR1_DC", "/MDT/DCS/DROPPEDCH", "Key of input MDT condition data for Run 1 data dropped chambers"};
    dataBaseKey_t m_readKey_folder_da_hv{this, "ReadKey_DataR2_HV", "/MDT/DCS/HV",
                                                                         "Key of input MDT condition data for Run 2 data HV"};
    dataBaseKey_t m_readKey_folder_da_lv{this, "ReadKey_DataR2_LV", "/MDT/DCS/LV",
                                                                         "Key of input MDT condition data for Run 2 data LV"};
    dataBaseKey_t m_readKey_folder_mc_droppedChambers{
        this, "ReadKey_MC_DC", "/MDT/DCS/DROPPEDCH", "Key of input MDT condition data for MC dropped chambers"};
    dataBaseKey_t m_readKey_folder_mc_deadElements{this, "ReadKey_MC_DE", "/MDT/DQMF/DEAD_ELEMENT",
                                                                                   "Key of input MDT condition data for MC dead elements"};
    dataBaseKey_t m_readKey_folder_mc_deadTubes{this, "ReadKey_MC_DT", "/MDT/TUBE_STATUS/DEAD_TUBE",
                                                                                "Key of input MDT condition data for MC dead tubes"};
    dataBaseKey_t m_readKey_folder_mc_noisyChannels{
        this, "ReadKey_MC_NC", "/MDT/DCS/PSLVCHSTATE", "Key of input MDT condition data for MC noisy channels"};

    Identifier identifyChamber(std::string chamber) const;

    std::map<std::string, Identifier> m_chamberNames{};
};

#endif
