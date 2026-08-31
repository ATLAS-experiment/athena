/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigTauMonitoring/TrigTauInfo.h"
#include <regex>
#include <ranges> //std::views::split
#include <cstdint>

TrigTauInfo::TrigTauInfo(const std::string& trigger)
    : m_trigger{trigger}
{
    parseTriggerString();
}

TrigTauInfo::TrigTauInfo(const std::string& trigger, const std::map<std::string, float>& L1Phase1_thresholds)
    : m_trigger{trigger}
{
    parseTriggerString(L1Phase1_thresholds);
}

TrigTauInfo::TrigTauInfo(const std::string& trigger, const std::map<std::string, float>& L1Phase1_thresholds, const std::map<std::string, uint64_t>& L1Phase1_threshold_patterns)
    : m_trigger{trigger}
{
    parseTriggerString(L1Phase1_thresholds, L1Phase1_threshold_patterns);
}

TrigTauInfo::TrigTauInfo(const std::string& trigger, const std::map<int, int>& L1Phase1ThrMap_eTAU)
    : m_trigger{trigger}
{
    parseTriggerString(L1Phase1ThrMap_eTAU);
}

void TrigTauInfo::parseTriggerString(bool remove_L1_phase1_thresholds)
{
    std::string clean_trigger = m_trigger;
    
    // Change the "L1_" prefix to "L1" internally, in case the trigger being parsed is a pure L1 trigger with the usual L1 standalone naming scheme
    if(clean_trigger.size() > 3 && clean_trigger.rfind("L1_", 0) == 0) {
        clean_trigger = "L1" + clean_trigger.substr(3);
    }

    std::vector<std::string> sections;
    for (auto&& subrange : std::views::split(m_trigger, '_')) sections.emplace_back(subrange.begin(), subrange.end());

    std::regex tau_rgx("^(\\d*)tau(\\d+)$");
    std::regex elec_rgx("^(\\d*)e(\\d+)$");
    std::regex muon_rgx("^(\\d*)mu(\\d+)$");
    std::regex gamma_rgx("^(\\d*)g(\\d+)$");
    std::regex jet_rgx("^(\\d*)j(\\d+)$");
    std::regex met_rgx("^xe(\\d+)$");
    std::regex noalg_rgx("^noalg$");
    std::regex l1_rgx("^L1.*$");
    std::regex l1_tau_rgx("(\\d*)(e|j|c|)TAU(\\d+)(L|M|T|HL|HM|HT|H|IM|I|)");
    std::regex l1_toposeparate_rgx("^(\\d{0,2})(DETA|DPHI)(\\d{0,2})$");
    std::regex topo_rgx("^.*(invm|dR|deta|dphi)AB.*$");
    std::regex ditauomni_rgx("^ditauOmni(\\d)+Trk(\\d)+$");
    std::vector<std::regex*> all_regexes = {&tau_rgx, &elec_rgx, &muon_rgx, &gamma_rgx, &jet_rgx, &met_rgx, &l1_rgx, &ditauomni_rgx};

    std::regex tau_type_rgx("^(ptonly|tracktwoMVA|tracktwoLLP|trackLRT)$");
    std::regex tau_ID_rgx("^(idperf|noperf|perfcore|perfiso|perf|veryloose.*|loose.*|medium.*|tight.*)$");
    std::regex tau_HitZ_rgx("^(([\\dp]*mmX[\\dp]*mm)?HitZ.*)$");
    std::regex tau_CHPreselID_rgx("^(idperfCHP|veryloose.*CHP.*|loose.*CHP.*|medium.*CHP.*|tight.*CHP.*)$");

    std::smatch match;
    std::regex_token_iterator<std::string::iterator> rend;

    std::vector<bool> is_tau_probe_leg;

    // Check each leg
    int hlt_leg_idx = -1;
    std::vector<std::string> leg;
    for(size_t i = 0; i < sections.size(); i++) {
        leg.push_back(sections[i]); // Attach to the current leg
        //Match the beginning of a new leg, or the end of the chain
        if(i == sections.size() - 1 || (std::any_of(all_regexes.begin(), all_regexes.end(), [&sections, i](const std::regex* rgx) { return std::regex_match(sections[i+1], *rgx); }))) {
            // Process the previous leg, which starts with the item, multiplicity, and threshold
            if(std::regex_match(leg[0], match, tau_rgx)) {
                hlt_leg_idx++;

                size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
                unsigned int threshold = std::stoi(match[2].str());
                
                // HLT Tau sequence
                auto itr = find_if(leg.begin(), leg.end(), [tau_type_rgx](const std::string& s) { return std::regex_match(s, tau_type_rgx); });
                std::string type = itr != leg.end() ? *itr : "tracktwoMVA"; // Default to the tracktwoMVA sequence

                // HLT Tau ID
                itr = find_if(leg.begin(), leg.end(), [tau_ID_rgx](const std::string& s) { return std::regex_match(s, tau_ID_rgx); });
                std::string tau_id = itr != leg.end() ? *itr : "";
                if(tau_id.starts_with("veryloose")) tau_id = tau_id.substr(9);
                else if(tau_id.starts_with("loose")) tau_id = tau_id.substr(5);
                else if(tau_id.starts_with("medium")) tau_id = tau_id.substr(6);
                else if(tau_id.starts_with("tight")) tau_id = tau_id.substr(5);
            
                // The WP is a variation (e.g. "mediumvar2GNTauDev1")
                if(tau_id.starts_with("var")) {
                    std::size_t i = 3; // Take out the "var" prefix
                    
                    // Now find the variation number
                    while(i < tau_id.size() && std::isdigit(static_cast<unsigned char>(tau_id[i]))) i++;

                    tau_id = tau_id.substr(i);
                }

                // Get the perf-selection suffix
                if(tau_id.starts_with("perfcore")) tau_id = tau_id.substr(8);
                else if(tau_id.starts_with("perfiso")) tau_id = tau_id.substr(7);
                else if(tau_id.starts_with("noperf")) tau_id = tau_id.substr(6);

                // Override for the old trigger names
                if(tau_id == "RNN") {
                    if(type == "tracktwoMVA") tau_id = "DeepSet";
                    if(type == "tracktwoLLP" || type == "trackLRT") tau_id = "RNNLLP";
                }

                // Replacements (this is temporary, the entire TrigTauInfo class will be removed soon, and all this will be handled centrally in Python using the already available infrastructure)
                if(tau_id == "DS") tau_id = "DeepSet";
                else if(tau_id == "GNT") tau_id = "GNTau";

                // HitZ algorithm
                itr = find_if(leg.begin(), leg.end(), [tau_HitZ_rgx](const std::string& s) { return std::regex_match(s, tau_HitZ_rgx); });
                std::string tau_hitz = itr != leg.end() ? *itr : "";
                std::string tau_hitz_alg = tau_hitz;
                if(!tau_hitz_alg.empty()) {
                    // Check if tau_hitz contains "mm", and if it does, extract the part after the last "mm"
                    size_t pos = tau_hitz_alg.rfind("mm");
                    if(pos != std::string::npos) {
                        tau_hitz_alg = tau_hitz_alg.substr(pos + 2);
                    }
                }

                // HLT Calo+Hits Presel Tau ID
                itr = find_if(leg.begin(), leg.end(), [tau_CHPreselID_rgx](const std::string& s) { return std::regex_match(s, tau_CHPreselID_rgx); });
                std::string tau_chpresel_id = itr != leg.end() ? *itr : "";
                if(tau_chpresel_id.starts_with("veryloose")) tau_chpresel_id = tau_chpresel_id.substr(9);
                else if(tau_chpresel_id.starts_with("loose")) tau_chpresel_id = tau_chpresel_id.substr(5);
                else if(tau_chpresel_id.starts_with("medium")) tau_chpresel_id = tau_chpresel_id.substr(6);
                else if(tau_chpresel_id.starts_with("tight")) tau_chpresel_id = tau_chpresel_id.substr(5);
            
                // The WP is a variation (e.g. "mediumvar2GNTauDev1")
                if(tau_chpresel_id.starts_with("var")) {
                    std::size_t i = 3; // Take out the "var" prefix
                    
                    // Now find the variation number
                    while(i < tau_chpresel_id.size() && std::isdigit(static_cast<unsigned char>(tau_chpresel_id[i]))) i++;

                    tau_chpresel_id = tau_chpresel_id.substr(i);
                }

                // TauJet container name suffix (base: HLT_TrigTauRecMerged_)
                std::string tau_jet_container_sfx = type;
                // Remove the tracktwo prefixes if they are present
                if(type == "ptonly") tau_jet_container_sfx = "CaloMVAOnly";
                else if(tau_jet_container_sfx.starts_with("tracktwo")) tau_jet_container_sfx = tau_jet_container_sfx.substr(8);
                else if(tau_jet_container_sfx.starts_with("track")) tau_jet_container_sfx = tau_jet_container_sfx.substr(5);

                // Check if the leg is a probe leg (to support bootstrapped tau triggers)
                const bool is_probe_leg = std::find(leg.begin(), leg.end(), "probe") != leg.end();

                for(size_t j = 0; j < multiplicity; j++) {
                    m_HLTThr.push_back(threshold);
                    m_HLTTauTypes.push_back(type);
                    m_HLTTauIDs.push_back(tau_id);
                    m_HLTTauHitZSelections.push_back(tau_hitz);
                    m_HLTTauHitZAlgs.push_back(tau_hitz_alg);
                    m_HLTTauCHPreselIDs.push_back(tau_chpresel_id);

                    m_HLTTauLegIndices.push_back(hlt_leg_idx);
                    m_HLTTauLegContainerSfxs.push_back(tau_jet_container_sfx);

                    is_tau_probe_leg.push_back(is_probe_leg);
                }
            } else if(std::regex_match(leg[0], match, elec_rgx)) {
                hlt_leg_idx++;
                size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
                unsigned int threshold = std::stoi(match[2].str());
                for(size_t j = 0; j < multiplicity; j++) m_HLTElecThr.push_back(threshold);
            } else if(std::regex_match(leg[0], match, muon_rgx)) {
                hlt_leg_idx++;
                size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
                unsigned int threshold = std::stoi(match[2].str());
                for(size_t j = 0; j < multiplicity; j++) m_HLTMuonThr.push_back(threshold);
            } else if(std::regex_match(leg[0], match, gamma_rgx)) {
                hlt_leg_idx++;
                size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
                unsigned int threshold = std::stoi(match[2].str());
                for(size_t j = 0; j < multiplicity; j++) m_HLTGammaThr.push_back(threshold);
            } else if(std::regex_match(leg[0], match, jet_rgx)) {
                hlt_leg_idx++;
                size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
                unsigned int threshold = std::stoi(match[2].str());
                for(size_t j = 0; j < multiplicity; j++) m_HLTJetThr.push_back(threshold);
            } else if(std::regex_match(leg[0], match, met_rgx)) {
                hlt_leg_idx++;
                unsigned int threshold = std::stoi(match[2].str());
                m_HLTMETThr.push_back(threshold);
            } else if(std::regex_match(leg[0], match, noalg_rgx)) {
                hlt_leg_idx++;
                m_isStreamer = true;
            } else if (std::regex_match(leg[0], match, ditauomni_rgx)) {
                hlt_leg_idx++;
                m_HLTBoostedDitauName.push_back(leg[0]);
            } else if(std::regex_match(leg[0], l1_rgx)){ // Treat the L1 items as a leg
                for(size_t j = 0; j < leg.size(); j++) {
                    if(std::regex_match(leg[j], topo_rgx)) continue; // Remove HLT topo sections, not part of the L1 item

                    // L1Topo items (they all include a "-" in the name, or have a separate "##DETA/PHI##_" prefix):
                    if(leg[j].find('-') != std::string::npos || std::regex_match(leg[j], l1_toposeparate_rgx)) {
                        // We only keep information from the legacy L1Topo item, from which we will not always use all thresholds
                        // Since we won't be adding any more Legacy thresholds, let's hard-code it...
                        if(leg[0] == "L1TAU60" && leg[j] == "DR-TAU12ITAU12I") leg[j] = "TAU12IM"; // L1_TAU60_DR-TAU20ITAU12I, uses "TAU12IM" threshold from the L1Topo item
                        else if(leg.size() == 1 && (leg[0] == "L1DR-TAU20ITAU12I" || leg[0] == "L1DR-TAU20ITAU12I-J25")) {
                            // Uses both TAU items, in the M isolation threshold
                            leg[0] = "L1TAU20IM";
                            leg.push_back("TAU12IM");
                            // Even on combined  chains using jets, we don't use the jets threshold
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

    // Support for HitZ bootstrapped tau triggers
    // We have to check if we have both tag (non-probe) and probe di-tau legs with the same HLT threshold
    std::map<float, std::pair<std::vector<size_t>, std::vector<size_t>>> tau_n_tag_probes;
    for(size_t i = 0; i < m_HLTThr.size(); i++) {
        if(tau_n_tag_probes.find(m_HLTThr.at(i)) == tau_n_tag_probes.end()) tau_n_tag_probes[m_HLTThr.at(i)] = {{}, {}};

        if(is_tau_probe_leg.at(i)) tau_n_tag_probes[m_HLTThr.at(i)].second.push_back(i);
        else tau_n_tag_probes[m_HLTThr.at(i)].first.push_back(i);
    }
    // Keep only the entries with equal number of tag and probe legs
    for(auto it = tau_n_tag_probes.begin(); it != tau_n_tag_probes.end(); ) {
        if(it->second.first.size() != it->second.second.size()) it = tau_n_tag_probes.erase(it);
        else it++;
    }
    // If we have tag-probe pairs remaining, this is a bootstrapped trigger.
    // Get the list of tag legs to remove:
    std::vector<size_t> legs_to_remove;
    for(const auto& [thr, n_tag_probe] : tau_n_tag_probes) {
        legs_to_remove.insert(legs_to_remove.end(), n_tag_probe.first.begin(), n_tag_probe.first.end());
    }
    m_isBootstrappedTauTrigger = !legs_to_remove.empty();
    // Sort them from last to first, so we can remove them without affecting the indices of the remaining legs to remove
    std::sort(legs_to_remove.begin(), legs_to_remove.end(), std::greater<size_t>());
    // Remove the tag legs
    for(size_t i : legs_to_remove) {
        m_HLTThr.erase(m_HLTThr.begin() + i);
        m_HLTTauTypes.erase(m_HLTTauTypes.begin() + i);
        m_HLTTauIDs.erase(m_HLTTauIDs.begin() + i);
        m_HLTTauHitZSelections.erase(m_HLTTauHitZSelections.begin() + i);
        m_HLTTauHitZAlgs.erase(m_HLTTauHitZAlgs.begin() + i);
        m_HLTTauCHPreselIDs.erase(m_HLTTauCHPreselIDs.begin() + i);

        m_HLTTauLegIndices.erase(m_HLTTauLegIndices.begin() + i);
        m_HLTTauLegContainerSfxs.erase(m_HLTTauLegContainerSfxs.begin() + i);

        is_tau_probe_leg.erase(is_tau_probe_leg.begin() + i);
    }


    if(!m_L1Items.empty()) {
        // Build the full L1 string
        m_L1Item = m_L1Items[0];
        for(size_t j = 1; j < m_L1Items.size(); j++) m_L1Item += "_" + m_L1Items[j];

        // Get all individual L1 TAU items
        std::regex_token_iterator<std::string::iterator> rgx_iter(m_L1Item.begin(), m_L1Item.end(), l1_tau_rgx);
        while(rgx_iter != rend) {
            const std::string & s = *rgx_iter;
            if (std::regex_match(s, match, l1_tau_rgx)){
              size_t multiplicity = match[1].str() == "" ? 1 : std::stoi(match[1].str());
              std::string item_type = match[2].str(); // e, j, c, or ""
              int threshold = std::stoi(match[3].str());
              std::string item_isolation = match[4].str(); // "", L, M, T, HL, HM, HT, IM, H
              
              // Set the Phase 1 thresholds to -1
              if(remove_L1_phase1_thresholds && (item_type == "e" || item_type == "j" || item_type == "c")) threshold = -1;
  
              for(size_t j = 0; j < multiplicity; j++) {
                  m_tauL1Items.push_back(s.substr(match[1].str().size()));
                  m_tauL1Thr.push_back(threshold);
                  m_tauL1Type.push_back(item_type + "TAU");
                  m_tauL1Iso.push_back(item_isolation);
                  m_tauL1ThresholdPattern.push_back(-1);
              }
            }
            rgx_iter++;
        }

        m_L1Item = "L1" + m_L1Items[0];
    }
}

void TrigTauInfo::parseTriggerString(const std::map<std::string, float>& L1Phase1_thresholds)
{
    parseTriggerString();

    for(size_t i = 0; i < m_tauL1Items.size(); i++) {
        if(m_tauL1Type.at(i) == "TAU") continue; // Skip the legacy items

        const std::string& item = m_tauL1Items.at(i);
        
        m_tauL1Thr[i] = L1Phase1_thresholds.at(item);
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

void TrigTauInfo::parseTriggerString(const std::map<int, int>& L1Phase1ThrMap_eTAU)
{
    parseTriggerString(false);

    // Correct the Phase 1 thresholds:
    for(size_t i = 0; i < m_tauL1Items.size(); i++) {
        const std::string& item_type = m_tauL1Type.at(i);
        if(item_type == "eTAU" || item_type == "cTAU") {
            m_tauL1Thr[i] = L1Phase1ThrMap_eTAU.at(m_tauL1Thr.at(i));
        } 
    }
}
