/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <GeoPrimitives/GeoPrimitives.h>
#include <MuonPatternHelpers/MatrixUtils.h>
#include <stdlib.h>
#include <iostream>
#include <set>

int main() {
    using namespace MuonR4;

    constexpr unsigned int n = 8;
    AmgSymMatrix(n) testMe{AmgSymMatrix(n)::Zero()};
    std::vector<double> testMeVector{};
    std::array<double, sumUp(n) > testMeArray{};
    unsigned int globN = 1;

    int retCode{EXIT_SUCCESS};
    std::set<unsigned int> usedIndices{};
    for (unsigned int  i =0; i < n; ++i) {
        for (unsigned int k=i; k< n; ++k){
            if (vecIdxFromSymMat<n>(k, i) != vecIdxFromSymMat<n>(i, k)) {
                std::cerr<<__FILE__<<":"<<__LINE__<<" Indeces: "<<i<<", "<<k<<" are not symmetric "<<vecIdxFromSymMat<n>(k, i) <<" "<<vecIdxFromSymMat<n>(i, k) <<std::endl;
                retCode = EXIT_FAILURE;
            }
            if (!usedIndices.insert(vecIdxFromSymMat<n>(k, i)).second) {
                std::cerr<<__FILE__<<":"<<__LINE__<<" Indeces: "<<i<<", "<<k<<" are not unique: "<<vecIdxFromSymMat<n>(k, i)<<std::endl;
                retCode = EXIT_FAILURE;
            }
            const std::array backIdx = symMatIdxFromVec<n>(globN-1); 
            if (backIdx[0] != i || backIdx[1] != k) {
                std::cerr<<__FILE__<<":"<<__LINE__<<"Back indexing of i: "<<i<<", k: "<<k<<" to "<<vecIdxFromSymMat<n>(k, i)<<" --> "
                         <<backIdx[0]<<", "<<backIdx[1]<<std::endl;
                retCode = EXIT_FAILURE;
            }
            std::cout<<"i: "<<i<<", k: "<<k<<" -> "<<vecIdxFromSymMat<n>(k,i)<<" "<<globN<<" --> backIdx: "<<backIdx[0]<<","<<backIdx[1]<<std::endl;
            testMe(i,k) = testMe(k,i) = globN;
            testMeVector.push_back(globN);
            testMeArray[vecIdxFromSymMat<n>(k,i)] = globN;
            ++globN;
        }
    }
    std::cout<<"Filled matrix: "<<std::endl<<testMe<<std::endl;
    std::cout<<"Filled vector: {";
    for (double v : testMeVector) {
        std::cout<<v<<",";
    }
    std::cout<<"}"<<std::endl;
    std::cout<<"Filled array : {";
    for (double a : testMeArray) {
        std::cout<<a<<",";
    }
    std::cout<<"}"<<std::endl;

    return retCode;
}