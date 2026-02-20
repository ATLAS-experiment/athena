/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsMonitoring_ReadoutGeoDumpAlg_H
#define ActsMonitoring_ReadoutGeoDumpAlg_H

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <ActsGeometryInterfaces/ITrackingGeometryTool.h>

#include "Identifier/Identifier.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/CoordTransformBranch.h"

#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Surfaces/Surface.hpp"

namespace ActsTrk {
    /** @brief Algorithm that scans the tracking geometry and dumps the transforms
     *         and surface bounds of the individual readout elements into a tree
     *         It's used in the context of the validtion of the SQLite workflow 
     *         applied on data */
    class ReadoutGeoDumpAlg : public AthHistogramAlgorithm {
        public:
            using AthHistogramAlgorithm::AthHistogramAlgorithm;


            StatusCode initialize() override;
            StatusCode execute() override;
            StatusCode finalize() override;
        private:
            /// Write a TTree for validation purposes
            MuonVal::MuonTesterTree m_tree{"ReadoutGeometryTest", "ActsReadoutGeoDump"};
            /** @brief the Ahtena identifier of the readout element */
            MuonVal::ScalarBranch<Identifier::value_type>& 
                  m_identifier{m_tree.newScalar<Identifier::value_type>("identifier")};
            /** @brief The DetectorType of the readout element */
            MuonVal::ScalarBranch<std::uint8_t>& m_detType{m_tree.newScalar<std::uint8_t>("detectorType")};
            /** @brief What is the type of the associated Acts Bounds */
            MuonVal::ScalarBranch<std::uint8_t>& m_boundsType{m_tree.newScalar<std::uint8_t>("boundType")};
            /** @brief The defining parameter of the surface bounds */
            MuonVal::VectorBranch<double>& m_boundValues{m_tree.newVector<double>("boundValues")};
            /** @brief Thickness of the readout surface */
            MuonVal::ScalarBranch<double>& m_thickness{m_tree.newScalar<double>("thickness")};
            /** @brief Final transform of the readout element */
            MuonVal::CoordTransformBranch m_readoutTransform{m_tree, "GeoModelTransform"};

            /** @brief Tool handle to the tracking geometry */
            PublicToolHandle<ITrackingGeometryTool> m_trackingGeoTool{this, "TrackingGeometryTool", ""};
            /** @brief Configure which detector types shall be dumped */
            Gaudi::Property<std::set<unsigned short>> m_detTypes{this, "Detectors", {
                                                            Acts::toUnderlying(DetectorType::Pixel),
                                                            Acts::toUnderlying(DetectorType::Sct),
                                                            Acts::toUnderlying(DetectorType::Hgtd)}};
            std::unordered_set<DetectorType> m_selTypes{};

            /** @brief Flag whether the algorithm is executed */
            bool m_executed{false};

    };
};

#endif