/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigTauHypo_TrigTauPrecisionIDHypoTool_H
#define TrigTauHypo_TrigTauPrecisionIDHypoTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "xAODTau/TauJet.h"

#include "Gaudi/Parsers/Factory.h"

#include "ITrigTauJetHypoTool.h"


/**
 * @class TrigTauPrecisionIDHypoTool
 * @brief Precision step hypothesis tool for applying ID cuts (standard chains)
 **/
class TrigTauPrecisionIDHypoTool : public extends<AthAlgTool, ITrigTauJetHypoTool> {
public:
    TrigTauPrecisionIDHypoTool(const std::string& type, const std::string& name, const IInterface* parent);

    virtual StatusCode initialize() override;

    virtual StatusCode decide(std::vector<ITrigTauJetHypoTool::ToolInfo>& input) const override;
    virtual bool decide(const ITrigTauJetHypoTool::ToolInfo& i) const override;

private:
    enum IDMethod {
        Disabled = 0,
        RNN = 1,
        Decorator = 2
    };

    enum IDWP {
        None = 0,
        Standard = 1,
        HighPt = 2
    };

    HLT::Identifier m_decisionId;

    Gaudi::Property<float> m_ptMin {this, "PtMin", 0, "Tau pT minimum cut"};

    Gaudi::Property<int> m_numTrackMin {this, "NTracksMin", 0, "Minimum number of tracks"};
    Gaudi::Property<int> m_numTrackMax {this, "NTracksMax", 5, "Maximum number of tracks"};
    Gaudi::Property<int> m_numIsoTrackMax {this, "NIsoTracksMax", 999, "Maximum number of isolation tracks"};
    Gaudi::Property<float> m_trackPtCut {this, "TrackPtCut", -1, "Only count tracks above this pT threshold (override the 1 GeV cut in the InDetTrackSelectorTool)"};

    Gaudi::Property<int> m_idMethod {this, "IDMethod", IDMethod::Disabled, "ID WP evaluation method (0: Disabled, 1: RNN, 2: Decorator)"};
    Gaudi::Property<std::string> m_idWP {this, "IDWP", "", "Minimum ID Working Point decorated flag (e.g. 'GNTau_Medium', or 'medium' for built-in RNN WPs)"};

    // High pT Tau selection
    Gaudi::Property<float> m_highPtTrkThr {this, "HighPtSelectionTrkThr", 200000, "Tau pT threshold for disabling the NTrackMin and NIsoTrackMax cuts" };
    Gaudi::Property<float> m_highPtIdThr {this, "HighPtSelectionIDThr", 280000, "Tau pT threshold for switching to the high-pT ID WP cut"};
    Gaudi::Property<std::string> m_highPtIdWP {this, "HighPtIDWP", "", "High pT ID Working Point (e.g. 'GNTau_Loose', or 'loose' for built-in RNN WPs)"};
    Gaudi::Property<float> m_highPtJetThr {this, "HighPtSelectionJetThr", 440000, "Tau pT threshold for disabling IDWP and NTrackMax cuts"};

    Gaudi::Property<bool> m_acceptAll {this, "AcceptAll", false, "Ignore selection"};

    ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "", "Monitoring tool"};
    Gaudi::Property<std::map<std::string, std::pair<std::string, std::string>>> m_monitoredIdScores {this, "MonitoredIDScores", {}, "Pairs of the TauID score and signal-transformed scores for each TauID algorithm to be monitored"};
    std::map<std::string, std::pair<SG::ConstAccessor<float>, SG::ConstAccessor<float>>> m_monitoredIdAccessors;

    // WP accessors
    SG::ConstAccessor<char> m_id_wp_acc {"none"};
    SG::ConstAccessor<char> m_highpt_id_wp_acc {"none"};

    // Built-in RNN WPs
    unsigned int m_rnn_id_wp = 0;
    unsigned int m_rnn_highpt_id_wp = 0;
};

#endif
