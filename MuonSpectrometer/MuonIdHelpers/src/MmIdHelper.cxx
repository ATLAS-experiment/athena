/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonIdHelpers/MmIdHelper.h"
#include "Identifier/RangeIterator.h"

#include "AthenaKernel/getMessageSvc.h"

/*******************************************************************************/
// Constructor/Destructor
MmIdHelper::MmIdHelper() : MuonIdHelper("MmIdHelper", "mm") {
    m_module_hashes.fill(-1);
    m_detectorElement_hashes.fill(-1);
}
/*******************************************************************************/
// Initialize dictionary
int MmIdHelper::initialize_from_dictionary(const IdDictMgr& dict_mgr) {
    int status = 0;

    // Check whether this helper should be reinitialized
    if (!reinitialize(dict_mgr)) {
        ATH_MSG_INFO("Request to reinitialize not satisfied - tags have not changed");
        return (0);
    } else {
        ATH_MSG_DEBUG("(Re)initialize ");
    }

    // init base object
    AtlasDetectorID::setMessageSvc(Athena::getMessageSvc());
    if (AtlasDetectorID::initialize_from_dictionary(dict_mgr)) return 1;

    // Register version of the MuonSpectrometer dictionary
    if (register_dict_tag(dict_mgr, "MuonSpectrometer")) return 1;

    m_dict = dict_mgr.find_dictionary("MuonSpectrometer");
    if (!m_dict) {
        ATH_MSG_ERROR(" initialize_from_dict - cannot access MuonSpectrometer dictionary ");
        return 1;
    }

    // Initialize some of the field indices
    if (initLevelsFromDict()) return 1;

    int index = technologyIndex("MM");
    if (index == -1) {
        ATH_MSG_DEBUG("initLevelsFromDict - there are no MM entries in the dictionary! ");
        return 0;
    }

    IdDictField* field = m_dict->find_field("mmMultilayer");
    if (field) {
        m_DETECTORELEMENT_INDEX = field->index();
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'mmMultilayer' field ");
        status = 1;
    }

    field = m_dict->find_field("mmGasGap");
    if (field) {
        m_GASGAP_INDEX = field->index();
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'mmGasGap' field ");
        status = 1;
    }

    field = m_dict->find_field("mmChannel");
    if (field) {
        m_CHANNEL_INDEX = field->index();
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find mmChannel' field ");
        status = 1;
    }

    // reinitialize the module ndex
    //// m_DETECTORELEMENT_INDEX = m_MODULE_INDEX;

    // save an index to the first region of MM
    IdDictGroup* mmGroup = m_dict->find_group("mm");
    if (!mmGroup) {
        ATH_MSG_ERROR("Cannot find mm group");
    } else {
        m_GROUP_INDEX = mmGroup->regions()[0]->index();
    }

    const IdDictRegion& region = m_dict->region(m_GROUP_INDEX);
    m_eta_impl = region.implementation(m_ETA_INDEX);
    m_phi_impl = region.implementation(m_PHI_INDEX);
    m_tec_impl = region.implementation(m_TECHNOLOGY_INDEX);
    m_mplet_impl = region.implementation(m_DETECTORELEMENT_INDEX);
    m_gap_impl = region.implementation(m_GASGAP_INDEX);
    m_cha_impl = region.implementation(m_CHANNEL_INDEX);

    ATH_MSG_DEBUG(" MicroMegas decode index and bit fields for each level: " << std::endl
                                                                             << " muon        " << m_muon_impl.show_to_string() << std::endl
                                                                             << " station     " << m_sta_impl.show_to_string() << std::endl
                                                                             << " eta         " << m_eta_impl.show_to_string() << std::endl
                                                                             << " phi         " << m_phi_impl.show_to_string() << std::endl
                                                                             << " technology  " << m_tec_impl.show_to_string() << std::endl
                                                                             << " multilayer   " << m_mplet_impl.show_to_string()
                                                                             << std::endl
                                                                             << " gasgap      " << m_gap_impl.show_to_string() << std::endl
                                                                             << " channel     " << m_cha_impl.show_to_string());

    //
    // Build multirange for the valid set of identifiers
    //

    // Find value for the field MuonSpectrometer
    int muonField = -1;
    const IdDictDictionary* atlasDict = dict_mgr.find_dictionary("ATLAS");
    if (atlasDict->get_label_value("subdet", "MuonSpectrometer", muonField)) {
        ATH_MSG_ERROR("Could not get value for label 'MuonSpectrometer' of field 'subdet' in dictionary " << atlasDict->name());
        return 1;
    }

    // Build MultiRange down to "technology" for all (muon) regions
    ExpandedIdentifier region_id;
    region_id.add(muonField);
    Range prefix;
    MultiRange muon_range = m_dict->build_multirange(region_id, prefix, "technology");
    if (muon_range.size() > 0) {
        ATH_MSG_INFO("MultiRange built successfully to Technology: "
                     << "MultiRange size is " << muon_range.size());
    } else {
        ATH_MSG_ERROR("Muon MultiRange is empty");
    }

    // Build MultiRange down to "detector element" for all mdt regions
    ExpandedIdentifier detectorElement_region;
    detectorElement_region.add(muonField);
    Range detectorElement_prefix;
    MultiRange muon_detectorElement_range = m_dict->build_multirange(detectorElement_region, detectorElement_prefix, "mmMultilayer");
    if (muon_detectorElement_range.size() > 0) {
        ATH_MSG_INFO("MultiRange built successfully to detector element: "
                     << "Multilayer MultiRange size is " << muon_detectorElement_range.size());
    } else {
        ATH_MSG_ERROR("Muon MicroMegas detector element MultiRange is empty");
    }

    // Build MultiRange down to "channel" for all MM regions
    ExpandedIdentifier mm_region;
    mm_region.add(muonField);
    Range mm_prefix;
    MultiRange muon_channel_range = m_dict->build_multirange(mm_region, mm_prefix, "mmChannel");
    if (muon_channel_range.size() > 0) {
        ATH_MSG_INFO("MultiRange built successfully to channel: "
                     << "MultiRange size is " << muon_channel_range.size());
    } else {
        ATH_MSG_ERROR("Muon MultiRange is empty for channels");
    }

    // build MicroMegas module ranges
    // Find the regions that have a "technology field" that matches the MM and save them
    int mmField = -1;
    status = m_dict->get_label_value("technology", "MM", mmField);

    for (int i = 0; i < (int)muon_range.size(); ++i) {
        const Range& range = muon_range[i];
        if (range.fields() > m_TECHNOLOGY_INDEX) {
            const Range::field& field = range[m_TECHNOLOGY_INDEX];
            if (field.match((ExpandedIdentifier::element_type)mmField)) {
                m_full_module_range.add(range);
                ATH_MSG_DEBUG("field size is " << (int)range.cardinality() << " field index = " << i);
            }
        }
    }

    for (int j = 0; j < (int)muon_detectorElement_range.size(); ++j) {
        const Range& range = muon_detectorElement_range[j];
        if (range.fields() > m_TECHNOLOGY_INDEX) {
            const Range::field& field = range[m_TECHNOLOGY_INDEX];
            if (field.match((ExpandedIdentifier::element_type)mmField)) {
                m_full_detectorElement_range.add(range);
                ATH_MSG_DEBUG("detector element field size is " << (int)range.cardinality() << " field index = " << j);
            }
        }
    }

    for (int j = 0; j < (int)muon_channel_range.size(); ++j) {
        const Range& range = muon_channel_range[j];
        if (range.fields() > m_TECHNOLOGY_INDEX) {
            const Range::field& field = range[m_TECHNOLOGY_INDEX];
            if (field.match((ExpandedIdentifier::element_type)mmField)) {
                m_full_channel_range.add(range);
                ATH_MSG_DEBUG("channel field size is " << (int)range.cardinality() << " field index = " << j);
            }
        }
    }

    // test to see that the multi range is not empty
    if (m_full_module_range.size() == 0) {
        ATH_MSG_ERROR("MicroMegas MultiRange ID is empty for modules");
        status = 1;
    }

    // test to see that the detector element multi range is not empty
    if (m_full_detectorElement_range.size() == 0) {
        ATH_MSG_ERROR("MicroMegas MultiRange ID is empty for detector elements");
        status = 1;
    }

    // test to see that the multi range is not empty
    if (m_full_channel_range.size() == 0) {
        ATH_MSG_ERROR("MicroMegas MultiRange ID is empty for channels");
        status = 1;
    }

    // Setup the hash tables for MicroMegas
    ATH_MSG_INFO("Initializing MicroMegas hash indices ... ");
    status = init_hashes();
    status = init_detectorElement_hashes();  // same as module hash
    status = init_id_to_hashes();

    // Setup hash tables for finding neighbors
    ATH_MSG_INFO("Initializing MicroMegas hash indices for finding neighbors ... ");
    status = init_neighbors();

    m_init = true;
    return (status);
}  // end MmIdHelper::initialize_from_dictionary
/*******************************************************************************/

inline unsigned int MmIdHelper::moduleHashIdx(const Identifier& id) const{
    /// Unfold the array [A][B][C] by
    /// a * BxC + b * C + c
    constexpr unsigned int C = s_phiDim;
    constexpr unsigned int BxC = C*s_etaDim;    
    const int stEta = stationEta(id);
    return (stationName(id) - m_stationShift)*BxC + (stEta + s_etaDim/2 - (stEta>0))*C + (stationPhi(id) -1);
}
inline unsigned int MmIdHelper::detEleHashIdx(const Identifier& id) const{
    return moduleHashIdx(id)  *s_mlDim +  (multilayer(id) -1);
}

int MmIdHelper::init_id_to_hashes() {
    for (const Identifier& id : m_module_vec) m_stationShift = std::min(m_stationShift, 1u* stationName(id));
    unsigned int hash_max = module_hash_max();
    for (unsigned int i = 0; i < hash_max; ++i) {
        const Identifier& id = m_module_vec[i];
        const unsigned idx = moduleHashIdx(id);
        if (idx >= m_module_hashes.size() || m_module_hashes[idx] < hash_max){
            ATH_MSG_FATAL("Failed to assign module hash to "<<show_to_string(id));
            return 1;
        }
        m_module_hashes[idx] = i;
    }

    hash_max = detectorElement_hash_max();
    for (unsigned int i = 0; i < hash_max; ++i) {
        const Identifier& id = m_detectorElement_vec[i];
        const unsigned idx = detEleHashIdx(id);
        if (idx >= m_detectorElement_hashes.size() || m_detectorElement_hashes[idx] < hash_max){
            ATH_MSG_FATAL("Failed to assign detector hash to "<<show_to_string(id));
            return 1;
        }
        m_detectorElement_hashes[idx] = i;
    }
    return 0;
}  // end MmIdHelper::init_id_to_hashes()
/*******************************************************************************/
int MmIdHelper::get_module_hash(const Identifier& id, IdentifierHash& hash_id) const {
    const unsigned int idx = moduleHashIdx(id);
    if (idx >= m_module_hashes.size()) return 1;
    hash_id = m_module_hashes[idx];
    return 0;
}  // end MmIdHelper::get_module_hash
/*******************************************************************************/
int MmIdHelper::get_detectorElement_hash(const Identifier& id, IdentifierHash& hash_id) const {
    const unsigned int idx = detEleHashIdx(id);
    if (idx >= m_detectorElement_hashes.size()) return 1;
    hash_id = m_detectorElement_hashes[idx];
    return 0;
    // return get_module_hash(id, hash_id);
}
/*******************************************************************************/
Identifier MmIdHelper::multilayerID(const Identifier& channelID) const {
    assert(is_mm(channelID));
    Identifier result(channelID);
    // m_mplet_impl.reset(result);
    m_gap_impl.reset(result);
    m_cha_impl.reset(result);
    return result;
}
/*******************************************************************************/
Identifier MmIdHelper::multilayerID(const Identifier& moduleID, int multilayer) const {
    Identifier result(moduleID);
    resetAndSet(m_mplet_impl, multilayer, result);
    return result;
}
Identifier MmIdHelper::multilayerID(const Identifier& moduleID, int multilayer, bool& isValid) const {
    try {
        const Identifier result = multilayerID(moduleID, multilayer);
        isValid = validElement(result);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}
/*******************************************************************************/
int MmIdHelper::getFirstPcbChnl(int stationEta, int pcb) {
	int pcbNb = std::abs(stationEta)==1 ? pcb : pcb-5;
	return (pcbNb-1)*1024+1;
}
Identifier MmIdHelper::pcbID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int pcb) const {
	int chnl = getFirstPcbChnl(stationEta,  pcb); 
	return channelID(stationName, stationEta, stationPhi, multilayer, gasGap, chnl);
}
Identifier MmIdHelper::pcbID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int pcb, bool& isValid) const {
	int chnl = getFirstPcbChnl(stationEta,  pcb); 
	return channelID(stationName, stationEta, stationPhi, multilayer, gasGap, chnl, isValid);
}
Identifier MmIdHelper::pcbID(const std::string& stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int pcb) const {
	int chnl = getFirstPcbChnl(stationEta,  pcb); 
	return channelID(stationName, stationEta, stationPhi, multilayer, gasGap, chnl);
}
Identifier MmIdHelper::pcbID(const std::string& stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int pcb, bool& isValid) const {
	int chnl = getFirstPcbChnl(stationEta,  pcb); 
	return channelID(stationName, stationEta, stationPhi, multilayer, gasGap, chnl, isValid);
}
Identifier MmIdHelper::pcbID(const Identifier& channelId, int pcb) const {
	int chnl = getFirstPcbChnl(stationEta(channelId),  pcb); 
	return channelID(channelId, multilayer(channelId), gasGap(channelId), chnl);
}
Identifier MmIdHelper::pcbID(const Identifier& channelId, int pcb, bool& isValid) const {
	int chnl = getFirstPcbChnl(stationEta(channelId),  pcb); 
	return channelID(channelId, multilayer(channelId), gasGap(channelId), chnl, isValid);
}
Identifier MmIdHelper::pcbID(const Identifier& channelId) const {
	int chnl = channel(channelId);
        // PCB counts from 1-8. PCBs 1-5 are in abs(stationEta)==1 and PCBs 6-8 in abs(stationEta)==2
        // each PCB consists of 1024 readout strips (strip number in athena is counting from 1 therefore chnl -1)
	int pcb  = (chnl-1)/1024+1 + (std::abs(stationEta(channelId))==2 ? 5:0); // int division should round downwards
	return pcbID(channelId, pcb);
}
/*******************************************************************************/
int MmIdHelper::getFirstRadiusChnl(int stationEta, int radius) {
	int radiusNb = std::abs(stationEta)==1 ? radius : radius-10;
	return radiusNb*512+1;
}
Identifier MmIdHelper::febID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int radius) const {
	int chnl = getFirstRadiusChnl(stationEta,  radius); 
	return channelID(stationName, stationEta, stationPhi, multilayer, gasGap, chnl);
}
Identifier MmIdHelper::febID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int radius, bool& isValid) const {
	int chnl = getFirstRadiusChnl(stationEta,  radius); 
	return channelID(stationName, stationEta, stationPhi, multilayer, gasGap, chnl, isValid);
}
Identifier MmIdHelper::febID(const std::string& stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int radius) const {
	int chnl = getFirstRadiusChnl(stationEta,  radius); 
	return channelID(stationName, stationEta, stationPhi, multilayer, gasGap, chnl);
}
Identifier MmIdHelper::febID(const std::string& stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int radius, bool& isValid) const {
	int chnl = getFirstRadiusChnl(stationEta,  radius); 
	return channelID(stationName, stationEta, stationPhi, multilayer, gasGap, chnl, isValid);
}
Identifier MmIdHelper::febID(const Identifier& channelId, int radius) const {
	int chnl = getFirstRadiusChnl(stationEta(channelId),  radius); 
	return channelID(channelId, multilayer(channelId), gasGap(channelId), chnl);
}
Identifier MmIdHelper::febID(const Identifier& channelId, int radius, bool& isValid) const {
	int chnl = getFirstRadiusChnl(stationEta(channelId),  radius); 
	return channelID(channelId, multilayer(channelId), gasGap(channelId), chnl, isValid);
}
Identifier MmIdHelper::febID(const Identifier& channelId) const {
	int chnl = channel(channelId);
        // radius counts from 0-15. Radii 0-9 are in abs(stationEta)==1 and radii 10-15 in abs(stationEta)==2
        // each radius consists of 512 readout strips (strip number in athena is counting from 1 therefore chnl -1)
	int radius  = (chnl-1)/512 + (std::abs(stationEta(channelId))==2 ? 10:0); // int division should round downwards
	return febID(channelId, radius);
}
/*******************************************************************************/
void MmIdHelper::idChannels(const Identifier& id, std::vector<Identifier>& vect) const {
    vect.clear();
    Identifier parent = parentID(id);
    for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
        const Range& range = m_full_channel_range[i];
        ConstRangeIterator rit(range);
        for (const auto & expId : rit) {
            Identifier child;
            get_id(expId, child);
            if (parentID(child) == parent) vect.push_back(child);
        }
    }
}
/*******************************************************************************/
int MmIdHelper::stationEtaMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext eta_context(expId, 0, m_ETA_INDEX);
    if (!get_expanded_id(id, expId, &eta_context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_module_range.size(); ++i) {
            const Range& range = m_full_module_range[i];
            if (range.match(expId)) {
                const Range::field& eta_field = range[m_ETA_INDEX];
                if (not eta_field.empty()) {
                    int etamin = eta_field.get_minimum();
                    if (-999 == result) {
                        result = etamin;
                    } else {
                        if (etamin < result) result = etamin;
                    }
                }
            }
        }
        return (result);
    }
    return (999);  // default
}  // end MmIdHelper::stationEtaMin
/*******************************************************************************/
int MmIdHelper::stationEtaMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext eta_context(expId, 0, m_ETA_INDEX);
    if (!get_expanded_id(id, expId, &eta_context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_module_range.size(); ++i) {
            const Range& range = m_full_module_range[i];
            if (range.match(expId)) {
                const Range::field& eta_field = range[m_ETA_INDEX];
                if (not eta_field.empty()) {
                    int etamax = eta_field.get_maximum();
                    if (result < etamax) result = etamax;
                }
            }
        }
        return (result);
    }
    return (-999);
}  // end MmIdHelper::stationEtaMax
/*******************************************************************************/
int MmIdHelper::stationPhiMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext phi_context(expId, 0, m_PHI_INDEX);

    if (!get_expanded_id(id, expId, &phi_context)) {
        for (unsigned int i = 0; i < m_full_module_range.size(); ++i) {
            const Range& range = m_full_module_range[i];
            if (range.match(expId)) {
                const Range::field& phi_field = range[m_PHI_INDEX];
                if (not phi_field.empty()) { return (phi_field.get_minimum()); }
            }
        }
    }
    // Failed to find the min
    return (999);
}
/*******************************************************************************/
int MmIdHelper::stationPhiMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext phi_context(expId, 0, m_PHI_INDEX);

    if (!get_expanded_id(id, expId, &phi_context)) {
        for (unsigned int i = 0; i < m_full_module_range.size(); ++i) {
            const Range& range = m_full_module_range[i];
            if (range.match(expId)) {
                const Range::field& phi_field = range[m_PHI_INDEX];
                if (not phi_field.empty()) { return (phi_field.get_maximum()); }
            }
        }
    }
    // Failed to find the max
    return (-999);
}
/*******************************************************************************/
int MmIdHelper::numberOfMultilayers(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext context = technology_context();
    if (!get_expanded_id(id, expId, &context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& multilayer_field = range[m_DETECTORELEMENT_INDEX];
                if (not multilayer_field.empty()) {
                    int multilayermax = multilayer_field.get_maximum();
                    if (result < multilayermax) result = multilayermax;
                }
            }
        }
        return (result);
    }
    return (-999);
}
/*******************************************************************************/
int MmIdHelper::multilayerMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext multilayer_context(expId, 0, m_DETECTORELEMENT_INDEX);
    if (!get_expanded_id(id, expId, &multilayer_context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& multilayer_field = range[m_DETECTORELEMENT_INDEX];
                if (not multilayer_field.empty()) {
                    int multilayermin = multilayer_field.get_minimum();
                    if (-999 == result) {
                        result = multilayermin;
                    } else {
                        if (multilayermin < result) result = multilayermin;
                    }
                }
            }
        }
        return (result);
    }
    return (999);  /// default
}
/*******************************************************************************/
int MmIdHelper::multilayerMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext multilayer_context(expId, 0, m_DETECTORELEMENT_INDEX);
    if (!get_expanded_id(id, expId, &multilayer_context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& multilayer_field = range[m_DETECTORELEMENT_INDEX];
                if (not multilayer_field.empty()) {
                    int multilayermax = multilayer_field.get_maximum();
                    if (result < multilayermax) result = multilayermax;
                }
            }
        }
        return (result);
    }
    return (-999);
}
/*******************************************************************************/
int MmIdHelper::gasGapMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext gasgap_context(expId, 0, m_GASGAP_INDEX);
    if (!get_expanded_id(id, expId, &gasgap_context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& gasgap_field = range[m_GASGAP_INDEX];
                if (not gasgap_field.empty()) {
                    int gasgapmin = gasgap_field.get_minimum();
                    if (-999 == result) {
                        result = gasgapmin;
                    } else {
                        if (gasgapmin < result) result = gasgapmin;
                    }
                }
            }
        }
        return (result);
    }
    return (999);
}
/*******************************************************************************/
int MmIdHelper::gasGapMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext gasgap_context(expId, 0, m_GASGAP_INDEX);
    if (!get_expanded_id(id, expId, &gasgap_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& gasgap_field = range[m_GASGAP_INDEX];
                if (not gasgap_field.empty()) { return (gasgap_field.get_maximum()); }
            }
        }
    }
    // Failed to find the max
    return (-999);
}
/*******************************************************************************/
int MmIdHelper::channelMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext channel_context(expId, 0, m_CHANNEL_INDEX);
    if (!get_expanded_id(id, expId, &channel_context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& channel_field = range[m_CHANNEL_INDEX];
                if (not channel_field.empty()) {
                    int channelmin = channel_field.get_minimum();
                    if (-999 == result) {
                        result = channelmin;
                    } else {
                        if (channelmin < result) result = channelmin;
                    }
                }
            }
        }
        return (result);
    }
    return (999);
}
/*******************************************************************************/
int MmIdHelper::channelMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext channel_context(expId, 0, m_CHANNEL_INDEX);
    if (!get_expanded_id(id, expId, &channel_context)) {
        int result = -999;
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& channel_field = range[m_CHANNEL_INDEX];
                if (not channel_field.empty()) {
                    int channelmax = channel_field.get_maximum();
                    if (result < channelmax) result = channelmax;
                }
            }
        }
        return (result);
    }
    return (-999);
}
/*******************************************************************************/
// validation of levels
bool MmIdHelper::valid(const Identifier& id) const {
    if (!validElement(id)) return false;

    int mplet = multilayer(id);
    if ((mplet < multilayerMin(id)) || (mplet > multilayerMax(id))) {
        ATH_MSG_DEBUG("Invalid multilayer=" << mplet << " multilayerMin=" << multilayerMin(id) << " multilayerMax=" << multilayerMax(id));
        return false;
    }

    int gasG = gasGap(id);
    if (gasG < gasGapMin(id) || gasG > gasGapMax(id)) {
        ATH_MSG_DEBUG("Invalid gasGap=" << gasG << " gasGapMin=" << gasGapMin(id) << " gasGapMax=" << gasGapMax(id));
        return false;
    }

    int element = channel(id);
    if (element < channelMin(id) || element > channelMax(id)) {
        ATH_MSG_DEBUG("Invalid channel=" << element << " channelMin=" << channelMin(id) << " channelMax=" << channelMax(id));
        return false;
    }
    return true;
}  // end MmIdHelper::valid
/*******************************************************************************/
bool MmIdHelper::isStNameInTech(const std::string& stationName) const { return stationName[0] == 'M'; }
bool MmIdHelper::validElement(const Identifier& id) const {
    int station = stationName(id);
    if (!validStation(station)) {
        ATH_MSG_DEBUG("Invalid stationName=" << stationNameString(station));
        return false;
    }

    int eta = stationEta(id);
    if (eta < stationEtaMin(id) || eta > stationEtaMax(id)) {
        ATH_MSG_DEBUG("Invalid stationEta=" << eta << " for stationName=" << stationNameString(station) << " stationIndex=" << station
                                            << " stationEtaMin=" << stationEtaMin(id) << " stationEtaMax=" << stationEtaMax(id));
        return false;
    }

    int phi = stationPhi(id);
    if (phi < stationPhiMin(id) || phi > stationPhiMax(id)) {
        ATH_MSG_DEBUG("Invalid stationPhi=" << phi << " for stationName=" << stationNameString(station) << " stationIndex=" << station
                                            << " stationPhiMin=" << stationPhiMin(id) << " stationPhiMax=" << stationPhiMax(id));
        return false;
    }
    return true;

}  // end MmIdHelper::validElement
/*******************************************************************************/
// Private validation of levels
bool MmIdHelper::validElement(const Identifier& id, int stationName, int stationEta, int stationPhi) const {
    if (!validStation(stationName)) {
        ATH_MSG_DEBUG("Invalid stationName=" << stationNameString(stationName));
        return false;
    }
    if (stationEta < stationEtaMin(id) || stationEta > stationEtaMax(id)) {
        ATH_MSG_DEBUG("Invalid stationEta=" << stationEta << " for stationName=" << stationNameString(stationName)
                                            << " stationIndex=" << stationName << " stationEtaMin=" << stationEtaMin(id)
                                            << " stationEtaMax=" << stationEtaMax(id));
        return false;
    }
    if (stationPhi < stationPhiMin(id) || stationPhi > stationPhiMax(id)) {
        ATH_MSG_DEBUG("Invalid stationPhi=" << stationPhi << " for stationName=" << stationNameString(stationName)
                                            << " stationIndex=" << stationName << " stationPhiMin=" << stationPhiMin(id)
                                            << " stationPhiMax=" << stationPhiMax(id));
        return false;
    }
    return true;
}  // end MmIdHelper::validElement
/*******************************************************************************/
// Check values down to readout channel level
bool MmIdHelper::validChannel(const Identifier& id, int stationName, int stationEta, int stationPhi, int multilayer, int gasGap,
                              int channel) const {
    if (!validElement(id, stationName, stationEta, stationPhi)) return false;

    if ((multilayer < multilayerMin(id)) || (multilayer > multilayerMax(id))) {
        ATH_MSG_DEBUG("Invalid multilayer=" << multilayer << " multilayerMin=" << multilayerMin(id)
                                            << " multilayerMax=" << multilayerMax(id));
        return false;
    }

    if (gasGap < gasGapMin(id) || gasGap > gasGapMax(id)) {
        ATH_MSG_DEBUG("Invalid gasGap=" << gasGap << " gasGapMin=" << gasGapMin(id) << " gasGapMax=" << gasGapMax(id));
        return false;
    }
    if (channel < channelMin(id) || channel > channelMax(id)) {
        ATH_MSG_DEBUG("Invalid channel=" << channel << " channelMin=" << channelMin(id) << " channelMax=" << channelMax(id));
        return false;
    }
    return true;
}  // end MmIdHelper::validChannel
   /*******************************************************************************/
   // Construct ID from components

Identifier MmIdHelper::elementID(int stationName, int stationEta, int stationPhi) const {
    // pack fields independently
    Identifier result((Identifier::value_type)0);
    m_muon_impl.pack(muon_field_value(), result);
    m_sta_impl.pack(stationName, result);
    m_eta_impl.pack(stationEta, result);
    m_phi_impl.pack(stationPhi, result);
    m_tec_impl.pack(mm_field_value(), result);
    return result;
}
Identifier MmIdHelper::elementID(int stationName, int stationEta, int stationPhi, bool& isValid) const {
    try {
        const Identifier result = elementID(stationName, stationEta, stationPhi);
        isValid = validElement(result, stationName, stationEta, stationPhi);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}

/*******************************************************************************/
Identifier MmIdHelper::elementID(const std::string& stationNameStr, int stationEta, int stationPhi) const {
    return elementID(stationNameIndex(stationNameStr), stationEta, stationPhi);
}
Identifier MmIdHelper::elementID(const std::string& stationNameStr, int stationEta, int stationPhi, bool& isValid) const {
    return elementID(stationNameIndex(stationNameStr), stationEta, stationPhi, isValid);
}

/*******************************************************************************/
Identifier MmIdHelper::elementID(const Identifier& id) const { return parentID(id); }
/*******************************************************************************/
Identifier MmIdHelper::channelID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channel) const {
    // pack fields independently
    Identifier result((Identifier::value_type)0);
    m_muon_impl.pack(muon_field_value(), result);
    m_sta_impl.pack(stationName, result);
    m_eta_impl.pack(stationEta, result);
    m_phi_impl.pack(stationPhi, result);
    m_tec_impl.pack(mm_field_value(), result);
    m_mplet_impl.pack(multilayer, result);
    m_gap_impl.pack(gasGap, result);
    m_cha_impl.pack(channel, result);
    return result;
}
Identifier MmIdHelper::channelID(int stationName, int stationEta, int stationPhi, int multilayer, int gasGap, int channel,
                                 bool& isValid) const {
    try{
        const Identifier result = channelID(stationName, stationEta, stationPhi, multilayer, gasGap, channel);
        isValid = validChannel(result, stationName, stationEta, stationPhi, multilayer, gasGap, channel);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}
/*******************************************************************************/
Identifier MmIdHelper::channelID(const std::string& stationNameStr, int stationEta, int stationPhi, int multilayer, int gasGap, int channel,
                                 bool& isValid) const {
    return channelID(stationNameIndex(stationNameStr), stationEta, stationPhi, multilayer, gasGap, channel, isValid);
}
Identifier MmIdHelper::channelID(const std::string& stationNameStr, int stationEta, int stationPhi, int multilayer, int gasGap,
                                 int channel) const {
    return channelID(stationNameIndex(stationNameStr), stationEta, stationPhi, multilayer, gasGap, channel);
}

/*******************************************************************************/
Identifier MmIdHelper::channelID(const Identifier& id, int multilayer, int gasGap, int channel) const {
    Identifier result{id};
    resetAndSet(m_mplet_impl, multilayer, result);
    resetAndSet(m_gap_impl, gasGap, result);
    resetAndSet(m_cha_impl, channel, result);
    return result;
}
Identifier MmIdHelper::channelID(const Identifier& id, int multilayer, int gasGap, int channel, bool& isValid) const {
    try {
        const Identifier result = channelID(id, multilayer, gasGap, channel);
        isValid = valid(result);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}
/*******************************************************************************/
// get parent id from strip or gang identifier
Identifier MmIdHelper::parentID(const Identifier& id) const {
    assert(is_mm(id));
    Identifier result(id);
    m_mplet_impl.reset(result);
    m_gap_impl.reset(result);
    m_cha_impl.reset(result);
    return result;
}
/*******************************************************************************/
// Access to components of the ID
int MmIdHelper::multilayer(const Identifier& id) const { return m_mplet_impl.unpack(id); }
/*******************************************************************************/
int MmIdHelper::gasGap(const Identifier& id) const { return m_gap_impl.unpack(id); }
/*******************************************************************************/
int MmIdHelper::channel(const Identifier& id) const { return m_cha_impl.unpack(id); }
/*******************************************************************************/
// Access to min and max of level ranges
int MmIdHelper::stationEtaMin() { return StationEtaMin; }
/*******************************************************************************/
int MmIdHelper::stationEtaMax() { return StationEtaMax; }
/*******************************************************************************/
int MmIdHelper::stationPhiMin() { return StationPhiMin; }
/*******************************************************************************/
int MmIdHelper::stationPhiMax() { return StationPhiMax; }
/*******************************************************************************/
int MmIdHelper::multilayerMin() { return MultilayerMin; }
/*******************************************************************************/
int MmIdHelper::multilayerMax() { return MultilayerMax; }
/*******************************************************************************/
int MmIdHelper::gasGapMin() { return GasGapMin; }
/*******************************************************************************/
bool MmIdHelper::isStereo(const Identifier& id) const {
    bool isStereo = false;
    int ml = multilayer(id);
    int gg = gasGap(id);
    if ((ml == 1 && gg > 2) || (ml == 2 && gg < 3)) isStereo = true;
    return isStereo;
}
/*******************************************************************************/
bool MmIdHelper::measuresPhi(const Identifier& /*id*/) const { return false; }
/*******************************************************************************/
int MmIdHelper::gasGapMax() { return GasGapMax; }
/*******************************************************************************/
int MmIdHelper::channelMin() { return ChannelMin; }
/*******************************************************************************/
int MmIdHelper::channelMax() { return ChannelMax; }
/*******************************************************************************/
/// Utility methods
int MmIdHelper::mmTechnology() const {
    int mmField = technologyIndex("MM");
    if (m_dict) { mmField = mm_field_value(); }
    return mmField;
}
/*******************************************************************************/
bool MmIdHelper::LargeSector(int stationName) const { return ('L' == stationNameString(stationName)[2]); }
/*******************************************************************************/
bool MmIdHelper::SmallSector(int stationName) const { return ('S' == stationNameString(stationName)[2]); }
/*******************************************************************************/
// Nektar: Modified for MicroMegas, but is almost certainly wrong
int MmIdHelper::sectorType(const std::string& stationName, int stationEta) {
    if ('L' == stationName[2]) {
        return (abs(stationEta) + 1);
    } else if ('S' == stationName[2]) {
        return (abs(stationEta) + 12);
    }
    assert(0);
    return -1;
}
/*******************************************************************************/
int MmIdHelper::sectorType(int stationName, int stationEta) const {
    std::string name = stationNameString(stationName);
    return sectorType(name, stationEta);
}
