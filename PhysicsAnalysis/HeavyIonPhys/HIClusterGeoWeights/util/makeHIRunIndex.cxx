/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>
#include <vector>

#include "TFile.h"
#include "TH1I.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Syntax: " << argv[0] << " GoodRunList.xml" << std::endl;
    return -1;
  }

  // get rid of everything except that one line with all the run numbers
  FILE *runList =
      popen(Form("grep '<Metadata Name=\"RunList\">' %s | sed -e 's/<Metadata "
                 "Name=\"RunList\">//' -e 's/<\\/Metadata>//' | sed 's/,/ /g' ",
                 argv[1]),
            "r");

  if (!runList) {
    return -1;
  }

  int run;
  std::vector<int> runs;
  while (fscanf(runList, "%d", &run) != EOF) {
    runs.push_back(run);
  }
  pclose(runList);

  std::sort(runs.begin(), runs.end());
  unsigned int size = runs.size();

  std::unique_ptr<TFile> f(TFile::Open("cluster.geo.RUNINDEX.root", "recreate"));
  auto h=std::make_unique<TH1I>("h1_run_index", "h1_run_index", size, 0, size);
  for (unsigned int i = 0; i < size; i++) {
    h->SetBinContent(i + 1, runs.at(i));
  }
  h->Write();

  std::cout << "Created output file cluster.geo.RUNINDEX.root" << std::endl;
  return 0;
}
