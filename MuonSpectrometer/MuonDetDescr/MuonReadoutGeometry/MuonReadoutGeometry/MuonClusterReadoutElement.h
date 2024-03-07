/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONREADOUTGEOMETRY_MUONCLUSTERREADOUTELEMENT_H
#define MUONREADOUTGEOMETRY_MUONCLUSTERREADOUTELEMENT_H


#include <stdexcept>
#include <string>
#include <vector>

#include "MuonReadoutGeometry/MuonReadoutElement.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/SurfaceBounds.h"

namespace MuonGM {

    class MuonDetectorManager;

    class MuonClusterReadoutElement : public MuonReadoutElement {
    public:
        /** struct to hold information needed for TrkDetElementBase, takes owership of all pointers */
        struct SurfaceData {
            /** all surfaces on the muon strip detectors have the same normal vector */
            std::vector<Amg::Vector3D> m_layerNormals{};

            /** the eta and phi bounds of the individual layers are not all identical in the muon system (sTGC staircase) */
            std::vector<std::unique_ptr<Trk::SurfaceBounds>> m_surfBounds{};

            /** there are two transformation per layer as the eta/phi surfaces are rotated by 90 degrees wrt eachother: Total number =
             * 2*number of layers */
            std::vector<Amg::Transform3D> m_layerTransforms{};

            /** there are two surfaces per layer as the eta/phi surfaces are rotated by 90 degrees wrt eachother: Total number = 2*number of
             * layers */
            std::vector<std::unique_ptr<Trk::PlaneSurface>> m_layerSurfaces{};

            /** the eta and phi surfaces for a given layer share the same center: Total number = number of layers */
            std::vector<Amg::Vector3D> m_layerCenters{};

            SurfaceData() = default;
            ~SurfaceData() = default;
        };

      
        ~MuonClusterReadoutElement();

        /** distance to readout.
            If the local position is outside the active volume, the function first shift the position back into the active volume */
        virtual double distanceToReadout(const Amg::Vector2D& pos, const Identifier& id) const = 0;

        /** strip number corresponding to local position.
            If the local position is outside the active volume, the function first shift the position back into the active volume */
        virtual int stripNumber(const Amg::Vector2D& pos, const Identifier& id) const = 0;

        /** strip position
            If the strip number is outside the range of valid strips, the function will return false */
        virtual bool stripPosition(const Identifier& id, Amg::Vector2D& pos) const = 0;

        /** space point position for a given pair of phi and eta identifiers
            The LocalPosition is expressed in the reference frame of the phi surface.
            If one of the identifiers is outside the valid range, the function will return false */
        virtual bool spacePointPosition(const Identifier& phiId, const Identifier& etaId, Amg::Vector2D& pos) const = 0;

        /** Global space point position for a given pair of phi and eta identifiers
            If one of the identifiers is outside the valid range, the function will return false */
        virtual bool spacePointPosition(const Identifier& phiId, const Identifier& etaId, Amg::Vector3D& pos) const = 0;

        /** number of layers in phi/eta projection */
        virtual int numberOfLayers(bool measuresPhi) const = 0;

        /** number of strips per layer */
        virtual int numberOfStrips(const Identifier& layerId) const = 0;
        virtual int numberOfStrips(int layer, bool measuresPhi) const = 0;

        /** clear the cache of the readout elememt */
        virtual void clearCache() override final;

        /** access to chamber surface (phi orientation), uses the first gas gap */
        virtual const Trk::PlaneSurface& surface() const override;
        virtual const Trk::SurfaceBounds& bounds() const override;
        virtual const Amg::Vector3D& center() const override;
        virtual const Amg::Vector3D& normal() const override;
        virtual const Amg::Transform3D& transform() const override;

        /** access to layer surfaces */
        virtual const Trk::PlaneSurface& surface(const Identifier& id) const override;
        virtual const Trk::SurfaceBounds& bounds(const Identifier& id) const override;
        virtual const Amg::Vector3D& center(const Identifier& id) const override;
        virtual const Amg::Vector3D& normal(const Identifier& id) const override;
        virtual const Amg::Transform3D& transform(const Identifier& id) const override;

        /** access to layer surfaces */
        const Trk::PlaneSurface& surface(int surfHash) const;
        const Trk::SurfaceBounds& bounds(int boundHash) const;
        const Amg::Vector3D& center(int layHash) const;
        const Amg::Vector3D& normal(int layHash) const;
        const Amg::Transform3D& transform(int surfHash) const;

        /** returns all the surfaces contained in this detector element */
        virtual std::vector<const Trk::Surface*> surfaces() const;

        /** returns the hash function to be used to look up the center and the normal of the tracking surface for a given identifier */
        virtual int layerHash(const Identifier& id) const = 0;

        /** returns the hash function to be used to look up the surface and surface transform for a given identifier */
        virtual int surfaceHash(const Identifier& id) const = 0;

        /** returns the hash function to be used to look up the surface boundary for a given identifier */
        virtual int boundaryHash(const Identifier& id) const = 0;

        /** returns whether the given identifier measures phi or not */
        virtual bool measuresPhi(const Identifier& id) const = 0;

    protected:
        
        MuonClusterReadoutElement(GeoVFullPhysVol* pv, MuonDetectorManager* mgr, Trk::DetectorElemType detType);

        std::unique_ptr<SurfaceData> m_surfaceData{};
    };

    inline const Trk::PlaneSurface& MuonClusterReadoutElement::surface() const { return surface(0); }
    inline const Amg::Transform3D& MuonClusterReadoutElement::transform() const { return transform(0); }
    inline const Amg::Vector3D& MuonClusterReadoutElement::center() const { return center(0); }
    inline const Amg::Vector3D& MuonClusterReadoutElement::normal() const { return normal(0); }
    inline const Trk::SurfaceBounds& MuonClusterReadoutElement::bounds() const { return bounds(0); }

    inline const Trk::PlaneSurface& MuonClusterReadoutElement::surface(const Identifier& id) const { return surface(surfaceHash(id)); }
    inline const Amg::Transform3D& MuonClusterReadoutElement::transform(const Identifier& id) const { return transform(surfaceHash(id)); }
    inline const Amg::Vector3D& MuonClusterReadoutElement::center(const Identifier& id) const { return center(layerHash(id)); }
    inline const Amg::Vector3D& MuonClusterReadoutElement::normal(const Identifier& id) const { return normal(layerHash(id)); }
    inline const Trk::SurfaceBounds& MuonClusterReadoutElement::bounds(const Identifier& id) const { return bounds(boundaryHash(id)); }

    inline const Trk::PlaneSurface& MuonClusterReadoutElement::surface(int hash) const {
        if (!m_surfaceData) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" "<<__func__<<" Requesting surface but cache is empty");
            throw std::runtime_error("Empty surface cache");
        }
        if (hash == -1 || hash >= (int)m_surfaceData->m_layerSurfaces.size()) {
            ATH_MSG_WARNING(" surface hash out of range: " << hash << " elements "
                            << m_surfaceData->m_layerSurfaces.size());
            return *m_surfaceData->m_layerSurfaces.front();
        }
        return *m_surfaceData->m_layerSurfaces[hash];
    }

    inline const Amg::Transform3D& MuonClusterReadoutElement::transform(int hash) const {
        if (!m_surfaceData) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" "<<__func__<<" Requesting transform but cache is empty");
            throw std::runtime_error("Empty transform cache");
        }
        if (hash == -1 || hash >= (int)m_surfaceData->m_layerTransforms.size()) {
            ATH_MSG_WARNING("transform hash out of range: " << hash << " elements "
                    << m_surfaceData->m_layerTransforms.size());
            return m_surfaceData->m_layerTransforms.front();
        }
        return m_surfaceData->m_layerTransforms[hash];
    }

    inline const Amg::Vector3D& MuonClusterReadoutElement::center(int hash) const {
        if (!m_surfaceData) {
           ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" "<<__func__<<" Requesting center but cache is empty");
           throw std::runtime_error("Empty center cache");
        }
        if (hash == -1 || hash >= (int)m_surfaceData->m_layerCenters.size()) {
            ATH_MSG_WARNING("center hash out of range: " << hash << " elements " << m_surfaceData->m_layerCenters.size());
            return m_surfaceData->m_layerCenters.front();
        }
        return m_surfaceData->m_layerCenters[hash];
    }

    inline const Amg::Vector3D& MuonClusterReadoutElement::normal(int hash) const {
        if (!m_surfaceData) {
           ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" "<<__func__<<" Requesting normal but cache is empty");
           throw std::runtime_error("Empty normal cache");
        }
        if (hash == -1 || hash >= (int)m_surfaceData->m_layerNormals.size()) {
            ATH_MSG_WARNING("normal hash out of range: " << hash << " elements " << m_surfaceData->m_layerNormals.size());
            return m_surfaceData->m_layerNormals.front();
        }
        return m_surfaceData->m_layerNormals[hash];
    }

    inline const Trk::SurfaceBounds& MuonClusterReadoutElement::bounds(int hash) const {
        if (!m_surfaceData) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" "<<__func__<<" Requesting bounds but cache is empty");
            throw std::runtime_error("Empty bounds cache");
        }
        if (hash == -1 || hash >= (int)m_surfaceData->m_surfBounds.size()) {
            ATH_MSG_WARNING("normal hash out of range: " << hash << " elements " << m_surfaceData->m_surfBounds.size());
            return *m_surfaceData->m_surfBounds.front();
        }
        return *m_surfaceData->m_surfBounds[hash];
    }

    inline std::vector<const Trk::Surface*> MuonClusterReadoutElement::surfaces() const {
        std::vector<const Trk::Surface*> elementSurfaces;

        // create when first time requested and when possible
        if (m_surfaceData) {
            elementSurfaces.reserve(m_surfaceData->m_layerSurfaces.size());
            for (const std::unique_ptr<Trk::PlaneSurface>& ptr : m_surfaceData->m_layerSurfaces) {
                elementSurfaces.emplace_back(ptr.get());
            }
        }
        // return the element surfaces
        return elementSurfaces;
    }

}  // namespace MuonGM
#endif  // MUONREADOUTGEOMETRY_MUONCLUSTERREADOUTELEMENT_H
