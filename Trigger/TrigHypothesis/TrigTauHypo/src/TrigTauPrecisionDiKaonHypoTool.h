/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************
 *
 * NAME:     TrigTauPrecisionDiKaonHypoTool.h
 * PACKAGE:  Trigger/TrigHypothesis/TrigTauHypo
 *
 * AUTHOR:   J. Silva based on TrigTauPrecisionIDHypoTool
 * CREATED:  Feb. 17, 2021
 *
  *********************************************************************/

#ifndef TrigTauHypo_TrigTauPrecisionDiKaonHypoTool_H
#define TrigTauHypo_TrigTauPrecisionDiKaonHypoTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "ITrigTauJetHypoTool.h"


/**
 * @class TrigTauPrecisionDiKaonHypoTool
 * @brief Precision step hypothesis tool for applying meson kinematic cuts (meson chains)
 **/
class TrigTauPrecisionDiKaonHypoTool : public extends<AthAlgTool, ITrigTauJetHypoTool>
{
public:
    TrigTauPrecisionDiKaonHypoTool(const std::string& type, const std::string& name, const IInterface* parent);

    virtual StatusCode initialize() override;

    virtual StatusCode decide(std::vector<ITrigTauJetHypoTool::ToolInfo>& input) const override;
    virtual bool decide(const ITrigTauJetHypoTool::ToolInfo& i) const override;

private:
    HLT::Identifier m_decisionId;

    Gaudi::Property<float> m_ptMin {this, "PtMin", 0, "Minimum tau pT value"};
    Gaudi::Property<int> m_numTrackMax {this, "NTracksMax", 2, "Maximum number of Tracks"};
    Gaudi::Property<int> m_numTrackMin {this, "NTracksMin", 1, "Minimum number of Tracks"};
    Gaudi::Property<int> m_numIsoTrackMax {this, "NIsoTracksMax", 1, "Maximum number of wide Tracks"};

    // Track system mass cuts
    Gaudi::Property<float> m_massTrkSysMin {this, "massTrkSysMin", 0, "Minimum DiTrack mass value"};
    Gaudi::Property<float> m_massTrkSysMax {this, "massTrkSysMax", 1000000000, "Maximum DiTrack mass value"};

    Gaudi::Property<float> m_massTrkSysKaonMin {this, "massTrkSysKaonMin", 0, "Minimum DiKaon mass value"};
    Gaudi::Property<float> m_massTrkSysKaonMax {this, "massTrkSysKaonMax", 1000000000, "Maximum DiKaon mass value"};

    Gaudi::Property<float> m_massTrkSysKaonPiMin {this, "massTrkSysKaonPiMin", 0, "Minimum KaonPi mass value"};
    Gaudi::Property<float> m_massTrkSysKaonPiMax {this, "massTrkSysKaonPiMax", 1000000000, "Maximum KaonPi mass value"};
    Gaudi::Property<float> m_targetMassTrkSysKaonPi {this, "targetMassTrkSysKaonPi", 0, "Target KaonPi mass value (parameter)"};

    Gaudi::Property<float> m_leadTrkPtMin {this, "leadTrkPtMin", 0, "Minimum Pt of Lead Track"};
    Gaudi::Property<float> m_dRmaxMax {this, "dRmaxMax", 10, "Maximum dRmax value"};
    Gaudi::Property<float> m_etOverPtLeadTrkMax {this, "etOverPtLeadTrkMax", 10, "Maximum et/pt(lead track)"};
    Gaudi::Property<float> m_etOverPtLeadTrkMin {this, "etOverPtLeadTrkMin", 0, "Minimum et/pt(lead track)"};
    Gaudi::Property<float> m_EMPOverTrkSysPMax {this, "EMPOverTrkSysPMax", 5, "Maximum Cluster pt over ditrack pt"};

    Gaudi::Property<bool> m_acceptAll {this, "AcceptAll", false, "Ignore selection"};

    ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "", "Monitoring tool"};
};

#endif
