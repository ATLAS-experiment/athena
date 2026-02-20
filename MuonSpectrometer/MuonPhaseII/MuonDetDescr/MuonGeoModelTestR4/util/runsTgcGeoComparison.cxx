
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @brief Helper macro to compare the output from the readout geometry dumps:
 *        python -m MuonGeoModelTest.runGeoModelTest
 *        python -m MuonGeoModelTestR4.runGeoModelTest 
 *  
*/
#include <GeoPrimitives/GeoPrimitives.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <GeoPrimitives/GeoPrimitivesToStringConverter.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <MuonIdHelpers/sTgcIdHelper.h>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <iostream>
#include <cmath>

#include <PathResolver/PathResolver.h>
#include <TFile.h>
#include <TTreeReader.h>

using namespace MuonGMR4;
using namespace ActsTrk;

constexpr double tolerance = 100.*Gaudi::Units::micrometer;

/// Helper struct to represent a full sTgc chamber
struct sTgcChamber{
    /// Default constructor
    sTgcChamber() = default;

    //// Identifier
    int stationIndex{0};
    int stationEta{0};
    int stationPhi{0};
    int stationMultilayer{0};
    std::string design{};

    /// Transformation of the underlying Alignable node
    Amg::Transform3D alignableTransform{Amg::Transform3D::Identity()};

    /// Sorting operator to insert the object into std::set
    bool operator<(const sTgcChamber& other) const {
        if (stationIndex != other.stationIndex) return stationIndex < other.stationIndex;
        if (stationEta != other.stationEta) return stationEta < other.stationEta;
        if (stationPhi != other.stationPhi) return stationPhi < other.stationPhi;
        return stationMultilayer < other.stationMultilayer;
    }

    /// Transformation of the underlying GeoModel element
    Amg::Transform3D geoModelTransform{Amg::Transform3D::Identity()};

    ////Chamber Details
    unsigned int numLayers{0};
    double yCutout{0.f};
    double gasTck{0.f};

    ////Chamber lengths for debug
    double sChamberLength{0.f};
    double lChamberLength{0.f};
    double chamberHeight{0.f};
    double sGapLength{0.f};
    double lGapLength{0.f};
    double gapHeight{0.f};


    //// Wires
    std::vector<unsigned int> numWires;
    std::vector<uint> firstWireGroupWidth;
    std::vector<uint> numWireGroups;
    std::vector<float> wireCutout;
    double wirePitch{0.f};
    double wireWidth{0.f};
    uint wireGroupWidth{0};

    //// Strips
    unsigned int numStrips{0};
    double stripPitch{0.f};
    double stripWidth{0.f};
    std::vector<float> firstStripPitch;

    //// Wires and Strips
    struct sTgcChannel{
        /// @brief local strip postion
        Amg::Vector2D localPosition{Amg::Vector2D::Zero()};
        /// @brief global strip postion
        Amg::Vector3D globalPosition{Amg::Vector3D::Zero()};
        /// @brief  wireGroup/strip number
        unsigned int channelNumber{0};
        /// @brief  Gas gap of the wireGroup/strip
        unsigned int gasGap{0};
        /// @brief Channel type to indicate wireGroup/strip
        unsigned int channelType{0};
        /// @brief Length of channel wireGroup/strip
        double channelLen{0};
        /// @brief Ordering operator to use the wireGroup with set
        bool operator<(const sTgcChannel& other) const {
            if (gasGap != other.gasGap) return gasGap < other.gasGap;
            if (channelType != other.channelType) return channelType < other.channelType;
            return channelNumber < other.channelNumber;
        }       
    };

    //// Pads
    double sPadLength{0.f};
    double lPadLength{0.f};
    double anglePadPhi{0.f};
    double beamlineRadius{0.f};
    
    std::vector<int> padNumber;
    std::vector<uint> numPads;
    std::vector<uint> numPadEta;
    std::vector<uint> numPadPhi;
    std::vector<float> firstPadHeight;
    std::vector<float> padHeight;
    std::vector<float> padPhiShift;
    std::vector<float> firstPadPhiDiv;

    //// Pads
    struct sTgcPad{
        /// @brief local pad postion
        Amg::Vector2D localPosition{Amg::Vector2D::Zero()};
        /// @brief global pad postion
        Amg::Vector3D globalPosition{Amg::Vector3D::Zero()};
        /// @brief local pad corner positions
        Amg::Vector2D localPadCornerBL{Amg::Vector2D::Zero()};
        Amg::Vector2D localPadCornerBR{Amg::Vector2D::Zero()};
        Amg::Vector2D localPadCornerTL{Amg::Vector2D::Zero()};
        Amg::Vector2D localPadCornerTR{Amg::Vector2D::Zero()};
        /// @brief global pad corner positions
        Amg::Vector3D globalPadCornerBL{Amg::Vector3D::Zero()};
        Amg::Vector3D globalPadCornerBR{Amg::Vector3D::Zero()};
        Amg::Vector3D globalPadCornerTL{Amg::Vector3D::Zero()};
        Amg::Vector3D globalPadCornerTR{Amg::Vector3D::Zero()};
        /// @brief hitPosition that is fed in to evaluate padNumber
        Amg::Vector2D hitPosition{Amg::Vector2D::Zero()};
        /// @brief padNumber given the hit position
        int padNumber{0};
        /// @brief  Pad  Eta number
        short padEta{0};
        /// @brief  Pad  Phi number
        short padPhi{0};
        /// @brief  Gas gap of the Pad
        unsigned int gasGap{0};

        /// @brief Ordering operator to use the Pad with set
        bool operator<(const sTgcPad& other) const {
            if (gasGap != other.gasGap) return gasGap < other.gasGap;
            if (padPhi != other.padPhi) return padPhi < other.padPhi;     
            return padEta < other.padEta;
        }       
    };

    /// Helper struct to assess that the layers are properly oriented
    struct sTgcLayer {
        /// @brief Gas gap number of the layer
        unsigned int gasGap{0};
        /// @brief channel type of the layer
        using chType_t = sTgcIdHelper::sTgcChannelTypes;
        chType_t chType{chType_t::Wire};
        /// @ transformation 
        Amg::Transform3D transform{Amg::Transform3D::Identity()};
        /// @brief Ordering operator
        bool operator<(const sTgcLayer& other) const {
            if (chType != other.chType) return chType < other.chType;
            return gasGap < other.gasGap;
        }       
    }; 
    std::set<sTgcChannel> channels{}; 
    std::set<sTgcPad> pads{};
    std::set<sTgcLayer> layers{};
  
};


/// Translation of the station Index -> station Name. Dictionary taken from
/// https://gitlab.cern.ch/atlas/athena/-/blob/main/DetectorDescription/IdDictParser/data/IdDictMuonSpectrometer_R.09.03.xml
std::ostream& operator<<(std::ostream& ostr, const sTgcChamber& chamb) {
    static const std::map<int, std::string> stationDict{
        {57, "STS"}, {58, "STL"}
    };
    ostr<<"sTgc chamber "<<stationDict.at(chamb.stationIndex)<<", eta: "<<chamb.stationEta
        <<", ml: "<<chamb.stationMultilayer;
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr,const sTgcChamber::sTgcChannel & channel) {
    ostr<<"channel (gasGap/number): ";
    ostr<<channel.gasGap<<"/";
    ostr<<channel.channelNumber<<", ";
    ostr<<channel.channelType<<", ";
    ostr<<" global Position: "<<Amg::toString(channel.globalPosition, 2)<<", ";
    ostr<<" local Position: "<<Amg::toString(channel.localPosition, 2);
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr,const sTgcChamber::sTgcPad & pad) {
    ostr<<"pad (gasGap/padEta/padPhi): ";
    ostr<<pad.gasGap<<"/";
    ostr<<pad.padEta<<"/"<<pad.padPhi<<", ";
    ostr<<"global position: "<<Amg::toString(pad.globalPosition, 2);
    ostr<<"Bottom-left globalPadCorner: "<<Amg::toString(pad.globalPadCornerBL, 2);
    ostr<<"Bottom-right globalpadCorner: "<<Amg::toString(pad.globalPadCornerBR, 2);
    ostr<<"Top-left globalpadCorner: "<<Amg::toString(pad.globalPadCornerTL, 2);
    ostr<<"Top-right globalpadCorner: "<<Amg::toString(pad.globalPadCornerTR, 2);

    ostr<<"local position: "<<Amg::toString(pad.localPosition, 2);
    ostr<<"Bottom-left localPadCorner: "<<Amg::toString(pad.localPadCornerBL, 2);
    ostr<<"Bottom-right localpadCorner: "<<Amg::toString(pad.localPadCornerBR, 2);
    ostr<<"Top-left localpadCorner: "<<Amg::toString(pad.localPadCornerTL, 2);
    ostr<<"Top-right localpadCorner: "<<Amg::toString(pad.localPadCornerTR, 2);
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr,const sTgcChamber::sTgcLayer & layer) {
    ostr<<"(gasGap/channelType): ";
    ostr<<layer.gasGap<<", ";
    switch (layer.chType) {
        case sTgcIdHelper::sTgcChannelTypes::Pad:
            ostr<<"pad,   ";
            break;
        case sTgcIdHelper::sTgcChannelTypes::Wire:
            ostr<<"wire,  ";
            break;
        case sTgcIdHelper::sTgcChannelTypes::Strip:
            ostr<<"strip, ";
            break;

    };
    //ostr<<"transform: "<<Amg::toString(layer.transform);
    return ostr;
}

std::set<sTgcChamber> readTreeDump(const std::string& inputFile) {
    std::set<sTgcChamber> to_ret{};
    std::cout<<"Read the sTgc geometry tree dump from "<<inputFile<<std::endl;
    std::unique_ptr<TFile> inFile{TFile::Open(inputFile.c_str())};
    if (!inFile || !inFile->IsOpen()) {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Failed to open "<<inputFile<<std::endl;
        return to_ret;
    }
    TTreeReader treeReader("sTgcGeoModelTree", inFile.get());
    if (treeReader.IsInvalid()){
        std::cerr<<__FILE__<<":"<<__LINE__<<" The file "<<inputFile<<" does not contain the 'sTgcGeoModelTree'"<<std::endl;
        return to_ret;
    }
    
    /// Identifier of the readout element
    TTreeReaderValue<short> stationIndex{treeReader, "stationIndex"};
    TTreeReaderValue<short> stationEta{treeReader, "stationEta"};
    TTreeReaderValue<short> stationPhi{treeReader, "stationPhi"};
    TTreeReaderValue<short> stationMultilayer{treeReader, "stationMultilayer"};
    TTreeReaderValue<std::string> chamberDesign{treeReader,"chamberDesign"};

    //// Chamber Details
    TTreeReaderValue<short> numLayers{treeReader, "numLayers"};
    TTreeReaderValue<float> yCutout{treeReader, "yCutout"};
    TTreeReaderValue<float> gasTck{treeReader, "gasTck"};
    /// Chamber Length for debug
    TTreeReaderValue<float> sChamberLength{treeReader, "sChamberLength"};
    TTreeReaderValue<float> lChamberLength{treeReader, "lChamberLength"};
    TTreeReaderValue<float> chamberHeight{treeReader, "chamberHeight"};
    /// GasGap Lengths for debug
    TTreeReaderValue<float> sGapLength{treeReader, "sGapLength"};
    TTreeReaderValue<float> lGapLength{treeReader, "lGapLength"};
    TTreeReaderValue<float> gapHeight{treeReader, "gapHeight"};

    //// Wire Dimensions
    TTreeReaderValue<std::vector<uint>> numWires{treeReader, "numWires"};
    TTreeReaderValue<std::vector<uint>> firstWireGroupWidth{treeReader, "firstWireGroupWidth"};
    TTreeReaderValue<std::vector<uint>> numWireGroups{treeReader, "numWireGroups"};
    TTreeReaderValue<std::vector<float>> wireCutout{treeReader, "wireCutout"};
    TTreeReaderValue<float> wirePitch{treeReader, "wirePitch"};
    TTreeReaderValue<float> wireWidth{treeReader, "wireWidth"};
    TTreeReaderValue<uint> wireGroupWidth{treeReader, "wireGroupWidth"};

    TTreeReaderValue<std::vector<float>> globalWireGroupPosX{treeReader, "globalWireGroupPosX"};
    TTreeReaderValue<std::vector<float>> globalWireGroupPosY{treeReader, "globalWireGroupPosY"};
    TTreeReaderValue<std::vector<float>> globalWireGroupPosZ{treeReader, "globalWireGroupPosZ"};

    TTreeReaderValue<std::vector<float>> localWireGroupPosX{treeReader, "localWireGroupPosX"};
    TTreeReaderValue<std::vector<float>> localWireGroupPosY{treeReader, "localWireGroupPosY"};
 
    TTreeReaderValue<std::vector<uint8_t>> wireGroupNum{treeReader, "wireGroupNum"};
    TTreeReaderValue<std::vector<uint8_t>> wireGroupGasGap{treeReader, "wireGroupGasGap"};

    /// Strip dimensions 
    TTreeReaderValue<uint> numStrips{treeReader, "numStrips"};
    TTreeReaderValue<float> stripPitch{treeReader, "stripPitch"};
    TTreeReaderValue<float> stripWidth{treeReader, "stripWidth"};

    TTreeReaderValue<std::vector<float>> globalStripPosX{treeReader, "globalStripPosX"};
    TTreeReaderValue<std::vector<float>> globalStripPosY{treeReader, "globalStripPosY"};
    TTreeReaderValue<std::vector<float>> globalStripPosZ{treeReader, "globalStripPosZ"};
    
    TTreeReaderValue<std::vector<float>> localStripPosX{treeReader, "localStripPosX"};
    TTreeReaderValue<std::vector<float>> localStripPosY{treeReader, "localStripPosY"};

    TTreeReaderValue<std::vector<uint>> stripNum{treeReader, "stripNumber"};
    TTreeReaderValue<std::vector<uint8_t>> stripGasGap{treeReader, "stripGasGap"};
    TTreeReaderValue<std::vector<float>> stripLengths{treeReader, "stripLengths"};

   /// Pad dimensions 
    TTreeReaderValue<float> sPadLength{treeReader, "sPadLength"};
    TTreeReaderValue<float> lPadLength{treeReader, "lPadLength"};
    TTreeReaderValue<float> anglePadPhi{treeReader, "anglePadPhi"};
    TTreeReaderValue<float> beamlineRadius{treeReader, "beamlineRadius"};
    TTreeReaderValue<std::vector<uint>> numPads{treeReader, "numPads"};
    TTreeReaderValue<std::vector<uint>> numPadEta{treeReader, "numPadEta"};
    TTreeReaderValue<std::vector<uint>> numPadPhi{treeReader, "numPadPhi"};
    TTreeReaderValue<std::vector<float>> firstPadHeight{treeReader, "firstPadHeight"};
    TTreeReaderValue<std::vector<float>> padHeight{treeReader, "padHeight"};
    TTreeReaderValue<std::vector<float>> padPhiShift{treeReader, "padPhiShift"};    
    TTreeReaderValue<std::vector<float>> firstPadPhiDiv{treeReader, "firstPadPhiDiv"};    

    TTreeReaderValue<std::vector<float>> globalPadCornerBRX{treeReader, "globalPadCornerBRX"};
    TTreeReaderValue<std::vector<float>> globalPadCornerBRY{treeReader, "globalPadCornerBRY"};
    TTreeReaderValue<std::vector<float>> globalPadCornerBRZ{treeReader, "globalPadCornerBRZ"};

    TTreeReaderValue<std::vector<float>> globalPadCornerBLX{treeReader, "globalPadCornerBLX"};
    TTreeReaderValue<std::vector<float>> globalPadCornerBLY{treeReader, "globalPadCornerBLY"};
    TTreeReaderValue<std::vector<float>> globalPadCornerBLZ{treeReader, "globalPadCornerBLZ"};

    TTreeReaderValue<std::vector<float>> globalPadCornerTRX{treeReader, "globalPadCornerTRX"};
    TTreeReaderValue<std::vector<float>> globalPadCornerTRY{treeReader, "globalPadCornerTRY"};
    TTreeReaderValue<std::vector<float>> globalPadCornerTRZ{treeReader, "globalPadCornerTRZ"};

    TTreeReaderValue<std::vector<float>> globalPadCornerTLX{treeReader, "globalPadCornerTLX"};
    TTreeReaderValue<std::vector<float>> globalPadCornerTLY{treeReader, "globalPadCornerTLY"};
    TTreeReaderValue<std::vector<float>> globalPadCornerTLZ{treeReader, "globalPadCornerTLZ"};

    TTreeReaderValue<std::vector<float>> globalPadPosX{treeReader, "globalPadPosX"};
    TTreeReaderValue<std::vector<float>> globalPadPosY{treeReader, "globalPadPosY"};
    TTreeReaderValue<std::vector<float>> globalPadPosZ{treeReader, "globalPadPosZ"};

    TTreeReaderValue<std::vector<float>> localPadCornerBRX{treeReader, "localPadCornerBRX"};
    TTreeReaderValue<std::vector<float>> localPadCornerBRY{treeReader, "localPadCornerBRY"};

    TTreeReaderValue<std::vector<float>> localPadCornerBLX{treeReader, "localPadCornerBLX"};
    TTreeReaderValue<std::vector<float>> localPadCornerBLY{treeReader, "localPadCornerBLY"};

    TTreeReaderValue<std::vector<float>> localPadCornerTRX{treeReader, "localPadCornerTRX"};
    TTreeReaderValue<std::vector<float>> localPadCornerTRY{treeReader, "localPadCornerTRY"};

    TTreeReaderValue<std::vector<float>> localPadCornerTLX{treeReader, "localPadCornerTLX"};
    TTreeReaderValue<std::vector<float>> localPadCornerTLY{treeReader, "localPadCornerTLY"};

    TTreeReaderValue<std::vector<float>> localPadPosX{treeReader, "localPadPosX"};
    TTreeReaderValue<std::vector<float>> localPadPosY{treeReader, "localPadPosY"};

    TTreeReaderValue<std::vector<float>> hitPositionX{treeReader, "hitPositionX"};
    TTreeReaderValue<std::vector<float>> hitPositionY{treeReader, "hitPositionY"};
    TTreeReaderValue<std::vector<int>> padNumber{treeReader, "padNumber"};

    TTreeReaderValue<std::vector<uint8_t>> padGasGap{treeReader, "padGasGap"};
    TTreeReaderValue<std::vector<uint>> padEta{treeReader, "padEtaNumber"};
    TTreeReaderValue<std::vector<uint>> padPhi{treeReader, "padPhiNumber"};

    /// Geo Model transformation
    TTreeReaderValue<std::vector<float>> geoModelTransformX{treeReader, "GeoModelTransformX"};
    TTreeReaderValue<std::vector<float>> geoModelTransformY{treeReader, "GeoModelTransformY"};
    TTreeReaderValue<std::vector<float>> geoModelTransformZ{treeReader, "GeoModelTransformZ"};

    TTreeReaderValue<std::vector<float>> alignableNodeX{treeReader, "AlignableNodeX"};
    TTreeReaderValue<std::vector<float>> alignableNodeY{treeReader, "AlignableNodeY"};
    TTreeReaderValue<std::vector<float>> alignableNodeZ{treeReader, "AlignableNodeZ"};
    /// Local to Global Strip Transformation
    TTreeReaderValue<std::vector<float>> stripRotCol1X{treeReader, "stripRotLinearCol1X"};
    TTreeReaderValue<std::vector<float>> stripRotCol1Y{treeReader, "stripRotLinearCol1Y"};
    TTreeReaderValue<std::vector<float>> stripRotCol1Z{treeReader, "stripRotLinearCol1Z"};

    TTreeReaderValue<std::vector<float>> stripRotCol2X{treeReader, "stripRotLinearCol2X"};
    TTreeReaderValue<std::vector<float>> stripRotCol2Y{treeReader, "stripRotLinearCol2Y"};
    TTreeReaderValue<std::vector<float>> stripRotCol2Z{treeReader, "stripRotLinearCol2Z"};

    TTreeReaderValue<std::vector<float>> stripRotCol3X{treeReader, "stripRotLinearCol3X"};
    TTreeReaderValue<std::vector<float>> stripRotCol3Y{treeReader, "stripRotLinearCol3Y"};
    TTreeReaderValue<std::vector<float>> stripRotCol3Z{treeReader, "stripRotLinearCol3Z"};

    TTreeReaderValue<std::vector<float>> stripRotTransX{treeReader, "stripRotTranslationX"};
    TTreeReaderValue<std::vector<float>> stripRotTransY{treeReader, "stripRotTranslationY"};
    TTreeReaderValue<std::vector<float>> stripRotTransZ{treeReader, "stripRotTranslationZ"};

    TTreeReaderValue<std::vector<uint8_t>> stripRotGasGap{treeReader, "stripRotGasGap"};
    
    /// Local to Global wire Group Transformation
    TTreeReaderValue<std::vector<float>> wireGroupRotCol1X{treeReader, "wireGroupRotLinearCol1X"};
    TTreeReaderValue<std::vector<float>> wireGroupRotCol1Y{treeReader, "wireGroupRotLinearCol1Y"};
    TTreeReaderValue<std::vector<float>> wireGroupRotCol1Z{treeReader, "wireGroupRotLinearCol1Z"};

    TTreeReaderValue<std::vector<float>> wireGroupRotCol2X{treeReader, "wireGroupRotLinearCol2X"};
    TTreeReaderValue<std::vector<float>> wireGroupRotCol2Y{treeReader, "wireGroupRotLinearCol2Y"};
    TTreeReaderValue<std::vector<float>> wireGroupRotCol2Z{treeReader, "wireGroupRotLinearCol2Z"};

    TTreeReaderValue<std::vector<float>> wireGroupRotCol3X{treeReader, "wireGroupRotLinearCol3X"};
    TTreeReaderValue<std::vector<float>> wireGroupRotCol3Y{treeReader, "wireGroupRotLinearCol3Y"};
    TTreeReaderValue<std::vector<float>> wireGroupRotCol3Z{treeReader, "wireGroupRotLinearCol3Z"};

    TTreeReaderValue<std::vector<float>> wireGroupRotTransX{treeReader, "wireGroupRotTranslationX"};
    TTreeReaderValue<std::vector<float>> wireGroupRotTransY{treeReader, "wireGroupRotTranslationY"};
    TTreeReaderValue<std::vector<float>> wireGroupRotTransZ{treeReader, "wireGroupRotTranslationZ"};

    TTreeReaderValue<std::vector<uint8_t>> wireGroupRotGasGap{treeReader, "wireGroupRotGasGap"};

    /// Local to Global pad Transformation
    TTreeReaderValue<std::vector<float>> padRotCol1X{treeReader, "padRotLinearCol1X"};
    TTreeReaderValue<std::vector<float>> padRotCol1Y{treeReader, "padRotLinearCol1Y"};
    TTreeReaderValue<std::vector<float>> padRotCol1Z{treeReader, "padRotLinearCol1Z"};

    TTreeReaderValue<std::vector<float>> padRotCol2X{treeReader, "padRotLinearCol2X"};
    TTreeReaderValue<std::vector<float>> padRotCol2Y{treeReader, "padRotLinearCol2Y"};
    TTreeReaderValue<std::vector<float>> padRotCol2Z{treeReader, "padRotLinearCol2Z"};

    TTreeReaderValue<std::vector<float>> padRotCol3X{treeReader, "padRotLinearCol3X"};
    TTreeReaderValue<std::vector<float>> padRotCol3Y{treeReader, "padRotLinearCol3Y"};
    TTreeReaderValue<std::vector<float>> padRotCol3Z{treeReader, "padRotLinearCol3Z"};

    TTreeReaderValue<std::vector<float>> padRotTransX{treeReader, "padRotTranslationX"};
    TTreeReaderValue<std::vector<float>> padRotTransY{treeReader, "padRotTranslationY"};
    TTreeReaderValue<std::vector<float>> padRotTransZ{treeReader, "padRotTranslationZ"};

    TTreeReaderValue<std::vector<uint8_t>> padRotGasGap{treeReader, "padRotGasGap"};

    while (treeReader.Next()) {
        sTgcChamber newchamber{};

     /// Identifier of the readout element       
        newchamber.stationIndex = (*stationIndex);
        newchamber.stationEta = (*stationEta);
        newchamber.stationPhi = (*stationPhi);
        newchamber.stationMultilayer = (*stationMultilayer);
        newchamber.design = (*chamberDesign);

        //// Chamber Details
        newchamber.numLayers = (*numLayers);
        newchamber.yCutout = (*yCutout);
        newchamber.gasTck = (*gasTck);

        /// Gas Gap lengths for debug
        newchamber.sGapLength = (*sGapLength);
        newchamber.lGapLength = (*lGapLength);
        newchamber.gapHeight = (*gapHeight);
        //// Chamber lengths for debug
        newchamber.sChamberLength = (*sChamberLength);
        newchamber.lChamberLength = (*lChamberLength);
        newchamber.chamberHeight = (*chamberHeight);

        //// Wires
        newchamber.numWires = (*numWires);
        newchamber.firstWireGroupWidth = (*firstWireGroupWidth);
        newchamber.numWireGroups = (*numWireGroups);
        newchamber.wireCutout = (*wireCutout);
        newchamber.wirePitch = (*wirePitch);
        newchamber.wireWidth = (*wireWidth);
        newchamber.wireGroupWidth = (*wireGroupWidth);

        //// Strips
        newchamber.numStrips = (*numStrips);
        newchamber.stripPitch = (*stripPitch);
        newchamber.stripWidth = (*stripWidth);

        //// Pads
        newchamber.sPadLength = (*sPadLength);
        newchamber.lPadLength = (*lPadLength);
        newchamber.anglePadPhi = (*anglePadPhi);
        newchamber.beamlineRadius = (*beamlineRadius);
        newchamber.numPads = (*numPads);
        newchamber.numPadEta = (*numPadEta);
        newchamber.numPadPhi = (*numPadPhi);
        newchamber.firstPadHeight = (*firstPadHeight);
        newchamber.padHeight = (*padHeight);
        newchamber.padPhiShift = (*padPhiShift);
        newchamber.firstPadPhiDiv = (*firstPadPhiDiv);

        Amg::Vector3D geoTrans{(*geoModelTransformX)[0], (*geoModelTransformY)[0], (*geoModelTransformZ)[0]};
        Amg::RotationMatrix3D geoRot{Amg::RotationMatrix3D::Identity()};
        geoRot.col(0) = Amg::Vector3D((*geoModelTransformX)[1], (*geoModelTransformY)[1], (*geoModelTransformZ)[1]);
        geoRot.col(1) = Amg::Vector3D((*geoModelTransformX)[2], (*geoModelTransformY)[2], (*geoModelTransformZ)[2]);
        geoRot.col(2) = Amg::Vector3D((*geoModelTransformX)[3], (*geoModelTransformY)[3], (*geoModelTransformZ)[3]);       
        newchamber.geoModelTransform = Amg::getTransformFromRotTransl(std::move(geoRot), std::move(geoTrans));       
        
        geoRot.col(0) = Amg::Vector3D((*alignableNodeX)[1], (*alignableNodeY)[1], (*alignableNodeZ)[1]);
        geoRot.col(1) = Amg::Vector3D((*alignableNodeX)[2], (*alignableNodeY)[2], (*alignableNodeZ)[2]);
        geoRot.col(2) = Amg::Vector3D((*alignableNodeX)[3], (*alignableNodeY)[3], (*alignableNodeZ)[3]);
        geoTrans = Amg::Vector3D{(*alignableNodeX)[0], (*alignableNodeY)[0], (*alignableNodeZ)[0]};
        newchamber.alignableTransform = Amg::getTransformFromRotTransl(std::move(geoRot), std::move(geoTrans));
        //WireGroups
        for (size_t wg = 0; wg < globalWireGroupPosX->size(); ++wg){
            sTgcChamber::sTgcChannel newWireGroup{};
            newWireGroup.localPosition = Amg::Vector2D{(*localWireGroupPosX)[wg], (*localWireGroupPosY)[wg]};    
            newWireGroup.globalPosition = Amg::Vector3D{(*globalWireGroupPosX)[wg], (*globalWireGroupPosY)[wg], (*globalWireGroupPosZ)[wg]};      
            newWireGroup.gasGap = (*wireGroupGasGap)[wg];
            newWireGroup.channelNumber = (*wireGroupNum)[wg];
            newWireGroup.channelType = 2;
            ///Uncomment to avoid wireGroupPositions dump
            if (newWireGroup.channelNumber > 0) continue;
            newchamber.channels.insert(std::move(newWireGroup));
        }

        //Strips Filling in global positions
        for (size_t s = 0; s < globalStripPosX->size(); ++s){
            sTgcChamber::sTgcChannel newStrip{};
            newStrip.localPosition = Amg::Vector2D{(*localStripPosX)[s], (*localStripPosY)[s]};    
            newStrip.globalPosition = Amg::Vector3D{(*globalStripPosX)[s], (*globalStripPosY)[s], (*globalStripPosZ)[s]};    
            newStrip.gasGap = (*stripGasGap)[s];
            newStrip.channelNumber = (*stripNum)[s];
            newStrip.channelType = 1;
            newStrip.channelLen = (*stripLengths)[s];
            ///Uncomment to avoid stripPositions dump
            if (newStrip.channelNumber > 0) continue;
            newchamber.channels.insert(std::move(newStrip));
        }


        //Pads
        for (size_t p = 0; p < globalPadPosX->size(); ++p){
            sTgcChamber::sTgcPad newPad{};
           
            newPad.globalPosition = Amg::Vector3D{(*globalPadPosX)[p], (*globalPadPosY)[p], (*globalPadPosZ)[p]};
            newPad.globalPadCornerBR = Amg::Vector3D{(*globalPadCornerBRX)[p], (*globalPadCornerBRY)[p], (*globalPadCornerBRZ)[p]};
            newPad.globalPadCornerBL = Amg::Vector3D{(*globalPadCornerBLX)[p], (*globalPadCornerBLY)[p], (*globalPadCornerBLZ)[p]};
            newPad.globalPadCornerTR = Amg::Vector3D{(*globalPadCornerTRX)[p], (*globalPadCornerTRY)[p], (*globalPadCornerTRZ)[p]};
            newPad.globalPadCornerTL = Amg::Vector3D{(*globalPadCornerTLX)[p], (*globalPadCornerTLY)[p], (*globalPadCornerTLZ)[p]};

            newPad.localPosition = Amg::Vector2D{(*localPadPosX)[p], (*localPadPosY)[p]};
            newPad.localPadCornerBR = Amg::Vector2D{(*localPadCornerBRX)[p], (*localPadCornerBRY)[p]};
            newPad.localPadCornerBL = Amg::Vector2D{(*localPadCornerBLX)[p], (*localPadCornerBLY)[p]};
            newPad.localPadCornerTR = Amg::Vector2D{(*localPadCornerTRX)[p], (*localPadCornerTRY)[p]};
            newPad.localPadCornerTL = Amg::Vector2D{(*localPadCornerTLX)[p], (*localPadCornerTLY)[p]};

            newPad.hitPosition = Amg::Vector2D{(*hitPositionX)[p], (*hitPositionY)[p]};
            newPad.padNumber = (*padNumber)[p];
            newPad.gasGap = (*padGasGap)[p];
            newPad.padEta = (*padEta)[p];
            newPad.padPhi = (*padPhi)[p];
            ///Uncomment to avoid padPositions dump
            if (newPad.padEta > 1 || newPad.padPhi > 6) continue;
            newchamber.pads.insert(std::move(newPad));
        }

        for (size_t l = 0; l < stripRotGasGap->size(); ++l){
            sTgcChamber::sTgcLayer stripLayer{};
            stripLayer.chType = sTgcIdHelper::sTgcChannelTypes::Strip;
            stripLayer.gasGap = (*stripRotGasGap)[l];
            Amg::RotationMatrix3D stripRot{Amg::RotationMatrix3D::Identity()};
            stripRot.col(0) = Amg::Vector3D((*stripRotCol1X)[l],(*stripRotCol1Y)[l], (*stripRotCol1Z)[l]);
            stripRot.col(1) = Amg::Vector3D((*stripRotCol2X)[l],(*stripRotCol2Y)[l], (*stripRotCol2Z)[l]);
            stripRot.col(2) = Amg::Vector3D((*stripRotCol3X)[l],(*stripRotCol3Y)[l], (*stripRotCol3Z)[l]);
            Amg::Vector3D layTrans{(*stripRotTransX)[l], (*stripRotTransY)[l], (*stripRotTransZ)[l]};
            stripLayer.transform = Amg::getTransformFromRotTransl(std::move(stripRot), std::move(layTrans));
            newchamber.layers.insert(std::move(stripLayer));
        }
        
        for (size_t l = 0; l < wireGroupRotGasGap->size(); ++l){
            sTgcChamber::sTgcLayer wireGroupLayer{};
            wireGroupLayer.chType = sTgcIdHelper::sTgcChannelTypes::Wire;
            wireGroupLayer.gasGap = (*wireGroupRotGasGap)[l];
            Amg::RotationMatrix3D wireGroupRot{Amg::RotationMatrix3D::Identity()};
            wireGroupRot.col(0) = Amg::Vector3D((*wireGroupRotCol1X)[l],(*wireGroupRotCol1Y)[l], (*wireGroupRotCol1Z)[l]);
            wireGroupRot.col(1) = Amg::Vector3D((*wireGroupRotCol2X)[l],(*wireGroupRotCol2Y)[l], (*wireGroupRotCol2Z)[l]);
            wireGroupRot.col(2) = Amg::Vector3D((*wireGroupRotCol3X)[l],(*wireGroupRotCol3Y)[l], (*wireGroupRotCol3Z)[l]);
            Amg::Vector3D layTrans{(*wireGroupRotTransX)[l], (*wireGroupRotTransY)[l], (*wireGroupRotTransZ)[l]};
            wireGroupLayer.transform = Amg::getTransformFromRotTransl(std::move(wireGroupRot), std::move(layTrans));
            newchamber.layers.insert(std::move(wireGroupLayer));
        }

        for (size_t l = 0; l < padRotGasGap->size(); ++l){
            sTgcChamber::sTgcLayer padLayer{};
            padLayer.chType = sTgcIdHelper::sTgcChannelTypes::Pad;
            padLayer.gasGap = (*padRotGasGap)[l];
            Amg::RotationMatrix3D padRot{Amg::RotationMatrix3D::Identity()};
            padRot.col(0) = Amg::Vector3D((*padRotCol1X)[l],(*padRotCol1Y)[l], (*padRotCol1Z)[l]);
            padRot.col(1) = Amg::Vector3D((*padRotCol2X)[l],(*padRotCol2Y)[l], (*padRotCol2Z)[l]);
            padRot.col(2) = Amg::Vector3D((*padRotCol3X)[l],(*padRotCol3Y)[l], (*padRotCol3Z)[l]);
            Amg::Vector3D layTrans{(*padRotTransX)[l], (*padRotTransY)[l], (*padRotTransZ)[l]};
            padLayer.transform = Amg::getTransformFromRotTransl(std::move(padRot), std::move(layTrans));
            newchamber.layers.insert(std::move(padLayer));
        }

        auto insert_itr = to_ret.insert(std::move(newchamber));
        if (!insert_itr.second) {
            std::stringstream err{};
            err<<__FILE__<<":"<<__LINE__<<" The chamber "<<(*insert_itr.first).stationIndex
               <<" has already been inserted. "<<std::endl;
            throw std::runtime_error(err.str());
        }
    }
    std::cout<<"File parsing is finished. Found in total "<<to_ret.size()<<" readout element dumps "<<std::endl;
    return to_ret;
}

#define TEST_BASICPROP(attribute, propName) \
    if (std::abs(1.*test.attribute - 1.*reference.attribute) > tolerance) {           \
        std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": The chamber "<<reference   \
                 <<" differs w.r.t "<<propName<<" "<< reference.attribute             \
                 <<" (ref) vs. " <<test.attribute << " (test)" << std::endl;          \
        chamberOkay = false;                                                          \
    }

int main1( int argc, char** argv ) {
    std::string refFile{}, testFile{};
    
    for (int arg = 1; arg < argc; ++arg) {
       std::string the_arg{argv[arg]};
       if (the_arg == "--refFile" && arg +1 < argc) {
          refFile = std::string{argv[arg+1]};
          ++arg;
       } else if (the_arg == "--testFile" && arg + 1 < argc) {
            testFile = std::string{argv[arg+1]};
            ++arg;
       }
    }
    if (refFile.empty()) {
        std::cerr<<"Please parse the path of the reference file via --refFile "<<std::endl;
        return EXIT_FAILURE;
    }
    if (testFile.empty()) {
        std::cerr<<"Please parse the path of the test file via --testFile "<<std::endl;
        return EXIT_FAILURE;
    }
    /// check whether the files are xroot d -> otherwise call path resovler
    if (!refFile.starts_with( "root://")) refFile = PathResolver::FindCalibFile(refFile);
    if (!testFile.starts_with( "root://")) testFile = PathResolver::FindCalibFile(testFile);
    /// Parse the tree dump
    std::set<sTgcChamber> refChambers = readTreeDump(refFile);
    if (refChambers.empty()) {
        std::cerr<<"The file "<<refFile<<" should contain at least one chamber "<<std::endl;
        return EXIT_FAILURE;
    }
    std::set<sTgcChamber> testChambers = readTreeDump(testFile);
    if (testChambers.empty()) {
        std::cerr<<"The file "<<testFile<<" should contain at least one chamber "<<std::endl;
        return EXIT_FAILURE;
    }
    int return_code = EXIT_SUCCESS;
    /// Start to loop over the chambers
    for (const sTgcChamber& reference : refChambers) {
        std::set<sTgcChamber>::const_iterator test_itr = testChambers.find(reference);
        
        if (test_itr == testChambers.end()) {
            std::cerr<<"The chamber "<<reference<<" is not part of the testing "<<std::endl;
            return_code = EXIT_FAILURE;
            continue;
        }
        bool chamberOkay = true;
        const sTgcChamber& test = {*test_itr};

        const Amg::Transform3D alignableDistort = test.alignableTransform.inverse()*(reference.alignableTransform );
        if (!Amg::doesNotDeform(alignableDistort) || alignableDistort.translation().mag() > tolerance) {
            std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": The alignable nodes are at differnt places for  "
                     <<test<<". " <<Amg::toString(alignableDistort, true)<<std::endl;
            chamberOkay = false;
        }
        
        TEST_BASICPROP(numLayers, "number of gas gaps");
        TEST_BASICPROP(yCutout, "yCutout of the Chamber");
        TEST_BASICPROP(gasTck, "thickness of the gas gap");

        ///Chamber and GasGap Lengths for debug
        TEST_BASICPROP(sChamberLength, "Chamber length on the short side");
        TEST_BASICPROP(lChamberLength, "Chamber length on the long side");
        TEST_BASICPROP(chamberHeight, "Chamber height");
        TEST_BASICPROP(sGapLength, "GasGap length on the short side");
        TEST_BASICPROP(lGapLength, "GasGap length on the long side");
        TEST_BASICPROP(gapHeight, "GasGap Height");
     
        TEST_BASICPROP(wirePitch, "pitch of a single wire");
        TEST_BASICPROP(wireWidth, "width of a single wire");
        TEST_BASICPROP(wireGroupWidth, "number of wires in a normal wiregroup");
        
        TEST_BASICPROP(numStrips, "number of strips in a chamber");
        TEST_BASICPROP(stripPitch, "pitch of a normal strip");
        TEST_BASICPROP(stripWidth, "width of a normal strip");
        TEST_BASICPROP(sPadLength, "gasGap length on the short side for pads and wires");
        TEST_BASICPROP(lPadLength, "gasGap length on the long side for pads and wires");
        TEST_BASICPROP(anglePadPhi, "angular width of a pad in phi direction");
        TEST_BASICPROP(beamlineRadius, "distance from the gapCenter to beamline");

        int c = 0;
        using sTgcLayer = sTgcChamber::sTgcLayer;
        for (const sTgcLayer& refLayer : reference.layers) {
            std::set<sTgcLayer>::const_iterator lay_itr = test.layers.find(refLayer);
            if (lay_itr == test.layers.end()) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
                         <<refLayer<<" is not found. "<<std::endl;
                chamberOkay = false;
                continue;
            }
            const sTgcLayer& testLayer{*lay_itr};
            const Amg::Transform3D layAlignment = refLayer.transform.inverse() *
                                                  testLayer.transform;
            ///Uncomment to dump the local to global layer transformation 
            if (false)
            std::cout <<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
                        << "The test layer transform for layer "<< c << " is: " << Amg::toString(testLayer.transform) 
                        << " and the reference layer transform is: " << Amg::toString(refLayer.transform)
                        <<"difference: "<<Amg::toString(layAlignment)<<std::endl;

            if (!Amg::isIdentity(layAlignment)) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
                            <<"the layer "<<testLayer<<" is misaligned w.r.t. reference by "
                            <<Amg::toString(layAlignment)<<std::endl;
                chamberOkay = false;
                continue;
            }
            /// Testing Wire Vectors
            TEST_BASICPROP(numWires[c], "number of wires in the layer "<< c + 1 << " are ");
            TEST_BASICPROP(firstWireGroupWidth[c], "number of wires in first wire group in the layer "<< c + 1 << " are ");
            /// Testing Pad Vectors
            TEST_BASICPROP(numPads[c], "number of pads in the layer "<< c + 1 << " are ");
            TEST_BASICPROP(numPadEta[c], "number of pads in the eta direction in the layer "<< c + 1 << " are ");
            TEST_BASICPROP(numPadPhi[c], "number of in the phi direction in the layer "<< c + 1 << " are ");
            TEST_BASICPROP(firstPadHeight[c], "height of the first pad row in the layer "<< c + 1 << " are ");
            TEST_BASICPROP(padHeight[c], "height of pads in the rest of the rows in the layer "<< c + 1 << " are ");
            TEST_BASICPROP(padPhiShift[c], "shift of inner pad edges in phi direction in the layer "<< c + 1 << " are ");
            TEST_BASICPROP(firstPadPhiDiv[c], "angular position of the outer edge of the first pad in the layer "<< c + 1 << " are ");
            c = (c+1)% 4;
        }
   
        using sTgcChannel = sTgcChamber::sTgcChannel;   
        for (const sTgcChannel& refChannel : reference.channels) {
            std::set<sTgcChannel>::const_iterator channel_itr = test.channels.find(refChannel);
            if (channel_itr == test.channels.end()) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
                        <<refChannel<<" is not found. "<<std::endl;
                chamberOkay = false;
                continue;
            }
            const sTgcChannel& testChannel{*channel_itr};
        
            const Amg::Vector3D diffGlobalPos{testChannel.globalPosition - refChannel.globalPosition};
            const Amg::Vector2D diffLocalPos{testChannel.localPosition - refChannel.localPosition};        
             if (diffGlobalPos.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "<<"channel (gasGap/number): "
                            <<testChannel.gasGap<<"/"<<testChannel.channelNumber<<", chType: "<<testChannel.channelType<<", "<< " global position: "
                            <<Amg::toString(testChannel.globalPosition, 2)<<" should be located at "<<Amg::toString(refChannel.globalPosition, 2)
                            <<" displacement: "<<Amg::toString(diffGlobalPos,2)<<std::endl;
                chamberOkay = false;
            }
           if (diffLocalPos.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "<<"channel (gasGap/number): "
                            <<testChannel.gasGap<<"/"<<testChannel.channelNumber<<", chType: "<<testChannel.channelType<<", "<< " local position: "
                            <<Amg::toString(testChannel.localPosition, 2)<<" should be located at "<<Amg::toString(refChannel.localPosition, 2)
                            <<" displacement: "<<Amg::toString(diffLocalPos,2)<<std::endl;
                chamberOkay = false;
            }
            const double diffChannelLen{testChannel.channelLen - refChannel.channelLen};        
            if (std::abs(diffChannelLen) > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "<<"channel (gasGap/number): "
                            <<testChannel.gasGap<<"/"<<testChannel.channelNumber<<", chType: "<<testChannel.channelType<<", "<< " Run 4 strip Length: "
                            <<testChannel.channelLen<<" Run 3 strip Length "<<refChannel.channelLen
                            <<" displacement: "<<diffChannelLen<<std::endl;
                chamberOkay = false;
            }
        }

        using sTgcPad = sTgcChamber::sTgcPad;    
        for (const sTgcPad& refPad : reference.pads) {
            std::set<sTgcPad>::const_iterator pad_itr = test.pads.find(refPad);
            if (pad_itr == test.pads.end()) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
                        <<refPad<<" is not found. "<<std::endl;                
                chamberOkay = false;
                continue;
            }
            const sTgcPad& testPad{*pad_itr};
            /// Local Pad position dump
            const Amg::Vector2D diffLocalPadPos{testPad.localPosition - refPad.localPosition};
            if (diffLocalPadPos.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "<<"pad (gasGap/(padEta, padPhi)): "
                            <<testPad.gasGap<<"/("<<testPad.padEta<<", "<<testPad.padPhi<<"), "<< " local position: "
                            <<Amg::toString(testPad.localPosition, 2)<<" should be located at "<<Amg::toString(refPad.localPosition, 2)
                            <<" displacement: "<<Amg::toString(diffLocalPadPos,2)<<std::endl;
                chamberOkay = false;
            } 
            /// bottom-left pad Corner          
            const Amg::Vector2D diffLocalPadCornerBL{testPad.localPadCornerBL - refPad.localPadCornerBL};
            if (diffLocalPadCornerBL.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "<<"pad (gasGap/(padEta, padPhi)): "
                            <<testPad.gasGap<<"/("<<testPad.padEta<<", "<<testPad.padPhi<<"), "<< " bottom-left corner: "
                            <<Amg::toString(testPad.localPadCornerBL, 2)<<"  should be located at "<<Amg::toString(refPad.localPadCornerBL, 2)
                            <<" displacement: "<<Amg::toString(diffLocalPadCornerBL,2)<<std::endl;
                chamberOkay = false;
            }
            /// bottom-right pad corner
            const Amg::Vector2D diffLocalPadCornerBR{testPad.localPadCornerBR - refPad.localPadCornerBR};
            if (diffLocalPadCornerBR.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "<<"pad (gasGap/(padEta, padPhi)): "
                            <<testPad.gasGap<<"/("<<testPad.padEta<<", "<<testPad.padPhi<<"), "<< " bottom-right corner: "
                            <<Amg::toString(testPad.localPadCornerBR, 2)<<"  should be located at "<<Amg::toString(refPad.localPadCornerBR, 2)
                            <<" displacement: "<<Amg::toString(diffLocalPadCornerBR,2)<<std::endl;
                chamberOkay = false;
            }  
            /// top-left pad corner        
            const Amg::Vector2D diffLocalPadCornerTL{testPad.localPadCornerTL - refPad.localPadCornerTL};
            if (diffLocalPadCornerTL.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "<<"pad (gasGap/(padEta, padPhi)): "
                            <<testPad.gasGap<<"/("<<testPad.padEta<<", "<<testPad.padPhi<<"), "<< " top-left corner: "
                            <<Amg::toString(testPad.localPadCornerTL, 2)<<"  should be located at "<<Amg::toString(refPad.localPadCornerTL, 2)
                            <<" displacement: "<<Amg::toString(diffLocalPadCornerTL,2)<<std::endl;
                chamberOkay = false;
            }
            /// top-right pad corner            
            const Amg::Vector2D diffLocalPadCornerTR{testPad.localPadCornerTR - refPad.localPadCornerTR};
            if (diffLocalPadCornerTR.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "<<"pad (gasGap/(padEta, padPhi)): "
                            <<testPad.gasGap<<"/("<<testPad.padEta<<", "<<testPad.padPhi<<"), "<< " top-right corner: "
                            <<Amg::toString(testPad.localPadCornerTR, 2)<<"  should be located at "<<Amg::toString(refPad.localPadCornerTR, 2)
                            <<" displacement: "<<Amg::toString(diffLocalPadCornerTR,2)<<std::endl;
                chamberOkay = false;
            }
    
            /// Global Pad position dump
            const Amg::Vector3D diffGlobalPadPos{testPad.globalPosition - refPad.globalPosition};
            if (diffGlobalPadPos.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "<<"pad (gasGap/(padEta, padPhi)): "
                            <<testPad.gasGap<<"/("<<testPad.padEta<<", "<<testPad.padPhi<<"), "<< " global position: "
                            <<Amg::toString(testPad.globalPosition, 2)<<" should be located at "<<Amg::toString(refPad.globalPosition, 2)
                            <<" displacement: "<<Amg::toString(diffGlobalPadPos,2)<<std::endl;
                chamberOkay = false;
            }
            /// bottom-left pad Corner          
            const Amg::Vector3D diffGlobalPadCornerBL{testPad.globalPadCornerBL - refPad.globalPadCornerBL};
            if (diffGlobalPadCornerBL.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "<<"pad (gasGap/(padEta, padPhi)): "
                            <<testPad.gasGap<<"/("<<testPad.padEta<<", "<<testPad.padPhi<<"), "<< " bottom-left corner: "
                            <<Amg::toString(testPad.globalPadCornerBL, 2)<<"  should be located at "<<Amg::toString(refPad.globalPadCornerBL, 2)
                            <<" displacement: "<<Amg::toString(diffGlobalPadCornerBL,2)<<std::endl;
                chamberOkay = false;
            }

            /// bottom-right pad corner
            const Amg::Vector3D diffGlobalPadCornerBR{testPad.globalPadCornerBR - refPad.globalPadCornerBR};
            if (diffGlobalPadCornerBR.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "<<"pad (gasGap/(padEta, padPhi)): "
                            <<testPad.gasGap<<"/("<<testPad.padEta<<", "<<testPad.padPhi<<"), "<< " bottom-right corner: "
                            <<Amg::toString(testPad.globalPadCornerBR, 2)<<"  should be located at "<<Amg::toString(refPad.globalPadCornerBR, 2)
                            <<" displacement: "<<Amg::toString(diffGlobalPadCornerBR,2)<<std::endl;
                chamberOkay = false;
            }  
            /// top-left pad corner        
            const Amg::Vector3D diffGlobalPadCornerTL{testPad.globalPadCornerTL - refPad.globalPadCornerTL};
            if (diffGlobalPadCornerTL.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "<<"pad (gasGap/(padEta, padPhi)): "
                            <<testPad.gasGap<<"/("<<testPad.padEta<<", "<<testPad.padPhi<<"), "<< " top-left corner: "
                            <<Amg::toString(testPad.globalPadCornerTL, 2)<<"  should be located at "<<Amg::toString(refPad.globalPadCornerTL, 2)
                            <<" displacement: "<<Amg::toString(diffGlobalPadCornerTL,2)<<std::endl;
                chamberOkay = false;
            }
            /// top-right pad corner            
            const Amg::Vector3D diffGlobalPadCornerTR{testPad.globalPadCornerTR - refPad.globalPadCornerTR};
            if (diffGlobalPadCornerTR.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "<<"pad (gasGap/(padEta, padPhi)): "
                            <<testPad.gasGap<<"/("<<testPad.padEta<<", "<<testPad.padPhi<<"), "<< " top-right corner: "
                            <<Amg::toString(testPad.globalPadCornerTR, 2)<<"  should be located at "<<Amg::toString(refPad.globalPadCornerTR, 2)
                            <<" displacement: "<<Amg::toString(diffGlobalPadCornerTR,2)<<std::endl;
                chamberOkay = false;
            }
            ///Hit Position used to evaluate padNumber
            const Amg::Vector2D diffHitPosition{testPad.hitPosition - refPad.hitPosition};
            if (diffHitPosition.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "<<"pad (gasGap/(padEta, padPhi)): "
                            <<testPad.gasGap<<"/("<<testPad.padEta<<", "<<testPad.padPhi<<"), "<< " Hit Position: "
                            <<Amg::toString(testPad.hitPosition, 2) <<"  should be "<<Amg::toString(refPad.hitPosition, 2) <<" displacement: "<< Amg::toString(diffHitPosition, 2) <<std::endl;
                chamberOkay = false;
            }
            ///padNumber given the hit position
            const int diffPadNumber{testPad.padNumber - refPad.padNumber};
            if (std::abs(diffPadNumber) > 0 || testPad.padNumber < 0 || refPad.padNumber < 0) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "<<"pad (gasGap/(padEta, padPhi)): "
                            <<testPad.gasGap<<"/("<<testPad.padEta<<", "<<testPad.padPhi<<"), "<< " padNumber: "
                            <<testPad.padNumber <<"  should be "<<refPad.padNumber <<" displacement: "<< diffPadNumber 
                            << " Hit Position: "<< Amg::toString(testPad.hitPosition, 2) << " BL Corner: " 
                            << Amg::toString(testPad.localPadCornerBL, 2) << std::endl;
                chamberOkay = false;
            }

            if (!chamberOkay) {
                return_code = EXIT_FAILURE;
            }
        }
    }
    return return_code;
}


int main (int argc, char** argv )
{
  int ret = 1;
  try {
    ret = main1 (argc, argv);
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
  }
  return ret;
}
