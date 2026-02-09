
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @brief Helper macro to compare the output from the readout geometry dumps:
 *          python -m MuonGeoModelTest.runGeoModelTest
 *          python -m MuonGeoModelTestR4.runGeoModelTest 
 *        for the Mdt subdetectors
 *  
*/
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <GeoPrimitives/GeoPrimitivesToStringConverter.h>


#include <ActsGeometryInterfaces/IDetectorElement.h>

#include <Identifier/Identifier.h>

#include "Acts/Surfaces/SurfaceBounds.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Definitions/Tolerance.hpp"

#include <PathResolver/PathResolver.h>
#include <TFile.h>
#include <TTreeReader.h>

#include <set>

namespace {
    template <typename T> std::ostream& operator<<(std::ostream& ostr, const std::vector<T>& v) {
         if (v.empty()) {
            return ostr;
         }
         ostr<<"[";
         for (std::size_t i =0 ; i < v.size() ; ++i) {
            ostr<<v[i];
            if (i +1 != v.size()){
                ostr<<", ";
            }
         }
         ostr<<"]";
         return ostr;
    }
}

using namespace ActsTrk;
using namespace Acts::UnitLiterals;
struct ActiveSensor {
    /// @brief Identifier of the sensor
    Identifier id{};
    /// @brief Technology type of the sensor
    DetectorType detType{DetectorType::UnDefined};
    /// @brief Transform of the sensor
    Amg::Transform3D transform{Amg::Transform3D::Identity()};
    /// @brief sensor thickness
    double thickness{0.};
    /// @brief Boundary type
    using BoundsType = Acts::SurfaceBounds::BoundsType;

    BoundsType boundType{BoundsType::eOther};
    /// @brief Defining properties of the boundary
    std::vector<double> boundaryValues{};
    /// @brief Ordering operator
    bool operator<(const ActiveSensor& other) const {
        return other.id < id;
    }
    friend std::ostream& operator<<(std::ostream& ostr, const ActiveSensor& sensor) {
        ostr<<"Id: "<<sensor.id.get_compact()<<", type: "<<to_string(sensor.detType)
            <<", bounds ("<<sensor.boundType<<") "<<sensor.boundaryValues;
        return ostr;
    }
};

std::set<ActiveSensor> readTreeDump(const std::string& inputFile) {
    std::set<ActiveSensor> to_ret{};
    std::cout<<"Read the Acts geometry dump from "<<inputFile<<std::endl;
    std::unique_ptr<TFile> inFile{TFile::Open(inputFile.c_str())};
    if (!inFile || !inFile->IsOpen()) {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Failed to open "<<inputFile<<std::endl;
        return to_ret;
    }
    TTreeReader treeReader("ReadoutGeometryTest", inFile.get());
    if (treeReader.IsInvalid()){
        std::cerr<<__FILE__<<":"<<__LINE__<<" The file "<<inputFile<<" does not contain the 'ReadoutGeometryTest'"<<std::endl;
        return to_ret;
    }

    TTreeReaderValue<Identifier::value_type> identifier{treeReader, "identifier"};
    TTreeReaderValue<std::uint8_t> detectorType{treeReader, "detectorType"};
    TTreeReaderValue<std::uint8_t> boundType{treeReader, "boundType"};
    
    TTreeReaderValue<std::vector<float>> geoModelTransformX{treeReader, "GeoModelTransformX"};
    TTreeReaderValue<std::vector<float>> geoModelTransformY{treeReader, "GeoModelTransformY"};
    TTreeReaderValue<std::vector<float>> geoModelTransformZ{treeReader, "GeoModelTransformZ"};
    
    TTreeReaderValue<std::vector<double>> boundValues{treeReader, "boundValues"};
    TTreeReaderValue<double> thickness{treeReader, "thickness"};
    
    while (treeReader.Next()) {
        ActiveSensor newElement{};

        newElement.id = (*identifier);
        newElement.detType = static_cast<DetectorType>(*detectorType);
        newElement.boundType = static_cast<ActiveSensor::BoundsType>(*detectorType);
        newElement.boundaryValues = (*boundValues);
        newElement.thickness = (*thickness);
       
        Amg::RotationMatrix3D geoRot{Amg::RotationMatrix3D::Identity()};
        geoRot.col(0) = Amg::Vector3D((*geoModelTransformX)[1], (*geoModelTransformY)[1], (*geoModelTransformZ)[1]);
        geoRot.col(1) = Amg::Vector3D((*geoModelTransformX)[2], (*geoModelTransformY)[2], (*geoModelTransformZ)[2]);
        geoRot.col(2) = Amg::Vector3D((*geoModelTransformX)[3], (*geoModelTransformY)[3], (*geoModelTransformZ)[3]);       
        Amg::Vector3D geoTrans{(*geoModelTransformX)[0], (*geoModelTransformY)[0], (*geoModelTransformZ)[0]};
        newElement.transform = Amg::getTransformFromRotTransl(std::move(geoRot), std::move(geoTrans));
        
        to_ret.insert(newElement);
    }
    std::cout<<"File parsing is finished. Found in total "<<to_ret.size()<<" readout element dumps "<<std::endl;
    return to_ret;
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
    std::set<ActiveSensor> refSensors = readTreeDump(refFile);
    if (refSensors.empty()) {
        std::cerr<<"The file "<<refFile<<" should contain at least one chamber "<<std::endl;
        return EXIT_FAILURE;
    }
    std::set<ActiveSensor> testSensors = readTreeDump(testFile);
    if (testSensors.empty()) {
        std::cerr<<"The file "<<testFile<<" should contain at least one chamber "<<std::endl;
        return EXIT_FAILURE;
    }
    int return_code = EXIT_SUCCESS;
    unsigned int goodSensors{0};
    /// Start to loop over the chambers
    for (const ActiveSensor& reference : refSensors) {
        std::set<ActiveSensor>::const_iterator test_itr = testSensors.find(reference);
        if (test_itr == testSensors.end()) {
            std::cerr<<"runActsGeoComparison() "<<__LINE__<<": The chamber "<<reference
                     <<" is not part of the testing "<<std::endl;
            return_code = EXIT_FAILURE;
            continue;
        }
        bool sensorOkay = true;
        const ActiveSensor& test = {*test_itr};


        if (test.detType != reference.detType) {
            std::cerr<<"runActsGeoComparison() "<<__LINE__<<": The detector element types are different for "<<reference<<" test: "<<to_string(test.detType)<<std::endl;
            sensorOkay = false;
        }

        if (test.boundType != reference.boundType) {
            std::cerr<<"runActsGeoComparison() "<<__LINE__<<": The bound types are different for "<<reference<<" test: "<<test.boundType<<std::endl;
            sensorOkay = false;
        } else {
            for (std::size_t b = 0; b < test.boundaryValues.size(); ++b) {
                if (std::abs(reference.boundaryValues[b] - test.boundaryValues[b]) > Acts::s_epsilon) {
                     std::cerr<<"runActsGeoComparison() "<<__LINE__<<": The bounds are different for "
                              <<reference<<" & test: "<<test.boundaryValues<<std::endl;
                    sensorOkay = false;
                }
            }
        }
        if (std::abs(test.thickness - reference.thickness) > Acts::s_epsilon) {
            std::cerr<<"runActsGeoComparison() "<<__LINE__<<": "<<reference
                     <<" has different thickness w.r.t the test "
                     <<reference.thickness<<" vs. "<< test.thickness<<std::endl;
            sensorOkay = false;
        }      
        const Amg::Transform3D distortion = test.transform.inverse() * reference.transform;
        /// We do not care whether the orientation of the coordinate system along the wire flips for negative
        /// chambers or not
        if (!Amg::doesNotDeform(distortion)) {
            std::cerr<<"runActsGeoComparison() "<<__LINE__<<": The chamber coordinate systems rotate differently for  "
                     <<reference<<". Difference in the coordinate transformation: "<<Amg::toString(distortion)
                     <<" --- refTrf: "<<Amg::toString(reference.transform)
                     <<" --- testTrf: "<<Amg::toString(test.transform)<<std::endl;
            sensorOkay = false;            
        }
        /// The ultimate goal is to have the tube positioned at the same place. 
        /// We maybe need the origin position later when we are adding the alignable transforms...
        if (distortion.translation().mag() > Acts::s_onSurfaceTolerance ) {
            std::cout<<"runActsGeoComparison() "<<__LINE__
                     <<": The origins of the chamber coordinate systems are not exactly at the same point for "
                     <<reference<<". Translation shift: "<<Amg::toString(distortion.translation(), 2)<<std::endl;
            sensorOkay = false;  
        }
        if (sensorOkay){
            ++goodSensors;
        }
    }
    for (const ActiveSensor& test : testSensors) {
        if (!refSensors.count(test)) {
            std::cerr<<"runActsGeoComparison() "<<__LINE__<<": "<<test<<" is only in the test set."<<std::endl;
            return_code = EXIT_FAILURE;
        }
    }
    std::cout<<"runActsGeoComparison() "<<__LINE__<<": "<<goodSensors<<"/"<<refSensors.size()
             <<" sensors are in perfect agreement. "<<std::endl;
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

