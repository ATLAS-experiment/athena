/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONRDO_VERSION_NSWTPRDO_V1_H
#define XAODMUONRDO_VERSION_NSWTPRDO_V1_H

#include "AthContainers/AuxElement.h"
#include "Identifier/Identifier.h"

#define DECLARE_VEC_MEMBER(DATA_TYPE, VEC_NAME) \
      const std::vector<DATA_TYPE>& VEC_NAME () const; \
      std::vector<DATA_TYPE>& VEC_NAME();

#define DECLARE_SCALAR_MEMBER(DATA_TYPE, SCALAR_NAME) \
        DATA_TYPE SCALAR_NAME() const; \
        void set_##SCALAR_NAME(const DATA_TYPE val);
namespace xAOD {

    /// @class NSWTPRDO_v1

    class NSWTPRDO_v1 : public SG::AuxElement {

    public:

        /// Default constructor
        NSWTPRDO_v1() = default;
        /// Virtual destructor
        virtual ~NSWTPRDO_v1() = default;

        // Method to access stored quantities
        DECLARE_SCALAR_MEMBER(uint16_t, moduleID)
        DECLARE_SCALAR_MEMBER(uint32_t, ROD_L1ID)
        DECLARE_SCALAR_MEMBER(uint8_t, sectID)
       
        DECLARE_SCALAR_MEMBER(uint8_t , EC)
        DECLARE_SCALAR_MEMBER(uint16_t, BCID)
        DECLARE_SCALAR_MEMBER(uint32_t, L1ID)
        DECLARE_SCALAR_MEMBER(uint16_t, window_open_bcid)
        DECLARE_SCALAR_MEMBER(uint16_t, l1a_request_bcid)
        DECLARE_SCALAR_MEMBER(uint16_t, window_close_bcid)
        DECLARE_SCALAR_MEMBER(uint16_t, config_window_open_bcid_offset)
        DECLARE_SCALAR_MEMBER(uint16_t, config_l1a_request_bcid_offset)
        DECLARE_SCALAR_MEMBER(uint16_t, config_window_close_bcid_offset)
     
        // these are the stream variables in the nTuple
        DECLARE_VEC_MEMBER(uint16_t, pad_coincidence_wedge)
        DECLARE_VEC_MEMBER(uint8_t , pad_candidateNumber)

        DECLARE_VEC_MEMBER(uint8_t , pad_phiID)
        DECLARE_VEC_MEMBER(uint8_t , pad_bandID)
        DECLARE_VEC_MEMBER(uint16_t , pad_BCID)
        DECLARE_VEC_MEMBER(uint8_t , pad_idleFlag)



      
        DECLARE_VEC_MEMBER(uint32_t, merge_LUT_choiceSelection)
        DECLARE_VEC_MEMBER(uint16_t, merge_nsw_segmentSelector)
        DECLARE_VEC_MEMBER(uint16_t, merge_valid_segmentSelector) 
        
       
        /// Encode the Monitoring, Spare, lowRes, phiRes, dTheta, phiID, rIndex information of each candidate
        DECLARE_VEC_MEMBER(uint32_t, merge_segments)
        /// Encode the BCID & sector ID of each candidate, 
        DECLARE_VEC_MEMBER(uint16_t, merge_BCID_sectorID) 
        
        DECLARE_VEC_MEMBER(uint8_t , merge_candidateNumber)

        ///// fields available for v3 of the TP L1A stream and above 

        const std::vector<uint32_t>& NSWTP_mm_segments() const;
        std::vector<uint32_t>& NSWTP_mm_segments();
        
        const std::vector<uint32_t>& NSWTP_mm_BCID() const;
        std::vector<uint32_t>& NSWTP_mm_BCID();
        
        const std::vector<uint8_t>& NSWTP_strip_bands_bandID() const;
        std::vector<uint8_t>& NSWTP_strip_bands_bandID();
        
        const std::vector<uint8_t>& NSWTP_strip_bands_phiID() const;
        std::vector<uint8_t>& NSWTP_strip_bands_phiID();
        
        const std::vector<uint16_t>& NSWTP_strip_bands_BCID() const;
        std::vector<uint16_t>& NSWTP_strip_bands_BCID();
        
        const std::vector<uint8_t>& NSWTP_strip_bands_HLbit() const;
        std::vector<uint8_t>& NSWTP_strip_bands_HLbit();
        
        const std::vector<uint8_t>& NSWTP_strip_bands_layer() const;
        std::vector<uint8_t>& NSWTP_strip_bands_layer();
        
        const std::vector<uint8_t>& NSWTP_strip_bands_charge() const;
        std::vector<uint8_t>& NSWTP_strip_bands_charge();
        
        const std::vector<uint8_t>& NSWTP_strip_BBbit() const;
        std::vector<uint8_t>& NSWTP_strip_BBbit();
        
        const std::vector<uint32_t>& NSWTP_strip_centroids_bandID() const;
        std::vector<uint32_t>& NSWTP_strip_centroids_bandID();
        
        const std::vector<uint32_t>& NSWTP_strip_centroids_phiID() const;
        std::vector<uint32_t>& NSWTP_strip_centroids_phiID();
        
        const std::vector<uint32_t>& NSWTP_strip_centroids_layer() const;
        std::vector<uint32_t>& NSWTP_strip_centroids_layer();
        
        const std::vector<uint32_t>& NSWTP_strip_centroids_offset() const;
        std::vector<uint32_t>& NSWTP_strip_centroids_offset();
        
        const std::vector<uint32_t>& NSWTP_strip_centroids_loc() const;
        std::vector<uint32_t>& NSWTP_strip_centroids_loc();
        
        const std::vector<uint8_t>& NSWTP_strip_segments_bandID() const;
        std::vector<uint8_t>& NSWTP_strip_segments_bandID();
        
        const std::vector<uint8_t>& NSWTP_strip_segments_phiID() const;
        std::vector<uint8_t>& NSWTP_strip_segments_phiID();
        
        const std::vector<uint32_t>& NSWTP_strip_segments_rA() const;
        std::vector<uint32_t>& NSWTP_strip_segments_rA();
        
        const std::vector<uint32_t>& NSWTP_strip_segments_rB() const;
        std::vector<uint32_t>& NSWTP_strip_segments_rB();
        
        const std::vector<uint32_t>& NSWTP_strip_segment() const;
        std::vector<uint32_t>& NSWTP_strip_segment();
        
        const std::vector<uint32_t>& NSWTP_strip_segments_BCID() const;
        std::vector<uint32_t>& NSWTP_strip_segments_BCID();


    };

}

#include "AthContainers/DataVector.h"
SG_BASE( xAOD::NSWTPRDO_v1, SG::AuxElement );

#undef DECLARE_VEC_MEMBER
#undef DECLARE_SCALAR_MEMBER
#endif // XAODMUONRDO_VERSION_NRPCRDO_V1_H
