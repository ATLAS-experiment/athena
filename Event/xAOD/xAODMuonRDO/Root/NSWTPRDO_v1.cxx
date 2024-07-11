/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODMuonRDO/versions/NSWTPRDO_v1.h"
namespace {
   static const std::string preFixStr {"NSWTP_"};
}
#define IMPLEMENT_VECTOR(DATA_TYPE, VAR_NAME) \
   const std::vector<DATA_TYPE>& NSWTPRDO_v1::VAR_NAME() const { \
       static const SG::AuxElement::Accessor<std::vector<DATA_TYPE>> dec{preFixStr+#VAR_NAME};\
       return dec (*this); \
   } \
   std::vector<DATA_TYPE>& NSWTPRDO_v1::VAR_NAME()  { \
       static const SG::AuxElement::Accessor<std::vector<DATA_TYPE>> dec{preFixStr+#VAR_NAME};\
       return dec(*this); \
   }

#define IMPLEMENT_SCALAR(DATA_TYPE, VAR_NAME) \
   DATA_TYPE NSWTPRDO_v1::VAR_NAME() const { \
      static const SG::AuxElement::Accessor<DATA_TYPE> dec{preFixStr+#VAR_NAME};\
      return dec (*this); \
   } \
   void NSWTPRDO_v1::set_##VAR_NAME(const DATA_TYPE val) { \
      static const SG::AuxElement::Accessor<DATA_TYPE> dec{preFixStr+#VAR_NAME}; \
      dec (*this) = val; \
   }
namespace xAOD{
   IMPLEMENT_VECTOR(uint16_t, pad_coincidence_wedge)
   IMPLEMENT_VECTOR(uint8_t , pad_candidateNumber)
   IMPLEMENT_VECTOR(uint8_t , pad_phiID)
   IMPLEMENT_VECTOR(uint8_t , pad_bandID)
   IMPLEMENT_VECTOR(uint16_t, pad_BCID)
   IMPLEMENT_VECTOR(uint8_t , pad_idleFlag)
   IMPLEMENT_VECTOR(uint32_t, merge_LUT_choiceSelection)
   IMPLEMENT_VECTOR(uint16_t, merge_nsw_segmentSelector)
   IMPLEMENT_VECTOR(uint16_t, merge_valid_segmentSelector) 
   IMPLEMENT_VECTOR(uint32_t, merge_segments)
   IMPLEMENT_VECTOR(uint16_t, merge_BCID_sectorID) 
   IMPLEMENT_VECTOR(uint8_t , merge_candidateNumber)

   IMPLEMENT_SCALAR(uint16_t, moduleID)
   IMPLEMENT_SCALAR(uint32_t, ROD_L1ID)
   IMPLEMENT_SCALAR(uint8_t, sectID)
   
   IMPLEMENT_SCALAR(uint8_t , EC)
   IMPLEMENT_SCALAR(uint16_t, BCID)
   IMPLEMENT_SCALAR(uint32_t, L1ID)
   IMPLEMENT_SCALAR(uint16_t, window_open_bcid)
   IMPLEMENT_SCALAR(uint16_t, l1a_request_bcid)
   IMPLEMENT_SCALAR(uint16_t, window_close_bcid)
   IMPLEMENT_SCALAR(uint16_t, config_window_open_bcid_offset)
   IMPLEMENT_SCALAR(uint16_t, config_l1a_request_bcid_offset)
   IMPLEMENT_SCALAR(uint16_t, config_window_close_bcid_offset)

const std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_mm_segments() const {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_mm_segments"};
    return dec(*this);
}

std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_mm_segments() {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_mm_segments"};
    return dec(*this);
}

const std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_mm_BCID() const {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_mm_BCID"};
    return dec(*this);
}

std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_mm_BCID() {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_mm_BCID"};
    return dec(*this);
}

const std::vector<uint8_t>& NSWTPRDO_v1::NSWTP_strip_bands_bandID() const {
    static const SG::AuxElement::Accessor<std::vector<uint8_t>> dec{"NSWTP_strip_bands_bandID"};
    return dec(*this);
}

std::vector<uint8_t>& NSWTPRDO_v1::NSWTP_strip_bands_bandID() {
    static const SG::AuxElement::Accessor<std::vector<uint8_t>> dec{"NSWTP_strip_bands_bandID"};
    return dec(*this);
}

const std::vector<uint8_t>& NSWTPRDO_v1::NSWTP_strip_bands_phiID() const {
    static const SG::AuxElement::Accessor<std::vector<uint8_t>> dec{"NSWTP_strip_bands_phiID"};
    return dec(*this);
}

std::vector<uint8_t>& NSWTPRDO_v1::NSWTP_strip_bands_phiID() {
    static const SG::AuxElement::Accessor<std::vector<uint8_t>> dec{"NSWTP_strip_bands_phiID"};
    return dec(*this);
}

const std::vector<uint16_t>& NSWTPRDO_v1::NSWTP_strip_bands_BCID() const {
    static const SG::AuxElement::Accessor<std::vector<uint16_t>> dec{"NSWTP_strip_bands_BCID"};
    return dec(*this);
}

std::vector<uint16_t>& NSWTPRDO_v1::NSWTP_strip_bands_BCID() {
    static const SG::AuxElement::Accessor<std::vector<uint16_t>> dec{"NSWTP_strip_bands_BCID"};
    return dec(*this);
}

const std::vector<uint8_t>& NSWTPRDO_v1::NSWTP_strip_bands_HLbit() const {
    static const SG::AuxElement::Accessor<std::vector<uint8_t>> dec{"NSWTP_strip_bands_HLbit"};
    return dec(*this);
}

std::vector<uint8_t>& NSWTPRDO_v1::NSWTP_strip_bands_HLbit() {
    static const SG::AuxElement::Accessor<std::vector<uint8_t>> dec{"NSWTP_strip_bands_HLbit"};
    return dec(*this);
}

const std::vector<uint8_t>& NSWTPRDO_v1::NSWTP_strip_bands_layer() const {
    static const SG::AuxElement::Accessor<std::vector<uint8_t>> dec{"NSWTP_strip_bands_layer"};
    return dec(*this);
}

std::vector<uint8_t>& NSWTPRDO_v1::NSWTP_strip_bands_layer() {
    static const SG::AuxElement::Accessor<std::vector<uint8_t>> dec{"NSWTP_strip_bands_layer"};
    return dec(*this);
}

const std::vector<uint8_t>& NSWTPRDO_v1::NSWTP_strip_bands_charge() const {
    static const SG::AuxElement::Accessor<std::vector<uint8_t>> dec{"NSWTP_strip_bands_charge"};
    return dec(*this);
}

std::vector<uint8_t>& NSWTPRDO_v1::NSWTP_strip_bands_charge() {
    static const SG::AuxElement::Accessor<std::vector<uint8_t>> dec{"NSWTP_strip_bands_charge"};
    return dec(*this);
}

const std::vector<uint8_t>& NSWTPRDO_v1::NSWTP_strip_BBbit() const {
    static const SG::AuxElement::Accessor<std::vector<uint8_t>> dec{"NSWTP_strip_BBbit"};
    return dec(*this);
}

std::vector<uint8_t>& NSWTPRDO_v1::NSWTP_strip_BBbit() {
    static const SG::AuxElement::Accessor<std::vector<uint8_t>> dec{"NSWTP_strip_BBbit"};
    return dec(*this);
}

const std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_centroids_bandID() const {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_centroids_bandID"};
    return dec(*this);
}

std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_centroids_bandID() {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_centroids_bandID"};
    return dec(*this);
}

const std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_centroids_phiID() const {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_centroids_phiID"};
    return dec(*this);
}

std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_centroids_phiID() {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_centroids_phiID"};
    return dec(*this);
}

const std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_centroids_layer() const {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_centroids_layer"};
    return dec(*this);
}

std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_centroids_layer() {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_centroids_layer"};
    return dec(*this);
}

const std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_centroids_offset() const {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_centroids_offset"};
    return dec(*this);
}

std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_centroids_offset() {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_centroids_offset"};
    return dec(*this);
}

const std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_centroids_loc() const {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_centroids_loc"};
    return dec(*this);
}

std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_centroids_loc() {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_centroids_loc"};
    return dec(*this);
}

const std::vector<uint8_t>& NSWTPRDO_v1::NSWTP_strip_segments_bandID() const {
    static const SG::AuxElement::Accessor<std::vector<uint8_t>> dec{"NSWTP_strip_segments_bandID"};
    return dec(*this);
}

std::vector<uint8_t>& NSWTPRDO_v1::NSWTP_strip_segments_bandID() {
    static const SG::AuxElement::Accessor<std::vector<uint8_t>> dec{"NSWTP_strip_segments_bandID"};
    return dec(*this);
}

const std::vector<uint8_t>& NSWTPRDO_v1::NSWTP_strip_segments_phiID() const {
    static const SG::AuxElement::Accessor<std::vector<uint8_t>> dec{"NSWTP_strip_segments_phiID"};
    return dec(*this);
}

std::vector<uint8_t>& NSWTPRDO_v1::NSWTP_strip_segments_phiID() {
    static const SG::AuxElement::Accessor<std::vector<uint8_t>> dec{"NSWTP_strip_segments_phiID"};
    return dec(*this);
}

const std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_segments_rA() const {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_segments_rA"};
    return dec(*this);
}

std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_segments_rA() {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_segments_rA"};
    return dec(*this);
}

const std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_segments_rB() const {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_segments_rB"};
    return dec(*this);
}

std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_segments_rB() {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_segments_rB"};
    return dec(*this);
}

const std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_segment() const {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_segment"};
    return dec(*this);
}

std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_segment() {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_segment"};
    return dec(*this);
}

const std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_segments_BCID() const {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_segments_BCID"};
    return dec(*this);
}

std::vector<uint32_t>& NSWTPRDO_v1::NSWTP_strip_segments_BCID() {
    static const SG::AuxElement::Accessor<std::vector<uint32_t>> dec{"NSWTP_strip_segments_BCID"};
    return dec(*this);
}

    
}

#undef IMPLEMENT_VECTOR
#undef IMPLEMENT_SCALAR
