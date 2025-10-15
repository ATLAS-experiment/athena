/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TFile.h"
#include "TH1I.h"
#include "TH2F.h"
#include "TH3F.h"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>
#include <cmath>

/// @brief Reads the GRL .xml file and creates an unordered map of run numbers and associated LB ranges
/// @param grl Path to the GRL .xml file
/// @return unordered map of run numbers (as keys) and vectors of LB ranges (as values)
std::unordered_map<int, std::vector<std::pair<int, int>>> getGRLMap(char *grl) {
  // get rid of everything except the run numbers and LB ranges
  // also removes the line with all the runs, so each run number is mentioned only once
  FILE *runList = popen(Form("grep \"Run\\|LBRange\" %s | grep -v Metadata | grep -v GoodRunsListWriter"
                             " | sed -e's/[<>A-Za-z=\"/]//g' -e 's/^ //g' -e 's/ $//g' -e 's/  */ /g' ",
                             grl),
                        "r");

  std::unordered_map<int, std::vector<std::pair<int, int>>> grlMap{};
  int a, LB1 = 0;
  int currentRun = -1;
  while (fscanf(runList, "%d", &a) != EOF) {
    if (a > 100000) {
      // new run
      if(LB1 != 0) {
        std::cout << "Start of a new run " << a << ", but the previous LB range is not closed, LB1 = " 
        << LB1 << "; check the GRL file " << grl << std::endl;
      }
      currentRun = a;
      continue;
    } else {
      // LB
      if (LB1 == 0) {
        LB1 = a; // starting LB
      }
      else {
        // ending LB
        grlMap[currentRun].push_back(std::make_pair(LB1, a));
        LB1 = 0;
        continue;
      }
    }
  }
  int status = pclose(runList);
  if (status == -1){
    std::cout<<"Error in pclose\n";
  }
  return grlMap;
}

/// @brief This macro reads outputs from "HIClusterGeoFiller.py" and performs a linear regression with prepared values.
/// The input file names shall contain the run number. There has to be one file per run; ideally, every run mentioned in
/// the GRL has its corresponding file and every LB mentioned in the GRL has its corresponding histogram in that file.
/// The slope and intercept of the linear regression are written to an output files as response and offset. Response
/// values are normalized to unity along the phi angle, i.e. in each eta slice. In the final root weight file, there are
/// also histograms "h3_w", "h3_eta", "h3_phi", and "h3_R". These are produced by "HICaloGeoExtract.py".
/// @param argv[0] Path to the GRL .xml file
/// @param argv[1] Part of the path to the input files before the run number
/// @param argv[2] Part of the path to the input files after the run number
/// @return Technically integer, practically "cluster.geo.RESPONSE_OFFSET_RUNINDEX.root" file with response, offset, and
/// run index
int main(int argc, char **argv) {
  if (argc != 4) {
    std::cout << "Syntax: " << argv[0] << " GoodRunList.xml /path/to/input/files/prefix. .suffix.root" << std::endl;
    return -1;
  }

  auto grlMap = getGRLMap(argv[1]);
  //// for debugging - print out the GRL
  // for(auto& run : grlMap)
  // {
  //   std::cout<<"Run "<<run.first<<", size: "<<run.second.size()<<std::endl;
  //   for(auto& lb : run.second)
  //   {
  //     std::cout<<"  LB "<<lb.first<<" "<<lb.second<<std::endl;
  //   }
  // }
  if (grlMap.size() == 0) {
    std::cout << "No runs found, is the GRL file (" << argv[1] << ") correct?" << std::endl;
    return -1;
  }

  std::vector<int> runs;
  for (auto &run : grlMap) {
    runs.push_back(run.first);
  }
  std::sort(runs.begin(), runs.end());

  // create run index, since we have all the runs now
  auto runIndex=std::make_unique<TH1I>("h1_run_index", "h1_run_index", runs.size(), 0, runs.size());
  for (unsigned int r = 0; r < runs.size(); r++) {
    runIndex->SetBinContent(r + 1, runs.at(r));
  }
  //runIndex cannot be nullptr here

  std::string prefix = argv[2];
  std::string suffix = argv[3];

  std::unique_ptr<TH3F> response;
  std::unique_ptr<TH3F> offset;
  std::unique_ptr<TH3F> entries;

  // loop over runs
  for (unsigned int r = 0; r < runs.size(); r++) {
    int run = runs.at(r);

    std::string fileName = Form("%s%d%s", prefix.c_str(), run, suffix.c_str());
    std::unique_ptr<TFile> file(TFile::Open(fileName.c_str()));
    if (!file) {
      std::cout << "Could not open file " << fileName << ", skipping the run " << run << std::endl;
      continue;
    }

    std::unique_ptr<TH2F> etaPhiMap(file->Get<TH2F>("h_etaPhiMapping"));

    if (!response) {
      // book response and offset once eta-phi mapping is known
      response =
          std::make_unique<TH3F>("h3_eta_phi_response", "h3_eta_phi_response", 
                  etaPhiMap->GetNbinsX(), etaPhiMap->GetXaxis()->GetXmin(), etaPhiMap->GetXaxis()->GetXmax(),
                  etaPhiMap->GetNbinsY(), etaPhiMap->GetYaxis()->GetXmin(), etaPhiMap->GetYaxis()->GetXmax(),
                  runs.size(), 0, runs.size());
      response->SetDirectory(nullptr);
      offset = std::make_unique<TH3F>(*(TH3F*)response->Clone("h3_eta_phi_offset"));
      offset->SetTitle("h3_eta_phi_offset");
      offset->SetDirectory(nullptr);
      entries = std::make_unique<TH3F>(*(TH3F*)response->Clone("h3_eta_phi_entries"));
      entries->SetTitle("h3_eta_phi_entries");
      entries->SetDirectory(nullptr);
    }

    std::unique_ptr<TH2F> sums;

    // for each range of LBs
    for (auto &lbs : grlMap[run]) {
      // for each LB in that range
      for (int lb = lbs.first; lb <= lbs.second; lb++) {
        std::unique_ptr<TH2F> tmp(file->Get<TH2F>(Form("h_clusterET_fcalET_%d_%d", run, lb)));
        if (!tmp) {
          std::cout << "Could not get h_clusterET_fcalET_" << run << "_" << lb << ", skipping this LB, r = "<<r << std::endl;
          continue;
        }

        // merge into a single histogram
        if (!sums) {
          sums = std::make_unique<TH2F>(*(TH2F*)tmp->Clone("h_clusterET_fcalET_sum"));
        }
        else {
          sums->Add(tmp.get());
        }
      }
    }

    if(!sums)
    {
      std::cout << "Could not get any LB, skipping the run " << run <<std::endl;
      continue;
    }

    // for each bin number
    for (int binID = 1; binID <= sums->GetNbinsX(); binID++) {
      float numEntries = sums->GetBinContent(binID, 6);
      if (numEntries == 0) {
        // skip empty bins - these are most likely under- and overflow bins
        continue;
      }

      int eta, phi, dummy;
      etaPhiMap->GetBinXYZ(binID - 1, eta, phi, dummy);

      float sumFCalET = sums->GetBinContent(binID, 1);
      float sumClusterET = sums->GetBinContent(binID, 2);
      float sumFCalET2 = sums->GetBinContent(binID, 3);
      // sumClusterET2 is not needed
      float sumFCalET_clusterET = sums->GetBinContent(binID, 5);

      float avgFCalET = sumFCalET / numEntries;
      float avgClusterET = sumClusterET / numEntries;
      float delta = sumFCalET2 - 2 * sumFCalET * avgFCalET + numEntries * avgFCalET * avgFCalET;
      if (delta == 0) {
        continue;
      }

      // response and offset; response is not normalized yet
      float resp = (sumFCalET_clusterET - sumClusterET * avgFCalET) / delta;
      float offs = avgClusterET - avgFCalET * resp;

      if (resp < 0) {
        resp = 0;
      }

      entries->SetBinContent(eta, phi, r + 1, numEntries);
      response->SetBinContent(eta, phi, r + 1, resp);
      offset->SetBinContent(eta, phi, r + 1, offs);
    }
  }

  if (!response || !offset) {
    std::cout << "Could not create response or offset" << std::endl;
    return -1;
  }

  entries->GetXaxis()->SetRangeUser(-1.99,1.99);
  std::unique_ptr<TH1F> avgEntries((TH1F *)entries->Project3D("z"));
  avgEntries->Scale(1.0 / ((entries->GetXaxis()->GetLast() - entries->GetXaxis()->GetFirst()+1) * entries->GetNbinsY()));
  entries->GetXaxis()->SetRange(0,0);

  // normalize response in phi
  for (int r = 1; r <= response->GetNbinsZ(); r++) {

    // use bins with at least 10% of average entries
    float limitEntries = 0.1 * avgEntries->GetBinContent(r);

    for (int eta = 1; eta <= response->GetNbinsX(); eta++) {
      int entr = 0;
      float sum = 0;

      for (int phi = 1; phi <= response->GetNbinsY(); phi++) {
        if (entries->GetBinContent(eta, phi, r) < limitEntries) {
          // skip this eta-phi bin
          continue;
        }
        sum += response->GetBinContent(eta, phi, r);
        ++entr;
      }

      if(entr > 0) {
        sum /= entr;
      }
      else {
        sum = 1.0;
      }

      for (int phi = 1; phi <= response->GetNbinsY(); phi++) {
        if (entries->GetBinContent(eta, phi, r) < limitEntries) {
          response->SetBinContent(eta, phi, r, 1.0);
          offset->SetBinContent(eta, phi, r, 0.0);
        } else {
          float resp = response->GetBinContent(eta, phi, r) / sum;
          if(std::isnan(resp) || resp<=0.) {
            resp = 1.0;
          }

          response->SetBinContent(eta, phi, r, resp);
        }
      }
    }
  }


  // save everything
  std::unique_ptr<TFile> outFile(TFile::Open("cluster.geo.RESPONSE_OFFSET_RUNINDEX.root", "RECREATE"));
  runIndex->Write();
  response->Write();
  offset->Write();

  std::cout << "Created output file cluster.geo.RESPONSE_OFFSET_RUNINDEX.root" << std::endl;
  return 0;
}
