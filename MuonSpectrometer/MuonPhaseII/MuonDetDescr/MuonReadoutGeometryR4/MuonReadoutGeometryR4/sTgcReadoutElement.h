/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_STGCREADOUTELEMENT_H
#define MUONREADOUTGEOMETRYR4_STGCREADOUTELEMENT_H

#include <MuonReadoutGeometryR4/MuonReadoutElement.h>
#include <MuonReadoutGeometryR4/StripDesign.h>
#include <MuonReadoutGeometryR4/WireGroupDesign.h>
#include <MuonReadoutGeometryR4/PadDesign.h>
#include <MuonReadoutGeometryR4/StripLayer.h>
#include <bit>


namespace Acts{
    class TrapezoidBounds;
} 

namespace MuonGMR4 {

class sTgcReadoutElement : public MuonReadoutElement {

   public:
    
     /** @brief ReadoutChannelType to distinguish the available readout channels
      *  Pad - pad readout channel
      *  Strip - eta strip readout channel
      *   Wire - phi wire group readout channel
      *   WireIngrp - a particular wire in a given group
     */
     enum ReadoutChannelType  {
        Pad = sTgcIdHelper::sTgcChannelTypes::Pad,
        Strip = sTgcIdHelper::sTgcChannelTypes::Strip,
        Wire  = sTgcIdHelper::sTgcChannelTypes::Wire,
        WireInGrp,

    };

    /// Set of parameters to describe an sTGC chamber
    struct parameterBook {
        /// sTGC Chamber Details
        
        /// Height of the chamber
        double halfChamberHeight{0.}; //Length
        /// Length of the chamber on the short side
        double sHalfChamberLength{0.}; //sWidth
        /// Length of the chamber on the long side
        double lHalfChamberLength{0.}; //lWidth
        /// Thickness of the chamber
        double halfChamberTck{0.}; //Tck
        /// Width of the chamber frame on the short side
        double sFrameWidth{0.};
        /// Width of the chamber frame on the long side
        double lFrameWidth{0.};
        /// Thickness of the gas gap
        double gasTck{0.};
        /// Number of gas gap layers
        unsigned int numLayers{0};
        /// Number of channel types
        unsigned int nChTypes{0};
        /// Diamond cutout height
        double yCutout{0.};
        //// firstStripPitch needed for the globalChannelPosition function
        std::vector<double> firstStripPitch{};

        std::vector<StripLayer> stripLayers{};
        std::vector<StripLayer> wireGroupLayers{};
        std::vector<StripLayer> padLayers{};

        StripDesignPtr stripDesign{nullptr};
        WireDesignPtr wireGroupDesign{nullptr};
        PadDesignPtr padDesign{nullptr};

#ifndef SIMULATIONBASE
        ActsTrk::SurfaceBoundSetPtr<Acts::TrapezoidBounds> layerBounds;
#endif

    };

    struct defineArgs : public MuonReadoutElement::defineArgs,
                        public parameterBook {};

    sTgcReadoutElement(defineArgs&& args);
    virtual ~sTgcReadoutElement();

    const parameterBook& getParameters() const;
    /// Overload from the ActsTrk::IDetectorElement
    ActsTrk::DetectorType detectorType() const override final {
        return ActsTrk::DetectorType::sTgc;
    }

    StatusCode initElement() override final;

    /// Height of the chamber
    double chamberHeight() const; //Length
    /// Length of the chamber on the short side
    double sChamberLength() const; //sWidth
    /// Length of the chamber on the long side
    double lChamberLength() const; //lWidth
    /// Distance between 2 gas gaps
    double gasGapPitch() const;

    /// Thickness of the chamber
    double thickness() const override final; //chamberTck
    /// Width of the chamber frame on the short side
    double sFrameWidth() const;
    /// Width of the chamber frame on the long side
    double lFrameWidth() const;
    /// Returns the multilayer of the sTgcReadoutElement
    int multilayer() const;
    /// Returns the number of gas gap layers
    unsigned int numLayers() const;
    /// Returns the thickness of the gas gap
    double gasGapThickness() const;

    /// Gas Gaps
    double firstStripPitch(const Identifier& measId) const;
    double firstStripPitch(const IdentifierHash& measHash) const;
        
    
    /** @brief Returns the number of strips / wires / pads in a given gasGap
     *  @param measId: Identifier corresponding to the gasGap of interest */
    unsigned int numChannels(const Identifier& measId) const;
    /** @brief Returns the number of strips / wires / pads in a given gasGap
     *  @param measHash: Measurement hash corresponding to the gasGap of interest */
    unsigned int numChannels(const IdentifierHash& measHash) const;

    ///Length of each strip
    double stripLength(const Identifier& measId) const;
    double stripLength(const IdentifierHash& measHash) const;
    /// Number of Channel Types
    unsigned int nChTypes() const;

    /// Returns the number of pads in the eta direction in the given layer
    unsigned int numPadEta(const Identifier& measId) const;
    unsigned int numPadEta(const IdentifierHash& measHash) const;
    /// Returns the number of pads in the Phi direction in the given gasGap layer
    unsigned int numPadPhi(const Identifier& measId) const;
    unsigned int numPadPhi(const IdentifierHash& measHash) const;
    /// Returns the height of all the pads that are not adjacent to the bottom edge of the trapezoid active area
    double padHeight(const Identifier& measId) const;
    double padHeight(const IdentifierHash& measHash) const;   
    /// Returns the staggering shift of inner pad edges in the phi direction
    double padPhiShift(const Identifier& measId) const;
    double padPhiShift(const IdentifierHash& measHash) const; 
    /// Returns the angular pitch of the pads in the phi direction
    double anglePadPhi(const Identifier& measId) const;
    double anglePadPhi(const IdentifierHash& measHash) const; 
    /// Returns the maximum number of pads that can be contained in a column of a pad. Used to match the pad numbering scheme
    unsigned int maxPadEta(const Identifier& measId) const;
    unsigned int maxPadEta(const IdentifierHash& measHash) const; 
    /// Returns the pad number in the sequential numbering (1,2,3,...17,18,19,20,...)scheme from a pad identifier with conventional pad number (1, 2, 3, 19, 20,...) 
    unsigned int padNumberSeq(const Identifier& measId) const;
    unsigned int padNumberSeq(const IdentifierHash& measHash) const;
    /// Returns a pair of Eta and Phi index for the given pad identifier
    std::pair<uint, uint> padEtaPhi(const Identifier& measId) const;
    std::pair<uint, uint> padEtaPhi(const IdentifierHash& measHash) const;   
    /// Returns the Eta index of the pad for the given pad identifier
    unsigned int padEta(const Identifier& measId) const;
    unsigned int padEta(const IdentifierHash& measHash) const;
    /// Returns the Phi index of the pad for the given pad identifier
    unsigned int padPhi(const Identifier& measId) const;
    unsigned int padPhi(const IdentifierHash& measHash) const;
    /// Returns the distance between the gasGap center and the beamline
    double beamlineRadius(const Identifier& measId) const;
    double beamlineRadius(const IdentifierHash& measHash) const;
    /// Returns an array of four 2D vectors representing corner positions of the pads
    using localCornerArray = std::array<Amg::Vector2D, 4>;
    localCornerArray localPadCorners(const Identifier& measId) const;
    localCornerArray localPadCorners(const IdentifierHash& measHash) const;
    /// Returns an array of four 3D vectors representing corner positions of the pads
    using globalCornerArray = std::array<Amg::Vector3D, 4>;
    globalCornerArray globalPadCorners(const ActsGeometryContext& ctx, const Identifier& measId) const;
    globalCornerArray globalPadCorners(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const;  
    /// Returns the pad Number given local position of hit and Identifier/Hash
    int padNumber(const Amg::Vector2D& hitPos, const Identifier& measId) const;
    int padNumber(const Amg::Vector2D& hitPos, const IdentifierHash& measHash) const;
       
    /// Retrieves the readoutElement Layer given the Identifier/Hash
    const StripDesign& stripDesign(const Identifier& measId) const;
    const StripDesign& stripDesign(const IdentifierHash& measHash) const;

    /// Retrieves the readoutElement Layer given the Identifier/Hash
    const WireGroupDesign& wireDesign(const Identifier& measId) const;
    const WireGroupDesign& wireDesign(const IdentifierHash& measHash) const;
    /// Retrieves the readoutElement Layer given the Identifier/Hash
    const PadDesign& padDesign(const Identifier& measId) const;
    const PadDesign& padDesign(const IdentifierHash& measHash) const;

    /// Returns the global pad/strip/wireGroup position
    Amg::Vector3D globalChannelPosition(const ActsGeometryContext& ctx, const Identifier& measId) const;
    Amg::Vector3D globalChannelPosition(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const;
    /// Returns the local pad/strip/wireGroup position
    Amg::Vector2D localChannelPosition(const Identifier& measId) const;
    Amg::Vector2D localChannelPosition(const IdentifierHash& measHash) const;

    // Returns the global left/right edge position of strip or wire
    Amg::Vector3D leftStripEdge(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const;
    Amg::Vector3D rightStripEdge(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const;

    /// Constructs the identifier hash from the full measurement Identifier. The
    /// hash is always defined w.r.t the specific detector element and used to
    /// access the information in memory quickly
    IdentifierHash measurementHash(const Identifier& measId) const override final;
    /// Transforms the Identifier into a layer hash
    IdentifierHash layerHash(const Identifier& measId) const override final;
    static IdentifierHash layerHash(const IdentifierHash& measHash);
    /// Converts the measurement hash back to the full Identifier
    Identifier measurementId(const IdentifierHash& measHash) const override final;

    const StripLayer& stripLayer(const Identifier& measId) const;
    const StripLayer& stripLayer(const IdentifierHash& measId) const;
    
    /** @brief Create a measurement hash from the Identifier fields
     *  @param: gasGap in which the measurment sits
     *  @param: channelType (strip / pad/ wire / wireInGrp)
     *  @param: channel - electronics channel connected with the readout element
     *  @param: wireInGrp - number of a specific wire in the group  (digi only)
    */
    
    static IdentifierHash createHash(const unsigned int gasGap, 
                                     const unsigned int channelType, 
                                     const unsigned int channel,
                                     const unsigned int wireInGrp = 0);
    friend class ActsTrk::TransformCacheDetEle<sTgcReadoutElement>;

    bool isEtaZero(const IdentifierHash& measurementHash, const Amg::Vector2D& localPosition) const;

   private:
        /// Returns channel position for a given identifierHash
        static unsigned int channelNumber(const IdentifierHash& measHash);
        /// Returns the channel type for a given identifierHash
        static unsigned int chType(const IdentifierHash& measHash);
        /// Returns the gasGap (0 to 3) for a given identifierHash
        static unsigned int gasGapNumber(const IdentifierHash& measHash);
        Amg::Transform3D fromGapToChamOrigin(const IdentifierHash& layerHash) const;
        Amg::Vector3D chamberStripPos(const IdentifierHash& measHash) const;

        parameterBook m_pars{};
        const sTgcIdHelper& m_idHelper{idHelperSvc()->stgcIdHelper()};
        const int m_multiLayer{m_idHelper.multilayer(identify())};
        double m_gasGapPitch{-1.};

        /// Auxillary variables to translate the Identifier to a measurement hash and back
        const unsigned int m_hashShiftChType = 2*std::popcount(static_cast<unsigned int>(numLayers()));
};

std::ostream& operator<<(std::ostream& ostr, const MuonGMR4::sTgcReadoutElement::parameterBook& pars);
}  // namespace MuonGMR4

namespace ActsTrk{
    template <> Amg::Transform3D 
        TransformCacheDetEle<MuonGMR4::sTgcReadoutElement>::fetchTransform(const DetectorAlignStore* store) const;
    template <> Identifier
        TransformCacheDetEle<MuonGMR4::sTgcReadoutElement>::identify() const;
}

#include <MuonReadoutGeometryR4/sTgcReadoutElement.icc>
#endif
