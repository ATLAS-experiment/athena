/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Rpc Readout Element properties
 -----------------------------------------
***************************************************************************/

#ifndef MUONREADOUTGEOMETRY_RPCREADOUTELEMENT_H
#define MUONREADOUTGEOMETRY_RPCREADOUTELEMENT_H

#include "MuonIdHelpers/RpcIdHelper.h"
#include "MuonReadoutGeometry/MuonClusterReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MuonStripDesign.h"
#include "CxxUtils/ArrayHelper.h"


class MuonReadoutGeomCnvAlg;

namespace MuonGM {
    /**
       An RpcReadoutElement corresponds to a single RPC module; therefore
       typicaly a barrel muon station contains:
       - 4 RpcReadoutElements in the middle layer, two adjacent to each
       other (in z) on the inner side (low-pt plane) and two on the
       outer side (pivot plane);
       - 2 RpcReadoutElements in the outer layer (inner or outer side
       depends on the Large or Small sector type).
       RpcReadoutElements are identified by StationName, StationEta, StationPhi,
       Technology=2, doubletR, doubletZ and doubletPhi (the latter being
       actually needed only in a few special cases). Therefore the granularity
       of the data-collections is less fine-grained than the granularity of the
       geometry description (1 collection -> 2 RpcReadoutElements, typically).

       Pointers to all RpcReadoutElements are created in the build() method of
       the MuonChamber class, and are held in arrays by the MuonDetectorManager,
       which is responsible for storing, deleting and providing access to these
       objects.

       An RpcReadoutElement holds properties related to its internal structure
       (i.e. number of strip panels) and general geometrical properties (size);
       it implements tracking interfaces and provide access to typical
       readout-geometry information: i.e. number of strips, strip positions, etc.

       The globalToLocalCoords and globalToLocalTransform methods (+ their
       opposite) define the link between the ATLAS global reference frame and
       the internal (geo-model defined) local reference frame of any gas gap
       volume (which is the frame where local coordinates of SimHits, in output
       from G4, are expressed).
    */

    class RpcReadoutElement final : public MuonClusterReadoutElement {
        friend class ::MuonReadoutGeomCnvAlg;
        friend class MuonChamber;
        friend class MuonChamberLite;

    public:
        /** constructor */
        RpcReadoutElement(GeoVFullPhysVol* pv, const std::string& stName, int zi, int fi, bool is_mirrored, MuonDetectorManager* mgr);

        /** destructor */
        ~RpcReadoutElement();

        int getDoubletR() const;  //!< return DoubletR value for the given readout element
        int getDoubletZ() const;  //!< return DoubletZ value for the given readout element
        int getDoubletPhi() const;  //!< return DoubletPhi value for the given readout element, be aware that one RE can contain two DoubletPhis!!!!
        bool hasDEDontop() const;  //!< return whether the RPC is 'up-side-down'

        /** function to be used to check whether a given Identifier is contained in the readout element */
        virtual bool containsId(const Identifier& id) const override;

        /** returns whether the RE is in the ribs of the muon spectrometer */
        bool inTheRibs() const;

        int nGasGapPerLay() const;            //!< returns the number of gasgaps
        int NphiStripPanels() const;                //!< returns the number of phi strip panels (1 or 2)
        int NphiStrips() const;                     //!< returns the number of phi strips
        int NetaStrips() const;                     //!< returns the number of eta strips
        int Nstrips(bool measphi  ) const;             //!< returns the number of strips for the phi or eta plane
        double StripWidth(bool measphi  ) const;       //!< returns the strip width for the phi or eta plane
        double StripLength(bool measphi  ) const;      //!< returns the strip length for the phi or eta plane
        double StripPitch(bool measphi  ) const;       //!< returns the strip pitch for the phi or eta plane
        double StripPanelDead(bool measphi  ) const;   //!< returns strip panel dead area for the phi or eta plane
        double stripPanelSsize(bool measphi  ) const;  //!< returns strip panel S size for the phi or eta plane
        double stripPanelZsize(bool measphi  ) const;  //!< returns strip panel Z size for the phi or eta plane
        double gasGapSsize() const;                 //!< returns the gas gap S size
        double gasGapZsize() const;                 //!< returns the gas gap Z size

        /** distance to readout.
            If the local position is outside the active volume, the function first shift the position back into the active volume */
        virtual double distanceToReadout(const Amg::Vector2D& pos, const Identifier& id) const override final;

        /** strip number corresponding to local position.
            If the local position is outside the active volume, the function first shift the position back into the active volume */
        virtual int stripNumber(const Amg::Vector2D& pos, const Identifier& id) const override final;

        /** strip position
            If the strip number is outside the range of valid strips, the function will return false */
        virtual bool stripPosition(const Identifier& id, Amg::Vector2D& pos) const override final;

        /** number of layers in phi/eta projection, same for eta/phi planes */
        virtual int numberOfLayers(bool measphi   = true) const override final;
        void setNumberOfLayers(const int = 2);

        /** number of strips per layer */
        virtual int numberOfStrips(const Identifier& layerId) const override final;
        virtual int numberOfStrips(int, bool measuresPhi) const override final;

        /** space point position for a given pair of phi and eta identifiers
            The LocalPosition is expressed in the reference frame of the phi surface.
            If one of the identifiers is outside the valid range, the function will return false */
        virtual bool spacePointPosition(const Identifier& phiId, const Identifier& etaId, Amg::Vector2D& pos) const override final;

        /** Global space point position for a given pair of phi and eta identifiers
            If one of the identifiers is outside the valid range, the function will
           return false */
        virtual bool spacePointPosition(const Identifier& phiId, const Identifier& etaId, Amg::Vector3D& pos) const override final;

        /** space point position for a pair of phi and eta local positions and a layer identifier
            The LocalPosition is expressed in the reference frame of the phi projection.
        */
        void spacePointPosition(const Amg::Vector2D& phiPos, const Amg::Vector2D& etaPos, Amg::Vector2D& pos) const;

        /** @brief center of an RPC RE is not trivially the center of the first surface, overloading MuonClusterReadoutElement */
        const Amg::Vector3D REcenter() const;

        /** @brief function to fill tracking cache */
        virtual void fillCache() override final;

        /** @brief returns the hash to be used to look up the surface and transform in the MuonClusterReadoutElement tracking cache */
        virtual int surfaceHash(const Identifier& id) const override final;

        /** @brief returns the hash to be used to look up the surface and transform in the MuonClusterReadoutElement tracking cache */
        int surfaceHash(int doubletPhi, int gasGap, bool measphi  ) const;

        /** @brief returns the hash to be used to look up the normal and center in the MuonClusterReadoutElement tracking cache */
        virtual int layerHash(const Identifier& id) const override final;
        /** @brief returns the hash to be used to look up the normal and center in the MuonClusterReadoutElement tracking cache */
        int layerHash(int doubletPhi, int gasGap) const;

        /** returns the hash function to be used to look up the surface boundary for a given identifier */
        virtual int boundaryHash(const Identifier& id) const override final;

        /** @brief returns whether the current identifier corresponds to a phi measurement */
        virtual bool measuresPhi(const Identifier& id) const override final;

        /** @brief initialize the design classes for this readout element */
        void initDesign();

        void setDoubletR(int doubletR);
        void setDoubletZ(int dobuletZ);
        void setDoubletPhi(int doubletPhi);
        bool rotatedRpcModule() const;

        void setYTranslation(const double y);
        void setZTranslation(const double z);


        // local to global
        /// Attention: these transformations do not represent the tracking transformations as obtained
        ///            by the transform() method. The latter follow the convention that the x-axis
        ///            is always perpendicular to the respective strips and z points towars the sky
        ///         These transformations follow the AMDB conventions with displaced origins
        Amg::Vector3D localToGlobalCoords(const Amg::Vector3D& x, const Identifier& id) const;
        Amg::Transform3D localToGlobalTransf(const Identifier& id) const;
        Amg::Transform3D localToGlobalTransf(int doubletPhi, int gasGap) const;
        // global to local
    
        Amg::Vector3D globalToLocalCoords(const Amg::Vector3D& x, const Identifier& id) const;
        Amg::Transform3D globalToLocalTransf(const Identifier& id) const;

        Amg::Vector3D stripPos(const Identifier& id) const;
        Amg::Vector3D stripPos(int doubletPhi, int gasGap, bool measphi, int strip) const;

        /// Returns the local strip position in the AMDB frame
        Amg::Vector3D localStripPos(int doubletPhi, int gasGap, bool measphi, int strip) const;       
        Amg::Vector3D localStripPos(const Identifier& id) const;

    private:    
        Amg::Vector3D localGasGapPos(const Identifier& id) const;
        Amg::Vector3D localGasGapPos(int doubletPhi, int gasGap) const;

        Amg::Vector3D gasGapPos(int doubletPhi, int gasGap) const;
        Amg::Vector3D gasGapPos(const Identifier& id) const;

    public:
        // Readout side infos
        // P is a point in the global reference frame
        // we want to have the distance from the side of the phi readout (length travelled along a phi strip) from a signal produced at P)
        double distanceToPhiReadout(const Amg::Vector3D& P) const;
        // P is a point in the global reference frame
        // we want to have the distance from the side of the eta readout (length travelled along a eta strip) from a signal produced at P)
        double distanceToEtaReadout(const Amg::Vector3D& P) const;

    
        inline bool isMirrored() const { return m_mirrored; }       
        inline bool isDescrAtNegZ() const { return m_descratzneg; }
    private: 
        double localStripSCoord(int doubletPhi, bool measphi, int strip) const;
        double localStripZCoord(bool measphi, int strip) const;
        const RpcIdHelper& m_idHelper{idHelperSvc()->rpcIdHelper()};
        bool m_mirrored{false};
        bool m_descratzneg{false};
        /** returns the MuonStripDesign class for the given identifier */
        const MuonStripDesign* getDesign(const Identifier& id) const;

        int m_dbR{0};
        int m_dbZ{0};
        int m_dbPhi{0};
        bool m_hasDEDontop{false};
        int m_nlayers{2};  
        // default=2, all BI RPCs always have 3 gas gaps, need this flag since amdb only supports a maximum of 2 gas
        // gaps, so this is steering the hardcoded third gas gap for Run3/4 layouts based on amdb primary numbers

        bool m_inTheRibs{false};

        int m_nphigasgaps{-1};

        double m_gasgapssize{-9999.};
        double m_gasgapzsize{-9999.};

        int m_nphistrippanels{-1};
        int m_nphistripsperpanel{-1};
        int m_netastripsperpanel{-1};
        double m_phistripwidth{-9999.};
        double m_etastripwidth{-9999.};
        double m_phistrippitch{-9999.};
        double m_etastrippitch{-9999.};
        double m_phistriplength{-9999.};
        double m_etastriplength{-9999.};
        double m_phipaneldead{-9999.};
        double m_etapaneldead{-9999.};
        
        static constexpr int s_maxphipanels = 2;
        static constexpr int s_nLayers = 3;
        std::array<double, s_maxphipanels> m_first_phistrip_s{ make_array<double, s_maxphipanels>(-9999.)};
        std::array<double, s_maxphipanels> m_etastrip_s{make_array<double, s_maxphipanels>(-9999.)};
        /// Array caching the distance of the gasGap center to the origin plane in global radial  direction
        std::array<double, s_nLayers>     m_gasGap_xPos{make_array<double,s_nLayers>(-9999.)};
        double m_phistrip_z{-9999.};
        double m_first_etastrip_z{-9999.};

        std::vector<MuonStripDesign> m_phiDesigns{};
        std::vector<MuonStripDesign> m_etaDesigns{};

        double m_y_translation{0.f};
        double m_z_translation{0.f};
    };


}  // namespace MuonGM

#include "MuonReadoutGeometry/RpcReadoutElement.icc"

#endif  // MUONREADOUTGEOMETRY_RPCREADOUTELEMENT_H
