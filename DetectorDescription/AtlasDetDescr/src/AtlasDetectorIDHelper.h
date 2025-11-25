/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SRC_ATLASDETECTORIDHELPER_H
#define SRC_ATLASDETECTORIDHELPER_H

#include "AthenaBaseComps/AthMessaging.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdContext.h"
#include "Identifier/IdHelper.h"
#include <string>
#include <vector>

class IdDictField;

class AtlasDetectorIDHelper : public AthMessaging {
public:
    
    enum ERRORS { UNDEFINED = 999 };


    AtlasDetectorIDHelper();

    using size_type =  Identifier::size_type; 
    
    /// Initialization from the identifier dictionary
    int initialize_from_dictionary(const IdDictMgr& dict_mgr);

    ~AtlasDetectorIDHelper()=default;
    
    size_type pixel_region_index() const {
       return m_pixel_region_index;
    }
    size_type sct_region_index() const{
      return m_sct_region_index;
    }
    size_type trt_region_index() const{
      return m_trt_region_index;
    }
    size_type lar_em_region_index() const{
      return m_lar_em_region_index;
    }
    size_type lar_hec_region_index() const{
      return m_lar_hec_region_index;
    }
    size_type lar_fcal_region_index() const{
      return m_lar_fcal_region_index;
    }
    size_type lvl1_region_index() const{
      return m_lvl1_region_index;
    }
    size_type dm_region_index() const{
      return m_dm_region_index;
    }
    size_type tile_region_index() const{
      return m_tile_region_index;
    }
    size_type mdt_region_index() const{
      return m_mdt_region_index;
    }
    size_type csc_region_index() const{
      return m_csc_region_index;
    }
    size_type rpc_region_index() const{
      return m_rpc_region_index;
    }
    size_type tgc_region_index() const{
      return m_tgc_region_index;
    }
    size_type stgc_region_index() const{
      return m_sct_region_index;
    }
    size_type mm_region_index() const{
      return m_mm_region_index;
    }
    size_type muon_station_index() const {
      return m_muon_station_index;
    }

    size_type alfa_region_index() const{
      return m_alfa_region_index;
    }
    size_type bcm_region_index() const{
      return m_bcm_region_index;
    }
    size_type lucid_region_index() const{
      return m_lucid_region_index;
    }
    size_type zdc_region_index() const{
      return m_zdc_region_index;
    }
    const IdDictField* station_field() const {
        return m_station_field;
    }
private:
    bool m_isHighLuminosityLHC{false};
    size_type m_pixel_region_index{UNDEFINED};
    size_type m_sct_region_index{UNDEFINED};
    size_type m_trt_region_index{UNDEFINED};
    size_type	m_lar_em_region_index{UNDEFINED};
    size_type	m_lar_hec_region_index{UNDEFINED};
    size_type	m_lar_fcal_region_index{UNDEFINED};
    size_type	m_lvl1_region_index{UNDEFINED};
    size_type	m_dm_region_index{UNDEFINED};
    size_type	m_tile_region_index{UNDEFINED};
    size_type	m_mdt_region_index{UNDEFINED};
    size_type	m_csc_region_index{UNDEFINED};
    size_type	m_rpc_region_index{UNDEFINED};
    size_type	m_tgc_region_index{UNDEFINED};
    size_type	m_mm_region_index{UNDEFINED};
    size_type	m_stgc_region_index{UNDEFINED};
    size_type	m_muon_station_index{UNDEFINED};
    size_type m_alfa_region_index{UNDEFINED};
    size_type m_bcm_region_index{UNDEFINED};
    size_type m_lucid_region_index{UNDEFINED};
    size_type m_zdc_region_index{UNDEFINED};
    bool m_initialized{false};
    const IdDictField *m_station_field{};

};

#endif // SRC_ATLASDETECTORIDHELPER_H
