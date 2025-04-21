/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSPACEPOINT_UTILFUNCTIONS_H
#define MUONSPACEPOINT_UTILFUNCTIONS_H

#include <MuonSpacePoint/SpacePoint.h>
#include <MuonSpacePoint/CalibratedSpacePoint.h>

namespace MuonR4{
    /** @brief Multiplies a 3D vector with the covariance matrix which can be either 2x2 or 3x3 matrix.
     *         In the former case, only the x-y components are transformed
     *  @param mat: Matrix variant
     *  @param v: Vector which shall be transformed */
    Amg::Vector3D multiply(const CalibratedSpacePoint::Covariance_t& mat, const Amg::Vector3D& v);
    /** @brief Multiplies a 2D vector with the covariance matrix which has to be a 2x2 matrix. Otherwise,
     *         an exception is thrown
     *  @param mat: Matrix variant
     *  @param v: Vector which shall be transformed */
    Amg::Vector2D multiply(const CalibratedSpacePoint::Covariance_t& mat, const Amg::Vector2D& v);
    /** @brief Inverts the parsed matrix*/
    CalibratedSpacePoint::Covariance_t inverse(const CalibratedSpacePoint::Covariance_t& mat);
    
    double contract(const CalibratedSpacePoint::Covariance_t& mat, const Amg::Vector3D& a, const Amg::Vector3D& b);
    double contract(const CalibratedSpacePoint::Covariance_t& mat, const Amg::Vector2D& a, const Amg::Vector2D& b);

    double contract(const CalibratedSpacePoint::Covariance_t& mat, const Amg::Vector3D& a);
    double contract(const CalibratedSpacePoint::Covariance_t& mat, const Amg::Vector2D& a);
    /** @brief Returns the matrix in string */
    std::string toString(const CalibratedSpacePoint::Covariance_t& mat);
}

#endif