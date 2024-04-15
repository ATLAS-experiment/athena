/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGTAUMONITORING_TRIGTAUINFO_H
#define TRIGTAUMONITORING_TRIGTAUINFO_H

#include <string>
#include <regex>
#include <map>
#include <iostream>

#include <boost/algorithm/string.hpp>

class TrigTauInfo {
public:
    TrigTauInfo() {} // Required for the dictionary
    TrigTauInfo(const std::string& trigger);
    TrigTauInfo(const std::string& trigger, const std::map<std::string, float>& L1Phase1_thresholds);
    TrigTauInfo(const std::string& trigger, const std::map<std::string, float>& L1Phase1_thresholds, const std::map<std::string, uint64_t>& L1Phase1_threshold_patterns);
    TrigTauInfo(const std::string& trigger, const std::map<int, int>& L1Phase1ThrMap_eTAU, const std::map<int, int>& L1Phase1ThrMap_jTAU);

    inline const std::string& getTriggerName() const { return m_trigger; }
    inline bool isStreamer() const { return m_isStreamer; }

    inline float getHLTTauThreshold() const { return m_HLTThr.size() ? m_HLTThr[0] : -1; } // Returns the main HLT threshold
    inline const std::vector<float>& getHLTTauThresholds() const { return m_HLTThr; }
    inline const std::string getHLTTauType() const { return m_HLTTauTypes.size() ? m_HLTTauTypes[0] : ""; } // Returns the main HLT tau leg type
    inline const std::vector<std::string>& getHLTTauTypes() const { return m_HLTTauTypes; }

    inline const std::string& getTriggerL1Name() const { return m_L1Item; }
    inline const std::vector<std::string>& getTriggerL1Items() const { return m_L1Items; }
    inline const std::string getL1TauItem() const { return m_tauL1Items.size() ? m_tauL1Items[0] : ""; } // Returns the main L1 tau item
    inline const std::vector<std::string>& getL1TauItems() const { return m_tauL1Items; }
    inline float getL1TauThreshold() const { return m_tauL1Thr.size() ? m_tauL1Thr[0] : -1; } // Returns the main L1 item threshold
    inline const std::vector<float>& getL1TauThresholds() const { return m_tauL1Thr; }
    inline const std::string getL1TauType() const { return m_tauL1Items.size() ? m_tauL1Type[0] : ""; } // Returns the main L1 item type
    inline const std::vector<std::string>& getL1TauTypes() const { return m_tauL1Type; }
    inline const std::string getL1TauIsolation() const { return m_tauL1Iso.size() ? m_tauL1Iso[0] : ""; } // Returns the main L1 item isolation
    inline const std::vector<std::string>& getL1TauIsolations() const { return m_tauL1Iso; }
    inline bool isL1TauIsolated(const size_t idx = 0) const { return idx < m_tauL1Iso.size() ? m_tauL1Iso[idx].size() : false; } // Returns true if the (main by default) L1 item is isolated
    inline bool areAnyL1TauIsolated() const { for(const std::string& iso : m_tauL1Iso) { if(iso.size()) return true; }; return false; } // Returns true any of the L1 items are isolated
    inline int64_t getL1TauThresholdPattern() const { return m_tauL1ThresholdPattern.size() ? m_tauL1ThresholdPattern[0] : -1; } // Returns the main L1 item thresholdPattern mask
    inline const std::vector<int64_t>& getL1TauThresholdPatterns() const { return m_tauL1ThresholdPattern; }

    inline bool isHLTSingleTau() const { return m_HLTThr.size() == 1 && (m_HLTElecThr.size() + m_HLTMuonThr.size() + m_HLTGammaThr.size() + m_HLTJetThr.size() + m_HLTMETThr.size()) == 0; }
    inline bool isHLTDiTau() const { return m_HLTThr.size() > 1 && (m_HLTElecThr.size() + m_HLTMuonThr.size() + m_HLTGammaThr.size() + m_HLTJetThr.size() + m_HLTMETThr.size()) == 0; }
    inline bool isHLTTandP() const { return m_HLTThr.size() == 1 && (m_HLTElecThr.size() + m_HLTMuonThr.size()) == 1 && (m_HLTGammaThr.size() + m_HLTJetThr.size() + m_HLTMETThr.size()) == 0; }
    inline bool isL1TauOnly() const { return (m_HLTThr.size() + m_HLTElecThr.size() + m_HLTMuonThr.size() + m_HLTGammaThr.size() + m_HLTJetThr.size() + m_HLTMETThr.size()) == 0 && !m_tauL1Items.empty(); }
    inline bool isTauStreamer() const { return m_isStreamer && !m_tauL1Items.empty(); }

    inline bool hasHLTElectronLeg() const { return m_HLTElecThr.size() >= 1; }
    inline bool hasHLTMuonLeg() const { return m_HLTMuonThr.size() >= 1; }
    inline bool hasHLTGammaLeg() const { return m_HLTGammaThr.size() >= 1; }
    inline bool hasHLTJetLeg() const { return m_HLTJetThr.size() >= 1; }
    inline bool hasHLTMETLeg() const { return m_HLTMETThr.size() >= 1; }

    inline float getHLTElecThreshold() const { return m_HLTElecThr.size() ? m_HLTElecThr[0] : -1; } // Returns the main leg threshold
    inline const std::vector<float>& getHLTElecThresholds() const { return m_HLTElecThr; }
    inline float getHLTMuonThreshold() const { return m_HLTMuonThr.size() ? m_HLTMuonThr[0] : -1; } // Returns the main leg threshold
    inline const std::vector<float>& getHLTMuonThresholds() const { return m_HLTMuonThr; }
    inline float getHLTGammaThreshold() const { return m_HLTGammaThr.size() ? m_HLTGammaThr[0] : -1; } // Returns the main leg threshold
    inline const std::vector<float>& getHLTGammaThresholds() const { return m_HLTGammaThr; }
    inline float getHLTJetThreshold() const { return m_HLTJetThr.size() ? m_HLTJetThr[0] : -1; } // Returns the main leg threshold
    inline const std::vector<float>& getHLTJetThresholds() const { return m_HLTJetThr; }
    inline float getHLTMETThreshold() const { return m_HLTMETThr.size() ? m_HLTMETThr[0] : -1; } // Returns the main leg threshold
    inline const std::vector<float>& getHLTMETThresholds() const { return m_HLTMETThr; }

private:
    std::string m_trigger; // Full trigger name (e.g. HLT_tau25_mediumRNN_tracktwoMVA_L1eTAU20)
    bool m_isStreamer; // Is a streamer HLT trigger
    std::vector<float> m_HLTThr; // List of all tau thresholds
    std::vector<std::string> m_HLTTauTypes; // Type for each tau leg (e.g. tracktwoMVA, trackwoLLP, etc...)

    std::string m_L1Item; // full L1 trigger string (e.g. L1eTAU20, or L1eTAU80_2eTAU60)
    std::vector<std::string> m_L1Items; // full L1 trigger items
    std::vector<float> m_tauL1Thr; // L1 Tau item thresholds, corrected for Phase1 items
    std::vector<std::string> m_tauL1Items; // Individual l1 tau items
    std::vector<std::string> m_tauL1Type; // Individual l1 tau item type (eTAU, jTAU, cTAU, TAU)
    std::vector<std::string> m_tauL1Iso; // Individual l1 tau item isolation ("", L, M, T, HL, HM, HT, H, IM)
    std::vector<int64_t> m_tauL1ThresholdPattern; // Individual l1 tau item thresholdPattern mask

    std::vector<float> m_HLTElecThr; // List of all electron leg thresholds
    std::vector<float> m_HLTMuonThr; // List of all muon leg thresholds
    std::vector<float> m_HLTGammaThr; // List of all photon leg thresholds
    std::vector<float> m_HLTJetThr; // List of all jet leg thresholds
    std::vector<float> m_HLTMETThr; // List of all MET leg thresholds

    inline bool is_number(const std::string& s) {
        return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) {return !std::isdigit(c);}) == s.end();
    }

    void parseTriggerString(bool remove_L1_phase1_thresholds = true); // Parse the trigger string, without applying the Phase1 remapping (L1 thresholds will be set to -1 on Phase1 items)
    void parseTriggerString(const std::map<std::string, float>& L1Phase1_thresholds); // Parse the trigger string, applying the Phase1 ET mapping
    void parseTriggerString(const std::map<std::string, float>& L1Phase1_thresholds, const std::map<std::string, uint64_t>& L1Phase1_threshold_patterns); // Parse the trigger string, applying the Phase1 threshold mapping
    void parseTriggerString(const std::map<int, int>& L1Phase1ThrMap_eTAU, const std::map<int, int>& L1Phase1ThrMap_jTAU); // Parse the trigger string, applying the Phase1 remapping
};

#endif
