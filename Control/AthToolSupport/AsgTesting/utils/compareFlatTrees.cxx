/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <ROOT/RDataFrame.hxx>
#include <ROOT/RLogger.hxx>
#include <TCanvas.h>
#include <TF1.h>
#include <TH1D.h>
#include <TRatioPlot.h>
#include <TROOT.h>
#include <TStyle.h>

#include <CxxUtils/checker_macros.h>

#include <boost/program_options.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>

// Intersection of two lists of vectors, needed to get the variables that are common to both samples
std::vector<std::string> intersection(std::vector<std::string> &v1,
                                      std::vector<std::string> &v2)
{
  std::vector<std::string> v3;

  std::sort(v1.begin(), v1.end());
  std::sort(v2.begin(), v2.end());

  std::set_intersection(v1.begin(), v1.end(),
                        v2.begin(), v2.end(),
                        back_inserter(v3));
  // Alphabetical order while we're at it
  std::sort(v3.begin(), v3.end(), [](const std::string &a, const std::string &b) -> bool
            { return a < b; });

  return v3;
}

// List of entries in a vector that are not in another
std::vector<std::string> remainder (const std::vector<std::string>& v1,
                                    const std::vector<std::string>& v2)
{
  std::vector<std::string> result;

  std::unordered_set<std::string> ignore {v2.begin(), v2.end()};
  for (const auto& value : v1)
  {
    if (ignore.find (value) == ignore.end())
      result.push_back (value);
  }
  return result;
}

int main ATLAS_NOT_THREAD_SAFE(int argc, char *argv[])
{
  namespace po = boost::program_options;
  po::options_description poDescription("Common options");
  poDescription.add_options()
    ("help", "produce help message")
    ("require-same-branches", "require both trees to have the same branches")
    ("tree-name", po::value<std::string>()->required(), "tree name")
    ("reference-file", po::value<std::string>()->required(), "reference file(s), wildcards supported")
    ("test-file", po::value<std::string>()->required(), "test file(s), wildcards supported")
    ("branch-name", po::value<std::string>(), "base branch name (optional)");

  po::options_description poDescriptionAdvanced("Advanced options");
  poDescriptionAdvanced.add_options()
    ("scale", "scale histograms that both have the same event count")
    ("rebin",  "do smart rebinning")
    ("benchmark", "benchmark the code")
    ("verbose", "verbose logging");

  po::options_description poDescriptionAll;
  poDescriptionAll.add(poDescription).add(poDescriptionAdvanced);

  po::positional_options_description poPositionalOptions;
  poPositionalOptions.add("tree-name", 1);
  poPositionalOptions.add("reference-file", 1);
  poPositionalOptions.add("test-file", 1);
  poPositionalOptions.add("branch-name", 1);

  po::variables_map poVariablesMap;
  po::store(po::command_line_parser(argc, argv)
    .options(poDescriptionAll)
    .positional(poPositionalOptions).run(),
    poVariablesMap);

  if (poVariablesMap.count("help"))
  {
    std::cout << "Usage: compareFlatTrees [OPTION] tree-name reference-file test-file [branch-name]" << std::endl;
    std::cout << poDescriptionAll << std::endl;
    return 0;
  }

  po::notify(poVariablesMap);

  // Base name of branches to read
  std::string treeName = poVariablesMap["tree-name"].as<std::string>();
  std::string treeNameOut = treeName;
  std::replace( treeNameOut.begin(), treeNameOut.end(), '/', '_');
  std::string referenceInput = poVariablesMap["reference-file"].as<std::string>();
  std::string testInput = poVariablesMap["test-file"].as<std::string>();
  std::string baseBranchName;
  std::string outputPDF;
  if (poVariablesMap.count("branch-name") > 0)
  {
    baseBranchName = poVariablesMap["branch-name"].as<std::string>();
    outputPDF = "comparison_" + treeNameOut + "_" + baseBranchName + ".pdf";
  }
  else
  {
    outputPDF = "comparison_" + treeNameOut + ".pdf";
  }

  bool scale = poVariablesMap.count("scale") > 0;
  bool rebin = poVariablesMap.count("rebin") > 0;
  bool benchmark = poVariablesMap.count("benchmark") > 0;
  bool verbose = poVariablesMap.count("verbose") > 0;

  // Verbose logging
  // auto verbosity = ROOT::Experimental::RLogScopedVerbosity(ROOT::Detail::RDF::RDFLogChannel(), ROOT::Experimental::ELogLevel::kInfo);

  // Run in batch mode - output is pdf
  gROOT->SetBatch(kTRUE);
  // Suppress logging
  if (!verbose)
  {
    gErrorIgnoreLevel = kWarning;
  }
  // No stats box
  gStyle->SetOptStat(0);
  // No error bar ticks
  gStyle->SetEndErrorSize(0);
  // Parallel processing where possible
  ROOT::EnableImplicitMT(4);

  // Create RDataFrame
  ROOT::RDataFrame dataFrameRefr(treeName, referenceInput);
  ROOT::RDataFrame dataFrameTest(treeName, testInput);

  // Event count and ratio
  auto eventsRefr{dataFrameRefr.Count()};
  auto eventsTest{dataFrameTest.Count()};
  double eventRatio{static_cast<float>(eventsRefr.GetValue()) / static_cast<float>(eventsTest.GetValue())};

  // Get column names for each file and then the intersection
  auto colNamesRefr = dataFrameRefr.GetColumnNames();
  auto colNamesTest = dataFrameTest.GetColumnNames();
  auto colNames = intersection(colNamesRefr, colNamesTest);

  bool checkMissColumns = false;
  std::vector<std::string> missColNamesRefr, missColNamesTest;
  if (poVariablesMap.count("require-same-branches"))
  {
    checkMissColumns = true;
    missColNamesRefr = remainder (colNamesRefr, colNames);
    missColNamesTest = remainder (colNamesTest, colNames);
  }

  // Loop over column names and get a list of the required columns
  std::vector<std::string> requiredColumns;
  std::cout << "Will attempt to plot the following columns:" << std::endl;
  for (auto &&colName : colNames)
  {
    if ((baseBranchName.empty() || colName.find(baseBranchName) != std::string::npos) &&  // include
        (colName.find("Trig") == std::string::npos) &&                                    // exclude, not meaningful
        (colName.find("Link") == std::string::npos) &&                                    // exclude, elementlinks
        (colName.find("m_persIndex") == std::string::npos) &&                             // exclude, elementlinks
        (colName.find("m_persKey") == std::string::npos) &&                               // exclude, elementlinks
        (colName.find("Parent") == std::string::npos) &&                                  // exclude, elementlinks
        (colName.find("original") == std::string::npos) &&                                // exclude, elementlinks
        (colName.find("EventInfoAuxDyn.detDescrTags") == std::string::npos) &&            // exclude, std::pair
        (dataFrameRefr.GetColumnType(colName).find("xAOD") == std::string::npos) &&       // exclude, needs ATLAS s/w
        (dataFrameRefr.GetColumnType(colName) != "ROOT::VecOps::RVec<string>") &&         // exclude, needs ATLAS s/w
        (!dataFrameRefr.GetColumnType(colName).starts_with("ROOT::VecOps::RVec<pair")) && // exclude, std::pair
        (dataFrameRefr.GetColumnType(colName).find("vector") == std::string::npos))
    { // exclude, needs unwrapping
      requiredColumns.push_back(colName);
      std::cout << "   " << colName << " " << dataFrameRefr.GetColumnType(colName) << std::endl;
    }
  }

  // Set binning
  int nBins{128};
  size_t nBinsU = static_cast<size_t>(nBins);

  // Loop over the required columns and plot them for each sample along with the ratio
  // Write resulting plots to a pdf file
  bool fileOpen{};
  size_t counter{};
  size_t failedCount{};
  std::chrono::seconds totalDuration{};
  std::unordered_map<std::string, ROOT::RDF::RResultPtr<double>> mapMinValuesRefr;
  std::unordered_map<std::string, ROOT::RDF::RResultPtr<double>> mapMinValuesTest;
  std::unordered_map<std::string, ROOT::RDF::RResultPtr<double>> mapMaxValuesRefr;
  std::unordered_map<std::string, ROOT::RDF::RResultPtr<double>> mapMaxValuesTest;
  std::unordered_map<std::string, ROOT::RDF::RResultPtr<TH1D>> mapHistRefr;
  std::unordered_map<std::string, ROOT::RDF::RResultPtr<TH1D>> mapHistTest;

  std::cout << "Preparing ranges..." << std::endl;
  for (const std::string &colName : requiredColumns)
  {
    mapMinValuesRefr.emplace(colName, dataFrameRefr.Min(colName));
    mapMinValuesTest.emplace(colName, dataFrameTest.Min(colName));
    mapMaxValuesRefr.emplace(colName, dataFrameRefr.Max(colName));
    mapMaxValuesTest.emplace(colName, dataFrameTest.Max(colName));
  }

  std::cout << "Preparing histograms..." << std::endl;
  auto start = std::chrono::high_resolution_clock::now();
  for (auto it = requiredColumns.begin(); it != requiredColumns.end();)
  {
    const std::string &colName = *it;
    const char* colNameCh = colName.c_str();

    // Initial histogram range
    double min = std::min(mapMinValuesRefr[colName].GetValue(), mapMinValuesTest[colName].GetValue());
    double max = std::max(mapMaxValuesRefr[colName].GetValue(), mapMaxValuesTest[colName].GetValue()) * 1.02;
    if (std::isinf(min) || std::isinf(max))
    {
      std::cout << "  skipping " << colName << " ..." << std::endl;
      it = requiredColumns.erase(it);
      continue;
    } else {
      ++it;
    }

    if (max > 250e3 && min > 0.0)
    {
      min = 0.0;
    }

    // Initial histograms
    mapHistRefr.emplace(colName, dataFrameRefr.Histo1D({colNameCh, colNameCh, nBins, min, max}, colNameCh));
    mapHistTest.emplace(colName, dataFrameTest.Histo1D({colNameCh, colNameCh, nBins, min, max}, colNameCh));
  }
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
  totalDuration += duration;
  if (benchmark)
  {
    std::cout << "   Time for this step: " << duration.count() << " seconds " << std::endl;
    std::cout << "   Elapsed time: " << totalDuration.count() << " seconds (" << std::chrono::duration_cast<std::chrono::minutes>(totalDuration).count() << " minutes)" << std::endl;
  }

  if (rebin)
  {
    std::cout << "Rebinning histograms..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (const std::string &colName : requiredColumns)
    {
      const char* colNameCh = colName.c_str();
      auto &histRefr = mapHistRefr[colName];
      auto &histTest = mapHistTest[colName];

      // Initial histogram range
      double min = std::min(mapMinValuesRefr[colName].GetValue(), mapMinValuesTest[colName].GetValue());
      double max = std::max(mapMaxValuesRefr[colName].GetValue(), mapMaxValuesTest[colName].GetValue()) * 1.02;
      if (max > 250e3 && min > 0.0)
      {
        min = 0.0;
      }

      // Check range - make sure that bins other than the first contain at least one per mille events
      // Avoids case where max is determined by a single outlier leading to most events being in the 1st bin
      bool rangeSatisfactory{};
      size_t rangeItrCntr{};
      while (!rangeSatisfactory && rangeItrCntr < 10)
      {
        ++rangeItrCntr;
        if (verbose)
        {
          std::cout << std::endl
                    << "   Range tuning... iteration number " << rangeItrCntr << std::endl;
        }
        double entriesFirstBin = histRefr.GetPtr()->GetBinContent(1);
        double entriesLastBin = histRefr.GetPtr()->GetBinContent(nBins);
        double entriesOtherBins{};
        for (size_t i{2}; i < nBinsU; ++i)
        {
          entriesOtherBins += histRefr.GetPtr()->GetBinContent(i);
        }
        bool firstBinOK{((entriesOtherBins + entriesLastBin) / entriesFirstBin > 0.001f)};
        bool lastBinOK{((entriesOtherBins + entriesFirstBin) / entriesLastBin > 0.001f)};
        rangeSatisfactory = ((firstBinOK && lastBinOK) || entriesOtherBins == 0.0f);
        if (!rangeSatisfactory)
        {
          if (verbose)
          {
            std::cout << "Min " << min << std::endl;
            std::cout << "Max " << max << std::endl;
            std::cout << "1st " << entriesFirstBin << std::endl;
            std::cout << "Mid " << entriesOtherBins << std::endl;
            std::cout << "End " << entriesLastBin << std::endl;
            std::cout << "R/F " << (entriesOtherBins + entriesLastBin) / entriesFirstBin << std::endl;
            std::cout << "R/L " << (entriesOtherBins + entriesFirstBin) / entriesLastBin << std::endl;
          }
          if (!firstBinOK)
          {
            max = (max - min) / static_cast<double>(nBins);
            histRefr = dataFrameRefr.Histo1D({colNameCh, colNameCh, nBins, min, max}, colNameCh);
            histTest = dataFrameTest.Histo1D({colNameCh, colNameCh, nBins, min, max}, colNameCh);
          }
          if (!lastBinOK)
          {
            min = max * (1.0f - (1.0f / static_cast<double>(nBins)));
            histRefr = dataFrameRefr.Histo1D({colNameCh, colNameCh, nBins, min, max}, colNameCh);
            histTest = dataFrameTest.Histo1D({colNameCh, colNameCh, nBins, min, max}, colNameCh);
          }
        }
      }
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
    totalDuration += duration;
    if (benchmark)
    {
      std::cout << "   Time for this step: " << duration.count() << " seconds " << std::endl;
      std::cout << "   Elapsed time: " << totalDuration.count() << " seconds (" << std::chrono::duration_cast<std::chrono::minutes>(totalDuration).count() << " minutes)" << std::endl;
    }
  }

  std::cout << "Running comparisons..." << std::endl;
  start = std::chrono::high_resolution_clock::now();

  // Store only last canvas
  std::unique_ptr<TCanvas> lastCanvas;

  for (const std::string &colName : requiredColumns)
  {
    ++counter;

    std::cout << "Processing column " << counter << " of " << requiredColumns.size() << " : " << colName << " ... ";

    auto h1 = mapHistRefr[colName].GetPtr();
    auto h2 = mapHistTest[colName].GetPtr();

    if (scale)
    {
      h2->Scale(eventRatio);
    }
    h2->SetMarkerStyle(20);
    h2->SetMarkerSize(0.8);

    if (!verbose)
    {
      gErrorIgnoreLevel = kError; // this is spammy due to empty bins
    }
    auto c1 = std::make_unique<TCanvas>();
    auto rp = std::make_unique<TRatioPlot>(h2, h1);
    if (!verbose)
    {
      gErrorIgnoreLevel = kWarning;
    }

    rp->SetH1DrawOpt("PE");
    rp->SetH2DrawOpt("hist");
    rp->SetGraphDrawOpt("PE");
    rp->Draw();
    rp->GetUpperRefXaxis()->SetTitle(colName.c_str());
    rp->GetUpperRefYaxis()->SetTitle("Count");
    rp->GetLowerRefYaxis()->SetTitle("Test / Ref.");
    rp->GetLowerRefGraph()->SetMarkerStyle(20);
    rp->GetLowerRefGraph()->SetMarkerSize(0.8);

    bool valid{true};
    for (int i{}; i < rp->GetLowerRefGraph()->GetN(); i++)
    {
      if (rp->GetLowerRefGraph()->GetY()[i] != 1.0)
      {
        valid = false;
        break;
      }
    }
    if (valid)
    {
      std::cout << "PASS" << std::endl;
      continue;
    }
    else
    {
      std::cout << "FAILED" << std::endl;
      ++failedCount;
    }

    rp->GetLowerRefGraph()->SetMinimum(0.5);
    rp->GetLowerRefGraph()->SetMaximum(1.5);
    rp->GetLowYaxis()->SetNdivisions(505);

    if (valid) {
      c1->SetTicks(0, 1);
      c1->Update();
    }

    if (!fileOpen)
    {
      // Open file
      c1->Print((outputPDF + "[").c_str());
      fileOpen = true;
    }
    // Actual plot
    c1->Print(outputPDF.c_str());
    c1->Clear();
    lastCanvas = std::move(c1);
  }

  if (fileOpen)
  {
    // Close file
    lastCanvas->Print((outputPDF + "]").c_str());
    lastCanvas.reset();
  }

  stop = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
  totalDuration += duration;
  if (benchmark)
  {
    std::cout << "   Time for this step: " << duration.count() << " seconds " << std::endl;
    std::cout << "   Elapsed time: " << totalDuration.count() << " seconds (" << std::chrono::duration_cast<std::chrono::minutes>(totalDuration).count() << " minutes)" << std::endl;
  }

  std::cout << "========================" << std::endl;
  std::cout << "Reference events: " << eventsRefr.GetValue() << std::endl;
  std::cout << "Test events: " << eventsTest.GetValue() << std::endl;
  std::cout << "Ratio: " << eventRatio << std::endl;
  std::cout << "========================" << std::endl;
  std::cout << "Tested columns: " << requiredColumns.size() << std::endl;
  std::cout << "Passed: " << requiredColumns.size() - failedCount << std::endl;
  std::cout << "Failed: " << failedCount << std::endl;
  std::cout << "========================" << std::endl;
  if (checkMissColumns)
  {
    std::cout << "Columns only in reference: " << missColNamesRefr.size();
    for (const auto& column : missColNamesRefr)
      std::cout << " " << column;
    std::cout << std::endl;
    std::cout << "Columns only in test: " << missColNamesTest.size();
    for (const auto& column : missColNamesTest)
      std::cout << " " << column;
    std::cout << std::endl;
    failedCount += missColNamesRefr.size() + missColNamesTest.size();
    std::cout << "========================" << std::endl;
  }

  if (failedCount)
  {
    return 1;
  }

  return 0;
}
