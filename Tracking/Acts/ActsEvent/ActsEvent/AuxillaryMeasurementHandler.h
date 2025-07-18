/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_AuxillaryMeasurementHandler_H
#define ActsEvent_AuxillaryMeasurementHandler_H

#include "xAODAuxillaryMeasurement/AuxillaryMeasurementContainer.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteHandle.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"

#include <unordered_map>

namespace ActsTrk{
    /** @brief Utility class to handle the creation of the Auxillary measurement used in an Acts track fit
     *         This class declares the additionally written xAOD::AuxillaryMeasurementContainers to the 
     *         AvalancheScheduler. Per event, the user creates an instance of the MeasurementProvider which
     *         setups the storage backend for the measurements and writes the containers to StoreGate. Auxillary
     *         measurements are created by calling the newMeasurement method. */
    class AuxillaryMeasurementHandler {
        public:
            using SurfacePtr_t = xAOD::AuxillaryMeasurement::SurfacePtr_t;
            /** @brief Constructor taking the pointer to the class holding the object used to declare the 
             *         data dependency from the WriteHandleKeys to the AvalancheScheduler. The object should be
             *         defined in the header like
             *             ActsTrk::AuxillaryMeasurementHandler m_pseudoHandle{this};*/ 
            template <class PropOwner> 
                AuxillaryMeasurementHandler(PropOwner* owner);
            /** @brief Initialize the write handle keys.
             *  @param preFix: Common prefix to be put in front of all keys */
            StatusCode initialize(const std::string& preFix);
            /** @brief Helper struct to create a new pseudo measurement.*/
            class MeasurementProvider {
                public:
                    friend class AuxillaryMeasurementHandler;
                    /** @brief Default destructor */
                    ~MeasurementProvider() = default;
                    /** @brief Default move constructor */
                    MeasurementProvider(MeasurementProvider&& other) = default;
                    /** @brief Default move assignment operator */
                    MeasurementProvider& operator=(MeasurementProvider&& other) = delete;
                    /** @brief Delete the copy constructor */
                    MeasurementProvider(const MeasurementProvider& other) = delete;
                    /** @brief Delete the copy assignment */
                    MeasurementProvider& operator=(const MeasurementProvider& other) = delete;

                    using ProjectorType = xAOD::AuxillaryMeasurement::ProjectorType;
                    template<size_t N>
                        xAOD::AuxillaryMeasurement* newMeasurement(const SurfacePtr_t&  surface,
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
                     *  @param parent: Pointer to the Utils class instantiating the object
                     *  @param surfaceBackend: Reference to the surface container into which
                     *                         all surfaces are stored. */
                    MeasurementProvider(const EventContext& ctx,
                                      const AuxillaryMeasurementHandler* parent,
                                      xAOD::TrackSurfaceContainer& surfaceBackend);
                    /** @brief Setup method to record the Auxillary measurement containers into StoreGate */
                    StatusCode setupContainers();

                    const EventContext& m_ctx;
                    const AuxillaryMeasurementHandler* m_parent{};
                    xAOD::TrackSurfaceContainer& m_surfaceContainer;
                    /** @brief Abrivation of the WriteHandle */
                    using WriteHandle_t = SG::WriteHandle<xAOD::AuxillaryMeasurementContainer>;
                    WriteHandle_t m_handle1D{m_parent->m_writeKey1D, m_ctx};
                    WriteHandle_t m_handle2D{m_parent->m_writeKey2D, m_ctx};
                    WriteHandle_t m_handle3D{m_parent->m_writeKey3D, m_ctx};
                    /** @brief List of precached surfaces */
                    std::unordered_map<SurfacePtr_t, const xAOD::TrackSurface*> m_cachedSurfs{};

            };
        /** @brief Creates a new MeasurementProvider and triggers the write of the
         *         container backend to StoreGate. The user needs to provide a
         *         reference to a mutable xAOD::TrackSurfaceContainer where the 
         *         persistified surfaces are appended to. The client needs to ensure
         *         that the lifetime of the surface container prevails the provider's 
         *         lifetime.
         *  @param ctx: EventContext to write the containers to store gate
         *  @param surfaceBackend: Reference to a mutable track surface container. */
        MeasurementProvider makeHandle(const EventContext& ctx,
                                       xAOD::TrackSurfaceContainer& surfaceBackend) const;

        private:
            using Key_t = SG::WriteHandleKey<xAOD::AuxillaryMeasurementContainer>;
            MsgStream& m_msg;
            Key_t m_writeKey1D;
            Key_t m_writeKey2D;
            Key_t m_writeKey3D;
            ActsGeometryContext m_gctx{};
    };
}
#include "ActsEvent/AuxillaryMeasurementHandler.icc"
#endif
