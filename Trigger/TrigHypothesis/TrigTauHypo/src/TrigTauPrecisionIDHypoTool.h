/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigTauHypo_TrigTauPrecisionIDHypoTool_H
#define TrigTauHypo_TrigTauPrecisionIDHypoTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "Gaudi/Parsers/Factory.h"

#include "ITrigTauPrecisionHypoTool.h"


/**
 * @class TrigTauPrecisionIDHypoTool
 * @brief Precision step hypothesis tool for applying ID cuts (standard chains)
 **/
class TrigTauPrecisionIDHypoTool : public extends<AthAlgTool, ITrigTauPrecisionHypoTool> {
public:
    TrigTauPrecisionIDHypoTool(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~TrigTauPrecisionIDHypoTool();

    virtual StatusCode initialize() override;

    virtual StatusCode decide(std::vector<ITrigTauPrecisionHypoTool::ToolInfo>& input) const override;
    virtual bool decide(const ITrigTauPrecisionHypoTool::ToolInfo& i) const override;

private:
    enum IDMethod {
        Disabled = 0,
        RNN = 1,
        Decorator = 2
    };

    enum IDWP {
        None = -1,
        VeryLoose = 0,
        Loose = 1,
        Medium = 2,
        Tight = 3
    };

    HLT::Identifier m_decisionId;

    Gaudi::Property<float> m_ptMin {this, "PtMin", 0, "Tau pT minimum cut"};

    Gaudi::Property<int> m_numTrackMin {this, "NTracksMin", 0, "Minimum number of tracks"};
    Gaudi::Property<int> m_numTrackMax {this, "NTracksMax", 5, "Maximum number of tracks"};
    Gaudi::Property<int> m_numIsoTrackMax {this, "NIsoTracksMax", 999, "Maximum number of isolation tracks"};
    Gaudi::Property<float> m_trackPtCut {this, "TrackPtCut", -1, "Only count tracks above this pT threshold (override the 1 GeV cut in the InDetTrackSelectorTool)"};

    Gaudi::Property<int> m_idMethod {this, "IDMethod", IDMethod::Disabled, "ID WP evaluation method (0: Disabled, 1: RNN, 2: Decorator)"};
    Gaudi::Property<int> m_idWP {this, "IDWP", IDWP::None, "Minimum ID Working Point (-1: None, 0: VeryLoose, 1: Loose, 2: Medium, 3: Tight)"};
    Gaudi::Property<std::vector<std::string>> m_idWPNames {this, "IDWPNames", {}, "ID WP decorated variable names; use with IDMethod=2"};

    // High pT Tau selection
    Gaudi::Property<bool> m_doHighPtSelection {this, "DoHighPtSelection", true , "Turn on/off high pT Tau selection"};
    Gaudi::Property<float> m_highPtTrkThr {this, "HighPtSelectionTrkThr", 200000, "Tau pT threshold for disabling the NTrackMin and NIsoTrackMax cuts" };
    Gaudi::Property<float> m_highPtLooseIDThr {this, "HighPtSelectionLooseIDThr", 280000, "Tau pT threshold for loosening the IDWP cut to Loose (IDWP=1)"};
    Gaudi::Property<float> m_highPtJetThr {this, "HighPtSelectionJetThr", 440000, "Tau pT threshold for disabling IDWP and NTrackMax cuts"};

    Gaudi::Property<bool> m_acceptAll {this, "AcceptAll", false, "Ignore selection"};

    ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "", "Monitoring tool"};
    Gaudi::Property<std::map<std::string, std::pair<std::string, std::string>>> m_monitoredIdScores {this, "MonitoredIDScores", {}, "Pairs of the TauID score and signal-transformed scores for each TauID algorithm to be monitored"};
    std::map<std::string, std::pair<SG::ConstAccessor<float>, SG::ConstAccessor<float>>> m_monitoredIdAccessors;
};

#endif
