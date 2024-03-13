/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_MDTTUBELAYER_H
#define MUONREADOUTGEOMETRYR4_MDTTUBELAYER_H

#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <GeoModelKernel/GeoVPhysVol.h>
#include <GeoModelKernel/GeoTransform.h>
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
        friend MdtTubeLayerSorter;
        /// @brief Standard constructor of a MdtTube layer. Taking a GeoVPhysVol
        ///       which is usually shared across multiple layers & chambers
        /// @param layer GeoVPhysVol representing this layer
        /// @param toLayTrf Transformation to reach the layer
        MdtTubeLayer(const PVConstLink layer, 
                     const GeoIntrusivePtr<const GeoTransform> toLayTrf);
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
        ///@brief returns the PVConst link to the n-th tube [0 - nTubes() -1]
        PVConstLink getTubeNode(unsigned int tube) const;
    private:
        PVConstLink m_layerNode{nullptr};
        GeoIntrusivePtr<const GeoTransform> m_layTrf{nullptr};      
    };

}
#endif