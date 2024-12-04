/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonSpacePoint/UtilFunctions.h>
#include <GeoModelKernel/throwExcept.h>
#include <GeoPrimitives/GeoPrimitivesToStringConverter.h>
#include <GaudiKernel/SystemOfUnits.h>
namespace MuonR4{

    double contract(const CalibratedSpacePoint::Covariance_t& mat, const Amg::Vector3D&a){
        return contract(mat,a,a);
    }
    double contract(const CalibratedSpacePoint::Covariance_t& mat, const Amg::Vector3D&a, const Amg::Vector3D&b){
        return std::visit([&a,&b](const auto& cov) ->double{
                using CovType = std::decay_t<decltype(cov)>;
                if constexpr(std::is_same_v<CovType, AmgSymMatrix(2)>) {
                    return a.block<2,1>(0,0).dot(cov*b.block<2,1>(0,0));
                } else if constexpr(std::is_same_v<CovType, AmgSymMatrix(3)>){
                    return a.dot(static_cast<const CovType&>(cov)*b);
                }
            return 0.;
        }, mat);
    }
    double contract(const CalibratedSpacePoint::Covariance_t& mat, const Amg::Vector2D&a){
        return contract(mat,a,a);
    }
    double contract(const CalibratedSpacePoint::Covariance_t& mat, const Amg::Vector2D&a, const Amg::Vector2D&b){
        return std::visit([&a,&b](const auto& cov) ->double {
                using CovType = std::decay_t<decltype(cov)>;
                if constexpr(std::is_same_v<CovType, AmgSymMatrix(2)>) {
                    return a.dot(cov*b);
                } else {
                    THROW_EXCEPTION("Cannot perform the bilinear product between "<<Amg::toString(a)
                                  <<", "<<Amg::toString(cov)<<", "<<Amg::toString(b));
                }
            return 0.;
        }, mat);
    }

    Amg::Vector3D multiply(const CalibratedSpacePoint::Covariance_t& mat, const Amg::Vector3D& v) {
        Amg::Vector3D result{Amg::Vector3D::Zero()};
        std::visit([&v, &result](const auto&cov) ->void {
                using CovType = std::decay_t<decltype(cov)>;
                if constexpr(std::is_same_v<CovType, AmgSymMatrix(2)>) {
                    result.block<2,1>(0,0) = cov * v.block<2,1>(0,0);
                } else{
                    result = cov * v;
                }
        },mat);
        return result;
    }
    Amg::Vector2D multiply(const CalibratedSpacePoint::Covariance_t& mat, const Amg::Vector2D& v) {
        Amg::Vector2D result{Amg::Vector2D::Zero()};
         std::visit([&v, &result](const auto&cov) ->void {
                using CovType = std::decay_t<decltype(cov)>;
                if constexpr(std::is_same_v<CovType, AmgSymMatrix(2)>) {
                    result.block<2,1>(0,0) = cov * v.block<2,1>(0,0);
                } else {
                    THROW_EXCEPTION("Cannot multiply "<<Amg::toString(cov)<<" with "<<Amg::toString(v));
                }
            }, mat);
        return result;
    }
    
    CalibratedSpacePoint::Covariance_t inverse(const CalibratedSpacePoint::Covariance_t& mat) {
        return std::visit([](const auto& cov)-> CalibratedSpacePoint::Covariance_t {
            using CovType = std::decay_t<decltype(cov)>;
            if constexpr(std::is_same_v<CovType, AmgSymMatrix(2)>) {
                return AmgSymMatrix(2){cov.inverse()};
            } else {
                return AmgSymMatrix(3){cov.inverse()};
            }
        }, mat);
    }
    std::string toString(const CalibratedSpacePoint::Covariance_t& mat) {
         return std::visit([](const auto& cov)-> std::string {
            return Amg::toString(cov);
        }, mat);
    }
    void sortByLayer(std::vector<const SpacePoint*>& spacePoints) {
        std::ranges::sort(spacePoints, 
              [](const SpacePoint*& a, const SpacePoint*& b){   
                    const Amg::Vector3D& hitA{a->positionInChamber()};
                    const Amg::Vector3D& hitB{b->positionInChamber()};
                    constexpr double layerTol = 1.*Gaudi::Units::mm;
                    if (std::abs(hitA.z() - hitB.z()) > layerTol) {
                        return hitA.z() < hitB.z();
                    }
                    return hitA.y() < hitB.y();
              });
    }
}