/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "TreeTesterAlg.h"
#include <memory>
#include <vector>

namespace MuonVal{
    namespace MuonTester{

        StatusCode TreeTestAlg::initialize(){
            auto arrayTest = std::make_unique<ArrayBranch<unsigned>>(m_tree.tree(), "testArray", 2);
            m_arrayTest = arrayTest.get();
            bool ok = m_tree.addBranch(std::move(arrayTest));
            if (not ok){
              ATH_MSG_ERROR("addBranch failed in TreeTestAlg::initialize");
              return StatusCode::FAILURE;
            }
            ATH_CHECK(m_tree.init(this));
            return StatusCode::SUCCESS;
        }
        StatusCode TreeTestAlg::execute() {
            const EventContext& ctx{Gaudi::Hive::currentContext()};
            for (unsigned int n = 0; n < 100; ++n) {
                m_scalarTest = n;
                m_vectorTest[n] = n +1;
                m_matrixTest[n+1] = std::vector<unsigned>{n,n+1,n+3};
                (*m_arrayTest)[0] = n +1;
                (*m_arrayTest)[1] = n+5;
                ATH_CHECK(m_tree.fill(ctx));
            }

            return StatusCode::SUCCESS;
        }
        StatusCode TreeTestAlg::finalize(){
            ATH_CHECK(m_tree.write());
            return StatusCode::SUCCESS;
        }
    }
}