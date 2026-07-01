/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <SampleHandler/Global.h>

#include <SampleHandler/SampleComposite.h>
#include <SampleHandler/SampleGrid.h>
#include <SampleHandler/SampleHist.h>
#include <SampleHandler/SampleLocal.h>
#include <SampleHandler/SampleMeta.h>

//
// main program
//

using namespace SH;

int main ()
{
  new SampleComposite;
  new SampleGrid;
  new SampleHist;
  new SampleLocal;
  new SampleMeta;

  return 0;
}
