/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigTauMonitoring/TrigTauInfo.h"

TrigTauInfo::TrigTauInfo(const std::string& trigger)
    : m_trigger{trigger}
{
    parseTriggerString();
}

TrigTauInfo::TrigTauInfo(const std::string& trigger, const std::map<int, int>& L1Phase1ThrMap_eTAU, const std::map<int, int>& L1Phase1ThrMap_jTAU)
    : m_trigger{trigger}
{
    parseTriggerString(L1Phase1ThrMap_eTAU, L1Phase1ThrMap_jTAU);
}

TrigTauInfo::TrigTauInfo(const std::string& trigger, const std::map<std::string, float>& L1Phase1_thresholds, const std::map<std::string, uint64_t>& L1Phase1_threshold_patterns)
    : m_trigger{trigger}
{
    parseTriggerString(L1Phase1_thresholds, L1Phase1_threshold_patterns);
}

void TrigTauInfo::parseTriggerString(bool remove_L1_phase1_thresholds)
{
    std::string clean_trigger = m_trigger;
    
    // Change the "L1_" prefix to "L1" internally, in case the trigger being parsed is a pure L1 trigger with the usual L1 standalone naming scheme
    if(clean_trigger.size() > 3 && clean_trigger.rfind("L1_", 0) == 0) {
        clean_trigger = "L1" + clean_trigger.substr(3);
    }

    std::vector<std::string> sections;
    boost::split(sections, m_trigger, boost::is_any_of("_"));

    std::regex tau_rgx("^(\\d*)tau(\\d+)$");
    std::regex elec_rgx("^(\\d*)e(\\d+)$");
    std::regex muon_rgx("^(\\d*)mu(\\d+)$");
    std::regex gamma_rgx("^(\\d*)g(\\d+)$");
    std::regex jet_rgx("^(\\d*)j(\\d+)$");
    std::regex met_rgx("^xe(\\d+)$");
    std::regex l1_rgx("^L1.*$");
    std::regex l1_tau_rgx("(\\d*)(e|j|c|)TAU(\\d+)(L|M|T|HM|H|IM|I|)");
    std::regex topo_rgx("^.*(invm|dR|deta|dphi)AB.*$");
    std::vector<std::regex*> all_regexes = {&tau_rgx, &elec_rgx, &muon_rgx, &gamma_rgx, &jet_rgx, &met_rgx, &l1_rgx};

    std::regex tau_type_rgx("^(ptonly|tracktwoMVA|tracktwoMVABDT|tracktwoLLP|trackLRT)$");

    std::smatch match;
    std::regex_token_iterator<std::string::iterator> rend;

    // Check each leg
    std::vector<std::string> leg;
    for(size_t i = 0; i < sections.size(); i++) {
        leg.push_back(sections[i]); // Attach to the current leg

        //Match the beginning of a new leg, or the end of the chain
        if(i == sections.size() - 1 || (std::any_of(all_regexes.begin(), all_regexes.end(), [&sections, i](const std::regex* rgx) { return std::regex_match(sections[i+1], *rgx); }))) {
            // Process the previous leg, which starts with the item, multiplicity, and threshold
            if(std::regex_match(leg[0], match, tau_rgx)) {
                size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
                unsigned int threshold = std::stoi(match[2].str());
                auto itr = find_if(leg.begin(), leg.end(), [tau_type_rgx](const std::string& s) { return std::regex_match(s, tau_type_rgx); });
                std::string type = itr != leg.end() ? *itr : "";

                for(size_t j = 0; j < multiplicity; j++) {
                    m_HLTThr.push_back(threshold);
                    m_HLTTauTypes.push_back(type);
                }
            } else if(std::regex_match(leg[0], match, elec_rgx)) {
                size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
                unsigned int threshold = std::stoi(match[2].str());
                for(size_t j = 0; j < multiplicity; j++) m_HLTElecThr.push_back(threshold);
            } else if(std::regex_match(leg[0], match, muon_rgx)) {
                size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
                unsigned int threshold = std::stoi(match[2].str());
                for(size_t j = 0; j < multiplicity; j++) m_HLTMuonThr.push_back(threshold);
            } else if(std::regex_match(leg[0], match, gamma_rgx)) {
                size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
                unsigned int threshold = std::stoi(match[2].str());
                for(size_t j = 0; j < multiplicity; j++) m_HLTGammaThr.push_back(threshold);
            } else if(std::regex_match(leg[0], match, jet_rgx)) {
                size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
                unsigned int threshold = std::stoi(match[2].str());
                for(size_t j = 0; j < multiplicity; j++) m_HLTJetThr.push_back(threshold);
            } else if(std::regex_match(leg[0], match, met_rgx)) {
                unsigned int threshold = std::stoi(match[2].str());
                m_HLTMETThr.push_back(threshold);
            } else if(std::regex_match(leg[0], l1_rgx)) { // Treat the L1 items as a leg
                for(size_t j = 0; j < leg.size(); j++) {
                    if(std::regex_match(leg[j], topo_rgx)) continue; // Remove HLT topo sections, not part of the L1 item

                    // L1Topo items (they all include a "-" in the name):
                    if(leg[j].find("-") != std::string::npos) {
                        // We only keep information from the legacy L1Topo item, from which we will not always use all thresholds
                        // Since we won't be adding any more Legacy thresholds, let's hard-code it...
                        if(leg[0] == "L1TAU60" && leg[j] == "DR-TAU12ITAU12I") leg[j] = "TAU12IM"; // L1_TAU60_DR-TAU20ITAU12I, uses "TAU12IM" threshold from the L1Topo item
                        else if(leg.size() == 1 && (leg[0] == "L1DR-TAU20ITAU12I" || leg[0] == "L1DR-TAU20ITAU12I-J25")) {
                            // Uses both TAU items, in the M isolation threshold
                            leg[0] = "L1TAU20IM";
                            leg.push_back("TAU12IM");
                            // Even on combined chains using jets, we don't use the jets threshold
                        }
                        else continue; // Remove the Phase 1 L1Topo items, since we always use a multiplicity threshold
                    }

                    m_L1Items.push_back(j == 0 ? leg[j].substr(2, leg[j].size()) : leg[j]); // Remove the "L1" prefix on the first L1 item
                }
            }

            // Start a new leg
            leg = {};
        }
    }

    if(!m_L1Items.empty()) {
        // Build the full L1 string
        m_L1Item = m_L1Items[0];
        for(size_t j = 1; j < m_L1Items.size(); j++) m_L1Item += "_" + m_L1Items[j];

        // Get all individual L1 TAU items
        std::regex_token_iterator<std::string::iterator> rgx_iter(m_L1Item.begin(), m_L1Item.end(), l1_tau_rgx);
        while(rgx_iter != rend) {
            std::string s = *rgx_iter;
            std::regex_match(s, match, l1_tau_rgx);
            size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
            std::string item_type = match[2].str(); // e, j, c, or ""
            int threshold = std::stoi(match[3].str());
            
            // Set the Phase 1 thresholds to -1
            if(remove_L1_phase1_thresholds && (item_type == "e" || item_type == "j" || item_type == "c")) threshold = -1;

            for(size_t j = 0; j < multiplicity; j++) {
                m_tauL1Items.push_back(s.substr(match[1].str().size()));
                m_tauL1Thr.push_back(threshold);
                m_tauL1Type.push_back(item_type + "TAU");
                m_tauL1ThresholdPattern.push_back(-1);
            }
            rgx_iter++;
        }

        m_L1Item = "L1" + m_L1Items[0];
    }
}

void TrigTauInfo::parseTriggerString(const std::map<int, int>& L1Phase1ThrMap_eTAU, const std::map<int, int>& L1Phase1ThrMap_jTAU)
{
    parseTriggerString(false);

    // Correct the Phase 1 thresholds:
    for(size_t i = 0; i < m_tauL1Items.size(); i++) {
        const std::string& item_type = m_tauL1Type.at(i);
        if(item_type == "eTAU" || item_type == "cTAU") {
            m_tauL1Thr[i] = L1Phase1ThrMap_eTAU.at(m_tauL1Thr.at(i));
        } else if(item_type == "jTAU") {
            m_tauL1Thr[i] = L1Phase1ThrMap_jTAU.at(m_tauL1Thr.at(i));
        }
    }
}

void TrigTauInfo::parseTriggerString(const std::map<std::string, float>& L1Phase1_thresholds, const std::map<std::string, uint64_t>& L1Phase1_threshold_patterns)
{
    parseTriggerString();

    for(size_t i = 0; i < m_tauL1Items.size(); i++) {
        if(m_tauL1Type.at(i) == "TAU") continue; // Skip the legacy items

        const std::string& item = m_tauL1Items.at(i);
        
        m_tauL1Thr[i] = L1Phase1_thresholds.at(item);
        m_tauL1ThresholdPattern[i] = L1Phase1_threshold_patterns.at(item);
    }
}