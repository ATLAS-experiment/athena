/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

// this is a small macro that I run to test the duplicate events
// checker

void print_duplicate_events (const std::string& dir, const std::string& tree)
{
  SH::SampleHandler sh;
  SH::ScanDir().scan (sh, dir);
  sh.setMetaString (SH::MetaFields::treeName, tree);
  SH::printDuplicateEventsSplit (sh);
}
