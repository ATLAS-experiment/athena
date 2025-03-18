
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

constexpr double tolerance = 0.003*Gaudi::Units::millimeter;

/// Helper struct to represent a full MicroMegas chamber
struct MmChamber{
    /// Default constructor
    MmChamber() = default;

    //// Identifier 
    int stationIndex{0};
    int stationEta{0};
    int stationPhi{0};
    int multilayer{0};
    std::string design{};

    /// Transformation of the underlying Alignable node
    Amg::Transform3D alignableTransform{Amg::Transform3D::Identity()};

    /// Sorting operator to insert the object into std::set
    bool operator<(const MmChamber& other) const {
        if (stationIndex != other.stationIndex) return stationIndex < other.stationIndex;
        if (stationEta != other.stationEta) return stationEta < other.stationEta;
        if (stationPhi != other.stationPhi) return stationPhi < other.stationPhi;
        return multilayer < other.multilayer;
    }

    /// Transformation of the underlying GeoModel element
    Amg::Transform3D geoModelTransform{Amg::Transform3D::Identity()};

    ////Chamber Details
    unsigned int nGasGaps{0};

    ////Gas Gap dimensions for debug
    double ActiveWidthS{0.};
    double ActiveWidthL{0.};
    double ActiveHeightR{0.};
    double stripPitch{0.};

    double moduleHeight{0.};
    double moduleWidthS{0.};
    double moduleWidthL{0.};


    struct MmChannel{
        /// Local center of the micromega strip
        Amg::Vector2D locCenter{Amg::Vector2D::Zero()};
        /// Global center of the micromega strip
        Amg::Vector3D globCenter{Amg::Vector3D::Zero()};
        /// Left edge of the strip
        Amg::Vector3D leftEdge{Amg::Vector3D::Zero()};
        /// Right edge of the strip
        Amg::Vector3D rightEdge{Amg::Vector3D::Zero()};
        /// @brief length of the strip
        double stripLength{0.};
        /// @brief  strip number
        unsigned int channel{0};
        /// @brief  Gas gap of the strip
        unsigned int gasGap{0};
        /// @brief Short flag whether the angle is stereo
        bool isStereo{false};
        /// @brief Odering operator to use the strip with set
        bool operator<(const MmChannel& other) const {
            if (gasGap != other.gasGap) return gasGap < other.gasGap;
            return channel < other.channel;
        } 

    };

    /// Helper struct to assess that the layers are properly oriented
    struct MmLayer {
        /// @brief Gas gap number of the layer
        unsigned int gasGap{0};
        /// @ transformation
        Amg::Transform3D transform{Amg::Transform3D::Identity()};
        /// @ Reference position of the first strip
        Amg::Vector2D firstStripPos{Amg::Vector2D::Zero()};
        /// @ Reference number of the first strip
        unsigned int firstStrip{0};
        /// @Reference number of all strips
        unsigned int nStrips{0};
        /// @ Readout side on the detector
        int readoutSide{0};
        /// @brief Ordering operator
        bool operator<(const MmLayer& other) const {
            return gasGap < other.gasGap;
        }       
    }; 
    std::set<MmChannel> channels{}; 
    std::set<MmLayer> layers{};
  
};


/// Translation of the station Index -> station Name. Dictionary taken from
/// https://gitlab.cern.ch/atlas/athena/-/blob/main/DetectorDescription/IdDictParser/data/IdDictMuonSpectrometer_R.09.03.xml
std::ostream& operator<<(std::ostream& ostr, const MmChamber& chamb) {
    static const std::map<int, std::string> stationDict{
        {55, "MMS"}, {56, "MML"}
    };
    ostr<<"MicroMegas chamber "<<stationDict.at(chamb.stationIndex)<<std::abs(chamb.stationEta)<<
        (chamb.stationEta>0 ? "A" : "C")<<chamb.stationPhi<<"-"<<chamb.multilayer<<" ";
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr,const MmChamber::MmChannel & channel) {
    ostr<<"channel (gasGap/number): ";
    ostr<<channel.gasGap<<"/";
    ostr<<std::setfill('0')<<std::setw(4)<<channel.channel<<", ";
    ostr<<"center: "<<Amg::toString(channel.globCenter, 2);
    return ostr;
}


std::ostream& operator<<(std::ostream& ostr,const MmChamber::MmLayer & layer) {
    ostr<<"Mmlayer (gasGap): ";
    ostr<<layer.gasGap<<", ";
    // ostr<<"transform: "<<Amg::toString(layer.transform);
    return ostr;
}

std::set<MmChamber> readTreeDump(const std::string& inputFile) {
    std::set<MmChamber> to_ret{};
    std::cout<<"Read the MicroMegas geometry tree dump from "<<inputFile<<std::endl;
    std::unique_ptr<TFile> inFile{TFile::Open(inputFile.c_str())};
    if (!inFile || !inFile->IsOpen()) {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Failed to open "<<inputFile<<std::endl;
        return to_ret;
    }
    TTreeReader treeReader("MmGeoModelTree", inFile.get());
    if (treeReader.IsInvalid()){
        std::cerr<<__FILE__<<":"<<__LINE__<<" The file "<<inputFile<<" does not contain the 'MmGeoModelTree'"<<std::endl;
        return to_ret;
    }
    
    /// Identifier of the readout element
    TTreeReaderValue<unsigned short> stationIndex{treeReader, "stationIndex"};
    TTreeReaderValue<short> stationEta{treeReader, "stationEta"};
    TTreeReaderValue<short> stationPhi{treeReader, "stationPhi"};
    TTreeReaderValue<short> multilayer{treeReader, "multilayer"};

    /// Strip Length and Positions
    TTreeReaderValue<std::vector<uint>> channel{treeReader, "channel"};
    TTreeReaderValue<std::vector<short>> gasGap{treeReader, "gasGap"};
    TTreeReaderValue<std::vector<float>> stripLength{treeReader, "stripLength"};

    TTreeReaderValue<std::vector<bool>> isStereo{treeReader, "isStereo"};
    TTreeReaderValue<std::vector<float>> locStripCenterX{treeReader, "locStripCenterX"};
    TTreeReaderValue<std::vector<float>> locStripCenterY{treeReader, "locStripCenterY"};

    TTreeReaderValue<std::vector<float>> stripCenterX{treeReader, "stripCenterX"};
    TTreeReaderValue<std::vector<float>> stripCenterY{treeReader, "stripCenterY"};
    TTreeReaderValue<std::vector<float>> stripCenterZ{treeReader, "stripCenterZ"};
    TTreeReaderValue<std::vector<float>> stripLeftEdgeX{treeReader, "stripLeftEdgeX"};
    TTreeReaderValue<std::vector<float>> stripLeftEdgeY{treeReader, "stripLeftEdgeY"};
    TTreeReaderValue<std::vector<float>> stripLeftEdgeZ{treeReader, "stripLeftEdgeZ"};
    TTreeReaderValue<std::vector<float>> stripRightEdgeX{treeReader, "stripRightEdgeX"};
    TTreeReaderValue<std::vector<float>> stripRightEdgeY{treeReader, "stripRightEdgeY"};
    TTreeReaderValue<std::vector<float>> stripRightEdgeZ{treeReader, "stripRightEdgeZ"};

    /// GasGap Dimensions for debugging
    TTreeReaderValue<float> ActiveHeightR{treeReader, "ActiveHeightR"};
    TTreeReaderValue<float> ActiveWidthS{treeReader, "ActiveWidthS"};
    TTreeReaderValue<float> ActiveWidthL{treeReader, "ActiveWidthL"};


    TTreeReaderValue<float> moduleHeight{treeReader, "moduleHeight"};
    TTreeReaderValue<float> moduleWidthS{treeReader, "moduleWidthS"};
    TTreeReaderValue<float> moduleWidthL{treeReader, "moduleWidthL"};

    TTreeReaderValue<float> stripPitch{treeReader, "stripPitch"};

    /// Geo Model transformation
    TTreeReaderValue<std::vector<float>> geoModelTransformX{treeReader, "GeoModelTransformX"};
    TTreeReaderValue<std::vector<float>> geoModelTransformY{treeReader, "GeoModelTransformY"};
    TTreeReaderValue<std::vector<float>> geoModelTransformZ{treeReader, "GeoModelTransformZ"};

    TTreeReaderValue<std::vector<float>> alignableNodeX{treeReader, "AlignableNodeX"};
    TTreeReaderValue<std::vector<float>> alignableNodeY{treeReader, "AlignableNodeY"};
    TTreeReaderValue<std::vector<float>> alignableNodeZ{treeReader, "AlignableNodeZ"};

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

    TTreeReaderValue<std::vector<float>> firstStripPosX{treeReader, "firstStripPosX"};
    TTreeReaderValue<std::vector<float>> firstStripPosY{treeReader, "firstStripPosY"};

    TTreeReaderValue<std::vector<int>> readoutSide{treeReader, "stripReadoutSide"};
    TTreeReaderValue<std::vector<unsigned int>> firstStrip{treeReader, "firstStrip"};
    TTreeReaderValue<std::vector<unsigned int>> nStrips{treeReader, "nStrips"};

    

    while (treeReader.Next()) {
        MmChamber newchamber{};

     /// Identifier of the readout element       
        newchamber.stationIndex = (*stationIndex);
        newchamber.stationEta = (*stationEta);
        newchamber.stationPhi = (*stationPhi);
        newchamber.multilayer = (*multilayer);

        /// Gas Gap lengths for debug
        newchamber.ActiveHeightR = (*ActiveHeightR);
        newchamber.ActiveWidthS = (*ActiveWidthS);
        newchamber.ActiveWidthL = (*ActiveWidthL);
        newchamber.stripPitch = (*stripPitch);
        newchamber.moduleHeight = (*moduleHeight);
        newchamber.moduleWidthS = (*moduleWidthS);
        newchamber.moduleWidthL = (*moduleWidthL);

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
        //Strips
        for (size_t s = 0; s < stripCenterX->size(); ++s){
            MmChamber::MmChannel newStrip{};
            newStrip.globCenter = Amg::Vector3D{(*stripCenterX)[s], (*stripCenterY)[s], (*stripCenterZ)[s]}; 
            newStrip.locCenter = Amg::Vector2D{(*locStripCenterX)[s], (*locStripCenterY)[s]};
            newStrip.leftEdge = Amg::Vector3D{(*stripLeftEdgeX)[s], (*stripLeftEdgeY)[s], (*stripLeftEdgeZ)[s]};
            newStrip.rightEdge = Amg::Vector3D{(*stripRightEdgeX)[s], (*stripRightEdgeY)[s], (*stripRightEdgeZ)[s]};
            
            newStrip.gasGap = (*gasGap)[s];
            newStrip.channel = (*channel)[s];
            newStrip.isStereo = (*isStereo)[s];
            newStrip.stripLength = (*stripLength)[s];
            newchamber.channels.insert(std::move(newStrip));
        }

        for (size_t l = 0; l < stripRotGasGap->size(); ++l){
            MmChamber::MmLayer newLayer{};
            newLayer.gasGap = (*stripRotGasGap)[l];
            Amg::RotationMatrix3D stripRot{Amg::RotationMatrix3D::Identity()};
            stripRot.col(0) = Amg::Vector3D((*stripRotCol1X)[l],(*stripRotCol1Y)[l], (*stripRotCol1Z)[l]);
            stripRot.col(1) = Amg::Vector3D((*stripRotCol2X)[l],(*stripRotCol2Y)[l], (*stripRotCol2Z)[l]);
            stripRot.col(2) = Amg::Vector3D((*stripRotCol3X)[l],(*stripRotCol3Y)[l], (*stripRotCol3Z)[l]);
            Amg::Vector3D layTrans{(*stripRotTransX)[l], (*stripRotTransY)[l], (*stripRotTransZ)[l]};
            newLayer.transform = Amg::getTransformFromRotTransl(std::move(stripRot), std::move(layTrans));
            newLayer.firstStripPos = Amg::Vector2D{(*firstStripPosX)[l], (*firstStripPosY)[l]};
            newLayer.readoutSide = (*readoutSide)[l];
            newLayer.firstStrip = (*firstStrip)[l];
            newLayer.nStrips = (*nStrips)[l];

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
        std::cerr<<"runMmGeoComparison() "<<__LINE__<<": The chamber "<<reference     \
                 <<" differs w.r.t "<<propName<<" "<< reference.attribute             \
                 <<" (ref) vs. " <<test.attribute << " (test)"                        \
                 <<", delta: "<<reference.attribute - test.attribute << std::endl;    \
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
    if (!refFile.starts_with ("root://")) refFile = PathResolver::FindCalibFile(refFile);
    if (!testFile.starts_with ("root://")) testFile = PathResolver::FindCalibFile(testFile);
    /// Parse the tree dump
    std::set<MmChamber> refChambers = readTreeDump(refFile);
    if (refChambers.empty()) {
        std::cerr<<"The file "<<refFile<<" should contain at least one chamber "<<std::endl;
        return EXIT_FAILURE;
    }
    std::set<MmChamber> testChambers = readTreeDump(testFile);
    if (testChambers.empty()) {
        std::cerr<<"The file "<<testFile<<" should contain at least one chamber "<<std::endl;
        return EXIT_FAILURE;
    }
    int return_code = EXIT_SUCCESS;
    /// Start to loop over the chambers
    for (const MmChamber& reference : refChambers) {
        std::set<MmChamber>::const_iterator test_itr = testChambers.find(reference);
        
        if (test_itr == testChambers.end()) {
            std::cerr<<"runMmGeoComparison() "<<__LINE__<<": The chamber "<<reference
                     <<" is not part of the testing "<<std::endl;
            return_code = EXIT_FAILURE;
            continue;
        }
        bool chamberOkay{true};
        const MmChamber& test = {*test_itr};

        const Amg::Transform3D alignableDistort = test.alignableTransform.inverse()*(reference.alignableTransform );
        if (!Amg::doesNotDeform(alignableDistort) || alignableDistort.translation().mag() > tolerance) {
            std::cerr<<"runMmGeoComparison() "<<__LINE__<<": The alignable nodes are at differnt places for  "
                     <<test<<". " <<Amg::toString(alignableDistort, 3)<<std::endl;
            chamberOkay = false;
        }

        ///GasGap Dimensions for debugging
        
        {
            constexpr double tolerance = 3. * Gaudi::Units::mm;
            TEST_BASICPROP(ActiveWidthS, "GasGap length on the short side");
            TEST_BASICPROP(ActiveWidthL, "GasGap length on the long side");

        }  
        TEST_BASICPROP(moduleWidthL, "Long module width");
        TEST_BASICPROP(moduleWidthS, "Short module width");
        TEST_BASICPROP(moduleHeight, "Module height ");

        TEST_BASICPROP(ActiveHeightR, "GasGap Height");
        TEST_BASICPROP(stripPitch, "Strip pitch");
        // if (!chamberOkay) continue;
        using MmLayer = MmChamber::MmLayer;
        for (const MmLayer& refLayer : reference.layers) {
            std::set<MmLayer>::const_iterator lay_itr = test.layers.find(refLayer);
            if (lay_itr == test.layers.end()) {
                std::cerr<<"runMmGeoComparison() "<<__LINE__<<": in "<<test<<" "
                         <<refLayer<<" is not found. "<<std::endl;
                chamberOkay = false;
                continue;
            }
            const MmLayer& testLayer{*lay_itr};
            if (!Amg::isIdentity(refLayer.transform.inverse()* testLayer.transform)){
                std::cerr<<"runMmGeoComparison() "<<__LINE__<<": in "<<test<<" "
                         <<testLayer<<" differs w.r.t. reference. delta: "
                         <<Amg::toString(refLayer.transform.inverse()*testLayer.transform)<<std::endl;
                chamberOkay = false;
            }
            if (refLayer.firstStrip != testLayer.firstStrip) {
                std::cerr<<"runMmGeoComparison() "<<__LINE__<<": in "<<test<<" "
                        <<testLayer.gasGap<<" starts from different strip "<<refLayer.firstStrip<<" vs. "
                        <<testLayer.firstStrip<<std::endl;
               chamberOkay = false;
            }
            if (refLayer.nStrips != testLayer.nStrips) {
                std::cerr<<"runMmGeoComparison() "<<__LINE__<<": in "<<test<<" "
                    <<testLayer.gasGap<<" has different number of strips "<<refLayer.nStrips<<" vs. "
                    <<testLayer.nStrips<<std::endl;
                chamberOkay = false;
            }
            /// Gnarf the channel design shifts by 1.5 pitches
            if (false && (refLayer.firstStripPos- testLayer.firstStripPos).mag() > tolerance) {
                std::cerr<<"runMmGeoComparison() "<<__LINE__<<": in "<<test<<" "
                         <<testLayer.gasGap<<" has different starting position "
                         <<Amg::toString(refLayer.firstStripPos, 2) <<" vs. "
                         <<Amg::toString(testLayer.firstStripPos, 2)
                         <<"difference: "<<Amg::toString(refLayer.firstStripPos- testLayer.firstStripPos, 2)
                         <<" / "<<(refLayer.firstStripPos- testLayer.firstStripPos).mag()/reference.stripPitch
                         <<std::endl;
                chamberOkay = false;
            }
        }
        if (!chamberOkay) continue;
        unsigned int failedEta{0}, lastGap{0};
        for (const MmChamber::MmChannel& refStrip : reference.channels) {
            std::set<MmChamber::MmChannel>::const_iterator strip_itr = test.channels.find(refStrip);
            if (strip_itr == test.channels.end()) {
                std::cerr<<"runMmGeoComparison() "<<__LINE__<<": in "<<test<<" "
                          <<refStrip<<" is not found. "<<std::endl;
                chamberOkay = false;
                continue;
            }
            if (lastGap != refStrip.gasGap) {
                lastGap = refStrip.gasGap;
                failedEta = 0;
            }
            const MmChamber::MmChannel& testStrip{*strip_itr};
            /// The centres of the Stereo layers are defined as the bisector of the 
            /// line between the two frame edges. However, thus far the parameter book
            /// deviates from the legacy Run-3 implementation.
            ///  --> Cannot compare the absolute position of the stereo layers. Instead check
            ///      that the left edge, right edge and center point in the new geometry
            ///      are on the same line as defined by the reference system.
            if (failedEta <= 10) {
                const Amg::Vector3D stripDir{Amg::getRotateZ3D(90*Gaudi::Units::deg)*
                                             (refStrip.rightEdge - refStrip.leftEdge).unit()};
                const Amg::Vector3D testDir{Amg::getRotateZ3D(90*Gaudi::Units::deg)*
                                            (testStrip.rightEdge - testStrip.leftEdge).unit()};
                const double centerDist = stripDir.dot(testStrip.globCenter - refStrip.globCenter);
                const double leftDist = stripDir.dot(testStrip.leftEdge -refStrip.globCenter);
                const double rightDist = stripDir.dot(testStrip.rightEdge - refStrip.globCenter);
                if ( std::abs(centerDist) > tolerance || std::abs(leftDist) > tolerance || std::abs(rightDist) > tolerance) {
                     std::cerr<<"runMmGeoComparison() "<<__LINE__<<": In "
                             <<test<<" " <<testStrip <<" + mu "<<Amg::toString(testDir,2) 
                             <<"/local: "<<Amg::toString(testStrip.locCenter, 2) 
                             <<" does not describe the same stereo strip as "
                             <<Amg::toString(refStrip.globCenter, 2)<<"/local:"
                             <<Amg::toString(refStrip.locCenter,2)<<" + lambda "<<Amg::toString(stripDir,2)
                             <<". Distances to the left-edge/center/right-edge: "
                             <<leftDist<<"/"<<centerDist<<"/"<<rightDist<<", dot: "
                             <<std::acos(std::clamp(stripDir.dot(testDir),- 1., 1.)) / Gaudi::Units::deg<<std::endl;
                    chamberOkay = false;
                }
                ++failedEta;
            }
        }

        if (!chamberOkay) {
            return_code = EXIT_FAILURE;
        }
    }
    return return_code;

}


