/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonIdHelpers/MdtIdHelper.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictField.h"
#include "IdDict/IdDictGroup.h"
#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictRegion.h"
#include "Identifier/RangeIterator.h"

MdtIdHelper::MdtIdHelper() : MuonIdHelper("MdtIdHelper", "mdt") {
    //m_detectorElement_hashes
    m_module_hashes.fill(-1);
    m_detectorElement_hashes.fill(-1);
}

/// initialize dictionary
int MdtIdHelper::initialize_from_dictionary(const IdDictMgr& dict_mgr) {
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

    IdDictField* field = m_dict->find_field("multiLayer");
    if (field) {
        m_DETECTORELEMENT_INDEX = field->index();
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'multiLayer' field ");
        status = 1;
    }

    field = m_dict->find_field("tubeLayer");
    if (field) {
        m_TUBELAYER_INDEX = field->index();
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'tubeLayer' field ");
        status = 1;
    }

    field = m_dict->find_field("tube");
    if (field) {
        m_CHANNEL_INDEX = field->index();
    } else {
        ATH_MSG_ERROR("initLevelsFromDict - unable to find 'tube' field ");
        status = 1;
    }

    // save an index to the first region of mdt

    IdDictGroup* mdtGroup = m_dict->find_group("mdt");
    if (!mdtGroup) {
        ATH_MSG_ERROR("Cannot find mdt group");
    } else {
        m_GROUP_INDEX = mdtGroup->regions()[0]->index();
    }

    const IdDictRegion& region = m_dict->region(m_GROUP_INDEX);
    m_eta_impl = region.implementation(m_ETA_INDEX);
    m_phi_impl = region.implementation(m_PHI_INDEX);
    m_tec_impl = region.implementation(m_TECHNOLOGY_INDEX);
    m_mla_impl = region.implementation(m_DETECTORELEMENT_INDEX);
    m_lay_impl = region.implementation(m_TUBELAYER_INDEX);
    m_tub_impl = region.implementation(m_CHANNEL_INDEX);

    ATH_MSG_DEBUG(" MDT decode index and bit fields for each level: " << std::endl
                                                                      << " muon        " << m_muon_impl.show_to_string() << std::endl
                                                                      << " station     " << m_sta_impl.show_to_string() << std::endl
                                                                      << " eta         " << m_eta_impl.show_to_string() << std::endl
                                                                      << " phi         " << m_phi_impl.show_to_string() << std::endl
                                                                      << " technology  " << m_tec_impl.show_to_string() << std::endl
                                                                      << " multilayer  " << m_mla_impl.show_to_string() << std::endl
                                                                      << " layer       " << m_lay_impl.show_to_string() << std::endl
                                                                      << " tube        " << m_tub_impl.show_to_string() << std::endl);

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
    MultiRange muon_detectorElement_range = m_dict->build_multirange(detectorElement_region, detectorElement_prefix, "multiLayer");
    if (muon_detectorElement_range.size() > 0) {
        ATH_MSG_INFO("MultiRange built successfully to detector element: "
                     << "Multilayer MultiRange size is " << muon_detectorElement_range.size());
    } else {
        ATH_MSG_ERROR("Muon MDT detector element MultiRange is empty");
    }

    // Build MultiRange down to "tube" for all mdt regions

    ExpandedIdentifier mdt_region;
    mdt_region.add(muonField);
    Range mdt_prefix;
    MultiRange muon_channel_range = m_dict->build_multirange(mdt_region, mdt_prefix, "tube");
    if (muon_channel_range.size() > 0) {
        ATH_MSG_INFO("MultiRange built successfully to tube: "
                     << "MultiRange size is " << muon_channel_range.size());
    } else {
        ATH_MSG_ERROR("Muon MDT channel MultiRange is empty");
    }

    /**
     * Build MDT module ranges:
     *
     * Find the regions that have a "technology field" that matches the MDT
     * and save them
     */

    int mdtField = -1;
    status = m_dict->get_label_value("technology", "MDT", mdtField);

    for (int i = 0; i < (int)muon_range.size(); ++i) {
        const Range& range = muon_range[i];
        if (range.fields() > m_TECHNOLOGY_INDEX) {
            const Range::field& field = range[m_TECHNOLOGY_INDEX];
            if (field.match((ExpandedIdentifier::element_type)mdtField)) {
                m_full_module_range.add(range);
                ATH_MSG_DEBUG("module field size is " << (int)range.cardinality() << " field index = " << i);
            }
        }
    }

    for (int j = 0; j < (int)muon_detectorElement_range.size(); ++j) {
        const Range& range = muon_detectorElement_range[j];
        if (range.fields() > m_TECHNOLOGY_INDEX) {
            const Range::field& field = range[m_TECHNOLOGY_INDEX];
            if (field.match((ExpandedIdentifier::element_type)mdtField)) {
                m_full_detectorElement_range.add(range);
                ATH_MSG_DEBUG("detector element field size is " << (int)range.cardinality() << " field index = " << j);
            }
        }
    }

    for (int k = 0; k < (int)muon_channel_range.size(); ++k) {
        const Range& range = muon_channel_range[k];
        if (range.fields() > m_TECHNOLOGY_INDEX) {
            const Range::field& field = range[m_TECHNOLOGY_INDEX];
            if (field.match((ExpandedIdentifier::element_type)mdtField)) {
                m_full_channel_range.add(range);
                ATH_MSG_DEBUG("channel field size is " << (int)range.cardinality() << " field index = " << k);
            }
        }
    }

    /// test to see that the module multi range is not empty

    if (m_full_module_range.size() == 0) {
        ATH_MSG_ERROR("MDT MultiRange ID is empty for modules");
        status = 1;
    }

    /// test to see that the detector element multi range is not empty

    if (m_full_detectorElement_range.size() == 0) {
        ATH_MSG_ERROR("MDT MultiRange ID is empty for detector elements");
        status = 1;
    }

    /// test to see that the tube multi range is not empty

    if (m_full_channel_range.size() == 0) {
        ATH_MSG_ERROR("MDT MultiRange ID is empty for channels");
        status = 1;
    }

    // To speed up the range scans needed by the MinMax functions,
    // keep indices of ranges by station name.

    m_module_ranges_by_station.resize (nStationNames());
    for (const Range& r : m_full_module_range) {
      const Range::field& station_f = r[m_NAME_INDEX];
      assert (not station_f.empty() && 
              station_f.get_minimum() == station_f.get_maximum());
      unsigned int station = station_f.get_minimum();
      if (station >= m_module_ranges_by_station.size()) {
        m_module_ranges_by_station.resize (station+1);
      }
      m_module_ranges_by_station[station].push_back (&r);
    }

    m_channel_ranges_by_station.resize (nStationNames());
    for (const Range& r : m_full_channel_range) {
      const Range::field& station_f = r[m_NAME_INDEX];
      assert (not station_f.empty() &&
              station_f.get_minimum() == station_f.get_maximum());
      unsigned int station = station_f.get_minimum();
      if (station >= m_channel_ranges_by_station.size()) {
        m_channel_ranges_by_station.resize (station+1);
      }
      m_channel_ranges_by_station[station].push_back (&r);
    }

    // Setup the hash tables for MDT

    ATH_MSG_INFO("Initializing MDT hash indices ... ");
    status = init_hashes();
    status = init_detectorElement_hashes();
    status = init_id_to_hashes();

    // Setup hash tables for finding neighbors

    ATH_MSG_INFO("Initializing MDT hash indices for finding neighbors ... ");
    status = init_neighbors();

    // retrieve the maximum number of tubes in any chamber
    ExpandedIdentifier expId;
    IdContext channel_context(expId, 0, m_CHANNEL_INDEX);
    for (const auto& id : m_detectorElement_vec) {
        if (!get_expanded_id(id, expId, &channel_context)) {
            for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
                const Range& range = m_full_channel_range[i];
                if (range.match(expId)) {
                    const Range::field& channel_field = range[m_CHANNEL_INDEX];
                    if (not channel_field.empty()) {
                        unsigned int max = channel_field.get_maximum();
                        if (m_tubesMax == UINT_MAX)
                            m_tubesMax = max;
                        else if (max > m_tubesMax)
                            m_tubesMax = max;
                    }
                }
            }
        }
    }
    if (m_tubesMax == UINT_MAX) {
        ATH_MSG_ERROR("No maximum number of MDT tubes was retrieved");
        status = 1;
    } else {
        ATH_MSG_DEBUG(" Maximum number of MDT tubes is " << m_tubesMax);
    }
    m_init = true;
    m_BME_stat = stationNameIndex("BME");
    m_BMG_stat = stationNameIndex("BMG");
    return status;
}

int MdtIdHelper::init_id_to_hashes() {
    unsigned int hash_max = this->module_hash_max();
    for (unsigned int i = 0; i < hash_max; ++i) {
        const Identifier& id = m_module_vec[i];
        const unsigned int idx = moduleHashIdx(id);
        if (idx >= m_module_hashes.size() || m_module_hashes[idx] < hash_max){
            ATH_MSG_FATAL("Failed to initialize module hash dict for "<<show_to_string(id)<<" index: "<<idx<<"/"<<m_module_hashes.size());
            return 1;
        }
        m_module_hashes[idx] = i;
    }

    hash_max = this->detectorElement_hash_max();
    for (unsigned int i = 0; i < hash_max; ++i) {
        const Identifier& id = m_detectorElement_vec[i];
        const unsigned int idx = detEleHashIdx(id);
        if (idx >= m_detectorElement_hashes.size() || m_detectorElement_hashes[idx] < hash_max){
            ATH_MSG_FATAL("Failed to initialize detector element hash dict for "<<show_to_string(id));
            return 1;
        }
        m_detectorElement_hashes[idx] = i;
    }
    return 0;
}
inline unsigned int MdtIdHelper::moduleHashIdx(const Identifier& id) const {
    /// Unfold the array [A][B][C] by
    /// a * BxC + b * C + c
    constexpr unsigned int C = s_phiDim;
    constexpr unsigned int BxC = C*s_etaDim;
    /// Station eta ranges from -8 to 8
    const int stEta = stationEta(id);
    return stationName(id)*BxC + (stEta + (s_etaDim/2))*C + (stationPhi(id)-1); 
    
}
inline unsigned int MdtIdHelper::detEleHashIdx(const Identifier& id) const {
    return moduleHashIdx(id)  *s_mlDim +  (multilayer(id) -1);
}
int MdtIdHelper::get_module_hash(const Identifier& id, IdentifierHash& hash_id) const {
    const unsigned int idx = moduleHashIdx(id);
    if (idx >= m_module_hashes.size()) return -1;
    hash_id = m_module_hashes[idx];
    return 0;
}

int MdtIdHelper::get_detectorElement_hash(const Identifier& id, IdentifierHash& hash_id) const {
    const unsigned int idx = detEleHashIdx(id);
    if (idx >= m_detectorElement_hashes.size()) return -1;
    hash_id = m_detectorElement_hashes[idx];    
    return 0;
}

Identifier MdtIdHelper::multilayerID(const Identifier& channelID) const {
    assert(is_mdt(channelID));
    Identifier result(channelID);
    m_lay_impl.reset(result);
    m_tub_impl.reset(result);
    return result;
}

Identifier MdtIdHelper::multilayerID(const Identifier& moduleID, int multilayer) const {
    Identifier result{moduleID};
    resetAndSet(m_mla_impl, multilayer, result);
    return result;
}

Identifier MdtIdHelper::multilayerID(const Identifier& moduleID, int multilayer, bool& isValid) const {
    try {
        const Identifier result = multilayerID(moduleID, multilayer);
        isValid = validElement(result);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}
void MdtIdHelper::idChannels(const Identifier& id, std::vector<Identifier>& vect) const {
    vect.clear();
    Identifier parent = parentID(id);
    for (unsigned int i = 0; i < m_full_channel_range.size(); ++i) {
        const Range& range = m_full_channel_range[i];
        ConstRangeIterator rit(range);
        for (const auto & expId: rit) {
            Identifier child;
            get_id(expId, child);
            if (parentID(child) == parent) vect.push_back(child);
        }
    }
}

/// Access to min and max of level ranges

int MdtIdHelper::stationEtaMin(const Identifier& id) const
{
  return stationEtaMinMax(id).first;
}
int MdtIdHelper::stationEtaMax(const Identifier& id) const
{
  return stationEtaMinMax(id).second;
}

int MdtIdHelper::stationPhiMin(const Identifier& id) const
{
  return stationPhiMinMax(id).first;
}
int MdtIdHelper::stationPhiMax(const Identifier& id) const
{
  return stationPhiMinMax(id).second;
}

int MdtIdHelper::multilayerMin(const Identifier& id) const
{
  return multilayerMinMax(id).first;
}
int MdtIdHelper::multilayerMax(const Identifier& id) const
{
  return multilayerMinMax(id).second;
}

int MdtIdHelper::tubeLayerMin(const Identifier& id) const
{
  return tubeLayerMinMax(id).first;
}
int MdtIdHelper::tubeLayerMax(const Identifier& id) const
{
  return tubeLayerMinMax(id).second;
}

int MdtIdHelper::tubeMin(const Identifier& id) const
{
  return tubeMinMax(id).first;
}
int MdtIdHelper::tubeMax(const Identifier& id) const
{
  return tubeMinMax(id).second;
}

std::pair<int, int>
MdtIdHelper::findMinMax(const Identifier& id,
                        const size_type field_index,
                        const ranges_by_station_t& ranges_by_station) const {
    int resultMin =  999;
    int resultMax = -999;
    ExpandedIdentifier expId;
    IdContext eta_context(expId, 0, field_index);
    if (!get_expanded_id(id, expId, &eta_context)) {
        unsigned int station = expId[m_NAME_INDEX];
        for (const Range* range : ranges_by_station.at (station)) {
            if (range->match(expId)) {
                const Range::field& field = (*range)[field_index];
                if (not field.empty()) {
                    resultMin = std::min (resultMin, field.get_minimum());
                }
                if (not field.empty()) {
                    resultMax = std::max (resultMax, field.get_maximum());
                }
            }
        }
    }
    return std::make_pair (resultMin, resultMax);
}


std::pair<int, int>
MdtIdHelper::stationEtaMinMax(const Identifier& id) const {
    return findMinMax (id, m_ETA_INDEX, m_module_ranges_by_station);
}

std::pair<int, int>
MdtIdHelper::stationPhiMinMax(const Identifier& id) const {
    return findMinMax (id, m_PHI_INDEX, m_module_ranges_by_station);
}

std::tuple<int, int, int, int>
MdtIdHelper::stationEtaPhiMinMax(const Identifier& id) const {
    int stationEtaMin =  999;
    int stationEtaMax = -999;
    int stationPhiMin =  999;
    int stationPhiMax = -999;

    ExpandedIdentifier expIdEta;
    ExpandedIdentifier expIdPhi;
    IdContext contextEta(expIdEta, 0, m_ETA_INDEX);
    IdContext contextPhi(expIdPhi, 0, m_PHI_INDEX);
    if (!get_expanded_id(id, expIdEta, &contextEta) &&
        !get_expanded_id(id, expIdPhi, &contextPhi)) {
        unsigned int station = expIdEta[m_NAME_INDEX];
        for (const Range* range : m_module_ranges_by_station.at (station)) {
            if (range->match(expIdEta)) {
                const Range::field& eta_field = (*range)[m_ETA_INDEX];
                if (not eta_field.empty()) {
                    stationEtaMin = std::min (stationEtaMin,
                                              eta_field.get_minimum());
                }
                if (not eta_field.empty()) {
                    stationEtaMax = std::max (stationEtaMax,
                                              eta_field.get_maximum());
                }

                if (range->match(expIdPhi)) {
                  const Range::field& phi_field = (*range)[m_PHI_INDEX];
                  if (not phi_field.empty()) {
                      stationPhiMin = std::min (stationPhiMin,
                                                phi_field.get_minimum());
                  }
                  if (not phi_field.empty()) {
                      stationPhiMax = std::max (stationPhiMax,
                                                phi_field.get_maximum());
                  }
                }
            }
        }
    }
    return std::make_tuple (stationEtaMin, stationEtaMax,
                            stationPhiMin, stationPhiMax);
}

int MdtIdHelper::numberOfMultilayers(const Identifier& id) const {
    int result = -999;
    ExpandedIdentifier expId;
    IdContext context = technology_context();
    if (!get_expanded_id(id, expId, &context)) {
        unsigned int station = expId[m_NAME_INDEX];
        for (const Range* range : m_channel_ranges_by_station.at (station)) {
            if (range->match(expId)) {
                const Range::field& multilayer_field = (*range)[m_DETECTORELEMENT_INDEX];
                if (not multilayer_field.empty()) {
                    result = std::max (result, multilayer_field.get_maximum());
                }
            }
        }
    }
    return result;
}

std::pair<int, int>
MdtIdHelper::multilayerMinMax(const Identifier& id) const {
    return findMinMax (id, m_DETECTORELEMENT_INDEX, m_channel_ranges_by_station);
}

std::pair<int, int>
MdtIdHelper::tubeLayerMinMax(const Identifier& id) const {
    return findMinMax (id, m_TUBELAYER_INDEX, m_channel_ranges_by_station);
}

std::pair<int, int>
MdtIdHelper::tubeMinMax(const Identifier& id) const {
    return findMinMax (id, m_CHANNEL_INDEX, m_channel_ranges_by_station);
}

/// Public validation of levels

bool MdtIdHelper::valid(const Identifier& id) const {
    if (!validElement(id)) return false;

    int mlayer = multilayer(id);
    auto [multilayerMin, multilayerMax] = multilayerMinMax (id);
    if (mlayer < multilayerMin || mlayer > multilayerMax) {
        ATH_MSG_DEBUG("Invalid multilayer=" << mlayer << " multilayerMin=" << multilayerMin << " multilayerMax=" << multilayerMax);
        return false;
    }

    int layer = tubeLayer(id);
    auto [tubeLayerMin, tubeLayerMax] = tubeLayerMinMax(id);
    if (layer < tubeLayerMin || layer > tubeLayerMax) {
        ATH_MSG_DEBUG("Invalid tubeLayer=" << layer << " tubeLayerMin=" << tubeLayerMin << " tubeLayerMax=" << tubeLayerMax);
        return false;
    }

    int tb = tube(id);
    auto [tubeMin, tubeMax ] = tubeMinMax (id);
    if (tb < tubeMin || tb > tubeMax) {
        ATH_MSG_DEBUG("Invalid tube=" << tb << " tubeMin=" << tubeMin << " tubeMax=" << tubeMax);
        return false;
    }
    return true;
}
bool MdtIdHelper::isStNameInTech(const std::string& name) const { return 'B' == name[0] || 'E' == name[0]; }
bool MdtIdHelper::validElement(const Identifier& id) const {
    int station = stationName(id);
    if (!validStation(station)) {
        ATH_MSG_DEBUG("Invalid stationName=" << stationNameString(station));
        return false;
    }

    auto [stationEtaMin, stationEtaMax, stationPhiMin, stationPhiMax] =
      stationEtaPhiMinMax (id);
    int eta = stationEta(id);
    if (eta < stationEtaMin || eta > stationEtaMax) {
        ATH_MSG_DEBUG("Invalid stationEta=" << eta << " for stationName=" << stationNameString(station)
                                            << " stationEtaMin=" << stationEtaMin << " stationEtaMax=" << stationEtaMax);
        return false;
    }

    int phi = stationPhi(id);
    if (phi < stationPhiMin || phi > stationPhiMax) {
        ATH_MSG_DEBUG("Invalid stationPhi=" << phi << " for stationName=" << stationNameString(station)
                                            << " stationPhiMin=" << stationPhiMin << " stationPhiMax=" << stationPhiMax);
        return false;
    }
    return true;
}

/// Private validation of levels

bool MdtIdHelper::validElement(const Identifier& id, int stationName, int stationEta, int stationPhi) const {
    if (!validStation(stationName)) {
        ATH_MSG_DEBUG("Invalid stationName=" << stationNameString(stationName));
        return false;
    }

    auto [stationEtaMin, stationEtaMax, stationPhiMin, stationPhiMax] =
      stationEtaPhiMinMax (id);
    if (stationEta < stationEtaMin || stationEta > stationEtaMax) {
        ATH_MSG_DEBUG("Invalid stationEta=" << stationEta << " for stationName=" << stationNameString(stationName)
                                            << " stationEtaMin=" << stationEtaMin << " stationEtaMax=" << stationEtaMax);
        return false;
    }

    if (stationPhi < stationPhiMin || stationPhi > stationPhiMax) {
        ATH_MSG_DEBUG("Invalid stationPhi=" << stationPhi << " for stationName=" << stationNameString(stationName)
                                            << " stationPhiMin=" << stationPhiMin << " stationPhiMax=" << stationPhiMax);
        return false;
    }
    return true;
}

bool MdtIdHelper::validChannel(const Identifier& id, int stationName, int stationEta, int stationPhi, int multilayer, int tubeLayer,
                               int tube) const {
    if (!validElement(id, stationName, stationEta, stationPhi)) return false;

    auto [multilayerMin, multilayerMax] = multilayerMinMax (id);
    if (multilayer < multilayerMin || multilayer > multilayerMax) {
        ATH_MSG_DEBUG("Invalid multilayer=" << multilayer << " multilayerMin=" << multilayerMin
                                            << " multilayerMax=" << multilayerMax);
        return false;
    }

    auto [tubeLayerMin, tubeLayerMax] = tubeLayerMinMax(id);
    if (tubeLayer < tubeLayerMin || tubeLayer > tubeLayerMax) {
        ATH_MSG_DEBUG("Invalid tubeLayer=" << tubeLayer << " tubeLayerMin=" << tubeLayerMin << " tubeLayerMax=" << tubeLayerMax);
        return false;
    }
    auto [tubeMin, tubeMax ] = tubeMinMax (id);
    if (tube < tubeMin || tube > tubeMax) {
        ATH_MSG_DEBUG("Invalid tube=" << tube << " tubeMin=" << tubeMin << " tubeMax=" << tubeMax);
        return false;
    }
    return true;
}

Identifier MdtIdHelper::elementID(int stationName, int stationEta, int stationPhi) const {
    if (stationName < 0) { return Identifier{-1}; }
    // pack fields independently
    Identifier result((Identifier::value_type)0);
    m_muon_impl.pack(muon_field_value(), result);
    m_sta_impl.pack(stationName, result);
    m_eta_impl.pack(stationEta, result);
    m_phi_impl.pack(stationPhi, result);
    m_tec_impl.pack(mdt_field_value(), result);
    return result;
}
Identifier MdtIdHelper::elementID(int stationName, int stationEta, int stationPhi, bool& isValid) const {
    try {
        const Identifier result = elementID(stationName, stationEta, stationPhi);
        isValid = stationName >= 0 && validElement(result, stationName, stationEta, stationPhi);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}

Identifier MdtIdHelper::elementID(const std::string& stationNameStr, int stationEta, int stationPhi) const {
    return elementID(stationNameIndex(stationNameStr), stationEta, stationPhi);
}
Identifier MdtIdHelper::elementID(const std::string& stationNameStr, int stationEta, int stationPhi, bool& isValid) const {
    return elementID(stationNameIndex(stationNameStr), stationEta, stationPhi, isValid);
}

Identifier MdtIdHelper::elementID(const Identifier& id) const { return parentID(id); }

Identifier MdtIdHelper::channelID(int stationName, int stationEta, int stationPhi, int multilayer, int tubeLayer, int tube) const {
    if (stationName < 0) { return Identifier{-1}; }

    // pack fields independently
    Identifier result((Identifier::value_type)0);
    m_muon_impl.pack(muon_field_value(), result);
    m_sta_impl.pack(stationName, result);
    m_eta_impl.pack(stationEta, result);
    m_phi_impl.pack(stationPhi, result);
    m_tec_impl.pack(mdt_field_value(), result);
    m_mla_impl.pack(multilayer, result);
    m_lay_impl.pack(tubeLayer, result);
    m_tub_impl.pack(tube, result);
    return result;
}
Identifier MdtIdHelper::channelID(int stationName, int stationEta, int stationPhi, int multilayer, int tubeLayer, int tube,
                                  bool& isValid) const {
    try{
        const Identifier result = channelID(stationName, stationEta, stationPhi, multilayer, tubeLayer, tube);
        isValid = stationName >= 0 && validChannel(result, stationName, stationEta, stationPhi, multilayer, tubeLayer, tube);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}

Identifier MdtIdHelper::channelID(const std::string& stationNameStr, int stationEta, int stationPhi, int multilayer, int tubeLayer,
                                  int tube) const {
    return channelID(stationNameIndex(stationNameStr), stationEta, stationPhi, multilayer, tubeLayer, tube);
}
Identifier MdtIdHelper::channelID(const std::string& stationNameStr, int stationEta, int stationPhi, int multilayer, int tubeLayer,
                                  int tube, bool& isValid) const {
    return channelID(stationNameIndex(stationNameStr), stationEta, stationPhi, multilayer, tubeLayer, tube, isValid);
}

Identifier MdtIdHelper::channelID(const Identifier& id, int multilayer, int tubeLayer, int tube) const {
    Identifier result(id);
    resetAndSet(m_mla_impl, multilayer, result);
    resetAndSet(m_lay_impl, tubeLayer, result);
    resetAndSet(m_tub_impl, tube, result);
    return result;
}
Identifier MdtIdHelper::channelID(const Identifier& id, int multilayer, int tubeLayer, int tube, bool& isValid) const {
    try {
        const Identifier result = channelID(id, multilayer, tubeLayer, tube);
        isValid = valid(result);
        return result;
    } catch (const std::out_of_range&) { isValid = false; }
    return Identifier{0};
}

/// get parent id from channel id

Identifier MdtIdHelper::parentID(const Identifier& id) const {
    assert(is_mdt(id));
    Identifier result(id);
    m_mla_impl.reset(result);
    m_lay_impl.reset(result);
    m_tub_impl.reset(result);
    return result;
}

/// Access to components of the ID

int MdtIdHelper::multilayer(const Identifier& id) const { return m_mla_impl.unpack(id); }

int MdtIdHelper::tubeLayer(const Identifier& id) const { return m_lay_impl.unpack(id); }

int MdtIdHelper::tube(const Identifier& id) const { return m_tub_impl.unpack(id); }

int MdtIdHelper::gasGap(const Identifier& id) const { return tubeLayer(id); }

bool MdtIdHelper::measuresPhi(const Identifier& /*id*/) const { return false; }

bool MdtIdHelper::isBME(const Identifier& id) const { return m_BME_stat == stationName(id); }

bool MdtIdHelper::isBMG(const Identifier& id) const { return m_BMG_stat == stationName(id); }

int MdtIdHelper::channel(const Identifier& id) const { return tube(id); }

/// Access to min and max of level ranges

int MdtIdHelper::stationEtaMin(bool barrel) {
    if (barrel) {
        return StationEtaBarrelMin;
    } else {
        return StationEtaEndcapMin;
    }
}

int MdtIdHelper::stationEtaMax(bool barrel) {
    if (barrel) {
        return StationEtaBarrelMax;
    } else {
        return StationEtaEndcapMax;
    }
}

int MdtIdHelper::stationPhiMin() { return StationPhiMin; }

int MdtIdHelper::stationPhiMax() { return StationPhiMax; }

int MdtIdHelper::multilayerMin() { return MultilayerMin; }

int MdtIdHelper::multilayerMax() { return MultilayerMax; }

int MdtIdHelper::tubeLayerMin() { return TubeLayerMin; }

int MdtIdHelper::tubeLayerMax() { return TubeLayerMax; }

int MdtIdHelper::tubeMin() { return TubeMin; }

int MdtIdHelper::tubeMax() const { return m_tubesMax; }
/// Utility methods

int MdtIdHelper::mdtTechnology() const {
    int mdtField = technologyIndex("MDT");
    if (m_dict) { mdtField = mdt_field_value(); }
    return mdtField;
}
