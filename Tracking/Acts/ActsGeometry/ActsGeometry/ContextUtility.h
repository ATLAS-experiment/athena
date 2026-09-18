/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRY_CONTEXTUTILITY_H
#define ACTSGEOMETRY_CONTEXTUTILITY_H


#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

#include "AthenaBaseComps/AthMessaging.h"

#include "ActsGeometryInterfaces/GeometryContext.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"

#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"

namespace ActsTrk{
    /** @brief Utility class to handle the three contexts neeeded in an ACTS reconstruction job
     *           1) GeometryContext -> handles the aligned transforms of the Surfaces & volumes
     *           2) MagneticFieldContext -> Contains the magnetic field
     *           3) CalibrationContext -> Pipes the StoreGate access through the Acts caller chain
     *         The utility class declares the data dependencies on the needed conditions by calling
     *         the declareProperty on its client. Likewise any StoreGateKey the Utility needs to 
     *         be initialized in in the initialize method. */
    class ContextUtility{
        public:
            /** @brief Constructor taking the pointer to the class holding the object used to declare the 
             *         data dependency from the WriteHandleKeys to the AvalancheScheduler. The object should be
             *         defined in the header like
             *             ActsTrk::ContextUtility m_ctxProvider{this};*/ 
            template <class PropOwner> 
                explicit ContextUtility(PropOwner* owner);
            /** @brief Initializes the ReadHandleKeys held by the utility class
             *  @param enable: Flag to toggle whether the key shall be used or not */
            StatusCode initialize(const bool enable = true);
            /** @brief Retrieves the current geometry context from StoreGate
             *  @param ctx: The event context to access the conditions store */
            Acts::GeometryContext getGeometryContext(const EventContext& ctx) const;
            /** @brief Retrieves the current calibration context from StoreGate
             *  @param ctx: The event context to access the conditions store */
            Acts::CalibrationContext getCalibrationContext(const EventContext& ctx) const;
            /** @brief Retrieves the current magnetic field context from StoreGate
             *  @param ctx: The event context to access the conditions store */
            Acts::MagneticFieldContext getMagneticFieldContext(const EventContext& ctx) const;
        private:
            /** @brief Data dependency on the aligned geometry transforms */
            GeoContextReadKey_t m_geoCtxKey;
            /** @brief Data dependency on the magnetic field map */
            SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_magCtxKey;
            /// @brief Lambda to access the parent's msg stream 
            std::function<MsgStream&(const MSG::Level)> m_msgPrinter;
            /// @brief Lambda to return the msg level from the parent's msg stream
            std::function<bool(const MSG::Level)> m_msgLevel;
            /// @brief Return the reference to the msg logging stream
            /// @param lvl: Logging level to be applied
            MsgStream& msg(const MSG::Level lvl) const;
            /// @brief Returns whether the stream satisfies the logging level
            bool msgLvl(const MSG::Level lvl) const;
    };

}
#include "ActsGeometry/ContextUtility.icc"
#endif
