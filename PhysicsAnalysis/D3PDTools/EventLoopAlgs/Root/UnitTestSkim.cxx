/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <EventLoopAlgs/UnitTestSkim.h>

#include <TTree.h>
#include <RootCoreUtils/Assert.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/IWorker.h>
#include <EventLoopAlgs/NTupleSvc.h>

//
// method implementations
//

namespace EL
{
  void UnitTestSkim ::
  testInvariant () const
  {}



  UnitTestSkim ::
  UnitTestSkim ()
  {
    RCU_NEW_INVARIANT (this);
  }



  StatusCode UnitTestSkim ::
  changeInput (bool /*firstFile*/)
  {
    RCU_CHANGE_INVARIANT (this);
    wk()->tree()->SetBranchAddress ("el_n", &el_n);
    return StatusCode::SUCCESS;
  }



  StatusCode UnitTestSkim ::
  initialize ()
  {
    RCU_CHANGE_INVARIANT (this);

    m_skim = getNTupleSvc (wk(), "output");
    m_skim->tree()->Branch ("el_n2", &el_n2, "el_n2/I");

    return StatusCode::SUCCESS;
  }



  StatusCode UnitTestSkim ::
  execute ()
  {
    RCU_CHANGE_INVARIANT (this);

    wk()->tree()->GetEntry (wk()->treeEntry());
    el_n2 = el_n * el_n;

    return StatusCode::SUCCESS;
  }
}
