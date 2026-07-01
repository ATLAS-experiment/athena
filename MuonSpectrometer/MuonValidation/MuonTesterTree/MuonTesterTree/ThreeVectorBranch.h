/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTESTER_MUONTHREEVECTORBRANCH_H
#define MUONTESTER_MUONTHREEVECTORBRANCH_H

#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <MuonTesterTree/MuonTesterTree.h>
#include <MuonTesterTree/VectorBranch.h>

/// Helper class to dump spatial vectors in their x,y,z representation
/// to the n-tuple
namespace MuonVal {
class ThreeVectorBranch : public MuonTesterBranch {
public:
    ThreeVectorBranch(MuonTesterTree& tree, const std::string& vec_name);

    /// interface using the Amg::Vector3D
    void push_back(const Amg::Vector3D& vec);
    void operator+=(const Amg::Vector3D& vec);
    void set(const Amg::Vector3D& vec, size_t pos);

    void push_back(const TVector3& vec);
    void operator+=(const TVector3& vec);
    void set(const TVector3& vec, size_t pos);

    void push_back(const float x, const float y, const float z);
    void set(const float x, const float y, const float z, size_t pos);

    size_t size() const;

    bool fill(const EventContext&) override final;
    bool init() override final;

private:
    VectorBranch<float>& m_x{parent().newVector<float>(name() + "X")};
    VectorBranch<float>& m_y{parent().newVector<float>(name() + "Y")};
    VectorBranch<float>& m_z{parent().newVector<float>(name() + "Z")};
};

class UnitThreeVectorBranch : public MuonTesterBranch {
    public:
        UnitThreeVectorBranch(MuonTesterTree& tree, const std::string& vec_name);

        /// interface using the Amg::Vector3D
        void push_back(const Amg::Vector3D& vec);
        void operator+=(const Amg::Vector3D& vec);
        void set(const Amg::Vector3D& vec, size_t pos);

        void push_back(const float x, const float y, const float z);
        void set(const float x, const float y, const float z, size_t pos);
        size_t size() const;

        bool fill(const EventContext&) override final;
        bool init() override final;
    private:
        VectorBranch<float>& m_theta{parent().newVector<float>(name() + "Theta")};
        VectorBranch<float>& m_phi{parent().newVector<float>(name() + "Phi")};
};


}
#endif
