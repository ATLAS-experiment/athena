/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONREADOUTGEOMETRY_TGCREADOUTELEMENT_H
#define MUONREADOUTGEOMETRY_TGCREADOUTELEMENT_H


#include "Identifier/Identifier.h"
#include "MuonReadoutGeometry/MuonClusterReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/TgcReadoutParams.h"
#include "CxxUtils/ArrayHelper.h"


namespace MuonGM {

    /**
       A TgcReadoutElement corresponds to a single TGC chamber; therefore typically
       a TGC station contains several TgcReadoutElements.  TgcReadoutElements are
       identified by StationName, StationEta, StationPhi, Technology=3. Therefore
       the granularity of the data-collections is equal to the granularity of the
       geometry description (1 collection -> 1 TgcReadoutElement).

       Pointers to all TgcReadoutElements are created in the build() method of the
       MuonChamber class, and are held in arrays by the MuonDetectorManager, which
       is responsible for storing, deleting and provide access to these objects.

       A TgcReadoutElement holds properties related to its internal structure
       (i.e. number of strip panels) and general geometrical properties (size);
       it implements tracking interfaces and provide access to typical
       readout-geometry information: i.e. number of strips, strip and wire-gang
       positions, etc.

       The globalToLocalCoords and globalToLocalTransform methods (+ their
       opposite) define the link between the ATLAS global reference frame and the
       internal (geo-model defined) local reference frame of any gas gap volume
       (which is the frame where local coordinates of SimHits, in output from G4,
       are expressed).
    */

    class TgcReadoutElement final : public MuonClusterReadoutElement {

    public:
        TgcReadoutElement(GeoVFullPhysVol* pv, const std::string& stName, MuonDetectorManager* mgr);

        virtual ~TgcReadoutElement() = default;

        /** distance to readout.
            If the local position is outside the active volume, the function first shift the position back into the active volume */
        virtual double distanceToReadout(const Amg::Vector2D& pos, const Identifier& id) const override;

        /** strip number corresponding to local position.
            If the local position is outside the active volume, the function first shift the position back into the active volume */
        virtual int stripNumber(const Amg::Vector2D& pos, const Identifier& id) const override final;

        /** strip position
            If the strip number is outside the range of valid strips, the function will return false */
        virtual bool stripPosition(const Identifier& id, Amg::Vector2D& pos) const override;

        /** returns the hash function to be used to look up the center and the normal of the tracking surface for a given identifier */
        virtual  int layerHash(const Identifier& id) const override;

        /** returns the hash function to be used to look up the surface and surface transform for a given identifier */
        virtual  int surfaceHash(const Identifier& id) const override;

        /** returns the hash function to be used to look up the surface boundary for a given identifier */
        virtual  int boundaryHash(const Identifier& id) const override;

        /** returns whether the given identifier measures phi or not */
        virtual  bool measuresPhi(const Identifier& id) const override;

        /** number of layers in phi/eta projection */
        virtual int numberOfLayers(bool isStrip) const override;

        /** number of strips per layer */
        virtual int numberOfStrips(const Identifier& layerId) const override;
        virtual int numberOfStrips(int layer, bool isStrip) const override;

        /** space point position for a given pair of phi and eta identifiers
            The LocalPosition is expressed in the reference frame of the phi projection.
            If one of the identifiers is outside the valid range, the function will return false */
        virtual  bool spacePointPosition(const Identifier& phiId, const Identifier& etaId, Amg::Vector2D& pos) const override;

        /** Global space point position for a given pair of phi and eta identifiers
            If one of the identifiers is outside the valid range, the function will return false */
        virtual  bool spacePointPosition(const Identifier& phiId, const Identifier& etaId, Amg::Vector3D& pos) const override;

        Amg::Vector3D localSpacePoint(const Identifier& stripId, 
                                      const Amg::Vector3D& etaHitPos,
                                      const Amg::Vector3D& phiHitPos) const;
        
        virtual bool containsId(const Identifier& id) const override;
        
        
        
        int chamberType() const;
        /// Returns the number of gas gaps associated with the readout element (2 or 3)
        int nGasGaps() const;
        /// Returns whether the gasGap is within the allowed range [1-nGasGaps()]
        bool validGap(int gasGap) const;


        /// Returns the number of strips in a given gas gap
        int nStrips(int gasGap) const;
        /// Returns whether the strip in the given gasgap is within the allowed range        
        bool validStrip(int gasGap, int strip) const;
        /// Returns the width of a given strip in the gasGap i
        double stripWidth(int gasGap, int strip) const;
        /// Returns the length of each strip which is equal to the height of the chamber
        double stripLength() const;
        /// Returns the pitch of the given strip in gasGap i
        double stripPitch(int gasGap, int strip) const;
        /// Returns the pitch of the given strip in gasGap i evaluated at a local positiion along the strip
        double stripPitch(int gasGap, int strip, double radialPos) const;
        /** @brief Returns the local X of the left edge of the strip at a given local radial position
         *  @param gasGap: Tgc gas gap of interest [1-3]
         *  @param strip: Tgc readout strip of interest [1-32]
         *  @param radialPos: Radial position along the strip (origin at trapezoid centre) */ 
        double stripLowEdgeLocX(int gasGap, int strip, double radialPos) const;
        /** @brief Returns the local X of the right edge of the strip at a given local radial position
         *  @param gasGap: Tgc gas gap of interest [1-3]
         *  @param strip: Tgc readout strip of interest [1-32]
         *  @param radialPos: Radial position along the strip (origin at trapezoid centre) */ 
        double stripHighEdgeLocX(int gasGap, int strip , double radialPos) const;
        /// Returns the local X of the strip center at a given local radial position
        double stripCenterLocX(int gasGap, int strip, double radialPos) const;

        double physicalDistanceFromBase() const;
        double stripPosOnLargeBase(int strip) const;
        double stripPosOnShortBase(int strip) const;

        
        double stripDeltaPhi() const;
        double stripDeltaPhi(int gasGap, int strip) const;

        
        /// Returns whether the wire gang in the given gasgap is within the allowed range
        bool validGang(int gasGap, int wireGang) const;
        /// Returns the total number of wires in a given gas gap
        int nWires(int gasGap) const;
        /// Returns the number of wires in a given gang in gas gap i
        int nWires(int gasGap, int gang) const;
        /// Returns the number of wire gangs (Random grouping of wires) in a given gas gap
        int nWireGangs(int gasGap) const;
        /// Returns the number of wire pitches that have to be travelled to reach gang i
        double nPitchesToGang(int gasGap, int gang) const;
        /// Returns the length of the wire gang along the radial direction [pitch x  N_{wire}^{gang}]
        double gangRadialLength(int gasGap, int gang) const;
        /// Returns the length of the most bottom wire in the gang
        double gangShortWidth(int gasGap, int gang) const;
        /// Returns the length of the central wire in the gang
        double gangCentralWidth(int gasGap, int gang) const;
        /// Returns the length of the most top wire in the gang
        double gangLongWidth(int gasGap, int gang) const;
        /// Returns the spatial thickness of the wire gang
        double gangThickness() const;
        /// Returns the length of a wire. The wire numbering is following scheme where 
        /// the 1st wire coincides with the bottom edge of the readout element
        double wireLength(int wire) const;
        /// Returns the pitch of the wires
        double wirePitch() const;



        double stripShortWidth(int, int) const;
        double stripLongWidth(int, int) const;

        /// Returns the local -> global transformation
        ///     x-axis: Parallel to the wires (strips) if the Identifier belongs to a wire (strip)
        ///     y-axis: Perpendicular axis in the transverse plane
        ///     z-axis: Along the beam-axis
        const Amg::Transform3D& localToGlobalTransf(const Identifier& id) const;
        /// Returns the global -> local transformation
        Amg::Transform3D globalToLocalTransf(const Identifier& id) const;

        /// Returns the position of the active channel (wireGang or strip)
        Amg::Vector3D channelPos(const Identifier& id) const;
        Amg::Vector3D channelPos(int gasGap, bool isStrip, int channel) const;

        /// Returns the global position of a wireGang
        Amg::Vector3D wireGangPos(const Identifier& id) const;
        Amg::Vector3D wireGangPos(int gasGap, int gang) const;

        /// Returns the global position of a strip
        Amg::Vector3D stripPos(int gasGap, int strip) const;
        Amg::Vector3D stripPos(const Identifier& id) const;
        /// Returns the direction of a strip
        Amg::Vector3D stripDir(int gasGap, int strip) const;
        Amg::Vector3D stripDir(const Identifier& id) const;

        
        /// Returns true if the chamber is belonging to the 48-fold TxE chambers
        bool isEndcap() const;
        /// Returns true if the chamber is mounted on the most inner ring, i.e. a TxF chamber
        bool isForward() const;
        /// Returns true if the chamber has 2 gasgaps
        bool isDoublet() const;
        /// Returns true if the chamber has 3 gasgaps
        bool isTriplet() const;
        /// Returns the minimum angle measured from the center that's covered by the chamber
        double chamberLocPhiMin() const;
        /// Returns the maximum angle measured from the center that's covered by the chamber
        double chamberLocPhiMax() const;
        

        double length() const;

        double frameZwidth() const;
        double frameXwidth() const;
        double chamberWidth(double z) const;


        int nPhiChambers() const;
        int nPhiSectors() const;
        
        /// Set the local Z coordinate of the i-th gasGap [1-3]
        void setPlaneZ(double value, int gasGap);
        
        /// Returns the gang number that's closest to the given external position
        int findGang(int gasGap, const Amg::Vector3D& extPos) const;
        /// Returns the strip number that's closest to the given external position
        int findStrip(int gasGap, const Amg::Vector3D& extPos) const;
        


        void setReadOutName(const std::string& rName);
        void setReadOutParams(GeoModel::TransientConstSharedPtr<TgcReadoutParams> pars);
        
        void setFrameThickness(const double frameH, const double frameAB);

       // Access to readout parameters
        const std::string& readOutName() const;

        const TgcReadoutParams* getReadoutParams() const;

        virtual void fillCache() override;

        
    private:
        /// Returns the local position of the active channel (wireGang or strip)
        Amg::Vector3D localChannelPos(const Identifier& id) const;
        Amg::Vector3D localChannelPos(int gasGap, bool isStrip, int channel) const;
        // Returns the local position of a strip
        Amg::Vector3D localStripPos(int gasGap, int strip) const;
        Amg::Vector3D localStripPos(const Identifier& id) const;
        /// Returns the local strip direction of a strip
        Amg::Vector3D localStripDir(int gasGap, int strip) const;
        Amg::Vector3D localStripDir(const Identifier& id) const;

        /// Returns the local position fo a wireGang
        Amg::Vector3D localWireGangPos(int gasGap, int gang) const;
        Amg::Vector3D localWireGangPos(const Identifier& id) const;
        /// Returns the local position of the gasGap in the AMDB coordinate system
        Amg::Vector3D localGasGapPos(int gg) const;

        static int surfaceHash(int GasGap, bool isStrip);

        /// Returns the local X given the reference point 
        ///   E.g. left edge, center, an external position along
        ///   the radial coordinate [-R/2; R/2], and the strip number
        double stripLocalX(const int stripNum,
                           const double locY,
                           const double refPoint) const;
        
        /// Returns the local X of the wire gang in gasGap i        
        double wireGangLocalX(const int gasGap,
                              const int gangNum) const;

        /// Returns the local X of the bottom wire in the wireGang i in gasGap j 
        double wireGangBottomX(int gasGap, int gangNum) const;
        /// Returns the local X of the top wire in the wireGang i in gasGap j
        double wireGangTopX(int gasGap, int gangNum) const;
        
        /// Returns whether a strip needs to be flipped and the final strip number parsed to
        /// the TgcReadoutParams to fetch its absolute position.
        std::pair<double, int> stripNumberToFetch(int gasGap, int inStrip) const;

        const TgcIdHelper& m_idHelper{idHelperSvc()->tgcIdHelper()};

        static constexpr int s_maxGasGap{3};
        std::array<double, s_maxGasGap> m_gasPlaneZ{make_array<double, s_maxGasGap>(-9999.)};

        std::string m_readout_name{};
        GeoModel::TransientConstSharedPtr <TgcReadoutParams> m_readoutParams{nullptr};
        /// Cache of the function call 1./ (getRsize() - 2. * physicalDistanceFromBase())
        /// which is the active height of the readout element 
        double m_stripSlope{0.};
        double m_locMinPhi{0.};
        double m_locMaxPhi{0.};

        int m_stIdxT4E{m_idHelper.stationNameIndex("T4E")};

        double m_frameH{0.};
        double m_frameAB{0.};

    };

}  // namespace MuonGM
#include <MuonReadoutGeometry/TgcReadoutElement.icc>
#endif  // MUONREADOUTGEOMETRY_TGCREADOUTELEMENT_H
