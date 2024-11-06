/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONRDO_VERSION_NSWTPRDOAuxContainer_v1_H
#define XAODMUONRDO_VERSION_NSWTPRDOAuxContainer_v1_H

#include <vector>

#include "xAODCore/AuxContainerBase.h"
#include "Identifier/IdentifierHash.h"
#include "Identifier/Identifier.h"



namespace xAOD {
    /// Auxiliary store for pixel clusters
    ///
    class NSWTPRDOAuxContainer_v1 : public AuxContainerBase {
    public:
        /// Default constructor
        NSWTPRDOAuxContainer_v1();

    private:
	std::vector<Identifier::value_type> moduleID {};
	std::vector<uint32_t> ROD_L1ID {} ; // ? bits
	std::vector<uint8_t>  sectID {} ; // 4bits
	std::vector<uint8_t>  EC {} ; // 1 bit
	std::vector<uint16_t> BCID {} ; // 12 bits
	std::vector<uint32_t> L1ID {}; //32 bits
	std::vector<uint16_t> window_open_bcid {}; // 12 bits
	std::vector<uint16_t> l1a_request_bcid {}; //12 bits
	std::vector<uint16_t> window_close_bcid {} ; //12 bits
	std::vector<uint16_t> config_window_open_bcid_offset {}; // 12 bits
	std::vector<uint16_t> config_l1a_request_bcid_offset {}; //12 bits
	std::vector<uint16_t> config_window_close_bcid_offset {}; // 12 bits
		


 	std::vector<std::vector<uint16_t> > pad_coincidence_wedge{}; // 16 bits
	std::vector<std::vector<uint8_t>  > pad_candidateNumber{}; 
	std::vector<std::vector<uint8_t>  > pad_phiID{}; // 6 bits
	std::vector<std::vector<uint8_t>  > pad_bandID{}; // 8 bits
	std::vector<std::vector<uint16_t> > pad_BCID{}; // 12 bits
	std::vector<std::vector<uint8_t>  > pad_idleFlag{}; // 1 bit


	std::vector<std::vector<uint32_t> > merge_LUT_choiceSelection{}; // 24 bit .. deprecated after version 3
	std::vector<std::vector<uint16_t> > merge_nsw_segmentSelector{}; // 12 bit
	std::vector<std::vector<uint16_t> > merge_valid_segmentSelector{}; // 12 bit	
		
	 // contains the information about the merged segmetns variables:
	 // monitor, lowRes, phiRes, dTheta, phiID and RIndex 
	std::vector<std::vector<uint32_t>> merge_segments{};
	 // contains the information about the sector ID and the BCID
        std::vector<std::vector<uint16_t> > merge_BCID_sectorID{};
	std::vector<std::vector<uint8_t>  > merge_candidateNumber{};

        std::vector<std::vector<uint32_t> > NSWTP_mm_segments{};
        std::vector<std::vector<uint8_t> >  NSWTP_mm_candidateNumber{};

        std::vector<std::vector<uint32_t> > NSWTP_mm_BCID{};

        std::vector<std::vector<uint8_t>  > NSWTP_strip_bands_bandID{}; 
        std::vector<std::vector<uint8_t>  > NSWTP_strip_bands_phiID{};
        std::vector<std::vector<uint16_t> > NSWTP_strip_bands_BCID{};
        std::vector<std::vector<uint8_t>  > NSWTP_strip_bands_HLbit{};
        std::vector<std::vector<uint8_t>  > NSWTP_strip_bands_layer{};
        std::vector<std::vector<uint8_t>  > NSWTP_strip_bands_charge{}; 
        std::vector<std::vector<uint8_t>  > NSWTP_strip_BBbit{};

        std::vector<std::vector<uint32_t> > NSWTP_strip_centroids_bandID{};
        std::vector<std::vector<uint32_t> > NSWTP_strip_centroids_phiID{};
        std::vector<std::vector<uint32_t> > NSWTP_strip_centroids_layer{};
        std::vector<std::vector<uint32_t> > NSWTP_strip_centroids_offset{};
        std::vector<std::vector<uint32_t> > NSWTP_strip_centroids_loc{};
    
        std::vector<std::vector<uint8_t>  > NSWTP_strip_segments_bandID{};
        std::vector<std::vector<uint8_t>  > NSWTP_strip_segments_phiID{};
        std::vector<std::vector<uint32_t> > NSWTP_strip_segments_rA{};
        std::vector<std::vector<uint32_t> > NSWTP_strip_segments_rB{};
        std::vector<std::vector<uint32_t> > NSWTP_strip_segment{};
        std::vector<std::vector<uint32_t> > NSWTP_strip_segments_BCID{};
    };
}

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::NSWTPRDOAuxContainer_v1, xAOD::AuxContainerBase );

#endif // XAODMUONRDO_VERSION_NRPCRDOAUXCONTAINER_V1_H
