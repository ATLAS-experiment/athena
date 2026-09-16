
/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_MUONMEASUREMENT_V1_H
#define XAODMUONPREPDATA_VERSION_MUONMEASUREMENT_V1_H


#include "GeoPrimitives/GeoPrimitives.h"
//
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "CxxUtils/CachedValue.h"
/// EDM include
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"
#include "MuonReadoutGeometryR4/MuonReadoutElement.h"

namespace xAOD{
    class MuonMeasurement_v1 : public UncalibratedMeasurement_v1 {
        public:
            /** @brief Default constructor */
            MuonMeasurement_v1() = default;
            /** @brief Copy constructor*/
            MuonMeasurement_v1(const MuonMeasurement_v1& other);
            
            /**  @brief Copy assignment */
            MuonMeasurement_v1& operator=(const MuonMeasurement_v1& other);
            /** @brief Default destructor */
            virtual ~MuonMeasurement_v1() = default;
            /** @brief Returns the local position of the measurement */
            virtual Amg::Vector3D localMeasurementPos() const = 0;
            /** @brief Returns the pointer to the associated readout element */
            virtual const MuonGMR4::MuonReadoutElement* readoutElement() const = 0;
            /** @brief Returns whether the phi coordinate is measured */
            virtual std::uint8_t measuresPhi() const = 0;

            /** @brief Returns the hash of the measurement channel */
            virtual IdentifierHash measurementHash() const = 0;
            /** @brief Returns the hash of the associated layer (Needed for surface retrieval)*/
            virtual  IdentifierHash layerHash() const = 0;

            /** @brief Returns the Athena identifier of the measurement */
            const Identifier& identify() const;
            /** @brief Returns the Acts surface associated with the measurement */
            const Acts::Surface& surface() const;
        protected:
            /** @brief Cache value of the  */
            CxxUtils::CachedValue<const MuonGMR4::MuonReadoutElement*> m_readoutEle{};
            /** @brief Cached associated Acts Surface */
            CxxUtils::CachedValue<const Acts::Surface*> m_surface{};
        private:
            CxxUtils::CachedValue<Identifier> m_identifier{};
    };
}

#endif
