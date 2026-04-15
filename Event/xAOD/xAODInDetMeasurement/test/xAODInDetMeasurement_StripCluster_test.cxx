/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <iostream>
#include <bitset>

// Local include(s):
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"

#include "GeoPrimitives/GeoPrimitives.h"

#include "xAODMeasurementBase/MeasurementDefs.h"


template< typename T >
std::ostream& operator<< ( std::ostream& out,
                           const std::vector< T >& vec ) {

   out << "[";
   for( size_t i = 0; i < vec.size(); ++i ) {
      out << vec[ i ];
      if( i < vec.size() - 1 ) {
         out << ", ";
      }
   }
   out << "]";
   return out;
}

/// Function fill one Strip cluster with information

void fill( xAOD::StripCluster& stripCluster) {

    constexpr xAOD::DetectorIDHashType idHash(123485);

    constexpr xAOD::DetectorIdentType id(1234855431);

    Eigen::Matrix<float,1,1> localPosition(0.15);

    Eigen::Matrix<float,1,1> localCovariance;
    localCovariance.setZero();
    localCovariance(0, 0) = 0.012;

    stripCluster.setMeasurement<1>(idHash, localPosition, localCovariance);
    stripCluster.setIdentifier(id);

    Eigen::Matrix<float, 3, 1> globalPosition(10, 10, 10);

    std::vector < Identifier > rdoList = { Identifier(Identifier::value_type(0x200921680c00000)),
                                           Identifier(Identifier::value_type(0x298094737200000)),
                                           Identifier(Identifier::value_type(0x24e105292800000)) };

    stripCluster.setRDOlist(rdoList);

    stripCluster.globalPosition() = globalPosition;

    stripCluster.setChannelsInPhi(3);

}

namespace {
template <typename T>
std::ostream &operator<<(std::ostream &out, CxxUtils::range_with_conv<CxxUtils::span<T> > element_range) {
   out << "[";
   if constexpr(std::is_same_v<std::remove_cvref_t<T>,Identifier::value_type>) {
      out << std::hex;
   }
   static constexpr const char *prefix=std::is_same_v<std::remove_cvref_t<T>,Identifier::value_type> ? "0x" : "";
   unsigned int count_i=0;
   for (const auto &elm : element_range) {
      if (count_i>0) {
         out << ", ";
      }
      out << prefix << elm;
      ++count_i;
   }
   out << std::dec << "]";
   return out;
}
}

void print ( const xAOD::StripCluster& stripCluster) {
    std::cout << " --------- MEASUREMENT BASE ------------ " << std::endl;
    std::cout << "Identifier Hash = " << stripCluster.identifierHash() << std::endl;
    std::cout << "Identifier = " << stripCluster.identifier() << std::endl;
    std::cout << "Local Position = " << stripCluster.localPosition<1>() << std::endl;
    std::cout << "Local Covariance = " << stripCluster.localCovariance<1>() << std::endl;
    std::cout << " ----------STRIP CLUSTER INFO ----------- " << std::endl;
    std::cout << "Global Position = " << stripCluster.globalPosition() << std::endl;
    std::cout << "RDOs = " << stripCluster.rdoList() << std::endl;
    std::cout << "Number of strips = " << stripCluster.channelsInPhi() << std::endl;
}

int main() {

    // create the main containers to test:
    xAOD::StripClusterAuxContainer aux;
    xAOD::StripClusterContainer tpc;

    tpc.setStore(&aux);

    // add one strip cluster to the container
    xAOD::StripCluster * str = new xAOD::StripCluster();
    tpc.push_back(str);

    // fill information
    fill(*str);

    //print information
    print(*str);

    return 0;
}
