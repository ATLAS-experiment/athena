/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEMONITORING_TILEJETMONITORALGORITHM
#define TILEMONITORING_TILEJETMONITORALGORITHM

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "TileConditions/ITileBadChanTool.h"

#include "StoreGate/ReadHandleKey.h"
#include "xAODJet/JetContainer.h"
#include "CaloEvent/CaloCellContainer.h"

// JVT
#include "JetInterface/IJetUpdateJvt.h"
// Jet cleaning
#include "JetInterface/IJetSelector.h"
// Event cleaning
#include "JetSelectorTools/IEventCleaningTool.h"


class TileCablingService;
class TileID;
class TileHWID;

/** @class TileJetMonitorAlgorithm
 *  @brief Class for Tile Jet based monitoring
 */

class TileJetMonitorAlgorithm : public AthMonitorAlgorithm {

  public:

    TileJetMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
    virtual ~TileJetMonitorAlgorithm();
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

  private:

    StatusCode fillTimeHistograms(const xAOD::Jet& jet, uint32_t lumiBlock, std::set<Identifier>& usedCells) const;
    unsigned int findIndex(const int gain, const float energy) const;
    bool isGoodChannel(int part, int module, int channel, uint32_t bad, unsigned int qbit, Identifier id) const;
    bool passesJvt(const xAOD::Jet& jet) const;
    bool isGoodJet(const xAOD::Jet& jet) const;
    bool isGoodEvent(const EventContext& ctx) const;
    std::string sampleName(const int ros, const int sample, const int tower) const;
    std::string cellName(const int ros, const int sample, const int tower, const int module) const;
    bool matchesEnergyRange(const int sample, const int tower, const float energy, const int gain) const;
  
    Gaudi::Property<float> m_jetPtMin{this, "JetPtMin", 20000., "Threshold in MeV"};
    Gaudi::Property<float> m_jetPtMax{this, "JetPtMax", 120000, ""};
    Gaudi::Property<float> m_jetEtaMax{this, "JetEtaMax", 1.6, ""};
    Gaudi::Property<float> m_energyChanMin{this, "ChannelEnergyMin", 2000, ""};
    Gaudi::Property<float> m_energyChanMax{this, "ChannelEnergyMax", 4000, ""};
    Gaudi::Property<float> m_gain{this, "Gain", 1, ""};
    Gaudi::Property<float> m_energyE1Min{this, "E1EnergyMin", -1, ""};
    Gaudi::Property<float> m_energyE1Max{this, "E1EnergyMax", -1, ""};
    Gaudi::Property<float> m_gainE1{this, "GainE1", -1, ""};
    Gaudi::Property<float> m_energyE2Min{this, "E2EnergyMin", -1, ""};
    Gaudi::Property<float> m_energyE2Max{this, "E2EnergyMax", -1, ""};
    Gaudi::Property<float> m_gainE2{this, "GainE2", -1, ""};
    Gaudi::Property<float> m_energyE3Min{this, "E3EnergyMin", -1, ""};
    Gaudi::Property<float> m_energyE3Max{this, "E3EnergyMax", -1, ""};
    Gaudi::Property<float> m_gainE3{this, "GainE3", -1, ""};
    Gaudi::Property<float> m_energyE4Min{this, "E4EnergyMin", -1, ""};
    Gaudi::Property<float> m_energyE4Max{this, "E4EnergyMax", -1, ""};
    Gaudi::Property<float> m_gainE4{this, "GainE4", -1, ""};
  //  
    Gaudi::Property<bool> m_do1DHistograms{this, "Do1DHistograms", false, ""};
    Gaudi::Property<bool> m_do2DHistograms{this, "Do2DHistograms", false, ""};
    Gaudi::Property<bool> m_doCellHistograms{this, "DoCellHistograms", false, ""};
    Gaudi::Property<bool> m_doEnergyDiffHistograms{this, "DoEnergyDiffHistograms", false, ""};
    Gaudi::Property<float> m_energyDiffThreshold{this, "EnergyDiffThreshold", 2000, ""};
    Gaudi::Property<bool> m_doEnergyProfiles{this, "DoEnergyProfiles", true, ""};
    Gaudi::Property<bool> m_doEventCleaning{this, "DoEventCleaning", true, ""};
    Gaudi::Property<bool> m_doJetCleaning{this, "DoJetCleaning", false, ""};
    Gaudi::Property<float> m_jetTrackingEtaLimit{this, "JetTrackingEtaLimit", 2.4, ""};
    Gaudi::Property<float> m_jvtThreshold{this, "JvtThreshold", 0.59, ""};
    Gaudi::Property<std::vector<float>> m_cellEnergyUpperLimitsHG{this, 
        "CellEnergyUpperLimitsHG", {}, "Energy upper limits of the HG cell-time histograms"};
    Gaudi::Property<std::vector<float>> m_cellEnergyUpperLimitsLG{this,
        "CellEnergyUpperLimitsLG", {}, "Energy upper limits of the LG cell-time histograms"};

    ToolHandle<ITileBadChanTool> m_tileBadChanTool{this, 
        "TileBadChanTool", "TileBadChanTool", "Tile bad channel tool"};

    // JVT
    ToolHandle<IJetUpdateJvt> m_jvt{this, "JVT", "", ""};

    // event/jet cleaning
    ToolHandle<IJetSelector> m_jetCleaningTool{this, "JetCleaningTool", "", ""};
    ToolHandle<ECUtils::IEventCleaningTool> m_eventCleaningTool{this, "EventCleaningTool", "", ""};


    SG::ReadHandleKey<xAOD::JetContainer> m_jetContainerKey{this,
        "JetContainer", "AntiKt4EMTopoJets", "Jet container for monitoring"};

    SG::ReadHandleKey<CaloCellContainer> m_caloCellContainerKey{this,
        "CaloCellContainer", "AllCalo", "Calo cell container name"};

   const TileID* m_tileID;
   const TileHWID* m_tileHWID;

   const TileCablingService* m_cabling; //!< TileCabling instance

};

#endif // TILEMONITORING_TILEJETMONITORALGORITHM
