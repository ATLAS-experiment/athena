/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGEFTAUMVHYPOTOOL_H
#define TRIGEFTAUMVHYPOTOOL_H

#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "ITrigEFTauMVHypoTool.h"


class TrigEFTauMVHypoTool : public extends<AthAlgTool, ITrigEFTauMVHypoTool> {
public:
    TrigEFTauMVHypoTool(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~TrigEFTauMVHypoTool();

    virtual StatusCode initialize() override;

    virtual StatusCode decide(std::vector<ITrigEFTauMVHypoTool::TauJetInfo>& input) const override;
    virtual bool decide(const ITrigEFTauMVHypoTool::TauJetInfo& i) const override;

private:
    enum IDMethod {
        Disabled = 0,
        RNN = 1
    };

    enum IDWP {
        None = -1,
        VeryLoose = 0,
        Loose = 1,
        Medium = 2,
        Tight = 3
    };

    HLT::Identifier m_decisionId;

    Gaudi::Property<float> m_ptMin {this, "PtMin", -10000, "Tau pT minimum cut"};

    Gaudi::Property<int> m_numTrackMin {this, "NTrackMin", 0, "Minimum number of tracks"};
    Gaudi::Property<int> m_numTrackMax {this, "NTrackMax", 5, "Maximum number of tracks"};
    Gaudi::Property<int> m_numWideTrackMax {this, "NWideTrackMax", 999, "Maximum number of wide tracks"};
    Gaudi::Property<float> m_trackPtCut {this, "TrackPtCut", -1, "Only count tracks above this pT threshold (override the 1 GeV cut in the InDetTrackSelectorTool)"};

    Gaudi::Property<int> m_idMethod {this, "IDMethod", IDMethod::Disabled, "ID WP evaluation method (0: Disabled, 1: RNN)"};
    Gaudi::Property<int> m_idWP {this, "IDWP", IDWP::None, "Minimum ID Working Point (-1: None, 0: VeryLoose, 1: Loose, 2: Medium, 3: Tight)"};

    // High pT Tau selection
    Gaudi::Property<bool> m_doHighPtSelection {this, "DoHighPtSelection", true , "Turn on/off high pT Tau selection"};
    Gaudi::Property<float> m_highPtTrkThr {this, "HighPtSelectionTrkThr", 200000, "Tau pT threshold for disabling the NTrackMin and NWideTrackMax cuts" };
    Gaudi::Property<float> m_highPtLooseIDThr {this, "HighPtSelectionLooseIDThr", 280000, "Tau pT threshold for loosening the IDWP cut to Loose (IDWP=1)"};
    Gaudi::Property<float> m_highPtJetThr {this, "HighPtSelectionJetThr", 440000, "Tau pT threshold for disabling IDWP and NTrackMax cuts"};

    Gaudi::Property<bool> m_acceptAll {this, "AcceptAll", false, "Ignore selection"};

    ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "", "Monitoring tool"};
};

#endif
