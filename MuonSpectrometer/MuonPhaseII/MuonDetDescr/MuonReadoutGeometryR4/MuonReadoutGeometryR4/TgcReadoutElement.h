/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_TGCREADOUTELEMENT_H
#define MUONREADOUTGEOMETRYR4_TGCREADOUTELEMENT_H

#include <MuonReadoutGeometryR4/MuonReadoutElement.h>
#include <MuonReadoutGeometryR4/WireGroupDesign.h>
#include <MuonReadoutGeometryR4/RadialStripDesign.h>
#include <MuonReadoutGeometryR4/StripLayer.h>

#ifndef SIMULATIONBASE
#   include "Acts/Utilities/BoundFactory.hpp"
#endif


namespace MuonGMR4 {

class TgcReadoutElement : public MuonReadoutElement {

   public:
    
    /// Set of parameters to describe a Tgc chamber
    struct parameterBook {
        /// Describe the chamber dimensions
        /// Half thickness of the chamber
        double halfThickness{0.};
        /// Half height of the chamber (Top - botom edge)
        double halfHeight{0.};
        /// Half length of the chamber short edge (Bottom)
        double halfWidthShort{0.};
        /// Half length of the chamber long edge (Top)
        double halfWidthLong{0.};
        // The number of gasgaps in a channel
        unsigned nGasGaps{0};
        /// We have maximum 3 gasgaps times eta / phi measurement
        std::array<StripLayerPtr, 6> sensorLayouts{};
#ifndef SIMULATIONBASE
        /// Set of surface boundaries
        std::shared_ptr<Acts::SurfaceBoundFactory> layerBounds;
#endif
    };

    struct defineArgs : public MuonReadoutElement::defineArgs,
                        public parameterBook {};

    TgcReadoutElement(defineArgs&& args);
    virtual ~TgcReadoutElement();

    const parameterBook& getParameters() const;
    /// Overload from the ActsTrk::IDetectorElement
    ActsTrk::DetectorType detectorType() const override final {
        return ActsTrk::DetectorType::Tgc;
    }  
    double thickness() const override final; 
    StatusCode initElement() override final;



    /// Constructs the identifier hash from the full measurement Identifier. The
    /// hash is always defined w.r.t the specific detector element and used to
    /// access the information in memory quickly
    IdentifierHash measurementHash(const Identifier& measId) const override final;    
    IdentifierHash layerHash(const Identifier& measId) const override final;
    IdentifierHash layerHash(const IdentifierHash& measHash) const;
    Identifier measurementId(const IdentifierHash& measHash) const override final;

    /// Returns the number of gasgaps described by this ReadOutElement (usally 2 or 3)
    unsigned nGasGaps() const;
    /// Returns the length of the bottom edge of the chamber (short width)
    double moduleWidthS() const;
    /// Returns the length of the top edge of the chamber (top width)
    double moduleWidthL() const;
    /// Returns the height of the chamber (Distance bottom - topWidth)
    double moduleHeight() const;
    /// Returns the thickness of the chamber
    double moduleThickness() const;
    
    /// Returns the number of readout channels
    unsigned numChannels(const IdentifierHash& measHash) const;
    /// Returns the number of strips for a given gasGap [1-3]
    unsigned numStrips(const IdentifierHash& layHash) const;
    /// Returns the number of wire gangs for a given gasGap [1-3]
    unsigned numWireGangs(const IdentifierHash& layHash) const;
    /// Returns the thickness of the gasGap
    double gasGapPitch() const;

    /// Returns the center of the measurement channel
    ///  eta measurement:  wire gang center
    ///  phi measurement:  strip center
    Amg::Vector3D channelPosition(const ActsGeometryContext& ctx, const Identifier& measId) const;
    Amg::Vector3D channelPosition(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const;

    /// Returns the pointer to the strip layer associated with the gas gap.
    const StripLayerPtr& sensorLayout(const IdentifierHash& hash) const;
    /// Returns access to the wire group design of the given gasGap [1-3]
    /// If the gap does not have a wires an exception is thrown
    const WireGroupDesign& wireGangLayout(const IdentifierHash& layHash) const;

    /// Returns access to the strip design of the given gasGap [1-3]
    /// If the gap does not have strips an exception is thrown
    const RadialStripDesign& stripLayout(const IdentifierHash& layHash) const;
    friend class ActsTrk::TransformCacheDetEle<TgcReadoutElement>;
   private:
        parameterBook m_pars{};
        const TgcIdHelper& m_idHelper{idHelperSvc()->tgcIdHelper()};
        /// Distance between 2 gas gaps (Z - direction)
        double m_gasThickness{0.};

        
        Amg::Transform3D fromGapToChamOrigin(const IdentifierHash& layerHash) const;
        /// Returns the local strip position w.r.t. to the chamber origin
        Amg::Vector3D chamberStripPos(const IdentifierHash& measHash) const;
    public:
        /// Constructs the Hash out of the Identifier fields 
        /// (channel, gasGap, isStrip)
        static IdentifierHash constructHash(unsigned measCh,
                                            unsigned gasGap,
                                            const bool isStrip);
        /** @brief Flips the isStrip bit from a parsed measurment hash
         *  @param meashHash: Measurement hash encoding channel, gasGap, isStrip
         *  @param isStrip: Flag what's the new isStrip from the measruement hash */        
        static IdentifierHash flipIsStrip(const IdentifierHash& meashHash,
                                          const bool isStrip);
        /** @brief Unpacks the channel number from the measurement hash */
        static unsigned channelNumber(const IdentifierHash& measHash);
        /** @brief Unpacks the gas gap number from the measurement hash */
        static unsigned gasGapNumber(const IdentifierHash& measHash);
        /** @brief Unpacks whether the measurement hash is a strip */
        static bool isStrip(const IdentifierHash& measHash);
 };
std::ostream& operator<<(std::ostream& ostr, const TgcReadoutElement::parameterBook& pars);
}  // namespace MuonGMR4

namespace ActsTrk{
    template <> Amg::Transform3D 
        TransformCacheDetEle<MuonGMR4::TgcReadoutElement>::fetchTransform(const DetectorAlignStore* store) const;
    template <> Identifier
        TransformCacheDetEle<MuonGMR4::TgcReadoutElement>::identify() const;
}

#include <MuonReadoutGeometryR4/TgcReadoutElement.icc>
#endif
