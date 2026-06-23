/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

// this is a small macro that I run to test the duplicate events
// checker

void print_duplicate_events (const std::string& dir, const std::string& tree)
{
#ifdef __CINT__
  gROOT->ProcessLine(".x $ROOTCOREDIR/scripts/load_packages.C");
#endif

  SH::SampleHandler sh;
  SH::scanDir (sh, dir);
  sh.setMetaString (SH::MetaFields::treeName, tree);
  SH::printDuplicateEventsSplit (sh);
}
