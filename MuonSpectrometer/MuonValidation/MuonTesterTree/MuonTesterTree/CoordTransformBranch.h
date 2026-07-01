/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTESTER_MUONCOORDTRANSFORMBRANCH_H
#define MUONTESTER_MUONCOORDTRANSFORMBRANCH_H

#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/ThreeVectorBranch.h"

namespace MuonVal {

/// Helper class to dump the Amg::Transformations into a single three vector branch
/// The first entry is the translation, the second one the first column, etc.
class CoordTransformBranch : public MuonTesterBranch {
public:   
    CoordTransformBranch(MuonTesterTree& tree, const std::string& vec_name);
    void operator=(const Amg::Transform3D& vec);
    void set(const Amg::Transform3D& vec);

    bool fill(const EventContext&) override final;
    bool init() override final;

private:
    ThreeVectorBranch m_transform{parent(), name()};   
};
/// Helper class to dump the Amg::Transformations into 4 three vector branches
class CoordSystemsBranch: public MuonTesterBranch {
    public:
        CoordSystemsBranch(MuonTesterTree& tree, const std::string& vec_name);
        void operator+=(const Amg::Transform3D& trans);
        void push_back(const Amg::Transform3D& trans);
        void set(const Amg::Transform3D& trans, size_t pos);

        size_t size() const;
        bool fill(const EventContext&) override final;
        bool init() override final;
    private:
        MuonVal::ThreeVectorBranch m_translation{parent(), name() + "Translation"};
        MuonVal::UnitThreeVectorBranch m_linearCol1{parent(), name() + "LinearCol0"};
        MuonVal::UnitThreeVectorBranch m_linearCol2{parent(), name() + "LinearCol1"};
        MuonVal::UnitThreeVectorBranch m_linearCol3{parent(), name() + "LinearCol2"};
};


}
#endif
