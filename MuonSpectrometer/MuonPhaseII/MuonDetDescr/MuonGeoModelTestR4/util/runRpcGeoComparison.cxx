
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @brief Helper macro to compare the output from the readout geometry dumps:
 *        python -m MuonGeoModelTest.runGeoModelTest
 *        python -m MuonGeoModelTestR4.runGeoModelTest 
 *  
*/
#include <algorithm>
#include <GeoPrimitives/GeoPrimitives.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <GeoPrimitives/GeoPrimitivesToStringConverter.h>
#include <GeoModelHelpers/TransformToStringConverter.h>
#include <MuonCablingData/RpcCablingData.h>
#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <GaudiKernel/SystemOfUnits.h>

#include <PathResolver/PathResolver.h>
#include <TFile.h>
#include <TTreeReader.h>

using namespace MuonGMR4;
using namespace ActsTrk;


constexpr double tolerance = 0.005*Gaudi::Units::millimeter;

/// Helper struct to represent a full Rpc chamber
struct RpcChamber{
    /// Default constructor
    RpcChamber() = default;
    
    /// Identifier of the Rpc chamber
    using  chamberIdentifier = Muon::RpcCablingOfflineID;
    chamberIdentifier id{};
    std::string design{};

    /// Sorting operator to insert the object into std::set
    bool operator<(const RpcChamber& other) const {
        return id < other.id;
    }
    /// Transformation of the underlying GeoModel element
    Amg::Transform3D geoModelTransform{Amg::Transform3D::Identity()};
    /// Transformation of the underlying Alignable node
    Amg::Transform3D alignableTransform{Amg::Transform3D::Identity()};


    float stripPitchEta{0.f};
    float stripPitchPhi{0.f};
    float stripWidthEta{0.f};
    float stripWidthPhi{0.f};

    float stripLengthEta{0.f};
    float stripLengthPhi{0.f};

    unsigned int numStripsEta{0};
    unsigned int numStripsPhi{0};
    /// Number of rpc singlets along the radial direction
    unsigned int numLayers{0};
    /// Number of rpc gasGaps along the phi direction
    unsigned int numGasGapsPhi{0};
    /// Number of rpc readout panels along the phi direction
    unsigned int numPhiPanels{0};

    struct RpcStrip{
        /// @brief global position of the strip
        Amg::Vector3D position{Amg::Vector3D::Zero()};
        /// @brief local position w.r.t tracking plane
        Amg::Vector2D locPos{Amg::Vector2D::Zero()};
        /// @brief  Strip number
        unsigned int strip{0};
        /// @brief  Gas gap of the strip
        unsigned int gasGap{0};
        /// @brief  Phi panel of the strip
        unsigned int doubletPhi{0};
        /// @brief flag whether the strip measures phi
        bool measPhi{false};

        /// @brief Odering operator to use the strip with set
        bool operator<(const RpcStrip& other) const {
            if (measPhi != other.measPhi) return !measPhi;
            if (doubletPhi != other.doubletPhi) return doubletPhi < other.doubletPhi;
            if (gasGap != other.gasGap) return gasGap < other.gasGap;
            return strip < other.strip;
        }
    };
    /// Helper struct to assess that the layers are properly oriented
    struct RpcLayer {
        /// @brief  Gas gap of the strip
        unsigned int gasGap{0};
        /// @brief  Phi panel of the strip
        unsigned int doubletPhi{0};
        /// @brief flag whether the strip measures phi
        bool measPhi{false};
        /// @ transformation
        Amg::Transform3D transform{Amg::Transform3D::Identity()};
        /// @brief Odering operator to use the strip with set
        bool operator<(const RpcLayer& other) const {
            if (measPhi != other.measPhi) return !measPhi;
            if (doubletPhi != other.doubletPhi) return doubletPhi < other.doubletPhi;
            return gasGap < other.gasGap;
        }
    };
    std::set<RpcStrip> strips{};
    std::set<RpcLayer> layers{};
 
};

/// Translation of the station Index -> station Name. Dictionary taken from
/// https://gitlab.cern.ch/atlas/athena/-/blob/main/DetectorDescription/IdDictParser/data/IdDictMuonSpectrometer_R.09.03.xml
std::ostream& operator<<(std::ostream& ostr, const RpcChamber& chamb) {
    static const std::map<int, std::string> stationDict{
        {0, "BIL"}, {1, "BIS"}, {7, "BIR"},
        {2, "BML"}, {3, "BMS"}, {8, "BMF"}, {53, "BME"}, {54, "BMG"}, {52, "BIM"},
        {4, "BOL"}, {5, "BOS"}, {9, "BOF"}, {10, "BOG"},
        {6, "BEE"}, {14, "EEL"}, {15, "EES"}, 
        {13, "EIL"}, 
        {17, "EML"}, {18, "EMS"}, 
        {20, "EOL"}, {21, "EOS"}
    };
    ostr<<"Rpc chamber "<<stationDict.at(chamb.id.stationIndex) <<"("<<chamb.design<<") "<<chamb.id;
    return ostr;
}

 std::ostream& operator<<(std::ostream& ostr,const RpcChamber::RpcStrip & strip) {
    ostr<<"strip (gasGap/phiPanel/isPhiStrip/number): ";
    ostr<<strip.gasGap<<"/"<<strip.doubletPhi<<"/";
    ostr<<(strip.measPhi ? "si" : "no")<<"/"<<strip.strip<<", ";
    // ostr<<"position: "<<Amg::toString(strip.position, 2);
    return ostr;
}
 std::ostream& operator<<(std::ostream& ostr,const RpcChamber::RpcLayer & layer) {
    ostr<<"rpclayer (gasGap/phiPanel/isPhiLayer): ";
    ostr<<layer.gasGap<<"/"<<layer.doubletPhi<<"/";
    ostr<<(layer.measPhi ? "si" : "no")<<", ";
    // ostr<<"transform: "<<Amg::toString(layer.transform);
    return ostr;
}

std::set<RpcChamber> readTreeDump(const std::string& inputFile) {
    std::set<RpcChamber> to_ret{};
    std::cout<<"Read the Rpc geometry tree dump from "<<inputFile<<std::endl;
    std::unique_ptr<TFile> inFile{TFile::Open(inputFile.c_str())};
    if (!inFile || !inFile->IsOpen()) {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Failed to open "<<inputFile<<std::endl;
        return to_ret;
    }
    TTreeReader treeReader("RpcGeoModelTree", inFile.get());
    if (treeReader.IsInvalid()){
        std::cerr<<__FILE__<<":"<<__LINE__<<" The file "<<inputFile<<" does not contain the 'RpcGeoModelTree'"<<std::endl;
        return to_ret;
    }

    TTreeReaderValue<unsigned short> stationIndex{treeReader, "stationIndex"};
    TTreeReaderValue<short> stationEta{treeReader, "stationEta"};
    TTreeReaderValue<short> stationPhi{treeReader, "stationPhi"};
    TTreeReaderValue<uint8_t> stationDoubletR{treeReader, "stationDoubletR"};
    TTreeReaderValue<uint8_t> stationDoubletZ{treeReader,"stationDoubletZ"};
    TTreeReaderValue<uint8_t> stationDoubletPhi{treeReader,"stationDoubletPhi"};
    TTreeReaderValue<std::string> chamberDesign{treeReader,"chamberDesign"};
    

     /// Number of strips, strip pitch in eta & phi direction
    TTreeReaderValue<uint8_t> numStripsEta{treeReader, "numEtaStrips"};
    TTreeReaderValue<uint8_t> numStripsPhi{treeReader, "numPhiStrips"};
    /// Number of eta & phi gas gaps
    // TTreeReaderValue<uint8_t> numGasGapsPhi{treeReader, "numPhiGasGaps"};
    TTreeReaderValue<uint8_t> numPhiPanels{treeReader, "numPhiPanels"};
    TTreeReaderValue<uint8_t> numLayers{treeReader, "numRpcLayers"};
       
    /// Strip dimensions 
    TTreeReaderValue<float> stripEtaPitch{treeReader, "stripEtaPitch"};
    TTreeReaderValue<float> stripPhiPitch{treeReader, "stripPhiPitch"};
    TTreeReaderValue<float> stripEtaWidth{treeReader, "stripEtaWidth"};
    TTreeReaderValue<float> stripPhiWidth{treeReader, "stripPhiWidth"};
    TTreeReaderValue<float> stripEtaLength{treeReader, "stripEtaLength"};
    TTreeReaderValue<float> stripPhiLength{treeReader, "stripPhiLength"};
 
    /// Geo Model transformation
    TTreeReaderValue<std::vector<float>> geoModelTransformX{treeReader, "GeoModelTransformX"};
    TTreeReaderValue<std::vector<float>> geoModelTransformY{treeReader, "GeoModelTransformY"};
    TTreeReaderValue<std::vector<float>> geoModelTransformZ{treeReader, "GeoModelTransformZ"};

    TTreeReaderValue<std::vector<float>> alignableNodeX{treeReader, "AlignableNodeX"};
    TTreeReaderValue<std::vector<float>> alignableNodeY{treeReader, "AlignableNodeY"};
    TTreeReaderValue<std::vector<float>> alignableNodeZ{treeReader, "AlignableNodeZ"};



    TTreeReaderValue<std::vector<float>> stripRotTranslationX{treeReader, "stripRotTranslationX"};
    TTreeReaderValue<std::vector<float>> stripRotTranslationY{treeReader, "stripRotTranslationY"};
    TTreeReaderValue<std::vector<float>> stripRotTranslationZ{treeReader, "stripRotTranslationZ"};


    TTreeReaderValue<std::vector<float>> stripRotCol1X{treeReader, "stripRotLinearCol1X"};
    TTreeReaderValue<std::vector<float>> stripRotCol1Y{treeReader, "stripRotLinearCol1Y"};
    TTreeReaderValue<std::vector<float>> stripRotCol1Z{treeReader, "stripRotLinearCol1Z"};

    TTreeReaderValue<std::vector<float>> stripRotCol2X{treeReader, "stripRotLinearCol2X"};
    TTreeReaderValue<std::vector<float>> stripRotCol2Y{treeReader, "stripRotLinearCol2Y"};
    TTreeReaderValue<std::vector<float>> stripRotCol2Z{treeReader, "stripRotLinearCol2Z"};

    TTreeReaderValue<std::vector<float>> stripRotCol3X{treeReader, "stripRotLinearCol3X"};
    TTreeReaderValue<std::vector<float>> stripRotCol3Y{treeReader, "stripRotLinearCol3Y"};
    TTreeReaderValue<std::vector<float>> stripRotCol3Z{treeReader, "stripRotLinearCol3Z"};

    TTreeReaderValue<std::vector<uint8_t>> stripRotGasGap{treeReader, "stripRotGasGap"};
    TTreeReaderValue<std::vector<uint8_t>> stripRotDblPhi{treeReader, "stripRotDoubletPhi"};
    TTreeReaderValue<std::vector<bool>>    stripRotMeasPhi{treeReader, "stripRotMeasPhi"};
     
    TTreeReaderValue<std::vector<float>> stripPosX{treeReader, "stripPosX"};
    TTreeReaderValue<std::vector<float>> stripPosY{treeReader, "stripPosY"};
    TTreeReaderValue<std::vector<float>> stripPosZ{treeReader, "stripPosZ"};

    TTreeReaderValue<std::vector<float>> stripLocPosX{treeReader, "stripLocPosX"};
    TTreeReaderValue<std::vector<float>> stripLocPosY{treeReader, "stripLocPosY"};

    
    TTreeReaderValue<std::vector<bool>> stripPosMeasPhi{treeReader, "stripPosMeasPhi"};
    TTreeReaderValue<std::vector<uint8_t>> stripPosGasGap{treeReader, "stripPosGasGap"};
    TTreeReaderValue<std::vector<uint8_t>> stripPosNum{treeReader, "stripPosNum"};
    TTreeReaderValue<std::vector<uint8_t>> stripDblPhi{treeReader, "stripPosDoubletPhi"};

    while (treeReader.Next()) {
        RpcChamber newchamber{};

        newchamber.id.stationIndex = (*stationIndex);
        newchamber.design = (*chamberDesign);
        newchamber.id.eta = (*stationEta);
        newchamber.id.phi = (*stationPhi);
        newchamber.id.doubletPhi = (*stationDoubletPhi);
        newchamber.id.doubletR = (*stationDoubletR);
        newchamber.id.doubletZ = (*stationDoubletZ);
        
        newchamber.stripPitchEta = (*stripEtaPitch);
        newchamber.stripPitchPhi = (*stripPhiPitch);
        newchamber.stripWidthEta = (*stripEtaWidth);
        newchamber.stripWidthPhi = (*stripPhiWidth);
        newchamber.stripLengthEta = (*stripEtaLength);
        newchamber.stripLengthPhi = (*stripPhiLength);

        newchamber.numStripsEta = (*numStripsEta);
        newchamber.numStripsPhi = (*numStripsPhi);
        // newchamber.numGasGapsPhi = (*numGasGapsPhi);
        newchamber.numPhiPanels = (*numPhiPanels);
        newchamber.numLayers = (*numLayers);
        

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
 
        //strips
        for (size_t s = 0; s < stripPosX->size(); ++s){
            RpcChamber::RpcStrip newStrip{};
            newStrip.position = Amg::Vector3D{(*stripPosX)[s], (*stripPosY)[s], (*stripPosZ)[s]};
            newStrip.locPos = Amg::Vector2D{(*stripLocPosX)[s], (*stripLocPosY)[s]};
           
            newStrip.gasGap = (*stripPosGasGap)[s];
            newStrip.doubletPhi = (*stripDblPhi)[s];
            newStrip.measPhi = (*stripPosMeasPhi)[s];
            newStrip.strip = (*stripPosNum)[s];
            newchamber.strips.insert(std::move(newStrip));
        }
        for (size_t l = 0; l < stripRotMeasPhi->size(); ++l){
            RpcChamber::RpcLayer newLayer{};
            newLayer.measPhi = (*stripRotMeasPhi)[l];
            newLayer.gasGap = (*stripRotGasGap)[l];
            newLayer.doubletPhi = (*stripRotDblPhi)[l];
            Amg::RotationMatrix3D stripRot{Amg::RotationMatrix3D::Identity()};
            Amg::Vector3D translation{(*stripRotTranslationX)[l],(*stripRotTranslationY)[l],(*stripRotTranslationZ)[l]};
            stripRot.col(0) = Amg::Vector3D((*stripRotCol1X)[l],(*stripRotCol1Y)[l], (*stripRotCol1Z)[l]);
            stripRot.col(1) = Amg::Vector3D((*stripRotCol2X)[l],(*stripRotCol2Y)[l], (*stripRotCol2Z)[l]);
            stripRot.col(2) = Amg::Vector3D((*stripRotCol3X)[l],(*stripRotCol3Y)[l], (*stripRotCol3Z)[l]);
            newLayer.transform = Amg::getTransformFromRotTransl(std::move(stripRot), std::move(translation));
            newchamber.layers.insert(std::move(newLayer));
        } 
        
        auto insert_itr = to_ret.insert(std::move(newchamber));
        if (!insert_itr.second) {
            std::stringstream err{};
            err<<__FILE__<<":"<<__LINE__<<" The chamber "<<(*insert_itr.first).id
               <<" has already been inserted. "<<std::endl;
            throw std::runtime_error(err.str());
        }
    }
    std::cout<<"File parsing is finished. Found in total "<<to_ret.size()<<" readout element dumps "<<std::endl;
    return to_ret;
}

#define TEST_BASICPROP(attribute, propName) \
    if (std::abs(1.*test.attribute - 1.*reference.attribute) > tolerance) {           \
        std::cerr<<"RpcGeoModelComparison() "<<__LINE__<<": The chamber "<<test       \
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
    std::set<RpcChamber> refChambers = readTreeDump(refFile);
    if (refChambers.empty()) {
        std::cerr<<"The file "<<refFile<<" should contain at least one chamber "<<std::endl;
        return EXIT_FAILURE;
    }
    std::set<RpcChamber> testChambers = readTreeDump(testFile);
    if (testChambers.empty()) {
        std::cerr<<"The file "<<testFile<<" should contain at least one chamber "<<std::endl;
        return EXIT_FAILURE;
    }
    std::cout<<"Read "<<refChambers.size()<<" chambers from reference: "<<refFile
             <<" & "<<testChambers.size()<<" from "<<testFile<<std::endl;
    int return_code = EXIT_SUCCESS;
    unsigned int goodChambers{0};
    /// Start to loop over the chambers
    for (const RpcChamber& reference : refChambers) {
        std::set<RpcChamber>::const_iterator test_itr = testChambers.find(reference);
        
        if (test_itr == testChambers.end()) {
            std::cerr<<"runRpcGeoComparison() "<<__LINE__<<": The chamber "<<reference<<" is not part of the testing "<<std::endl;
            return_code = EXIT_FAILURE;
            continue;
        }
        bool chamberOkay = true;
        const RpcChamber& test = {*test_itr};
      
        TEST_BASICPROP(numLayers, "number of rpc singlets");
        TEST_BASICPROP(numGasGapsPhi, "number of phi gas gaps");
        TEST_BASICPROP(numPhiPanels, "number of phi readout panels");
        
        
        TEST_BASICPROP(numStripsEta, "number of eta strips");
        TEST_BASICPROP(numStripsPhi, "number of phi strips");
        
        TEST_BASICPROP(stripPitchEta, "eta strip pitch");
        TEST_BASICPROP(stripPitchPhi, "phi strip pitch");
        
        TEST_BASICPROP(stripWidthEta, "eta strip width");
        TEST_BASICPROP(stripWidthPhi, "phi strip width");

        TEST_BASICPROP(stripLengthEta, "eta strip length");
        TEST_BASICPROP(stripLengthPhi, "phi strip length");
        if (!chamberOkay) continue;

        Amg::Transform3D moduleDiff = reference.geoModelTransform.inverse() *
                                      test.geoModelTransform;
        
        if (false && !Amg::doesNotDeform(moduleDiff)){
            std::cerr<<"runRpcGeoComparison() "<<__LINE__<<": "<<test<<" is displaced by "
                       <<Amg::toString(moduleDiff)<<std::endl;
            continue;
        }


        using RpcLayer = RpcChamber::RpcLayer;
        for (const RpcLayer& refLayer : reference.layers) {
            break;
            std::set<RpcLayer>::const_iterator lay_itr = test.layers.find(refLayer);
            if (lay_itr == test.layers.end()) {
                std::cerr<<"runRpcGeoComparison() "<<__LINE__<<": "<<test<<" "
                         <<refLayer<<" is not found. "<<std::endl;
                chamberOkay = false;
                continue;
            }
            // break;
            const RpcLayer& testLayer{*lay_itr};
            const Amg::Transform3D layAlignment = testLayer.transform.inverse() *
                                                  refLayer.transform;
            if (layAlignment.translation().mag() > tolerance) {
                std::cerr<<"runRpcGeoComparison() "<<__LINE__<<": "<<test<<" "
                         <<"the layer "<<testLayer<<" is misplaced w.r.t. reference by "
                         <<Amg::toString(testLayer.transform.translation())<<" vs. "
                         <<Amg::toString(refLayer.transform.translation()) <<" delta : "
                         <<Amg::toString(layAlignment.translation())
                         <<", perp: "<<layAlignment.translation().perp()
                         <<", mag: "<<layAlignment.translation().mag()<<std::endl;
                chamberOkay = false;
            }
            if (!Amg::doesNotDeform(layAlignment) && 
                !Amg::doesNotDeform(layAlignment * Amg::getRotateX3D(180*Gaudi::Units::deg)) &&
                !Amg::doesNotDeform(layAlignment * Amg::getRotateZ3D(180*Gaudi::Units::deg))) {
                std::cerr<<"runRpcGeoComparison() "<<__LINE__<<": "<<test<<" "
                         <<"the layer "<<testLayer<<" is misaligned w.r.t. reference by "
                         <<GeoTrf::toString(layAlignment, true)<<std::endl;
                chamberOkay = false;
            }
        }
        using RpcStrip = RpcChamber::RpcStrip;
        if (!chamberOkay) continue;
        unsigned int failedEta{0}, failedPhi{0};
        for (const RpcStrip& refStrip : reference.strips) {
            std::set<RpcStrip>::const_iterator strip_itr = test.strips.find(refStrip);
            if (strip_itr == test.strips.end()) {
                std::cerr<<"runRpcGeoComparison() "<<__LINE__<<": "<<test<<" "
                         <<refStrip<<" is not found. "<<std::endl;
                chamberOkay = false;
                continue;
            }
            const RpcStrip& testStrip{*strip_itr};
            const Amg::Vector3D diffStrip{testStrip.position - refStrip.position};
            if (diffStrip.mag() > tolerance) {
                constexpr unsigned int maxFail = 3;
                if ( (!refStrip.measPhi && ( (++failedEta) <= maxFail || refStrip.strip <= 3)) ||
                      (refStrip.measPhi && ( (++failedPhi) <= maxFail || refStrip.strip <= 3)) ) {
                    std::cerr<<"runRpcGeoComparison() "<<__LINE__<<": "<<test<<" "
                             <<testStrip<<" should be located at "<<Amg::toString(refStrip.position, 2)
                             <<" displacement: "<<Amg::toString(diffStrip,2)<<", perp: "
                             <<diffStrip.perp()<<", mag: "<<diffStrip.mag()<<std::endl;
                } else if (failedEta > maxFail && failedPhi > maxFail) {
                    break;
                }
                chamberOkay = false;
            }
            continue;
            const Amg::Vector2D diffLocStrip{testStrip.locPos - refStrip.locPos};
            if (diffStrip.mag() > tolerance) {
                std::cerr<<"runRpcGeoComparison() "<<__LINE__<<": "<<test<<" "
                         <<testStrip<<" should be located at "<<Amg::toString(refStrip.locPos, 2)
                         <<" displacement: "<<Amg::toString(diffLocStrip,2)<<", mag: "<<diffLocStrip.mag()<<std::endl;
                chamberOkay = false;
            }
        }
        if (!chamberOkay) {
            return_code = EXIT_FAILURE;
            continue;
        } 

        const Amg::Transform3D alignableDistort = test.alignableTransform.inverse()*(reference.alignableTransform );
        if (!Amg::doesNotDeform(alignableDistort) || alignableDistort.translation().mag() > tolerance) {
            std::cerr<<"runRpcGeoComparison() "<<__LINE__<<": The alignable nodes are at differnt places for  "
                     <<test<<". " <<Amg::toString(alignableDistort, true)<<std::endl;
            return_code = EXIT_FAILURE;
        } else {
            ++goodChambers;
        }

    }
    for (const RpcChamber& test : testChambers){
        if (refChambers.find(test) == refChambers.end()) {
            std::cerr<<"runRpcGeoComparison() "<<__LINE__<<": The chamber "<<test<<" is not in the references."<<std::endl;
            return_code = EXIT_FAILURE;
        }
    }
    std::cout<<"runRpcGeoComparison() "<<__LINE__<<": "<<
                goodChambers<<"/"<<refChambers.size()<<" chambers are in complete agreement. "<<std::endl;
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
