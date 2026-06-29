/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonTesterTree/ThreeVectorBranch.h>

#include "GaudiKernel/SystemOfUnits.h"

constexpr float inDeg(const float rad) {
    return rad / Gaudi::Units::deg;
}
namespace MuonVal {

bool ThreeVectorBranch::fill(const EventContext&) { return true; }
bool ThreeVectorBranch::init() { return true; }

ThreeVectorBranch::ThreeVectorBranch(MuonTesterTree& tree, const std::string& vec_name) : MuonTesterBranch(tree, vec_name) {}
size_t ThreeVectorBranch::size() const { return m_x.size(); }

void ThreeVectorBranch::push_back(const float x, const float y, const float z) {
    m_x += x;
    m_y += y;
    m_z += z;
}
void ThreeVectorBranch::set(const float x, const float y, const float z, size_t pos) {
    m_x[pos] = x;
    m_y[pos] = y;
    m_z[pos] = z;
}
void ThreeVectorBranch::push_back(const Amg::Vector3D& vec) { push_back(vec[0], vec[1], vec[2]); }
void ThreeVectorBranch::set(const Amg::Vector3D& vec, size_t pos) { set(vec[0], vec[1], vec[2], pos); }
void ThreeVectorBranch::operator+=(const Amg::Vector3D& vec) { push_back(vec); }
void ThreeVectorBranch::push_back(const TVector3& vec) { push_back(vec.X(), vec.Y(), vec.Z()); }
void ThreeVectorBranch::operator+=(const TVector3& vec) { push_back(vec); }
void ThreeVectorBranch::set(const TVector3& vec, size_t pos) { set(vec.X(), vec.Y(), vec.Z(), pos); }

UnitThreeVectorBranch::UnitThreeVectorBranch(MuonTesterTree& tree, const std::string& vec_name): 
    MuonTesterBranch{tree, vec_name} {}
size_t UnitThreeVectorBranch::size() const { return m_theta.size(); }
void UnitThreeVectorBranch::push_back(const Amg::Vector3D& vec) {set (vec, size()); }
void UnitThreeVectorBranch::operator+=(const Amg::Vector3D& vec) { push_back(vec); }

void UnitThreeVectorBranch::set(const Amg::Vector3D& vec, size_t pos) {
    m_theta[pos] = inDeg(vec.theta());
    m_phi[pos] = inDeg(vec.phi());
}

void UnitThreeVectorBranch::push_back(const float x, const float y, const float z) {
    push_back(Amg::Vector3D{x,y,z});
}
void UnitThreeVectorBranch::set(const float x, const float y, const float z, size_t pos) {
    set(Amg::Vector3D{x,y,z}, pos);
}

bool UnitThreeVectorBranch::fill(const EventContext&) { return true; }
bool UnitThreeVectorBranch::init() { return true; }

}