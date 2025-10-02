/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSINTEROPS_UNITCONVERTER_H
#define ACTSINTEROPS_UNITCONVERTER_H


#include "GeoPrimitives/GeoPrimitives.h"
#include "GaudiKernel/SystemOfUnits.h"
/// Put athena Eigen inlcude first

#include "Acts/Definitions/Units.hpp"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/Common.hpp"

#include <utility>

namespace ActsTrk{
    /// @brief Converts an energy scalar from Athena to Acts units 
    /// @param athenaE: Energy value to convert
    inline constexpr double energyToActs(const double athenaE) {
        using namespace Acts::UnitLiterals;
        constexpr double energyCnv = 1_MeV / Gaudi::Units::MeV;
        return energyCnv * athenaE;
    }
    /// @brief Converts an energy scalar from Acts to Athena units
    /// @param athenaE: Energy value to convert
    inline constexpr double energyToAthena(const double actsE) {
        using namespace Acts::UnitLiterals;
        constexpr double energyCnv = Gaudi::Units::MeV / 1_MeV;
        return energyCnv * actsE;
    }
    /// @brief Converts a length scalar from Acts to Athena units
    /// @param athenaL: Length value to convert
    inline constexpr double lengthToActs(const double athenaL) {
        using namespace Acts::UnitLiterals;
         constexpr double lengthCnv = 1_mm / Gaudi::Units::mm;
         return lengthCnv * athenaL;
    }
    /// @brief Converts a length scalar from Acts to Athena units
    /// @param athenaL: Length value to convert
    inline constexpr double lengthToAthena(const double actsL) {
        using namespace Acts::UnitLiterals;
         constexpr double lengthCnv = Gaudi::Units::mm / 1_mm;
         return lengthCnv * actsL;
    }
    /// @brief Converts a time unit from Athena to Acts units
    /// @param athenaT: Time interval to convert
    inline constexpr double timeToActs(const double athenaT) {
        using namespace Acts::UnitLiterals;
        constexpr double timeCnv = 1_ns / Gaudi::Units::ns;
        return timeCnv * athenaT;
    }
    /// @brief Converts a time unit from Acts to Athena units
    /// @param actsT: Time interval to convert
    inline constexpr double timeToAthena(const double actsT) {
        using namespace Acts::UnitLiterals;
        constexpr double timeCnv = Gaudi::Units::ns/ 1_ns;
        return timeCnv * actsT;
    }
    /// @brief Converts a direction vector from athena units into acts units
    /// @param athenaDir: Unit normalized vector to convert
    inline Acts::Vector3 convertDirToActs(const Amg::Vector3D& athenaDir) {
        return athenaDir;
    }
    /// @brief Converts a direction vector from acts units into athena units
    /// @param actsDir: Unit normalized vector to convert
    inline Amg::Vector3D convertDirFromActs(const Acts::Vector3& actsDir) {
        return actsDir;
    }
    /// @brief Converts a position vector & time from Athena units into Acts units
    /// @param athenaPos: 3D-spatial position vector to convert
    /// @param athenaTime: Time counts to convert
    inline Acts::Vector4 convertPosToActs(const Amg::Vector3D& athenaPos,
                                          const double athenaTime =0.) {
        Acts::Vector4 pos{Acts::Vector4::Zero()};
        pos[Acts::eTime] = timeToActs(athenaTime);
        pos[Acts::ePos0] = lengthToActs(athenaPos.x());
        pos[Acts::ePos1] = lengthToActs(athenaPos.y());
        pos[Acts::ePos2] = lengthToActs(athenaPos.z());
        return pos;
    }
    /// @brief Converts an Acts 4-vector into a pair of an Athena spatial vector and 
    ///        the passed time
    /// @param actsPos: Position from the Acts framework
    inline std::pair<Amg::Vector3D, double> convertPosFromActs(const Acts::Vector4& actsPos) {
        Amg::Vector3D pos{Amg::Vector3D::Zero()};
        pos[Amg::x] = lengthToAthena(actsPos[Acts::ePos0]);
        pos[Amg::y] = lengthToAthena(actsPos[Acts::ePos1]);
        pos[Amg::z] = lengthToAthena(actsPos[Acts::ePos2]);
        return std::make_pair(std::move(pos), timeToAthena(actsPos[Acts::eTime]));
    }
    /// @brief Converts a three momentum vector from Athena together with the associated particle mass 
    ///        into an Acts four-momentum vector
    /// @param threeMom: Three momentum vector in Athena units
    /// @param mass: Particle's mass in Athena units
    inline Acts::Vector4 convertMomToActs(const Amg::Vector3D& threeMom, const double mass = 0.) {
        using namespace Acts::UnitLiterals;
        Acts::Vector4 fourMom{Acts::Vector4::Zero()};
        fourMom[Acts::eEnergy] = energyToActs(std::sqrt(threeMom.dot(threeMom) + mass*mass));
        fourMom[Acts::eMom0] = energyToActs(threeMom.x());
        fourMom[Acts::eMom1] = energyToActs(threeMom.y());
        fourMom[Acts::eMom2] = energyToActs(threeMom.z());
        return fourMom;
    }
    /// @brief Converts an Acts four-momentum vector into an pair of an Athena three-momentum and 
    ///        the paritcle's energy
    /// @param actsMom: The four momentum vector from Acts
    inline std::pair<Amg::Vector3D, double> convertMomFromActs(const Acts::Vector4& actsMom) {
        Amg::Vector3D threeMom{Amg::Vector3D::Zero()};
        threeMom[Amg::x] = energyToAthena(actsMom[Acts::eMom0]);
        threeMom[Amg::y] = energyToAthena(actsMom[Acts::eMom1]);
        threeMom[Amg::z] = energyToAthena(actsMom[Acts::eMom2]);
        return std::make_pair(std::move(threeMom), energyToAthena(actsMom[Acts::eEnergy]));
    }
}
#endif