/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSEVENT_CALOEXTENSION_H
#define ACTSEVENT_CALOEXTENSION_H

#include "GeoPrimitives/GeoPrimitives.h"
///
#include "xAODTracking/TrackParticle.h"
#include "xAODCaloEvent/CaloCluster.h"

#include "Acts/EventData/BoundTrackParameters.hpp"
#include "Acts/Utilities/TransformRange.hpp"

#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"

#include <span>


namespace ActsTrk{
    class CaloExtension;
    /** @brief Retrieve a pointer to the CaloExtension linked with the passed TrackParticle
     *         @note Clients should ensure that they've scheduled a dependency on the `caloExtensionLink`
     *         decoration for the underlying track particle container. */
    const CaloExtension* getCaloExtension(const xAOD::TrackParticle& track);


    /** @brief The CaloExtension holds the ID extrapolation states through the calorimeter 
      *        and the associated CaloClusters. */
    class CaloExtension {
        public:
            /** @brief Constructor taking the pointer to the ID track used
             *         for the extrapolations */
            CaloExtension(const xAOD::TrackParticle* track);
            /*** @brief Returns the associated ID track particle */
            const xAOD::TrackParticle* track() const;
            /** @brief Number of extensions stored in the instance */
            std::size_t size() const;
            /** @brief  */
            bool empty() const;
            /** @brief Returns the extension parameters associated with the n-th extension */
            const Acts::BoundTrackParameters& parameters(const std::size_t parIdx) const;
            
            /** @brief Abrivation of the calo cluster vector */
            using ClusterVec_t = std::vector<const xAOD::CaloCluster*>;
            /** @brief Abrivation of the Bound track parameter vector */
            using ParamVec_t = std::vector<Acts::BoundTrackParameters>;
            /** @brief Append a list of recorded hit parameters */
            void appendParameters(Acts::BoundTrackParameters&& pars);
            /** @brief Associates a calorimeter cluster with the calo extension
             *  @param clust: Pointer to the calo cluster */
            void associateCluster(const xAOD::CaloCluster* clust);
            /** @brief Returns the outermost parameters known to the extension */
            std::optional<Acts::BoundTrackParameters> lastParameters() const;
            /** @brief  Returns the view onto the cached BoundTackParameters*/
            const ParamVec_t& parameters() const;
            /** @brief Returns the view on the asociated clusters */
            const ClusterVec_t& associatedClusters() const;
            /** @brief Returns the track parameters asssociated with the last measurement
             *         state of the ID track */
            std::optional<Acts::BoundTrackParameters> lastTrackParameters() const;
        private:
            /** @brief Pointer to the associated ID track particle */
            const xAOD::TrackParticle* m_idTrack{nullptr};
            /** @brief Define the list of bound track parameters  */
            ParamVec_t m_parameters{};
            /** @brief The list of assoicated clusters */
            ClusterVec_t m_clusters{};

    };
    using CaloExtensionContainer = DataVector<CaloExtension>;

}
CLASS_DEF( ActsTrk::CaloExtensionContainer , 1237047548 , 1 )

#endif
