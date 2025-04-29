/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONRDO_VERSION_NSWMMTPRDOAuxContainer_v1_H
#define XAODMUONRDO_VERSION_NSWMMTPRDOAuxContainer_v1_H

#include <vector>

#include "xAODCore/AuxContainerBase.h"
#include "Identifier/IdentifierHash.h"
#include "Identifier/Identifier.h"



namespace xAOD {

    class NSWMMTPRDOAuxContainer_v1 : public AuxContainerBase {
    public:
        /// Default constructor
        NSWMMTPRDOAuxContainer_v1();

    private:
        std::vector<uint32_t> sourceID{};
	std::vector<uint16_t> moduleID{};
	std::vector<uint32_t> ROD_L1ID{};
	std::vector<uint16_t> ROD_BCID{};
	
	//TP head
	std::vector<uint8_t > EC{};
	std::vector<uint8_t > sectID{};
	std::vector<uint32_t> L1ID{};
	std::vector<uint16_t> BCID{};

	//TP L1A head
	std::vector<uint16_t> l1a_request_BCID{};
	std::vector<uint16_t> l1a_release_BCID{};
	std::vector<uint16_t> l1a_window_open{};
	std::vector<uint16_t> l1a_window_center{};
	std::vector<uint16_t> l1a_window_close{};
	std::vector<uint16_t> l1a_window_open_offset{};
	std::vector<uint16_t> l1a_window_center_offset{};
	std::vector<uint16_t> l1a_window_close_offset{};

	//L1A data quality
	std::vector<uint16_t> l1a_timeout{};
	std::vector<uint16_t> l1a_engines{};

	//ART data
	std::vector< std::vector<uint16_t>> art_BCID{};
	std::vector< std::vector<uint8_t>>  art_layer{};
	std::vector< std::vector<uint16_t>> art_channel{};

	//trigger data
	std::vector< std::vector<uint16_t>> trig_globalX{};
	std::vector< std::vector<uint16_t>> trig_globalU{};
	std::vector< std::vector<uint16_t>> trig_BCID{};
	std::vector< std::vector<uint8_t>>  trig_dTheta{};
	std::vector< std::vector<uint8_t>>  trig_ROI_rID{};
	std::vector< std::vector<uint8_t>>  trig_ROI_phiID{};


    };
}

// Set up the StoreGate inheritance for the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::NSWMMTPRDOAuxContainer_v1, xAOD::AuxContainerBase );

#endif // XAODMUONRDO_VERSION_NSWMMTPRDOAuxContainer_v1_H
