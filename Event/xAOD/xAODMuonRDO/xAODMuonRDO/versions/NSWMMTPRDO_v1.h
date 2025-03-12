/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODMUONRDO_VERSION_NSWMMTPRDO_V1_H
#define XAODMUONRDO_VERSION_NSWMMTPRDO_V1_H

#include "AthContainers/AuxElement.h"
#include "Identifier/Identifier.h"

#define DECLARE_VEC_MEMBER(DATA_TYPE, VEC_NAME) \
      const std::vector<DATA_TYPE>& VEC_NAME () const; \
      std::vector<DATA_TYPE>& VEC_NAME();

#define DECLARE_SCALAR_MEMBER(DATA_TYPE, SCALAR_NAME) \
        DATA_TYPE SCALAR_NAME() const; \
        void set_##SCALAR_NAME(const DATA_TYPE val);

namespace xAOD {

    /// @class NSWMMTPRDO_v1

    class NSWMMTPRDO_v1 : public SG::AuxElement {

    public:

        /// Default constructor
        NSWMMTPRDO_v1() = default;
        /// Virtual destructor
        virtual ~NSWMMTPRDO_v1() = default;

        // Method to access stored quantities
	//ROD info
        DECLARE_SCALAR_MEMBER(uint32_t, sourceID)
	DECLARE_SCALAR_MEMBER(uint16_t, moduleID)
	DECLARE_SCALAR_MEMBER(uint32_t, ROD_L1ID)
        DECLARE_SCALAR_MEMBER(uint16_t, ROD_BCID)
       
	//TP head
        DECLARE_SCALAR_MEMBER(uint8_t , EC)
        DECLARE_SCALAR_MEMBER(uint8_t , sectID)
        DECLARE_SCALAR_MEMBER(uint32_t, L1ID)
        DECLARE_SCALAR_MEMBER(uint16_t, BCID)

	//TP L1A head
	DECLARE_SCALAR_MEMBER(uint16_t, l1a_request_BCID)
	DECLARE_SCALAR_MEMBER(uint16_t, l1a_release_BCID)
        DECLARE_SCALAR_MEMBER(uint16_t, l1a_window_open)
        DECLARE_SCALAR_MEMBER(uint16_t, l1a_window_center)
        DECLARE_SCALAR_MEMBER(uint16_t, l1a_window_close)
        DECLARE_SCALAR_MEMBER(uint16_t, l1a_window_open_offset)
        DECLARE_SCALAR_MEMBER(uint16_t, l1a_window_center_offset)
        DECLARE_SCALAR_MEMBER(uint16_t, l1a_window_close_offset)
 
	//l1a data quality
        DECLARE_SCALAR_MEMBER(uint16_t, l1a_timeout)
        DECLARE_SCALAR_MEMBER(uint16_t, l1a_engines)    

        //ART data
        DECLARE_VEC_MEMBER(uint16_t, art_BCID)
        DECLARE_VEC_MEMBER(uint8_t,  art_layer)
        DECLARE_VEC_MEMBER(uint16_t, art_channel)

        //trigger data
        DECLARE_VEC_MEMBER(uint16_t, trig_globalX)
        DECLARE_VEC_MEMBER(uint16_t, trig_globalU)
        DECLARE_VEC_MEMBER(uint16_t, trig_BCID)
        DECLARE_VEC_MEMBER(uint8_t,  trig_dTheta)
        DECLARE_VEC_MEMBER(uint8_t,  trig_ROI_rID)
        DECLARE_VEC_MEMBER(uint8_t,  trig_ROI_phiID)



    };

}

#include "AthContainers/DataVector.h"
SG_BASE( xAOD::NSWMMTPRDO_v1, SG::AuxElement );

#undef DECLARE_VEC_MEMBER
#undef DECLARE_SCALAR_MEMBER
#endif // XAODMUONRDO_VERSION_NSWMMTPRDO_V1_H
