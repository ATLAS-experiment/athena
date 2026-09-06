/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_AuxiliaryMeasurementHandler_H
#define ActsEvent_AuxiliaryMeasurementHandler_H

#include "xAODAuxiliaryMeasurement/AuxiliaryMeasurementContainer.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteHandle.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "Acts/Utilities/Result.hpp"

#include <unordered_map>

namespace ActsTrk{
    /** @brief Utility class to handle the creation of the Auxiliary measurement used in an Acts track fit
     *         This class declares the additionally written xAOD::AuxiliaryMeasurementContainers to the 
     *         AvalancheScheduler. Per event, the user creates an instance of the MeasurementProvider which
     *         setups the storage backend for the measurements and writes the containers to StoreGate. Auxiliary
     *         measurements are created by calling the newMeasurement method. */
    class AuxiliaryMeasurementHandler {
        public:
            using SurfacePtr_t = xAOD::AuxiliaryMeasurement::SurfacePtr_t;
            /** @brief Constructor taking the pointer to the class holding the object used to declare the 
             *         data dependency from the WriteHandleKeys to the AvalancheScheduler. The object should be
             *         defined in the header like
             *             ActsTrk::AuxiliaryMeasurementHandler m_pseudoHandle{this};*/ 
            template <class PropOwner> 
                AuxiliaryMeasurementHandler(PropOwner* owner);
            /** @brief Initialize the write handle keys.
             *  @param preFix: Common prefix to be put in front of all keys
             *  @param used: Flag indicating whether the auxiliary keys are used at all */
            StatusCode initialize(const std::string& preFix,
                                  bool used = true);

            /** @brief Helper struct to create a new pseudo measurement.*/
            class MeasurementProvider {
                public:
                    friend class AuxiliaryMeasurementHandler;
                    /** @brief Default destructor */
                    ~MeasurementProvider() = default;
                    /** @brief Default move constructor */
                    MeasurementProvider(MeasurementProvider&& other) = default;
                    /** @brief Default move assignment operator */
                    MeasurementProvider& operator=(MeasurementProvider&& other) = default;
                    /** @brief Default copy constructor */
                    MeasurementProvider(const MeasurementProvider& other) = delete;
                    /** @brief Default copy assignment operator */
                    MeasurementProvider& operator=(const MeasurementProvider& other) = delete;

                    using ProjectorType = xAOD::AuxiliaryMeasurement::ProjectorType;
                    /** @brief Create a new auxiliary measurement
                     *  @param surface: Pointer to the Acts surface with which the measurment
                     *                  is associated
                     *  @param projector: Projector enum to indicate which local coordinates
                     *                     (d0,z0,t0) are constrained by the measurement
                     * @param locCov: The covariance on the auxiliary measurement
                     * @param locPos: Local displacement of the measurement w.r.t. surface */
                    template<size_t N>
                        xAOD::AuxiliaryMeasurement* newMeasurement(const SurfacePtr_t&  surface,
                                                                   const ProjectorType projector,
                                                                   const AmgSymMatrix(N)& locCov,
                                                                   const AmgVector(N) locPos = AmgVector(N)::Zero());
                
                private:
                    /** @brief Constructor called by the MeasurementUtils. It takes
                     *         the current EventContext & pointer to the Utils class
                     *         to create the container & a mutable reference to the
                     *         track surface container. The latter is used to simultaenously
                     *         fill the surfaces with the pseduo measurements
                     *  @param ctx: EventContext to put the container into StoreGate
                     *  @param gctx: The geometry context to access the surface's location
                     *  @param parent: Pointer to the handler class instantiating the object */
                    MeasurementProvider(const EventContext& ctx,
                                        const Acts::GeometryContext& gctx,
                                        const AuxiliaryMeasurementHandler* parent);
                    /** @brief Setup method to record the Auxiliary measurement containers into StoreGate */
                    StatusCode setupContainers();
                    /** @brief */
                    template<typename AuxCont_t, typename Cont_t>
                        StatusCode recordContainer(SG::WriteHandle<Cont_t>& handle);


                    std::reference_wrapper<const EventContext> m_ctx;
                    std::reference_wrapper<const Acts::GeometryContext> m_gctx;
                    const AuxiliaryMeasurementHandler* m_parent{};
                    /** @brief Abrivation of the WriteHandle */
                    using WriteHandle_t = SG::WriteHandle<xAOD::AuxiliaryMeasurementContainer>;
                    using SurfaceHandle_t = SG::WriteHandle<xAOD::TrackSurfaceContainer>;
                    
                    /** @brief Write handle to the VIEW_Elements container containing all
                     *         created measurements*/
                    WriteHandle_t m_viewHandle{m_parent->m_viewKey, m_ctx};
                    /** @brief Dedicated write handle to store 1D measurements */
                    WriteHandle_t m_handle1D{m_parent->m_writeKey1D, m_ctx};
                    /** @brief Dedicated write handle to store 2D measurements */
                    WriteHandle_t m_handle2D{m_parent->m_writeKey2D, m_ctx};
                    /** @brief Dedicated write handle to store 2D + time measurements */
                    WriteHandle_t m_handle3D{m_parent->m_writeKey3D, m_ctx};
                    SurfaceHandle_t m_surfaceContainer{m_parent->m_surfaceKey, m_ctx};
                    /** @brief List of precached surfaces */
                    std::unordered_map<SurfacePtr_t, const xAOD::TrackSurface*> m_cachedSurfs{};

            };

            enum class HandleStatus: std::uint8_t{
                ok = 0,
                emptyKey = 1,
                recordFail = 2,
            };
            using HandleReturn_t = Acts::Result<MeasurementProvider, HandleStatus>;
           
            /** @brief Creates a new MeasurementProvider and triggers the write of the
             *         container backend to StoreGate. The user needs to provide a
             *         reference to a mutable xAOD::TrackSurfaceContainer where the 
             *         persistified surfaces are appended to. The client needs to ensure
             *         that the lifetime of the surface container prevails the provider's 
             *         lifetime.
             *  @param ctx: EventContext to write the containers to store gate
             *  @param gctx: The geometry context to place the surface in space */
            HandleReturn_t makeHandle(const EventContext& ctx,
                                      const Acts::GeometryContext& gctx) const;

        private:
            using Key_t = SG::WriteHandleKey<xAOD::AuxiliaryMeasurementContainer>;
            /// @brief Lambda to access the parent's msg stream 
            std::function<MsgStream&(const MSG::Level)> m_msgPrinter{};
            /// @brief Lambda to return the msg level from the parent's msg stream
            std::function<bool(const MSG::Level)> m_msgLevel{};
            /// @brief Key to write the view container
            Key_t m_viewKey;
            /// @brief Key to write the 1D measurements
            Key_t m_writeKey1D;
            /// @brief Key to write the 2D measurements
            Key_t m_writeKey2D;
            /// @brief Key to write the 2D + time measurements
            Key_t m_writeKey3D;
            /// @brief Key to write the surfaces associated to the measurements
            SG::WriteHandleKey<xAOD::TrackSurfaceContainer> m_surfaceKey;
            /// @brief Return the reference to the msg logging stream
            /// @param lvl: Logging level to be applied
            MsgStream& msg(const MSG::Level lvl) const;
            /// @brief Returns whether the stream satisfies the logging level
            bool msgLvl(const MSG::Level lvl) const;
    };
}
#include "ActsEvent/AuxiliaryMeasurementHandler.icc"
#endif
