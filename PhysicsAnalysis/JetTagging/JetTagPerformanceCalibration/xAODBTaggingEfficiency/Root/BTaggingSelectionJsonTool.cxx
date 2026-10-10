/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODBTaggingEfficiency/BTaggingSelectionJsonTool.h"
#include "xAODBTaggingEfficiency/BTaggingToolUtil.h"
#include "PathResolver/PathResolver.h"

#include <fstream>
#include <cmath>  //std::log
#include <algorithm>  //std::max
#include <limits> //std::numeric_limits

BTaggingSelectionJsonTool::BTaggingSelectionJsonTool(const std::string& name)
  : asg::AsgTool(name)
{
    m_initialised = false;
}

StatusCode BTaggingSelectionJsonTool::initialize() {
    m_initialised = true;

    if (m_minPt > 0.){ 
        // Convert MeV cut to GeV as in the JSON files numbers are in GeV 
        m_minPt = m_minPt * BTaggingToolUtil::MeVToGeV; 
    }

    std::string pathToJsonConfigFile = PathResolverFindCalibFile(m_json_config_path);
    std::ifstream jsonFile(pathToJsonConfigFile);
    if (!jsonFile.is_open()) {
        ATH_MSG_ERROR("JSON file " + m_json_config_path + " does not exist. Please put the correct path of the file.");
        return StatusCode::FAILURE;
    }

    m_json_config = json::parse(jsonFile);
    jsonFile.close();

    if (m_outputName.empty()) {
        ATH_MSG_ERROR("Must specify the output name property for the tagger");
        return StatusCode::FAILURE;
    }

    if (!m_json_config.contains(m_outputName)) {
        ATH_MSG_ERROR("The output name " + m_outputName + " not found in JSON file: " + m_json_config_path);
        return StatusCode::FAILURE;
    }

    if (m_jetAuthor.empty() || !m_json_config[m_outputName].contains(m_jetAuthor)) {
        ATH_MSG_ERROR("Tagger: " + m_outputName + " and Jet Collection: " + m_jetAuthor +
                      " not found in JSON file: " + m_json_config_path);
        return StatusCode::FAILURE;
    }

    if (m_OP.empty() || !m_json_config[m_outputName][m_jetAuthor].contains(m_OP)) {
        ATH_MSG_ERROR("OP " + m_OP + " not available for " + m_outputName + " tagger.");
        return StatusCode::FAILURE;
    }

    const json& meta = m_json_config[m_outputName][m_jetAuthor]["meta"];
    m_fractionAccessors = loadFractionValues(meta);

    // Pre-load cut values
    json& pT_mass_2d_tagger_cutvalue = m_json_config[m_outputName][m_jetAuthor][m_OP]["pT_mass_2d_cutvalue"];
    ATH_CHECK(loadBinConfig(pT_mass_2d_tagger_cutvalue, m_BinConfig));

    // Get mass decorator if specified
    if (meta.contains("Mass")) {
        std::string massDecoratorName = meta["Mass"].get<std::string>();
        if (massDecoratorName != "default") {
            m_massAcc = std::make_unique<SG::ConstAccessor<float>>(massDecoratorName);
            ATH_MSG_INFO("Using decorated mass '" << massDecoratorName << "' for Xbb FM WP.");
        }
    }

    // Get pT decorator if specified
    if (meta.contains("PT")) {
        std::string ptDecoratorName = meta["PT"].get<std::string>();
        if (ptDecoratorName != "default") {
            m_ptAcc = std::make_unique<SG::ConstAccessor<float>>(ptDecoratorName);
            ATH_MSG_INFO("Using decorated pT '" << ptDecoratorName << "' for Xbb FM WP.");
        }
    }

    // Veto setup
    m_veto = meta.contains("Veto");
    if (m_veto) {
        if (!meta["Veto"].contains("VetoTagger")) {
            ATH_MSG_ERROR("Specific veto tagger not found in JSON file: " + m_json_config_path);
            return StatusCode::FAILURE;
        }
        m_vetoTagger = meta["Veto"]["VetoTagger"];
        if (!meta["Veto"].contains("OperatingPoint")) {
            ATH_MSG_ERROR("Specific operating point not found in JSON file: " + m_json_config_path);
            return StatusCode::FAILURE;
        }
        m_vetoOP = meta["Veto"]["OperatingPoint"];

        if (!m_json_config.contains(m_vetoTagger) ||
            !m_json_config[m_vetoTagger].contains(m_jetAuthor) ||
            !m_json_config[m_vetoTagger][m_jetAuthor].contains(m_vetoOP)) {
            ATH_MSG_ERROR("Veto tagger configuration missing in JSON: " + m_json_config_path);
            return StatusCode::FAILURE;
        }

        const json& veto_meta = m_json_config[m_vetoTagger][m_jetAuthor]["meta"];
        m_vetoFractionAccessors = loadFractionValues(veto_meta);

        json& pT_mass_2d_veto_cutvalue = m_json_config[m_vetoTagger][m_jetAuthor][m_vetoOP]["pT_mass_2d_cutvalue"];
        ATH_CHECK(loadBinConfig(pT_mass_2d_veto_cutvalue, m_VetoBinConfig));
    }

    return StatusCode::SUCCESS;
}

StatusCode BTaggingSelectionJsonTool::loadBinConfig(const json& pT_mass_2d_cutvalue, BinConfig& config) const
{
    // pTbins defines the bin edges, so there must be exactly one fewer pT bin (WP entry) than
    // there are edges. The pT_mass_2d_cutvalue dict holds the "pTbins" edge list plus one entry
    // per pT bin, so its total size must equal the number of pT bin edges.
    if (pT_mass_2d_cutvalue["pTbins"].size() != pT_mass_2d_cutvalue.size()) {
        const std::string binCountMsg = "We expect N bin edges for N-1 bins, but there are " +
            std::to_string(pT_mass_2d_cutvalue["pTbins"].size()) + " pT bin edges but " +
            std::to_string(pT_mass_2d_cutvalue.size() - 1) +
            " entries in the pT_mass_2d_cutvalue dict. Please check the JSON file: " + m_json_config_path;
        if (m_allowBinCountMismatch) {
            ATH_MSG_WARNING(binCountMsg + " Continuing because AllowBinCountMismatch is set.");
        } else {
            ATH_MSG_ERROR(binCountMsg);
            return StatusCode::FAILURE;
        }
    }

    for (unsigned int ipT = 0; ipT < pT_mass_2d_cutvalue["pTbins"].size(); ++ipT) {
        const json& pt = pT_mass_2d_cutvalue["pTbins"][ipT];
        config.pTbins.push_back(BTaggingToolUtil::getExtendedFloat(pt));

        if (ipT != pT_mass_2d_cutvalue["pTbins"].size() - 1) {
            const json& ptUp = pT_mass_2d_cutvalue["pTbins"][ipT + 1];
            std::string pT_key = "pT_" + BTaggingToolUtil::getExtendedString(pt) +
                                  "_" + BTaggingToolUtil::getExtendedString(ptUp);

            auto itr = pT_mass_2d_cutvalue.find(pT_key);
            if (itr == pT_mass_2d_cutvalue.end()) {
                ATH_MSG_ERROR("pT_key=" + pT_key + " not found in JSON file: " + m_json_config_path);
                return StatusCode::FAILURE;
            }

            std::vector<float> mass_values;
            for (const auto& m : itr->at("mass")) {
                mass_values.push_back(BTaggingToolUtil::getExtendedFloat(m));
            }
            std::vector<float> cut_values = itr->at("cutvalues").get<std::vector<float>>();

            config.massbins.push_back(std::move(mass_values));
            config.OPCutValues.push_back(std::move(cut_values));

            // We expect N mass bins (edges) and N-1 cut values for each pT bin. Accessing the
            // cut value for a jet in the final mass bin would otherwise read outside the vector.
            if (config.massbins.back().size() != config.OPCutValues.back().size() + 1) {
                const std::string binCountMsg = "Expected to have N mass bins and N-1 cut values for pT bin " +
                    pT_key + " Instead found " + std::to_string(config.massbins.back().size()) +
                    " mass bins and " + std::to_string(config.OPCutValues.back().size()) + " cut values.";
                if (m_allowBinCountMismatch) {
                    ATH_MSG_WARNING(binCountMsg + " Continuing because AllowBinCountMismatch is set.");
                } else {
                    ATH_MSG_ERROR(binCountMsg);
                    return StatusCode::FAILURE;
                }
            }
        }
    }

    return StatusCode::SUCCESS;
}


std::vector<BTaggingSelectionJsonTool::FractionAccessor> BTaggingSelectionJsonTool::loadFractionValues(const json &meta) const
{
    std::string taggerName;
    if (meta.contains("TaggerName")) {
        taggerName = meta["TaggerName"];
    } else {
        ATH_MSG_INFO("No 'TaggerName' section found in the meta data for " + m_outputName +
                     " tagger. Using " + m_outputName + " as the tagger name.");
        taggerName = m_outputName;
    }

    std::string target = meta["TaggingTarget"];
    std::vector<BTaggingSelectionJsonTool::FractionAccessor> fractionAccessors;

    // Pre-load fraction values
    for (const json& outclass : meta["categories"]) {
        std::string outclassStr = std::string(outclass);
        float fraction = meta["fraction_" + outclassStr].get<float>();
        SG::ConstAccessor<float> accessor(taggerName + "_p" + outclassStr);
        bool isTarget = (outclassStr == target);
        fractionAccessors.emplace_back(fraction, accessor, isTarget);
    }
    
    return fractionAccessors;
}

double BTaggingSelectionJsonTool::getTaggerDiscriminantInternal(const xAOD::Jet& jet, const std::vector<FractionAccessor>& fractionAccessors) const
{
    float numerator = 0.;
    float denominator = 0.;

    for (const auto& frac : fractionAccessors) {
        float p_output = frac.accessor(jet);
        if (frac.isTarget) {
            numerator += frac.fraction * p_output;
        } else {
            denominator += frac.fraction * p_output;
        }
    }

    // Smallest positive normal float; below this, reciprocals may overflow.
    const float ep = std::numeric_limits<float>::min();
    //coverity[DIVIDE_BY_ZERO]
    const float ratio = (std::abs(denominator) < ep ? std::numeric_limits<float>::infinity() : numerator / denominator);
    const double tagger_discriminant = (std::abs(ratio) < ep ? -std::numeric_limits<double>::infinity() : std::log( ratio ));

    return tagger_discriminant;    
}

double BTaggingSelectionJsonTool::getTaggerDiscriminant(const xAOD::Jet& jet) const
{
    return getTaggerDiscriminantInternal(jet, m_fractionAccessors);
}

double BTaggingSelectionJsonTool::getVetoDiscriminant(const xAOD::Jet& jet) const
{
    return getTaggerDiscriminantInternal(jet, m_vetoFractionAccessors);
}

int BTaggingSelectionJsonTool::accept(const xAOD::Jet& jet) const
{
    if (!m_initialised) throw std::runtime_error("BTaggingSelectionJsonTool has not been initialised.");

    double pt = getJetPtInGeV(jet);
    double eta = jet.eta();
    double mass = getJetMassInGeV(jet);
    double tagger_discriminant = getTaggerDiscriminant(jet);
    int index = 0;

    if (std::abs(eta) > m_maxEta || pt < m_minPt) return index;

    int pt_bin_index = findBin(m_BinConfig.pTbins, pt);
    if (pt_bin_index == -1) return index;

    int mass_bin_index = findBin(m_BinConfig.massbins[pt_bin_index], mass);
    if (mass_bin_index == -1) return index;

    float cutvalue = m_BinConfig.OPCutValues[pt_bin_index][mass_bin_index];

    // Check if 1D or 2D boosted flavour tagging is needed
    if (!m_veto) {
        index = (tagger_discriminant > cutvalue) ? 1 : 0;
    } else {
        // If jet fails tagging requirement 
        // there is no need to check against veto requirement   
        if (tagger_discriminant <= cutvalue) {
            return index;
        }

        // Check if the jet does not pass the veto requirement
        double veto_discriminant = getVetoDiscriminant(jet);

        int pt_veto_bin_index = findBin(m_VetoBinConfig.pTbins, pt);
        if (pt_veto_bin_index == -1) return index;

        int mass_veto_bin_index = findBin(m_VetoBinConfig.massbins[pt_veto_bin_index], mass);
        if (mass_veto_bin_index == -1) return index;

        float veto_cutvalue = m_VetoBinConfig.OPCutValues[pt_veto_bin_index][mass_veto_bin_index];

        index = (veto_discriminant < veto_cutvalue) ? 1 : 0;
    }

    return index;
}

// the following function is only for Xbb calibration team, for physics analyses, please use the one above.
int BTaggingSelectionJsonTool::acceptOnlyForXbbCalibrationUsage(double pt, double eta, double mass, double tagger_discriminant) const
{
    if (!m_initialised) throw std::runtime_error("BTaggingSelectionJsonTool has not been initialised.");

    int index = 0;
    if (std::abs(eta) > m_maxEta || pt < m_minPt) return index;

    int pt_bin_index = findBin(m_BinConfig.pTbins, pt);
    if (pt_bin_index == -1) return index;

    int mass_bin_index = findBin(m_BinConfig.massbins[pt_bin_index], mass);
    if (mass_bin_index == -1) return index;

    float cutvalue = m_BinConfig.OPCutValues[pt_bin_index][mass_bin_index];
    index = (tagger_discriminant > cutvalue) ? 1 : 0;

    return index;
}

int BTaggingSelectionJsonTool::findBin(const std::vector<float>& bins, float value) const
{
    for (size_t i = 0; i < bins.size() - 1; i++) {
        if ((std::min(bins[i], bins[i + 1]) <= value && value < std::max(bins[i], bins[i + 1]))) {
            return i;
        }
    }
    return -1;
}

float BTaggingSelectionJsonTool::getJetMassInGeV(const xAOD::Jet& jet) const
{   
    // Convert mass from MeV to GeV 
    if (!m_massAcc) {
        return jet.m() * BTaggingToolUtil::MeVToGeV;
    }
    if (!m_massAcc->isAvailable(jet)) {
        ATH_MSG_ERROR("Decorated mass '" << SG::AuxTypeRegistry::instance().getName( m_massAcc->auxid() ) << "' not available on jet. Cannot proceed.");
        throw std::runtime_error("Decorated mass not available on jet.");
    }
    return (*m_massAcc)(jet) * BTaggingToolUtil::MeVToGeV;
}

float BTaggingSelectionJsonTool::getJetPtInGeV(const xAOD::Jet& jet) const
{   
    // Convert pT from MeV to GeV 
    if (!m_ptAcc) {
        return jet.pt() * BTaggingToolUtil::MeVToGeV;
    }
    if (!m_ptAcc->isAvailable(jet)) {
        ATH_MSG_ERROR("Decorated pT '" << SG::AuxTypeRegistry::instance().getName( m_ptAcc->auxid() ) << "' not available on jet. Cannot proceed.");
        throw std::runtime_error("Decorated pT not available on jet.");
    }
    return (*m_ptAcc)(jet) * BTaggingToolUtil::MeVToGeV;
}
