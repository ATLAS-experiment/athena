
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
#include "CxxUtils/starts_with.h"
#include <string>
#include <set>
#include <vector>
#include <map>
#include <iostream>
#include <cmath>

using namespace MuonGMR4;
using namespace ActsTrk;

#include <PathResolver/PathResolver.h>
#include <TFile.h>
#include <TTreeReader.h>

constexpr double tolerance = 1.*Gaudi::Units::millimeter;

/// Helper struct to represent a full sTgc chamber
struct sTgcChamber{
    /// Default constructor
    sTgcChamber() = default;

    //// Identifier
    std::string stationName{}; 
    int stationIndex{0};
    int stationEta{0};
    int stationPhi{0};
    int stationMultilayer{0};
    std::string design{};

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
    float yCutout{0.f};
    float gasTck{0.f};

    ////Chamber lengths for debug
    float sChamberLength{0.f};
    float lChamberLength{0.f};
    float chamberHeight{0.f};
    float sGapLength{0.f};
    float lGapLength{0.f};
    float gapHeight{0.f};


    //// Wires
    std::vector<unsigned int> numWires;
    std::vector<short> firstWireGroupWidth;
    std::vector<short> numWireGroups;
    std::vector<float> wireCutout;
    float wirePitch{0.f};
    float wireWidth{0.f};
    short wireGroupWidth{0};

    //// Strips
    unsigned int numStrips{0};
    float stripPitch{0.f};
    float stripWidth{0.f};
    std::vector<float> firstStripPitch;

    //// Wires and Strips
    struct sTgcChannel{
        Amg::Vector3D position{Amg::Vector3D::Zero()};
        /// @brief  wireGroup/strip number
        unsigned int channelNumber{0};
        /// @brief  Gas gap of the wireGroup/strip
        unsigned int gasGap{0};
        /// @brief Channel type to indicate wireGroup/strip
        unsigned int channelType{0};

        /// @brief Ordering operator to use the wireGroup with set
        bool operator<(const sTgcChannel& other) const {
            if (gasGap != other.gasGap) return gasGap < other.gasGap;
            if (channelType != other.channelType) return channelType < other.channelType;
            return channelNumber < other.channelNumber;
        }       
    };

    //// Pads
    std::vector<uint> numPads;
    std::vector<uint> numPadEta;
    std::vector<uint> numPadPhi;

    //// Pads
    struct sTgcPad{
        Amg::Vector3D position{Amg::Vector3D::Zero()};
        Amg::Vector3D padCornerBR{Amg::Vector3D::Zero()};
        Amg::Vector3D padCornerBL{Amg::Vector3D::Zero()};
        Amg::Vector3D padCornerTR{Amg::Vector3D::Zero()};
        Amg::Vector3D padCornerTL{Amg::Vector3D::Zero()};

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
        /// @ transformation
        Amg::Transform3D transform{Amg::Transform3D::Identity()};
        /// @brief Ordering operator
        bool operator<(const sTgcLayer& other) const {
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
    ostr<<"sTgc chamber "<<stationDict.at(chamb.stationIndex)<<"("<<chamb.design<<") "<<chamb.stationName;
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr,const sTgcChamber::sTgcChannel & channel) {
    ostr<<"channel (gasGap/number): ";
    ostr<<channel.gasGap<<"/";
    ostr<<channel.channelNumber<<", ";
    ostr<<channel.channelType<<", ";
    ostr<<"position: "<<Amg::toString(channel.position, 2);
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr,const sTgcChamber::sTgcPad & pad) {
    ostr<<"pad (gasGap/padEta/padPhi): ";
    ostr<<pad.gasGap<<"/";
    ostr<<pad.padEta<<"/"<<pad.padPhi<<", ";
    ostr<<"position: "<<Amg::toString(pad.position, 2);
    ostr<<"Bottom-right padCorner: "<<Amg::toString(pad.padCornerBR, 2);
    ostr<<"Bottom-left padCorner: "<<Amg::toString(pad.padCornerBL, 2);
    ostr<<"Top-right padCorner: "<<Amg::toString(pad.padCornerTR, 2);
    ostr<<"Top-left padCorner: "<<Amg::toString(pad.padCornerTL, 2);
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr,const sTgcChamber::sTgcLayer & layer) {
    ostr<<"stgclayer (gasGap/channelType): ";
    ostr<<layer.gasGap<<", ";
    ostr<<"transform: "<<Amg::toString(layer.transform);
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
/*
    //// Wire Dimensions
    TTreeReaderValue<std::vector<uint>> numWires{treeReader, "numWires"};
    TTreeReaderValue<std::vector<short>> firstWireGroupWidth{treeReader, "firstWireGroupWidth"};
    TTreeReaderValue<std::vector<short>> numWireGroups{treeReader, "numWireGroups"};
    TTreeReaderValue<std::vector<float>> wireCutout{treeReader, "wireCutout"};
    TTreeReaderValue<float> wirePitch{treeReader, "wirePitch"};
    TTreeReaderValue<float> wireWidth{treeReader, "wireWidth"};
    TTreeReaderValue<short> wireGroupWidth{treeReader, "wireGroupWidth"};

    TTreeReaderValue<std::vector<float>> globalWireGroupPosX{treeReader, "globalWireGroupPosX"};
    TTreeReaderValue<std::vector<float>> globalWireGroupPosY{treeReader, "globalWireGroupPosY"};
    TTreeReaderValue<std::vector<float>> globalWireGroupPosZ{treeReader, "globalWireGroupPosZ"};
 
    TTreeReaderValue<std::vector<uint8_t>> wireGroupNum{treeReader, "wireGroupNum"};
    TTreeReaderValue<std::vector<uint8_t>> wireGroupGasGap{treeReader, "wireGroupGasGap"};
*/
    /// Strip dimensions 
    TTreeReaderValue<uint> numStrips{treeReader, "numStrips"};
    TTreeReaderValue<float> stripPitch{treeReader, "stripPitch"};
    TTreeReaderValue<float> stripWidth{treeReader, "stripWidth"};
    //TTreeReaderValue<std::vector<float>> firstStripPitch{treeReader, "firstStripPitch"};

    TTreeReaderValue<std::vector<float>> globalStripPosX{treeReader, "globalStripPosX"};
    TTreeReaderValue<std::vector<float>> globalStripPosY{treeReader, "globalStripPosY"};
    TTreeReaderValue<std::vector<float>> globalStripPosZ{treeReader, "globalStripPosZ"};

    TTreeReaderValue<std::vector<uint>> stripNum{treeReader, "stripNumber"};
    TTreeReaderValue<std::vector<uint8_t>> stripGasGap{treeReader, "stripGasGap"};
    TTreeReaderValue<std::vector<float>> stripLengths{treeReader, "stripLengths"};
/*
   /// Pad dimensions 
    TTreeReaderValue<std::vector<uint>> numPads{treeReader, "numPads"};
    TTreeReaderValue<std::vector<uint>> numPadEta{treeReader, "numPadEta"};
    TTreeReaderValue<std::vector<uint>> numPadPhi{treeReader, "numPadPhi"};

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

    TTreeReaderValue<std::vector<uint8_t>> padGasGap{treeReader, "padGasGap"};
    TTreeReaderValue<std::vector<uint>> padEta{treeReader, "padEtaNumber"};
    TTreeReaderValue<std::vector<uint>> padPhi{treeReader, "padPhiNumber"};
*/
    /// Geo Model transformation
    TTreeReaderValue<std::vector<float>> geoModelTransformX{treeReader, "GeoModelTransformX"};
    TTreeReaderValue<std::vector<float>> geoModelTransformY{treeReader, "GeoModelTransformY"};
    TTreeReaderValue<std::vector<float>> geoModelTransformZ{treeReader, "GeoModelTransformZ"};

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

/*
        //// Wires
        newchamber.numWires = (*numWires);
        newchamber.firstWireGroupWidth = (*firstWireGroupWidth);
        newchamber.numWireGroups = (*numWireGroups);
        newchamber.wireCutout = (*wireCutout);
        newchamber.wirePitch = (*wirePitch);
        newchamber.wireWidth = (*wireWidth);
        newchamber.wireGroupWidth = (*wireGroupWidth);
*/
        //// Strips
        newchamber.numStrips = (*numStrips);
        newchamber.stripPitch = (*stripPitch);
        newchamber.stripWidth = (*stripWidth);
        //newchamber.firstStripPitch = (*firstStripPitch);
/*
        //// Pads
        newchamber.numPads = (*numPads);
        newchamber.numPadEta = (*numPadEta);
        newchamber.numPadPhi = (*numPadPhi);
*/
        Amg::Vector3D geoTrans{(*geoModelTransformX)[0], (*geoModelTransformY)[0], (*geoModelTransformZ)[0]};
        Amg::RotationMatrix3D geoRot{Amg::RotationMatrix3D::Identity()};
        geoRot.col(0) = Amg::Vector3D((*geoModelTransformX)[1], (*geoModelTransformY)[1], (*geoModelTransformZ)[1]);
        geoRot.col(1) = Amg::Vector3D((*geoModelTransformX)[2], (*geoModelTransformY)[2], (*geoModelTransformZ)[2]);
        geoRot.col(2) = Amg::Vector3D((*geoModelTransformX)[3], (*geoModelTransformY)[3], (*geoModelTransformZ)[3]);       
        newchamber.geoModelTransform = Amg::getTransformFromRotTransl(std::move(geoRot), std::move(geoTrans));       
/*                
        //WireGroups
        for (size_t wg = 0; wg < globalWireGroupPosX->size(); ++wg){
            sTgcChamber::sTgcChannel newWireGroup{};
            newWireGroup.position = Amg::Vector3D{(*globalWireGroupPosX)[wg], (*globalWireGroupPosY)[wg], (*globalWireGroupPosZ)[wg]};      
            newWireGroup.gasGap = (*wireGroupGasGap)[wg];
            newWireGroup.channelNumber = (*wireGroupNum)[wg];
            newWireGroup.channelType = 2;
            newchamber.channels.insert(std::move(newWireGroup));
        }
*/        
        //Strips
        for (size_t s = 0; s < globalStripPosX->size(); ++s){
            sTgcChamber::sTgcChannel newStrip{};
            newStrip.position = Amg::Vector3D{(*globalStripPosX)[s], (*globalStripPosY)[s], (*globalStripPosZ)[s]};    
            newStrip.gasGap = (*stripGasGap)[s];
            newStrip.channelNumber = (*stripNum)[s];
            newStrip.channelType = 1;
            if (newStrip.channelNumber > 3 /* || newStrip.gasGap != 1 || newStrip.channelType != 1*/) continue;
            newchamber.channels.insert(std::move(newStrip));
        }
/*
        //Pads
        for (size_t p = 0; p < globalPadPosX->size(); ++p){
            sTgcChamber::sTgcPad newPad{};
            newPad.position = Amg::Vector3D{(*globalPadPosX)[p], (*globalPadPosY)[p], (*globalPadPosZ)[p]};
            newPad.padCornerBR = Amg::Vector3D{(*globalPadCornerBRX)[p], (*globalPadCornerBRY)[p], (*globalPadCornerBRZ)[p]};
            newPad.padCornerBL = Amg::Vector3D{(*globalPadCornerBLX)[p], (*globalPadCornerBLY)[p], (*globalPadCornerBLZ)[p]};
            newPad.padCornerTR = Amg::Vector3D{(*globalPadCornerTRX)[p], (*globalPadCornerTRY)[p], (*globalPadCornerTRZ)[p]};
            newPad.padCornerTL = Amg::Vector3D{(*globalPadCornerTLX)[p], (*globalPadCornerTLY)[p], (*globalPadCornerTLZ)[p]};

            newPad.gasGap = (*padGasGap)[p];
            newPad.padEta = (*padEta)[p];
            newPad.padPhi = (*padPhi)[p];
            newchamber.pads.insert(std::move(newPad));
        }
*/
        for (size_t l = 0; l < stripRotGasGap->size(); ++l){
            sTgcChamber::sTgcLayer newLayer{};
            newLayer.gasGap = (*stripRotGasGap)[l];
            Amg::RotationMatrix3D stripRot{Amg::RotationMatrix3D::Identity()};
            stripRot.col(0) = Amg::Vector3D((*stripRotCol1X)[l],(*stripRotCol1Y)[l], (*stripRotCol1Z)[l]);
            stripRot.col(1) = Amg::Vector3D((*stripRotCol2X)[l],(*stripRotCol2Y)[l], (*stripRotCol2Z)[l]);
            stripRot.col(2) = Amg::Vector3D((*stripRotCol3X)[l],(*stripRotCol3Y)[l], (*stripRotCol3Z)[l]);
            Amg::Vector3D layTrans{(*stripRotTransX)[l], (*stripRotTransY)[l], (*stripRotTransZ)[l]};
            newLayer.transform = Amg::getTransformFromRotTransl(std::move(stripRot), std::move(layTrans)/*Amg::Vector3D::Zero()*/);
            newchamber.layers.insert(std::move(newLayer));
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
        std::cerr<<"sTgcGeoModelComparison() "<<__LINE__<<": The chamber "<<reference  \
                 <<" differs w.r.t "<<propName<<" "<< reference.attribute             \
                 <<" (ref) vs. " <<test.attribute << " (test)" << std::endl;          \
        chamberOkay = false;                                                          \
    }

int main( int argc, char** argv ) {
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
    if (!CxxUtils::starts_with (refFile, "root://")) refFile = PathResolver::FindCalibFile(refFile);
    if (!CxxUtils::starts_with (testFile, "root://")) testFile = PathResolver::FindCalibFile(testFile);
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


/*     
        TEST_BASICPROP(wirePitch, "pitch of a single wire");
        TEST_BASICPROP(wireWidth, "width of a single wire");
        TEST_BASICPROP(wireGroupWidth, "number of wires in a normal wiregroup");
*/        
        TEST_BASICPROP(numStrips, "number of strips in a chamber");
        TEST_BASICPROP(stripPitch, "pitch of a normal strip");
        TEST_BASICPROP(stripWidth, "width of a normal strip");
/*       
        using sTgcWireGroup = sTgcChamber::sTgcChannel;    
        for (const sTgcWireGroup& refWireGroup : reference.channels) {
            std::set<sTgcWireGroup>::const_iterator wireGroup_itr = test.channels.find(refWireGroup);
            if (wireGroup_itr == test.channels.end()) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
                        <<refWireGroup<<" is not found. "<<std::endl;
                chamberOkay = false;
                continue;
            }
            const sTgcWireGroup& testWireGroup{*wireGroup_itr};
            const Amg::Vector3D diffWireGroup{testWireGroup.position - refWireGroup.position};
            if (diffWireGroup.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
                        <<testWireGroup<<" should be located at "
                        <<Amg::toString(refWireGroup.position, 2)<<" in chamber "
                        <<" displacement: "<<Amg::toString(diffWireGroup,2)<<std::endl;
                chamberOkay = false;
            }
        }
*/      int c = 0;
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
            const Amg::Transform3D layAlignment = testLayer.transform.inverse() *
                                                  refLayer.transform;
            ++c;
            if (c!=1) continue;
            std::cout <<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
                     << "The R4 GeoModel transform is: " << Amg::toString(test.geoModelTransform) 
                     << " and the R3 GeoModel is  transform is: " << Amg::toString(reference.geoModelTransform) << std::endl;
            //std::cout <<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
            //          << "The test layer transform for layer: "<< c << " is: " << Amg::toString(testLayer.transform) 
            //          << " and the reference layer transform is: " << Amg::toString(refLayer.transform) <<std::endl;

            if (!Amg::doesNotDeform(layAlignment)) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
                         <<"the layer "<<testLayer<<" is misaligned w.r.t. reference by "
                         <<Amg::toString(layAlignment)<<std::endl;
                chamberOkay = false;
                continue;
            }
        }

        using sTgcStrip = sTgcChamber::sTgcChannel;   
        for (const sTgcStrip& refStrip : reference.channels) {
            std::set<sTgcStrip>::const_iterator strip_itr = test.channels.find(refStrip);
            if (strip_itr == test.channels.end()) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
                        <<refStrip<<" is not found. "<<std::endl;
                chamberOkay = false;
                continue;
            }
            const sTgcStrip& testStrip{*strip_itr};
        
            const Amg::Vector3D diffStrip{testStrip.position - refStrip.position};
            if (diffStrip.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
                         <<testStrip<<" should be located at "<<Amg::toString(refStrip.position, 2)
                         <<" displacement: "<<Amg::toString(diffStrip,2)<<std::endl;
                chamberOkay = false;
            }
        }
/*
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
            const Amg::Vector3D diffPadPos{testPad.position - refPad.position};
            if (diffPadPos.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
                         <<testPad<<" should be located at "<<Amg::toString(refPad.position, 2)
                         <<" displacement: "<<Amg::toString(diffPadPos,2)<<std::endl;
                chamberOkay = false;
            }
            
            const Amg::Vector3D diffPadCornerBR{testPad.padCornerBR - refPad.padCornerBR};
            if (diffPadCornerBR.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
                         <<testPad<<" bottom-right corner should be located at "<<Amg::toString(refPad.padCornerBR, 2)
                         <<" displacement: "<<Amg::toString(diffPadCornerBR,2)<<std::endl;
                chamberOkay = false;
            }
            const Amg::Vector3D diffPadCornerBL{testPad.padCornerBL - refPad.padCornerBL};
            if (diffPadCornerBL.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
                         <<testPad<<" bottom-left corner should be located at "<<Amg::toString(refPad.padCornerBL, 2)
                         <<" displacement: "<<Amg::toString(diffPadCornerBL,2)<<std::endl;
                chamberOkay = false;
            }
            const Amg::Vector3D diffPadCornerTR{testPad.padCornerTR - refPad.padCornerTR};
            if (diffPadCornerTR.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
                         <<testPad<<" top-right corner should be located at "<<Amg::toString(refPad.padCornerTR, 2)
                         <<" displacement: "<<Amg::toString(diffPadCornerTR,2)<<std::endl;
                chamberOkay = false;
            }   
            const Amg::Vector3D diffPadCornerTL{testPad.padCornerTL - refPad.padCornerTL};
            if (diffPadCornerTL.mag() > tolerance) {
                std::cerr<<"runsTgcGeoComparison() "<<__LINE__<<": in chamber "<<test<<" "
                         <<testPad<<" top-left corner should be located at "<<Amg::toString(refPad.padCornerTL, 2)
                         <<" displacement: "<<Amg::toString(diffPadCornerTL,2)<<std::endl;
                chamberOkay = false;
            }    
        }
*/
        if (!chamberOkay) {
            return_code = EXIT_FAILURE;
        }
    }
    return return_code;

}


