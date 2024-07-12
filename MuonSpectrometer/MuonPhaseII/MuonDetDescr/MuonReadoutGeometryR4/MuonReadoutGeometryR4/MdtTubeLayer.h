/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_MDTTUBELAYER_H
#define MUONREADOUTGEOMETRYR4_MDTTUBELAYER_H

#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <GeoModelKernel/GeoVPhysVol.h>
#include <GeoModelKernel/GeoTransform.h>
#include <GeoModelKernel/GeoVolumeCursor.h>
#include <GeoModelUtilities/TransientConstSharedPtr.h>

#include <set>
namespace MuonGMR4{
    /// Forward declaration of the MdtTubeLayer
    class MdtTubeLayer;
    using MdtTubeLayerPtr = GeoModel::TransientConstSharedPtr<MdtTubeLayer>;
    /// Helper struct to sort equivalent MdtTubeLayers into a std::set
    struct  MdtTubeLayerSorter{
        bool operator()(const MdtTubeLayer& a, const MdtTubeLayer& b) const;
        bool operator()(const MdtTubeLayerPtr&a, const MdtTubeLayerPtr& b) const;
    };
    using MdtTubeLayerSet = std::set<MdtTubeLayerPtr, MdtTubeLayerSorter>;

    /**
     * Helper struct to retrieve the tube lengths and the tube centers directly from the GeoModel tree
    */
    class MdtTubeLayer{
        /**
         * Constructor taking the GeoModel parent node of the tube nodes
        */
    public:
        
        /** @brief Helper struct to store information about the uncut lengths of 
         *         cut tubes */
        struct CutTubes {
            /** @brief First tube of the cut */
            unsigned int firstTube{0};
            /** @brief Last tube of the cut */
            unsigned int lastTube{0};
            /** @brief Tube length before cut  */
            double unCutHalfLength{0.};
            /** @brief ordering operator for set look up */
            bool operator<(const CutTubes& other) const {
                return lastTube <= other.firstTube;
            }
        };
        using CutTubeSet = std::set<CutTubes, std::less<>>;

        /** @brief Helper struct to avoid browsing with the serial transformer */
        struct TubePositioner{
            /** @brief First tube of the positioning */
            unsigned int firstTube{0};
            /** @brief Last tube of the positioning */
            unsigned int lastTube{0};
            /** @brief Underlying physical volume of the tube */
            PVConstLink tubeVol{};
            /** @brief Transform to reach the first tube */
            Amg::Vector3D firstTubePos{Amg::Vector3D::Zero()};
            /** @brief ordering operator for set look up */
            bool operator<(const TubePositioner& other) const {
                return lastTube < other.firstTube;
            }
        };
        using TubePositionerSet = std::set<TubePositioner, std::less<>>;
        
        friend MdtTubeLayerSorter;
        /// @brief Standard constructor of a MdtTube layer. Taking a GeoVPhysVol
        ///       which is usually shared across multiple layers & chambers
        /// @param layer GeoVPhysVol representing this layer
        /// @param toLayTrf Transformation to reach the layer
        /// @param cutTubes: List of tubes that are cut
        MdtTubeLayer(const PVConstLink layer, 
                     const GeoIntrusivePtr<const GeoTransform> toLayTrf,
                     const CutTubeSet& cutTubes);
        ///@brief Returns the number of tubes in the layer
        unsigned int nTubes() const;
        ///@brief: Returns the transformation from the layer to the muon station
        const Amg::Transform3D& layerTransform() const;
        ///@brief Returns the transformation of the tube to the muon station
        ///       Index counting [0 - nTubes()-1]
        const Amg::Transform3D tubeTransform(const unsigned int tube) const;
        ///@brief Returns the tube position within the given tube layer
        const Amg::Vector3D tubePosInLayer(const unsigned int tube) const;
        ///@brief Returns the half-length of the given tube 
        double tubeHalfLength(const unsigned int tube) const;
        ///@brief Returns the uncut-half length of the given tube
        double uncutHalfLength(const unsigned int tube) const;
        ///@brief returns the PVConst link to the n-th tube [0 - nTubes() -1]
        PVConstLink getTubeNode(unsigned int tube) const;


    private:
        PVConstLink m_layerNode{nullptr};
        GeoIntrusivePtr<const GeoTransform> m_layTrf{nullptr};
        CutTubeSet m_cutTubes{};
        TubePositionerSet m_tubePos{};
        Amg::Vector3D m_tubePitch{Amg::Vector3D::Zero()};
    };

    bool operator<(const MdtTubeLayer::CutTubes& a, const unsigned int tube);
    bool operator<(const unsigned int tube, const MdtTubeLayer::CutTubes& a);

    bool operator<(const MdtTubeLayer::TubePositioner&a,const unsigned int tube);
    bool operator<(const unsigned int tube, const MdtTubeLayer::TubePositioner& a);

}
#endif
