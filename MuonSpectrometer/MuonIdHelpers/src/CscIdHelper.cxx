/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonIdHelpers/CscIdHelper.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictField.h"
#include "IdDict/IdDictGroup.h"
#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictRegion.h"
#include "Identifier/RangeIterator.h"

CscIdHelper::CscIdHelper() : MuonIdHelper("CscIdHelper", "csc") {
    m_module_hashes.fill(-1);
    m_detectorElement_hashes.fill(-1);
}

/// Initialize dictionary
int CscIdHelper::initialize_from_dictionary(const IdDictMgr& dict_mgr) {
    int status = 0;

    // Check whether this helper should be reinitialized
    if (!reinitialize(dict_mgr)) {
        ATH_MSG_INFO("Request to reinitialize not satisfied - tags have not changed");
        return 0;
    } else {
        ATH_MSG_DEBUG("(Re)initialize");
    }

    /// init base object
    if (AtlasDetectorID::initialize_from_dictionary(dict_mgr)) return 1;

    // Register version of the MuonSpectrometer dictionary
    if (register_dict_tag(dict_mgr, "MuonSpectrometer")) return 1;

    m_dict = dict_mgr.find_dictionary("MuonSpectrometer");
    if (!m_dict) {
        ATH_MSG_ERROR(" initialize_from_dict - cannot access MuonSpectrometer dictionary ");
        return 1;
    }

    /// Initialize some of the field indices

    if (initLevelsFromDict()) return 1;

    int index = technologyIndex("CSC");
    if (index == -1) {
        ATH_MSG_DEBUG("initLevelsFromDict - there are no CSC entries in the dictionary! ");
        return 0;
    }

    IdDictField* field = m_dict->find_field("chamberLayer");
    if (field) {
        m_CHAMBERLAYER_INDEX = field->index();
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'chamberLayer' field ");
        status = 1;
    }

    field = m_dict->find_field("wireLayer");
    if (field) {
        m_WIRELAYER_INDEX = field->index();
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'wireLayer' field ");
        status = 1;
    }

    field = m_dict->find_field("cscMeasuresPhi");
    if (field) {
        m_MEASURESPHI_INDEX = field->index();
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'cscMeasuresPhi' field ");
        status = 1;
    }

    field = m_dict->find_field("cscStrip");
    if (field) {
        m_CHANNEL_INDEX = field->index();
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'cscStrip' field ");
        status = 1;
    }

    // initialize the multilayer index
    m_DETECTORELEMENT_INDEX = m_CHAMBERLAYER_INDEX;

    /// save an index to the first region of csc

    IdDictGroup* cscGroup = m_dict->find_group("csc");
    if (!cscGroup) {
        ATH_MSG_ERROR("Cannot find csc group");
    } else {
        m_GROUP_INDEX = cscGroup->regions()[0]->index();
    }

    const IdDictRegion& region = m_dict->region(m_GROUP_INDEX);
    m_eta_impl = region.implementation(m_ETA_INDEX);
    m_phi_impl = region.implementation(m_PHI_INDEX);
    m_tec_impl = region.implementation(m_TECHNOLOGY_INDEX);
    m_cla_impl = region.implementation(m_CHAMBERLAYER_INDEX);
    m_lay_impl = region.implementation(m_WIRELAYER_INDEX);
    m_mea_impl = region.implementation(m_MEASURESPHI_INDEX);
    m_str_impl = region.implementation(m_CHANNEL_INDEX);

    ATH_MSG_DEBUG(" CSC decode index and bit fields for each level: " << std::endl
                                                                      << " muon        " << m_muon_impl.show_to_string() << std::endl
                                                                      << " station     " << m_sta_impl.show_to_string() << std::endl
                                                                      << " eta         " << m_eta_impl.show_to_string() << std::endl
                                                                      << " phi         " << m_phi_impl.show_to_string() << std::endl
                                                                      << " technology  " << m_tec_impl.show_to_string() << std::endl
                                                                      << " cham layer  " << m_cla_impl.show_to_string() << std::endl
                                                                      << " layer       " << m_lay_impl.show_to_string() << std::endl
                                                                      << " phi         " << m_mea_impl.show_to_string() << std::endl
                                                                      << " strip       " << m_str_impl.show_to_string());

    /**
     * Build multirange for the valid set of identifiers
     */

    /// Find value for the field MuonSpectrometer

    int muonField = -1;
    const IdDictDictionary* atlasDict = dict_mgr.find_dictionary("ATLAS");
    if (atlasDict->get_label_value("subdet", "MuonSpectrometer", muonField)) {
        ATH_MSG_ERROR("Could not get value for label 'MuonSpectrometer' of field "
                      << "'subdet' in dictionary " << atlasDict->name());
        return 1;
    }

    /// Build MultiRange down to "technology" for all (muon) regions

    ExpandedIdentifier region_id;
    region_id.add(muonField);
    Range prefix;
    MultiRange muon_range = m_dict->build_multirange(region_id, prefix, "technology");
    if (muon_range.size() > 0) {
        ATH_MSG_INFO("MultiRange built successfully to Technology: "
                     << "MultiRange size is " << muon_range.size());
    } else {
        ATH_MSG_ERROR("Muon MultiRange is empty for modules");
    }

    // Build MultiRange down to "detector element" for all mdt regions

    ExpandedIdentifier detectorElement_region;
    detectorElement_region.add(muonField);
    Range detectorElement_prefix;
    MultiRange muon_detectorElement_range = m_dict->build_multirange(detectorElement_region, detectorElement_prefix, "chamberLayer");
    if (muon_detectorElement_range.size() > 0) {
        ATH_MSG_INFO("MultiRange built successfully to detector element: "
                     << "Multilayer MultiRange size is " << muon_detectorElement_range.size());
    } else {
        ATH_MSG_ERROR("Muon CSC detector element MultiRange is empty");
    }

    /// Build MultiRange down to "cscStrip" for all CSC regions

    ExpandedIdentifier csc_region;
    csc_region.add(muonField);
    Range csc_prefix;
    MultiRange muon_channel_range = m_dict->build_multirange(csc_region, csc_prefix, "cscStrip");
    if (muon_channel_range.size() > 0) {
        ATH_MSG_INFO("MultiRange built successfully to cscStrip: "
                     << "MultiRange size is " << muon_channel_range.size());
    } else {
        ATH_MSG_ERROR("Muon MultiRange is empty for channels");
    }

    /**
     * Build CSC module ranges:
     *
     * Find the regions that have a "technology field" that matches the MDT
     * and save them
     */

    int cscField = -1;
    status = m_dict->get_label_value("technology", "CSC", cscField);

    for (int i = 0; i < (int)muon_range.size(); ++i) {
        const Range& range = muon_range[i];
        if (range.fields() > m_TECHNOLOGY_INDEX) {
            const Range::field& field = range[m_TECHNOLOGY_INDEX];
            if (field.match((ExpandedIdentifier::element_type)cscField)) {
                m_full_module_range.add(range);
                ATH_MSG_DEBUG("field size is " << (int)range.cardinality() << " field index = " << i);
            }
        }
    }

    for (int j = 0; j < (int)muon_detectorElement_range.size(); ++j) {
        const Range& range = muon_detectorElement_range[j];
        if (range.fields() > m_TECHNOLOGY_INDEX) {
            const Range::field& field = range[m_TECHNOLOGY_INDEX];
            if (field.match((ExpandedIdentifier::element_type)cscField)) {
                m_full_detectorElement_range.add(range);
                ATH_MSG_DEBUG("detector element field size is " << (int)range.cardinality() << " field index = " << j);
            }
        }
    }

    for (int j = 0; j < (int)muon_channel_range.size(); ++j) {
        const Range& range = muon_channel_range[j];
        if (range.fields() > m_TECHNOLOGY_INDEX) {
            const Range::field& field = range[m_TECHNOLOGY_INDEX];
            if (field.match((ExpandedIdentifier::element_type)cscField)) {
                m_full_channel_range.add(range);
                ATH_MSG_DEBUG("channel field size is " << (int)range.cardinality() << " field index = " << j);
            }
        }
    }

    /// test to see that the multi range is not empty

    if (m_full_module_range.size() == 0) {
        ATH_MSG_ERROR("CSC MultiRange ID is empty for modules");
        status = 1;
    }

    /// test to see that the detector element multi range is not empty

    if (m_full_detectorElement_range.size() == 0) {
        ATH_MSG_ERROR("CSC MultiRange ID is empty for detector elements");
        status = 1;
    }

    /// test to see that the multi range is not empty

    if (m_full_channel_range.size() == 0) {
        ATH_MSG_ERROR("CSC MultiRange ID is empty for channels");
        status = 1;
    }

    /// Setup the hash tables for CSC

    ATH_MSG_INFO("Initializing CSC hash indices ... ");
    status = init_hashes();
    status = init_detectorElement_hashes();  // for chamber layer - a chamber
    status = init_channel_hashes();
    status = strip_hash_offsets();
    status = init_id_to_hashes();

    /// Setup hash tables for finding neighbors

    ATH_MSG_INFO("Initializing CSC hash indices for finding neighbors ... ");
    status = init_neighbors();

    // now we have to set the stripMax values (for the stripMax(id) function)
    // this could be also done on an event-by-event basis as for stripMin(id)
    // however, for all existing layouts there are only 2 possible values for stripMax,
    // namely those for layers which measure phi (measuresPhi(id)=true) and the rest.
    // thus, we initialize 2 member variables here to speed up calling the function during runtime
    // loop on the channel Identifiers and check (for consistency!) that really only
    // two maximum numbers of strips (w/o measuresPhi) are around
    ExpandedIdentifier expId;
    IdContext strip_context = channel_context();
    for (const auto& id : m_channel_vec) {
        if (get_expanded_id(id, expId, &strip_context)) {
            ATH_MSG_ERROR("Failed to retrieve ExpandedIdentifier from Identifier " << id.get_compact());
            return 1;
        }
        bool measPhi = measuresPhi(id);
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& phi_field = range[m_CHANNEL_INDEX];
                if (not !phi_field.empty()) {
                    ATH_MSG_ERROR("Range::field for phi at position " << i << " does not have a maximum");
                    return 1;
                }
                unsigned int max = phi_field.get_maximum();
                if (measPhi) {
                    if (m_stripMaxPhi != UINT_MAX && m_stripMaxPhi != max) {
                        ATH_MSG_ERROR("Maximum of Range::field for phi (" << max << ") is not equal to m_stripMaxPhi=" << m_stripMaxPhi);
                        return 1;
                    } else
                        m_stripMaxPhi = max;
                } else {
                    if (m_stripMaxEta != UINT_MAX && m_stripMaxEta != max) {
                        ATH_MSG_ERROR("Maximum of Range::field for phi (" << max << ") is not equal to m_stripMaxEta=" << m_stripMaxEta);
                        return 1;
                    } else
                        m_stripMaxEta = max;
                }
            }
        }
    }

    // check whether the current layout contains chamberLayer 1 Identifiers (pre-Run3) in the vector of module Identifiers
    if (!m_module_vec.empty() && chamberLayer(m_module_vec.at(0)) == 1) m_hasChamLay1 = true;
    m_init = true;
    return (status);
}

inline unsigned int CscIdHelper::moduleHashIdx(const Identifier& id) const{
    /// Unfold the array [A][B][C] by
    /// a * BxC + b * C + c
    constexpr unsigned int C = s_phiDim;
    constexpr unsigned int BxC = C*s_etaDim; 
    const int stEta = stationEta(id);
    return (stationName(id) - m_stationShift)*BxC + (stEta + s_etaDim/2 - (stEta>0))*C + (stationPhi(id) -1);
}
inline unsigned int CscIdHelper::detEleHashIdx(const Identifier& id) const{
    return moduleHashIdx(id)  *s_mlDim +  (chamberLayer(id) -1);
}

int CscIdHelper::init_id_to_hashes() {
    
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
}

int CscIdHelper::get_module_hash(const Identifier& id, IdentifierHash& hash_id) const {
    const unsigned int idx = moduleHashIdx(id);
    if (idx >= m_module_hashes.size()) return 1;
    hash_id = m_module_hashes[idx];
    return 0;
}
int CscIdHelper::get_detectorElement_hash(const Identifier& id, IdentifierHash& hash_id) const {
    const unsigned int idx = detEleHashIdx(id);
    if (idx >= m_detectorElement_hashes.size()) return 1;
    hash_id = m_detectorElement_hashes[idx];
    return 0;
}



void CscIdHelper::idChannels(const Identifier& id, std::vector<Identifier>& vect) const {
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

int CscIdHelper::stationEtaMin(const Identifier& id) const {
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
    return (999);  /// default
}

int CscIdHelper::stationEtaMax(const Identifier& id) const {
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
}

int CscIdHelper::stationPhiMin(const Identifier& id) const {
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
    /// Failed to find the min
    return (999);
}

int CscIdHelper::stationPhiMax(const Identifier& id) const {
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
    /// Failed to find the max
    return (-999);
}

int CscIdHelper::chamberLayerMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext chamberLayer_context(expId, 0, m_CHAMBERLAYER_INDEX);
    if (!get_expanded_id(id, expId, &chamberLayer_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& phi_field = range[m_CHAMBERLAYER_INDEX];
                if (not phi_field.empty()) { return (phi_field.get_minimum()); }
            }
        }
    }
    /// Failed to find the min
    return (999);
}

int CscIdHelper::chamberLayerMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext chamberLayer_context(expId, 0, m_CHAMBERLAYER_INDEX);
    if (!get_expanded_id(id, expId, &chamberLayer_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& phi_field = range[m_CHAMBERLAYER_INDEX];
                if (not phi_field.empty()) { return (phi_field.get_maximum()); }
            }
        }
    }
    /// Failed to find the max
    return (-999);
}

int CscIdHelper::wireLayerMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext wireLayer_context(expId, 0, m_WIRELAYER_INDEX);
    if (!get_expanded_id(id, expId, &wireLayer_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& phi_field = range[m_WIRELAYER_INDEX];
                if (not phi_field.empty()) { return (phi_field.get_minimum()); }
            }
        }
    }
    /// Failed to find the min
    return (999);
}

int CscIdHelper::wireLayerMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext chamberLayer_context(expId, 0, m_WIRELAYER_INDEX);
    if (!get_expanded_id(id, expId, &chamberLayer_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& phi_field = range[m_WIRELAYER_INDEX];
                if (not phi_field.empty()) { return (phi_field.get_maximum()); }
            }
        }
    }
    /// Failed to find the max
    return (-999);
}

int CscIdHelper::measuresPhiMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext measuresPhi_context(expId, 0, m_MEASURESPHI_INDEX);
    if (!get_expanded_id(id, expId, &measuresPhi_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& phi_field = range[m_MEASURESPHI_INDEX];
                if (not phi_field.empty()) { return (phi_field.get_minimum()); }
            }
        }
    }
    /// Failed to find the min
    return (999);
}

int CscIdHelper::measuresPhiMax(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext measuresPhi_context(expId, 0, m_MEASURESPHI_INDEX);
    if (!get_expanded_id(id, expId, &measuresPhi_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& phi_field = range[m_MEASURESPHI_INDEX];
                if (not phi_field.empty()) { return (phi_field.get_maximum()); }
            }
        }
    }
    /// Failed to find the max
    return (-999);
}

int CscIdHelper::stripMin(const Identifier& id) const {
    ExpandedIdentifier expId;
    IdContext strip_context = channel_context();
    if (!get_expanded_id(id, expId, &strip_context)) {
        for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
            const Range& range = m_full_channel_range[i];
            if (range.match(expId)) {
                const Range::field& phi_field = range[m_CHANNEL_INDEX];
                if (not phi_field.empty()) { return (phi_field.get_minimum()); }
            }
        }
    }
    /// Failed to find the min
    return (999);
}

int CscIdHelper::stripMax(const Identifier& id) const {
    if (measuresPhi(id))
        return m_stripMaxPhi;
    else
        return m_stripMaxEta;
}

/// Public validation of levels

bool CscIdHelper::valid(const Identifier& id) const {
    if (!validElement(id)) return false;

    int cLayer = chamberLayer(id);
    if ((cLayer < chamberLayerMin(id)) || (cLayer > chamberLayerMax(id))) {
        ATH_MSG_DEBUG("Invalid chamberLayer=" << cLayer << " chamberLayerMin=" << chamberLayerMin(id)
                                              << " chamberLayerMax=" << chamberLayerMax(id));
        return false;
    }

    int wLayer = wireLayer(id);
    if ((wLayer < wireLayerMin(id)) || (wLayer > wireLayerMax(id))) {
        ATH_MSG_DEBUG("Invalid wireLayer=" << wLayer << " wireLayerMin=" << wireLayerMin(id) << " wireLayerMax=" << wireLayerMax(id));
        return false;
    }

    int mPhi = measuresPhi(id);
    if ((mPhi < measuresPhiMin(id)) || (mPhi > measuresPhiMax(id))) {
        ATH_MSG_DEBUG("Invalid measuresPhi=" << mPhi << " measuresPhiMin=" << measuresPhiMin(id)
                                             << " measuresPhiMax=" << measuresPhiMax(id));
        return false;
    }

    int channel = strip(id);
    if ((channel > stripMax(id)) || (channel < stripMin(id))) {
        ATH_MSG_DEBUG("Invalid strip=" << channel << " stripMin=" << stripMin(id) << " stripMax=" << stripMax(id));
        return false;
    }
    return true;
}
bool CscIdHelper::isStNameInTech(const std::string& stationName) const { return stationName[0] == 'C'; }
bool CscIdHelper::validElement(const Identifier& id) const {
    int station = stationName(id);
    if (!validStation(station)) {
        ATH_MSG_DEBUG("Invalid stationName=" << stationNameString(station));
        return false;
    }

    int eta = stationEta(id);
    if ((eta < stationEtaMin(id)) || (eta > stationEtaMax(id)) || (0 == eta)) {
        ATH_MSG_DEBUG("Invalid stationEta=" << eta << " for stationName=" << stationNameString(station)
                                            << " stationEtaMin=" << stationEtaMin(id) << " stationEtaMax=" << stationEtaMax(id));
        return false;
    }

    int phi = stationPhi(id);
    if ((phi < stationPhiMin(id)) || (phi > stationPhiMax(id))) {
        ATH_MSG_DEBUG("Invalid stationPhi=" << phi << " for stationName=" << stationNameString(station)
                                            << " stationPhiMin=" << stationPhiMin(id) << " stationPhiMax=" << stationPhiMax(id));
        return false;
    }
    return true;
}

/// Private validation of levels

bool CscIdHelper::validElement(const Identifier& id, int stationName, int stationEta, int stationPhi) const {
    if (!validStation(stationName)) {
        ATH_MSG_DEBUG("Invalid stationName=" << stationNameString(stationName));
        return false;
    }
    if ((stationEta < stationEtaMin(id)) || (stationEta > stationEtaMax(id)) || (0 == stationEta)) {
        ATH_MSG_DEBUG("Invalid stationEta=" << stationEta << " for stationName=" << stationNameString(stationName)
                                            << " stationEtaMin=" << stationEtaMin(id) << " stationEtaMax=" << stationEtaMax(id));
        return false;
    }
    if ((stationPhi < stationPhiMin(id)) || (stationPhi > stationPhiMax(id))) {
        ATH_MSG_DEBUG("Invalid stationPhi=" << stationPhi << " for stationName=" << stationNameString(stationName)
                                            << " stationPhiMin=" << stationPhiMin(id) << " stationPhiMax=" << stationPhiMax(id));
        return false;
    }
    return true;
}

bool CscIdHelper::validChannel(const Identifier& id, int stationName, int stationEta, int stationPhi, int chamberLayer, int wireLayer,
                               int measuresPhi, int strip) const {
    if (!validElement(id, stationName, stationEta, stationPhi)) return false;

    if ((chamberLayer < chamberLayerMin(id)) || (chamberLayer > chamberLayerMax(id))) {
        ATH_MSG_DEBUG("Invalid chamberLayer=" << chamberLayer << " chamberLayerMin=" << chamberLayerMin(id)
                                              << " chamberLayerMax=" << chamberLayerMax(id));
        return false;
    }
    if ((wireLayer < wireLayerMin(id)) || (wireLayer > wireLayerMax(id))) {
        ATH_MSG_DEBUG("Invalid wireLayer=" << wireLayer << " wireLayerMin=" << wireLayerMin(id) << " wireLayerMax=" << wireLayerMax(id));
        return false;
    }
    if ((measuresPhi < measuresPhiMin(id)) || (measuresPhi > measuresPhiMax(id))) {
        ATH_MSG_DEBUG("Invalid measuresPhi=" << measuresPhi << " measuresPhiMin=" << measuresPhiMin(id)
                                             << " measuresPhiMax=" << measuresPhiMax(id));
        return false;
    }
    if ((strip > stripMax(id)) || (strip < stripMin(id))) {
        ATH_MSG_DEBUG("Invalid strip=" << strip << " stripMin=" << stripMin(id) << " stripMax=" << stripMax(id));
        return false;
    }
    return true;
}

// calculate the hash offset
int CscIdHelper::strip_hash_offsets() {
    m_hashOffset[0][0] = 0;
    std::string version = m_dict->version();

    if (version == "H8 2004") {
        m_hashOffset[0][1] = 1536;
        m_hashOffset[1][0] = m_hashOffset[0][0];
        m_hashOffset[1][1] = m_hashOffset[0][1];
    } else if (version == "CSC Cosmic") {
        m_hashOffset[0][1] = 3072;
        m_hashOffset[1][0] = m_hashOffset[0][0];
        m_hashOffset[1][1] = m_hashOffset[0][1];
    } else if (version == "P.03" || version == "H8 2003" || version == "H8 2002" || version == "M2.8") {
        m_hashOffset[0][1] = 27392;
        m_hashOffset[1][0] = m_hashOffset[0][1] + 3584;
        m_hashOffset[1][1] = m_hashOffset[0][1] + m_hashOffset[1][0];
    } else {
        m_hashOffset[0][1] = 24576;
        m_hashOffset[1][0] = m_hashOffset[0][1] + 6144;
        m_hashOffset[1][1] = m_hashOffset[0][1] + m_hashOffset[1][0];
    }
    return 0;
}
Identifier CscIdHelper::elementID(int stationName, int stationEta, int stationPhi) const {
    // pack fields independently
    Identifier result((Identifier::value_type)0);
    m_muon_impl.pack(muon_field_value(), result);
    m_sta_impl.pack(stationName, result);
    m_eta_impl.pack(stationEta, result);
    m_phi_impl.pack(stationPhi, result);
    m_tec_impl.pack(csc_field_value(), result);
    return result;
}
Identifier CscIdHelper::elementID(int stationName, int stationEta, int stationPhi, bool& isValid) const {
    try {
        const Identifier result = elementID(stationName, stationEta, stationPhi);
        isValid = validElement(result, stationName, stationEta, stationPhi);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}

Identifier CscIdHelper::elementID(const std::string& stationNameStr, int stationEta, int stationPhi) const {
    return elementID(stationNameIndex(stationNameStr), stationEta, stationPhi);
}
Identifier CscIdHelper::elementID(const std::string& stationNameStr, int stationEta, int stationPhi, bool& isValid) const {
    return elementID(stationNameIndex(stationNameStr), stationEta, stationPhi, isValid);
}

Identifier CscIdHelper::elementID(const Identifier& id) const { return parentID(id); }

Identifier CscIdHelper::channelID(int stationName, int stationEta, int stationPhi, int chamberLayer, int wireLayer, int measuresPhi,
                                  int strip) const {
    // pack fields independently
    Identifier result((Identifier::value_type)0);
    m_muon_impl.pack(muon_field_value(), result);
    m_sta_impl.pack(stationName, result);
    m_eta_impl.pack(stationEta, result);
    m_phi_impl.pack(stationPhi, result);
    m_tec_impl.pack(csc_field_value(), result);
    m_cla_impl.pack(chamberLayer, result);
    m_lay_impl.pack(wireLayer, result);
    m_mea_impl.pack(measuresPhi, result);
    m_str_impl.pack(strip, result);
    return result;
}
Identifier CscIdHelper::channelID(int stationName, int stationEta, int stationPhi, int chamberLayer, int wireLayer, int measuresPhi,
                                  int strip, bool& isValid) const {
    try {
        const Identifier result = channelID(stationName, stationEta, stationPhi, chamberLayer, wireLayer, measuresPhi, strip);
        isValid = validChannel(result, stationName, stationEta, stationPhi, chamberLayer, wireLayer, measuresPhi, strip);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}

Identifier CscIdHelper::channelID(const std::string& stationNameStr, int stationEta, int stationPhi, int chamberLayer, int wireLayer,
                                  int measuresPhi, int strip) const {
    return channelID(stationNameIndex(stationNameStr), stationEta, stationPhi, chamberLayer, wireLayer, measuresPhi, strip);
}
Identifier CscIdHelper::channelID(const std::string& stationNameStr, int stationEta, int stationPhi, int chamberLayer, int wireLayer,
                                  int measuresPhi, int strip, bool& isValid) const {
    return channelID(stationNameIndex(stationNameStr), stationEta, stationPhi, chamberLayer, wireLayer, measuresPhi, strip, isValid);
}

Identifier CscIdHelper::channelID(const Identifier& id, int chamberLayer, int wireLayer, int measuresPhi, int strip) const {
    Identifier result(id);
    resetAndSet(m_cla_impl, chamberLayer, result);
    resetAndSet(m_lay_impl, wireLayer, result);
    resetAndSet(m_mea_impl, measuresPhi, result);
    resetAndSet(m_str_impl, strip, result);
    return result;
}
Identifier CscIdHelper::channelID(const Identifier& id, int chamberLayer, int wireLayer, int measuresPhi, int strip, bool& isValid) const {
    try {
        const Identifier result = channelID(id, chamberLayer, wireLayer, measuresPhi, strip);
        isValid = valid(result);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}
/// get parent id from channel id

Identifier CscIdHelper::parentID(const Identifier& id) const {
    assert(is_csc(id));
    Identifier result(id);
    m_cla_impl.reset(result);
    m_lay_impl.reset(result);
    m_mea_impl.reset(result);
    m_str_impl.reset(result);
    return result;
}

// Access to components of the ID

int CscIdHelper::chamberLayer(const Identifier& id) const { return m_cla_impl.unpack(id); }

int CscIdHelper::wireLayer(const Identifier& id) const { return m_lay_impl.unpack(id); }

bool CscIdHelper::measuresPhi(const Identifier& id) const { return m_mea_impl.unpack(id); }

int CscIdHelper::strip(const Identifier& id) const { return m_str_impl.unpack(id); }

int CscIdHelper::channel(const Identifier& id) const { return strip(id); }

/// Access to min and max of level ranges

int CscIdHelper::stationEtaMin() { return StationEtaMin; }

int CscIdHelper::stationEtaMax() { return StationEtaMax; }

int CscIdHelper::stationPhiMin() { return StationPhiMin; }

int CscIdHelper::stationPhiMax() { return StationPhiMax; }

int CscIdHelper::chamberLayerMin() { return ChamberLayerMin; }

int CscIdHelper::chamberLayerMax() { return ChamberLayerMax; }

int CscIdHelper::wireLayerMin() { return WireLayerMin; }

int CscIdHelper::wireLayerMax() { return WireLayerMax; }

int CscIdHelper::measuresPhiMin() { return MeasuresPhiMin; }

int CscIdHelper::measuresPhiMax() { return MeasuresPhiMax; }

int CscIdHelper::stripMin() { return StripMin; }

int CscIdHelper::stripMax() { return StripMax; }

/// Utility methods

int CscIdHelper::cscTechnology() const {
    int cscField = technologyIndex("CSC");
    if (m_dict) { cscField = csc_field_value(); }
    return cscField;
}

int CscIdHelper::sector(const Identifier& id) const { return stationEta(id) * (2 * stationPhi(id) - (stationName(id) - 49) + 1); }

int CscIdHelper::gasGap(const Identifier& id) const { return chamberLayer(id); }
