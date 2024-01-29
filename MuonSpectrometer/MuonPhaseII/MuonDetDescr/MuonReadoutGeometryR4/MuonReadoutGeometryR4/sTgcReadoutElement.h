/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_STGCREADOUTELEMENT_H
#define MUONREADOUTGEOMETRYR4_STGCREADOUTELEMENT_H

#include <MuonReadoutGeometryR4/MuonReadoutElement.h>
#include <MuonReadoutGeometryR4/StripDesign.h>
//#include <MuonReadoutGeometryR4/DiamondStripDesign.h>
#include <MuonReadoutGeometryR4/StripLayer.h>
#ifndef SIMULATIONBASE
#   include "Acts/Surfaces/TrapezoidBounds.hpp"
#endif


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

        /// Pitch of the first strip of the gas gap
        std::vector<double> firstStripPitch{};
        std::vector<StripLayer> stripLayers{};

        StripDesignPtr stripDesign{nullptr};
        StripDesignPtr wireGroupDesign{nullptr};

#ifndef SIMULATIONBASE
        ActsTrk::SurfaceBoundSetPtr<Acts::TrapezoidBounds> layerBounds{};
#endif

    };

    struct defineArgs : public MuonReadoutElement::defineArgs,
                        public parameterBook {};

    sTgcReadoutElement(defineArgs&& args);

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

    /// Length of gas Gap on short side
    double sGapLength() const;
    /// Length of gas Gap on long side
    double lGapLength() const;
    /// Height of gas Gap
    double gapHeight() const;
    /// Thickness of the chamber
    double thickness() const override final; //chamberTck
    /// Width of the chamber frame on the short side
    double sFrameWidth() const;
    /// Width of the chamber frame on the long side
    double lFrameWidth() const;
    /// Returns the multilayer of the sTgcReadoutElement
    int multilayer() const;
    /// Returns the number of gas gap layers
    int numLayers() const;
    /// Returns the thickness of the gas gap
    double gasGapThickness() const;
    /// Returns the yCutout value of the chamber
    double yCutout() const;
    ////Strips
    /// Number of strips in a chamber
    unsigned int numStrips() const;
    /// Pitch of a strip
    double stripPitch() const;
    /// Width of a strip
    double stripWidth() const;
    ///Length of each strip
    double stripLength(const int& stripNumb) const;
    /// Number of Channel Types
    unsigned int nChTypes() const;
/*
    //// Wires
    /// Number of wires in the gas gap
    std::vector<unsigned int> numWires() const;
    /// Number of wires in the first wire group
    std::vector<short> firstWireGroupWidth() const;
    /// Number of wire groups in the gas gap
    std::vector<short> numWireGroups() const;
    /// Wire Cutout
    std::vector<double> wireCutout() const;
    /// Pitch of the wire
    double wirePitch{0.};
    /// Width of a single wire
    double wireWidth{0.};
    /// Number of wires in a normal wire group
    short wireGroupWidth{0};
*/


    Amg::Vector3D stripPosition(const ActsGeometryContext& ctx, const Identifier& measId) const;
    Amg::Vector3D stripPosition(const ActsGeometryContext& ctx, const IdentifierHash& measHash) const;

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


   private:
        static unsigned int stripNumber(const IdentifierHash& measHash);
        static unsigned int chType(const IdentifierHash& measHash);
        static unsigned int gasGapNumber(const IdentifierHash& measHash);
        Amg::Transform3D fromGapToChamOrigin(const IdentifierHash& layerHash) const;
        Amg::Vector3D chamberStripPos(const IdentifierHash& measHash) const;

        parameterBook m_pars{};
        const sTgcIdHelper& m_idHelper{idHelperSvc()->stgcIdHelper()};
        const int m_multiLayer{m_idHelper.multilayer(identify())};
        double m_gasGapPitch{-1.};

        /// Auxillary variables to translate the Identifier to a measurement hash and back
        const unsigned int m_hashShiftChType{2*CxxUtils::count_ones(static_cast<unsigned int>(numLayers()))};
        const unsigned int m_hashShiftChannel{2*m_hashShiftChType};
};

std::ostream& operator<<(
    std::ostream& ostr, const MuonGMR4::sTgcReadoutElement::parameterBook& pars);
}  // namespace MuonGMR4

#include <MuonReadoutGeometryR4/sTgcReadoutElement.icc>
#endif
