/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#pragma once

#include <AthenaBaseComps/AthHistogramAlgorithm.h>
#include <MuonTesterTree/MuonTesterTree.h>
#include <MuonTesterTree/ArrayBranch.h>
namespace MuonVal{
    namespace MuonTester {
        class TreeTestAlg : public AthHistogramAlgorithm {
            public:
                using AthHistogramAlgorithm::AthHistogramAlgorithm;
                virtual StatusCode initialize() override final;
                virtual StatusCode execute() override final;
                virtual StatusCode finalize() override final;

            private:
                MuonTesterTree m_tree{"TestTree", "TestStream"};

                MuonVal::ScalarBranch<unsigned>& m_scalarTest{m_tree.newScalar<unsigned>("scalar")};
                MuonVal::VectorBranch<unsigned>& m_vectorTest{m_tree.newVector<unsigned>("vector")};
                MuonVal::MatrixBranch<unsigned>& m_matrixTest{m_tree.newMatrix<unsigned>("matrix")};
                MuonVal::ArrayBranch<unsigned>* m_arrayTest{nullptr};
        };
    }
}