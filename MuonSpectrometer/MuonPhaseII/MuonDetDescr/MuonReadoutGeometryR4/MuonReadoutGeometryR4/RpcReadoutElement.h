/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_RPCREADOUTELEMENT_H
#define MUONREADOUTGEOMETRYR4_RPCREADOUTELEMENT_H

#include <MuonReadoutGeometryR4/MuonReadoutElement.h>
#include <MuonReadoutGeometryR4/StripDesign.h>
#include <MuonReadoutGeometryR4/StripLayer.h>

#ifndef SIMULATIONBASE
#   include "Acts/Utilities/BoundFactory.hpp"
#endif

namespace MuonGMR4 {

class RpcReadoutElement : public MuonReadoutElement {

   public:
    /// Set of parameters to describe a RPC chamber
    struct parameterBook {
        /// RPC panel dimensions
        
        /// Elongation along the Z axis
        double halfLength{0.};
        /// Half thickness of the Rpc module
        double halfThickness{0.};
        /// Elongation within the sector        
        double halfWidth{0.};        
        /// The number of gas gaps (along the radial direction)
        /// in the RPC chamber (2 or 3) 
        unsigned nGasGaps{0};
        /// Each gas gap is usually subdivided into 2 phi panels
        /// which is actually the sector granularity of the Rpc trigger
        int nPanelsInPhi{0};
        std::vector<StripLayerPtr> layers{};

        StripDesignPtr phiDesign{nullptr};
        StripDesignPtr etaDesign{nullptr};
        /** @brief at which side is the readout plane of the strip */
        double readoutSide{-1};
#ifndef SIMULATIONBASE
        std::shared_ptr<Acts::SurfaceBoundFactory> layerBounds{};
#endif
    };

    struct defineArgs : public MuonReadoutElement::defineArgs,
                        public parameterBook {};

    RpcReadoutElement(defineArgs&& args);
    virtual ~RpcReadoutElement();
    const parameterBook& getParameters() const;
    /// Overload from the ActsTrk::IDetectorElement
    ActsTrk::DetectorType detectorType() const override final {
        return ActsTrk::DetectorType::Rpc;
    }
    /// Overload from the Acts::DetectorElement (2 * halfheight)
    double thickness() const override final;

    StatusCode initElement() override final;

    /// Returns the doublet Z field of the MuonReadoutElement identifier
    int doubletZ() const;
    /// Returns the doublet R field of the MuonReadoutElement identifier
    int doubletR() const;
    /// Returns the doublet Phi field of the MuonReadoutElement identifier
    int doubletPhi() const;

    /// Returns the number of gasgaps described by this ReadOutElement (usally 2 or 3)
    unsigned nGasGaps() const;
    /// Returns the number of phi panels (1 or 2)
    int nPhiPanels() const;
    /// Returns the maximum phi panel
    int doubletPhiMax() const;

    /// Number of strips measuring the eta coordinate
    unsigned nEtaStrips() const;
    /// Number of strips measuring the phi coordinate
    unsigned nPhiStrips() const;
    /// Returns the number of strips
    unsigned nStrips(const IdentifierHash& hash) const;

    /// Strip pitch in eta
    double stripEtaPitch() const;
    /// Strip pitch in phi
    double stripPhiPitch() const;
    /// Strip width in eta
    double stripEtaWidth() const;
    /// Strip width in phi
    double stripPhiWidth() const;
    /// Returns the length of an eta strip
    double stripEtaLength() const;
    /// Returns the length of a phi strip
    double stripPhiLength() const;
    /// Returns the thickness of a RPC gasgap
    double gasGapPitch() const;


    /// Returns the position of the strip center
    Amg::Vector3D stripPosition(const ActsGeometryContext& ctx, const Identifier& measId) const;
    Amg::Vector3D stripPosition(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const;
    /// Returns the global position of the strip edge at negative local Y
    Amg::Vector3D rightStripEdge(const ActsGeometryContext& ctx, const Identifier& measId) const;
    Amg::Vector3D rightStripEdge(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const;
    /// Returns the global posiition of the strip edge at positive local Y
    Amg::Vector3D leftStripEdge(const ActsGeometryContext& ctx, const Identifier& measId) const;
    Amg::Vector3D leftStripEdge(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const;

    enum class EdgeSide{
        readOut,
        highVoltage,
    };
    /** @brief Returns the disance to the readout
     *  @param measHash: Hash of the measurement to check
     *  @param posInStripPlane: Local position of the crossing point on the plane (In gasGap frame)
     *  @param side: Switch indicating whether the readout or the highVoltage side is targeted. */
    double distanceToEdge(const IdentifierHash& measHash, 
                          const Amg::Vector3D& posInStripPlane,
                          const EdgeSide side) const;

    /// Constructs the identifier hash from the full measurement Identifier. The
    /// hash is always defined w.r.t the specific detector element and used to
    /// access the information in memory quickly
    IdentifierHash measurementHash(const Identifier& measId) const override final;
    
    IdentifierHash layerHash(const Identifier& measId) const override final;
    IdentifierHash layerHash(const IdentifierHash& measHash) const;

    Identifier measurementId(const IdentifierHash& measHash) const override final;

    /// Constructs an Identifier hash from the Identifier fields controlled by this
    /// readout element 
    static IdentifierHash createHash(const unsigned strip, 
                                     const unsigned gasGap, 
                                     const unsigned doubPhi, 
                                     const bool measPhi);
    
    friend class ActsTrk::TransformCacheDetEle<RpcReadoutElement>;


        /// Access to the StripLayer associated to a given measurement Hash
        const StripLayerPtr& sensorLayout(const IdentifierHash& measHash) const;
        /// @brief Access the associated strip design for a given measurement hash
        const StripDesign& stripDesign(const IdentifierHash& meaHash) const;
    private:
        static unsigned stripNumber(const IdentifierHash& measHash);
        static unsigned gasGapNumber(const IdentifierHash& measHash);
        static unsigned doubletPhiNumber(const IdentifierHash& measHash);
        static bool measuresPhi(const IdentifierHash& measHash);


        Amg::Transform3D fromGapToChamOrigin(const IdentifierHash& layerHash) const;
        /// Returns the local strip position w.r.t. to the chamber origin
        Amg::Vector3D chamberStripPos(const IdentifierHash& measHash) const;



        parameterBook m_pars{};
        const RpcIdHelper& m_idHelper{idHelperSvc()->rpcIdHelper()};
        /// doublet R -> 1: chamber is mounted below the Mdts 
        //            -> 2: chamber is mounted on top of the Mdts
        const int m_doubletR{m_idHelper.doubletR(identify())};
        /// Associated doublet Z (Ranges from  1-3)
        /// If doubletZ is 3, there's generally the possibility that the module is
        /// additionally split according to doublet Phi
        const int m_doubletZ{m_idHelper.doubletZ(identify())};
        const int m_doubletPhi{m_idHelper.doubletPhi(identify())};

        /// Distance between 2 gas gaps (Radial direction)
        double m_gasThickness{0.};
        
 };
std::ostream& operator<<(std::ostream& ostr, const RpcReadoutElement::parameterBook& pars);
}  // namespace MuonGMR4

namespace ActsTrk{
    template <> Amg::Transform3D 
        TransformCacheDetEle<MuonGMR4::RpcReadoutElement>::fetchTransform(const DetectorAlignStore* store) const;
    /** Identifier of the transform cache equivalent to the Identifier of the first strip inside the gasGap */
    template <> Identifier
        TransformCacheDetEle<MuonGMR4::RpcReadoutElement>::identify() const;
}


#include <MuonReadoutGeometryR4/RpcReadoutElement.icc>
#endif
