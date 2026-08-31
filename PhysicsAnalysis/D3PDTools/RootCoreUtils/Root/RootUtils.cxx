/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <RootCoreUtils/RootUtils.h>

#include <TDirectory.h>
#include <TEfficiency.h>
#include <TH1.h>
#include <TTree.h>
#include <RootCoreUtils/Assert.h>

//
// method implementations
//

namespace RCU
{
  bool SetDirectory (TObject *object, TDirectory *directory)
  {
    RCU_ASSERT (object != nullptr);

    TH1 *const hist = dynamic_cast<TH1*>(object);
    if (hist)
    {
      hist->SetDirectory (directory);
      return true;
    }

    TEfficiency *const efficiency = dynamic_cast<TEfficiency*>(object);
    if (efficiency)
    {
      efficiency->SetDirectory (directory);
      return true;
    }

    TTree *const tree = dynamic_cast<TTree*>(object);
    if (tree)
    {
      tree->SetDirectory (directory);
      return true;
    }

    return false;
  }
}
