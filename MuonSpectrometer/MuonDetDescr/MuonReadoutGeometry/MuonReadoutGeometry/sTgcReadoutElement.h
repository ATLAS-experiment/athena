/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONREADOUTGEOMETRY_STGCREADOUTELEMENT_H
#define MUONREADOUTGEOMETRY_STGCREADOUTELEMENT_H

#include <string>
#include <utility>

#include "MuonIdHelpers/sTgcIdHelper.h"
#include "MuonReadoutGeometry/MuonChannelDesign.h"
#include "MuonReadoutGeometry/MuonClusterReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MuonPadDesign.h"

class BLinePar;
class GeoVFullPhysVol;

namespace MuonGM {
    /**
       An sTgcReadoutElement corresponds to a single STGC module; therefore
       typicaly a barrel muon station contains:
    */

    class sTgcReadoutElement final : public MuonClusterReadoutElement {
    public:
        /** constructor */

        sTgcReadoutElement(GeoVFullPhysVol* pv, const std::string& stName, int zi, int fi, int mL, MuonDetectorManager* mgr);

        /** destructor */
        ~sTgcReadoutElement();

        /** function to be used to check whether a given Identifier is contained in the readout element */
        virtual bool containsId(const Identifier& id) const override final;

        /** distance to readout.
            If the local position is outside the active volume, the function first shift the position back into the active volume */
        virtual double distanceToReadout(const Amg::Vector2D& pos, const Identifier& id) const override final;

        /** strip number corresponding to local position.
            Should be renamed to channelNumber : the only public access for all hit types */
        virtual int stripNumber(const Amg::Vector2D& pos, const Identifier& id) const override final;

        /** Channel pitch. Gives full pitch for strips, width of a full wire group
        Gives the Height of a pad */
        double channelPitch(const Identifier& id) const;

        /** strip position - should be renamed to channel position
            If the strip number is outside the range of valid strips, the function will return false */
        virtual bool stripPosition(const Identifier& id, Amg::Vector2D& pos) const override final;

        bool stripGlobalPosition(const Identifier& id, Amg::Vector3D& gpos) const;

        /** pad number corresponding to local position */
        int padNumber(const Amg::Vector2D& pos, const Identifier& id) const;

        /** wire number corresponding to local position */
        int wireNumber(const Amg::Vector2D& pos, const Identifier& id) const;

        /** single wire pitch.
         *  sTGC wire pitch is the same for all chambers,
         *  so the default gas gap is set to the 1st gap */
        double wirePitch(int gas_gap = 1) const;

        /** Get the local position of the first wire of the chamber corresponding to the identifier */
        double positionFirstWire(const Identifier& id) const;

        /** Get the total number of wires (single wires) of a chamber **/
        int numberOfWires(const Identifier& id) const;

        /** pad position */
        bool padPosition(const Identifier& id, Amg::Vector2D& pos) const;

        /** pad global position */
        bool padGlobalPosition(const Identifier& id, Amg::Vector3D& gpos) const;

        /** pad corners */
        bool padCorners(const Identifier& id, std::array<Amg::Vector2D,4>& corners) const;

        /** pad global corners */
        bool padGlobalCorners(const Identifier& id, std::array<Amg::Vector3D, 4>& gcorners) const;

        /** is eta=0 of QL1 or QS1?
            Support for Strip and Pad cathodes is valid when the
            Strip, Pad and Wire surfaces have the same dimensions. */
        bool isEtaZero(const Identifier& id, const Amg::Vector2D& localPosition) const;

        /** number of layers in phi/eta projection */
        virtual int numberOfLayers(bool) const override final;

        /** number of strips per layer */
        virtual int numberOfStrips(const Identifier& layerId) const override final;
        virtual int numberOfStrips(int, bool measuresPhi) const override final;

        /** Get the number of pad per layer */
        int numberOfPads(const Identifier& layerId) const;

        /** Get largest pad number, which is different to the number of pads in 
            a gas volume due to the pad numbering in Athena */
        int maxPadNumber(const Identifier& layerId) const;

        /** space point position for a given pair of phi and eta identifiers
            The LocalPosition is expressed in the reference frame of the phi
           surface. If one of the identifiers is outside the valid range, the
           function will return false */
        virtual bool spacePointPosition(const Identifier& phiId, const Identifier& etaId, Amg::Vector2D& pos) const override final;

        /** Global space point position for a given pair of phi and eta identifiers
            If one of the identifiers is outside the valid range, the function will
           return false */
        virtual bool spacePointPosition(const Identifier& phiId, const Identifier& etaId, Amg::Vector3D& pos) const override final;

        /** space point position for a pair of phi and eta local positions and a layer identifier
            The LocalPosition is expressed in the reference frame of the phi projection.
        */
        void spacePointPosition(const Amg::Vector2D& phiPos, const Amg::Vector2D& etaPos, Amg::Vector2D& pos) const;

        /** space point position, corrected for chamber deformations (b-lines), if b-lines are enabled.
            Accepts a precision (x) coordinate and a y-seed, in the local layer frame, and returns a 3D position, 
            in the same frame so that sTgcReadoutElement::transform() can be directly cast on it. Accounts for:
            a) PCB deformations (as-built), if as-built conditions are enabled
            b) Chamber deformations (b-lines), if b-lines are enabled
        */
        void spacePointPosition(const Identifier& layerId, double locXpos, double locYpos, Amg::Vector3D& pos) const;

        /** simHit local (SD) To Global position - to be used by MuonGeoAdaprors only      */
        Amg::Vector3D localToGlobalCoords(const Amg::Vector3D& locPos, Identifier id) const;

        /** @brief function to fill tracking cache */
        virtual void fillCache() override final;

        /** @brief returns the hash to be used to look up the surface and transform in the MuonClusterReadoutElement tracking cache */
        virtual int surfaceHash(const Identifier& id) const override final;

        /** @brief returns the hash to be used to look up the surface and transform in the MuonClusterReadoutElement tracking cache */
        int surfaceHash(int gasGap, int channelType) const;

        /** @brief returns the hash to be used to look up the normal and center in the MuonClusterReadoutElement tracking cache */
        virtual int layerHash(const Identifier& id) const override;

        /** @brief returns the hash to be used to look up the normal and center in the MuonClusterReadoutElement tracking cache */
        // int layerHash( int gasGap) const;        // does not fit in the scheme ( layer hash needs to follow surface hash )

        /** returns the hash function to be used to look up the surface boundary for a given identifier */
        virtual int boundaryHash(const Identifier& id) const override final;

        /** @brief returns whether the current identifier corresponds to a phi measurement */
        virtual bool measuresPhi(const Identifier& id) const override final;

        /** @brief initialize the design classes for this readout element */
        void initDesign(double largeX, double smallX, double lengthY, double stripPitch, double wirePitch, double stripWidth,
                        double wireWidth, double thickness);

        /** returns the MuonChannelDesign class for the given identifier */
        const MuonChannelDesign* getDesign(const Identifier& id) const;

        /** returns the MuonChannelDesign class  */
        const MuonChannelDesign* getDesign(int gasGap, int channelType) const;

        /** returns the MuonChannelDesign class for the given identifier */
        const MuonPadDesign* getPadDesign(const Identifier& id) const;
        MuonPadDesign* getPadDesign(const Identifier& id);

        /** returns the MuonChannelDesign */
        const MuonPadDesign* getPadDesign(int gasGap) const;

        /** set methods only to be used by MuonGeoModel */
        void setChamberLayer(int ml) { m_ml = ml; }

        // double getSectorOpeningAngle(bool isLargeSector);

        /** read A-line parameters and include the chamber rotation/translation 
            in the local-to-global (ATLAS) reference frame transformaton */
        const Amg::Transform3D& getDelta() const { return m_delta; }
        void setDelta(const ALinePar& aline);
       
        /** read B-line (chamber-deformation) parameters */
        void setBLinePar(const BLinePar& bLine);
    
        /** transform a position (in chamber-frame coordinates) to the deformed-chamber geometry */
        void posOnDefChamber(Amg::Vector3D& locPosML) const;

        bool  has_ALines() const { return (m_ALinePar != nullptr); }
        bool  has_BLines() const { return (m_BLinePar != nullptr); }
        const ALinePar* getALinePar() const { return m_ALinePar; }
        const BLinePar* getBLinePar() const { return m_BLinePar; }
        void  clearALinePar();
        void  clearBLinePar() { m_BLinePar = nullptr; }

        // Amdb local (szt) to global coord
        virtual Amg::Vector3D AmdbLRSToGlobalCoords(const Amg::Vector3D& x) const override final { return AmdbLRSToGlobalTransform()*x; }
        virtual Amg::Transform3D AmdbLRSToGlobalTransform() const override final { return absTransform()*Amg::Translation3D(0, 0, m_offset)*getDelta(); }
        // Global to Amdb local (szt) coord
        virtual Amg::Vector3D GlobalToAmdbLRSCoords(const Amg::Vector3D& x) const override final { return GlobalToAmdbLRSTransform()*x; }
        virtual Amg::Transform3D GlobalToAmdbLRSTransform() const override final { return AmdbLRSToGlobalTransform().inverse(); }

        static double triggerBandIdToRadius(bool isLarge, int triggerBand); 
        
    private:
        std::vector<MuonChannelDesign> m_phiDesign;
        std::vector<MuonChannelDesign> m_etaDesign;
        std::vector<MuonPadDesign> m_padDesign;

        std::vector<int> m_nStrips;
        std::vector<int> m_nWires;
        std::vector<int> m_nPads;
        int    m_nlayers{0};
        int    m_ml{0};
        double m_offset{0.};
        
        double m_sWidthChamber{0.}; // bottom base length (full chamber)
        double m_lWidthChamber{0.}; // top base length (full chamber)
        double m_lengthChamber{0.}; // radial size (full chamber)
        double m_tckChamber{0.};    // thickness (full chamber)
        bool   m_diamondShape{false};
        const ALinePar*  m_ALinePar{nullptr};
        const BLinePar*  m_BLinePar{nullptr};
        Amg::Transform3D m_delta{Amg::Transform3D::Identity()};

        // const double m_largeSectorOpeningAngle = 28.0;
        // const double m_smallSectorOpeningAngle = 17.0;

        // surface dimensions for strips
        std::vector<double> m_halfX;
        std::vector<double> m_minHalfY;
        std::vector<double> m_maxHalfY;
        // surface dimensions for pads and wires
        std::vector<double> m_PadhalfX;
        std::vector<double> m_PadminHalfY;
        std::vector<double> m_PadmaxHalfY;

        // transforms (RE->layer)
        Amg::Transform3D m_Xlg[4];


        // The radial positions of the trigger bands cannot be derived from the readout geometry, therefore hard-coding them for now
        //Position of Band IDs for Large sectors
        static constexpr std::array<double, 94>  LBANDIDSP = {
          10, 10, 10, 10, 10, 10, 1156.72, 1187.98, 1231.87, 1271.34, 1312.87, 1354.56, 1396.38, 1438.07,
          1479.73, 1521.44, 1563.11, 1604.8, 1646.48, 1688.02, 1729.84, 1771.51, 1813.2, 1854.89, 1896.57, 1938.26, 1979.93, 2021.61,
          2063.14, 2104.98, 2146.55, 2181.64, 2209.01, 2251.65, 2282.54, 2313.27, 2356.24, 2396.73, 2438.29, 2480.09, 2521.75, 2563.46,
          2605.11, 2646.85, 2688.48, 2730.19, 2771.86, 2813.41, 2855.21, 2896.93, 2938.61, 2980.26, 3021.95, 3063.63, 3105.31, 3146.98,
          3188.85, 3230.37, 3272.05, 3313.77, 3353.77, 3376.19, 3426.09, 3464.49, 3506.78, 3563.91, 3589.03, 3626.17, 3667.84, 3709.56,
          3751.33, 3792.92, 3834.58, 3876.27, 3917.9, 3959.62, 4001.29, 4043.03, 4084.66, 4126.39, 4168.05, 4209.74, 4251.38, 4293.16,
          4334.72, 4376.47, 4417.54, 4459.75, 4496.31, 4543.27, 4584.77, 4626.47, 4668.25, 4701.14
        };

        //Position of Band IDs for Small Sector
        static constexpr std::array<double, 92> SBANDIDSP  = {
          10.0, 10.0, 958.077, 998.248, 1037.405, 1076.535, 1115.69, 1154.82, 1193.97, 1233.135, 1272.265, 1311.395,
          1350.59, 1389.705, 1428.865, 1468.01, 1507.175, 1546.305, 1585.435, 1624.58, 1663.71, 1702.895, 1742.055,
          1781.165, 1820.315, 1859.44, 1898.575, 1937.75, 1976.885, 2016.04, 2055.15, 2094.345, 2136.125, 2172.61,
          2217.68, 2255.125, 2316.115, 2348.91, 2388.06, 2427.245, 2466.385, 2505.515, 2544.69, 2583.8, 2622.99,
          2662.115, 2701.31, 2740.395, 2779.55, 2818.715, 2857.905, 2897.0, 2936.185, 2975.315, 3014.47, 3053.615,
          3092.775, 3131.895, 3171.075, 3210.225, 3249.375, 3288.485, 3317.74, 3347.075, 3396.65, 3440.175, 3475.575,
          3540.81, 3581.97, 3621.13, 3660.285, 3699.41, 3738.535, 3777.73, 3816.89, 3856.055, 3895.105, 3934.3, 3974.34,
          4012.565, 4051.71, 4090.865, 4130.04, 4169.145, 4208.285, 4247.55, 4286.65, 4320.075, 4364.84, 4404.12, 4443.14, 4482.29
        };

    };

    inline int sTgcReadoutElement::surfaceHash(const Identifier& id) const {
        return surfaceHash(manager()->stgcIdHelper()->gasGap(id), manager()->stgcIdHelper()->channelType(id));
    }

    inline int sTgcReadoutElement::surfaceHash(int gasGap, int channelType) const {
        return (gasGap - 1) * 3 + (2 - channelType);  // assumes channelType=2 (wires), 1(strips), 0(pads)
    }

    inline int sTgcReadoutElement::layerHash(const Identifier& id) const {
        return surfaceHash(id);  // don't have a choice here : rewrite MuonClusterReadoutElement first
    }

    inline int sTgcReadoutElement::boundaryHash(const Identifier& id) const {
        int iphi = manager()->stgcIdHelper()->channelType(id) != sTgcIdHelper::sTgcChannelTypes::Strip;  // wires and pads have locX oriented along phi
        if (std::abs(getStationEta()) < 3) iphi += 2 * (manager()->stgcIdHelper()->gasGap(id) - 1);
        return iphi;
    }

    inline bool sTgcReadoutElement::measuresPhi(const Identifier& id) const { 
        return (manager()->stgcIdHelper()->channelType(id) != sTgcIdHelper::sTgcChannelTypes::Strip); 
    }

    inline const MuonChannelDesign* sTgcReadoutElement::getDesign(const Identifier& id) const {
        if (manager()->stgcIdHelper()->channelType(id) == sTgcIdHelper::sTgcChannelTypes::Strip) return &(m_etaDesign[manager()->stgcIdHelper()->gasGap(id) - 1]);
        if (manager()->stgcIdHelper()->channelType(id) == sTgcIdHelper::sTgcChannelTypes::Wire) return &(m_phiDesign[manager()->stgcIdHelper()->gasGap(id) - 1]);
        return nullptr;
    }

    inline const MuonPadDesign* sTgcReadoutElement::getPadDesign(const Identifier& id) const {
        if (manager()->stgcIdHelper()->channelType(id) == sTgcIdHelper::sTgcChannelTypes::Pad) return &(m_padDesign[manager()->stgcIdHelper()->gasGap(id) - 1]);
        return nullptr;
    }

    inline MuonPadDesign* sTgcReadoutElement::getPadDesign(const Identifier& id) {
        if (manager()->stgcIdHelper()->channelType(id) == sTgcIdHelper::sTgcChannelTypes::Pad) return &(m_padDesign[manager()->stgcIdHelper()->gasGap(id) - 1]);
        return nullptr;
    }

    inline const MuonChannelDesign* sTgcReadoutElement::getDesign(int gasGap, int channelType) const {
        if (channelType == 1) return &(m_etaDesign[gasGap - 1]);
        if (channelType == 2) return &(m_phiDesign[gasGap - 1]);
        return 0;
    }

    inline const MuonPadDesign* sTgcReadoutElement::getPadDesign(int gasGap) const { return &(m_padDesign[gasGap - 1]); }

    inline double sTgcReadoutElement::distanceToReadout(const Amg::Vector2D& pos, const Identifier& id) const {
        const MuonChannelDesign* design = getDesign(id);
        if (!design) return -10000.;
        return design->distanceToReadout(pos);
    }

    inline int sTgcReadoutElement::stripNumber(const Amg::Vector2D& pos, const Identifier& id) const {
        if (manager()->stgcIdHelper()->channelType(id) == sTgcIdHelper::sTgcChannelTypes::Pad) return padNumber(pos, id);

        const MuonChannelDesign* design = getDesign(id);
        if (!design) return -1;
        return design->channelNumber(pos);
    }

    inline bool sTgcReadoutElement::stripPosition(const Identifier& id, Amg::Vector2D& pos) const {
        if (manager()->stgcIdHelper()->channelType(id) == sTgcIdHelper::sTgcChannelTypes::Pad) return padPosition(id, pos);

        const MuonChannelDesign* design = getDesign(id);
        if (!design) return 0;
        return design->center(manager()->stgcIdHelper()->channel(id), pos);
    }

    inline bool sTgcReadoutElement::stripGlobalPosition(const Identifier& id, Amg::Vector3D& gpos) const {
        Amg::Vector2D lpos{Amg::Vector2D::Zero()};
        if (!stripPosition(id, lpos)) return false;
        surface(id).localToGlobal(lpos, Amg::Vector3D::Zero(), gpos);
        return true;
    }

    inline bool sTgcReadoutElement::padPosition(const Identifier& id, Amg::Vector2D& pos) const {
        const MuonPadDesign* design = getPadDesign(id);
        if (!design) return false;

        int padEta = manager()->stgcIdHelper()->padEta(id);
        int padPhi = manager()->stgcIdHelper()->padPhi(id);

        return design->channelPosition(std::make_pair(padEta, padPhi), pos);
    }

    inline bool sTgcReadoutElement::padGlobalPosition(const Identifier& id, Amg::Vector3D& gpos) const {
        Amg::Vector2D lpos{Amg::Vector2D::Zero()};
        if (!padPosition(id, lpos)) return false;
        surface(id).localToGlobal(lpos, Amg::Vector3D::Zero(), gpos);
        return true;
    }

    inline bool sTgcReadoutElement::padCorners(const Identifier& id, std::array<Amg::Vector2D, 4>& corners) const {
        const MuonPadDesign* design = getPadDesign(id);
        if (!design) return false;

        int padEta = manager()->stgcIdHelper()->padEta(id);
        int padPhi = manager()->stgcIdHelper()->padPhi(id);

        return design->channelCorners(std::make_pair(padEta, padPhi), corners);
    }

    inline bool sTgcReadoutElement::padGlobalCorners(const Identifier& id, std::array<Amg::Vector3D, 4>& gcorners) const {
        std::array<Amg::Vector2D,4> lcorners{make_array<Amg::Vector2D, 4>(Amg::Vector2D::Zero())};
        if (!padCorners(id, lcorners)) {
            return false;
        }
        for (size_t c = 0; c < lcorners.size() ; ++c) {
            surface(id).localToGlobal(lcorners[c], Amg::Vector3D::Zero(), gcorners[c]);
        }
        return true;
    }

    // This function returns true if we are in the eta 0 region of QL1/QS1
    inline bool sTgcReadoutElement::isEtaZero(const Identifier& id, const Amg::Vector2D& localPosition) const {
        const sTgcIdHelper* idHelper = manager()->stgcIdHelper();

        // False if not a QL1 or QS1 quadruplet
        if (std::abs(idHelper->stationEta(id)) != 1) return false; 

        const MuonChannelDesign*
        wireDesign = (idHelper->channelType(id) == sTgcIdHelper::sTgcChannelTypes::Wire) ?
                     getDesign(id) :
                     getDesign(idHelper->channelID(id,
                                                   idHelper->multilayer(id),
                                                   idHelper->gasGap(id),
                                                   sTgcIdHelper::sTgcChannelTypes::Wire,
                                                   1));
        if (!wireDesign) return false;

        // Require the x coordinate for strips, and the y coordinate for wires and pads
        double lpos = (idHelper->channelType(id) == sTgcIdHelper::sTgcChannelTypes::Strip) ?
                      localPosition.x() : localPosition.y();
        if (lpos < 0.5 * wireDesign->xSize() - wireDesign->wireCutout) return true;

        return false;
    }

    inline int sTgcReadoutElement::numberOfLayers(bool) const { return m_nlayers; }

    inline int sTgcReadoutElement::numberOfStrips(const Identifier& layerId) const {
        return numberOfStrips(manager()->stgcIdHelper()->gasGap(layerId) - 1, 
                             manager()->stgcIdHelper()->channelType(layerId) == sTgcIdHelper::sTgcChannelTypes::Wire);
    }

    inline int sTgcReadoutElement::numberOfStrips(int lay, bool measPhi) const {
        if (lay > -1 && lay < m_nlayers) { return measPhi ? m_nWires[lay] : m_nStrips[lay]; }
        return -1;
    }

    inline int sTgcReadoutElement::numberOfPads(const Identifier& layerId) const {
        const MuonPadDesign* design = getPadDesign(layerId);
        if (!design) {
            MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
            log << MSG::WARNING << "no pad design found when trying to get the number of pads" << endmsg;
            return 0;
        }
        return design->nPadColumns * design->nPadH;
    }

    inline int sTgcReadoutElement::maxPadNumber(const Identifier& layerId) const {
        const MuonPadDesign* design = getPadDesign(layerId);
        if (!design) {
            MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
            log << MSG::WARNING << "no pad design found when trying to get the largest pad number" << endmsg;
            return 0;
        }
        return (design->nPadColumns - 1) * manager()->stgcIdHelper()->padEtaMax() + design->nPadH;
    }

    inline bool sTgcReadoutElement::spacePointPosition(const Identifier& phiId, const Identifier& etaId, Amg::Vector2D& pos) const {
        Amg::Vector2D phiPos;
        Amg::Vector2D etaPos;
        if (!stripPosition(phiId, phiPos) || !stripPosition(etaId, etaPos)) return false;
        spacePointPosition(phiPos, etaPos, pos);
        return true;
    }

    inline bool sTgcReadoutElement::spacePointPosition(const Identifier& phiId, const Identifier& etaId, Amg::Vector3D& pos) const {
        Amg::Vector2D lpos;
        spacePointPosition(phiId, etaId, lpos);
        surface(phiId).localToGlobal(lpos, pos, pos);
        return true;
    }

    inline void sTgcReadoutElement::spacePointPosition(const Amg::Vector2D& phiPos, const Amg::Vector2D& etaPos, Amg::Vector2D& pos) const {
        pos[0] = phiPos.x();
        pos[1] = etaPos.x();
    }


    inline double sTgcReadoutElement::triggerBandIdToRadius(bool isLarge, int triggerBand) {
        if(isLarge){
            return LBANDIDSP[triggerBand];
        } else {
            return SBANDIDSP[triggerBand]; 
        }
    }; 

}  // namespace MuonGM

#endif  // MUONREADOUTGEOMETRY_STGCREADOUTELEMENT_H
