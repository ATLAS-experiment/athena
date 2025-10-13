#include <iostream>
#include <memory>
#include <string>

#include "TFile.h"
#include "TH1F.h"
#include "TH3F.h"

void compareHIClusterGeoFiles(const std::string& file1,
                              const std::string& file2, int testOnly = 127,
                              int* result = 0) {
  std::unique_ptr<TFile> f1( TFile::Open(file1.c_str()) );
  std::unique_ptr<TFile> f2( TFile::Open(file2.c_str()) );

  if (!f1 || f1->IsZombie() || !f2 || f2->IsZombie()) {
    if (!f1 || f1->IsZombie()) {
      std::cout << "could not open file1 " << file1 << std::endl;
    }
    if (!f2 || f2->IsZombie()) {
      std::cout << "could not open file2 " << file2 << std::endl;
    }

    if (result) {
      *result = 1;
    }
    return;
  }

  const int hnum = 7;
  const char hnames[hnum][100] = { 
      "h3_w", "h3_eta", "h3_phi", "h3_R", 
      "h3_eta_phi_response", "h3_eta_phi_offset", 
      "h1_run_index"};  
      // testOnly: 1-h3_w; 2-h3_eta; 4-h3_phi; 8-h3_R;
      // 16-h3_eta_phi_response; 32-h3_eta_phi_offset,
      // 64-h1_run_index
  int hdiff[hnum] = {
      0, 0, 0, 0,
      0, 0, 0};  
      // 0-same; 1-not found; 2-different dimensions; 3-different
      // axes; 4-different bins; 5-different bin content
  char hresult[hnum][30] = {
      "same",           "not found",      "different dimensions",
      "different axes", "different bins", "different bin content"};

  for (int h = 0; h < hnum; h++) {
    if (!(testOnly & (1 << h))) {
      continue;
    }

    // TH1 is for both TH1F and TH3F
    TH1* h1 = (TH1*)f1->Get(hnames[h]);
    if (h1) {
      h1->SetDirectory(0);
      h1->SetName(Form("%s_h1", hnames[h]));
    }

    TH1* h2 = (TH1*)f2->Get(hnames[h]);
    if (h2) {
      h2->SetDirectory(0);
      h2->SetName(Form("%s_h2", hnames[h]));
    }

    if (!h1 || !h2) {
      if (!h1) {
        std::cout << "could not find histogram " << hnames[h] << " in file1 "
                  << file1 << std::endl;
      }
      if (!h2) {
        std::cout << "could not find histogram " << hnames[h] << " in file2 "
                  << file2 << std::endl;
      }

      // not found
      hdiff[h] = 1;
      continue;
    }

    int dim1 = h1->GetDimension();
    int dim2 = h2->GetDimension();
    if (dim1 != dim2) {
      std::cout << "h1 " << h1->GetName() << " has dimension " << dim1
                << ", h2 " << h2->GetName() << " has dimension " << dim2
                << std::endl;
      // different dimensions
      hdiff[h] = 2;
      continue;
    }

    if (h1->GetNbinsX() != h2->GetNbinsX() ||
        (dim1 > 1 && h1->GetNbinsY() != h2->GetNbinsY()) ||
        (dim1 > 2 && h1->GetNbinsZ() != h2->GetNbinsZ())) {
      if (h1->GetNbinsX() != h2->GetNbinsX()) {
        std::cout << "h1 " << h1->GetName() << " has " << h1->GetNbinsX()
                  << " bins in X, h2 " << h2->GetName() << " has "
                  << h2->GetNbinsX() << " bins in X" << std::endl;
      }
      if (h1->GetNbinsY() != h2->GetNbinsY()) {
        std::cout << "h1 " << h1->GetName() << " has " << h1->GetNbinsY()
                  << " bins in Y, h2 " << h2->GetName() << " has "
                  << h2->GetNbinsY() << " bins in Y" << std::endl;
      }
      if (h1->GetNbinsZ() != h2->GetNbinsZ()) {
        std::cout << "h1 " << h1->GetName() << " has " << h1->GetNbinsZ()
                  << " bins in Z, h2 " << h2->GetName() << " has "
                  << h2->GetNbinsZ() << " bins in Z" << std::endl;
      }

      // different axes
      hdiff[h] = 3;
      continue;
    }

    int doBreak = 0;
    int dim = h1->GetDimension();
    for (int d = 1; d <= dim; d++) {
      TAxis* a1 = nullptr;
      TAxis* a2 = nullptr;
      if (d == 1) {
        a1 = h1->GetXaxis();
        a2 = h2->GetXaxis();
      }
      if (d == 2) {
        a1 = h1->GetYaxis();
        a2 = h2->GetYaxis();
      }
      if (d == 3) {
        a1 = h1->GetZaxis();
        a2 = h2->GetZaxis();
      }

      for (int b = 1; b <= a1->GetNbins() + 1; ++b) {
        if (a1->GetBinLowEdge(b) != a2->GetBinLowEdge(b)) {
          // different bins
          hdiff[h] = 4;
          std::cout << "h1 " << h1->GetName() << " has bin " << b
                    << " with low edge " << a1->GetBinLowEdge(b) << ", h2 "
                    << h2->GetName() << " has bin " << b << " with low edge "
                    << a2->GetBinLowEdge(b) << std::endl;
          doBreak = 1;
          break;
        }
      }
      if (doBreak) {
        break;
      }
    }
    if (doBreak) {
      continue;
    }

    for (int bx = 1; bx <= h1->GetNbinsX(); ++bx) {
      for (int by = 1; by <= h1->GetNbinsY(); ++by) {
        for (int bz = 1; bz <= h1->GetNbinsZ(); ++bz) {
          if (h1->GetBinContent(bx, by, bz) != h2->GetBinContent(bx, by, bz)) {
            // different bin content
            hdiff[h] = 5;
            std::cout << "h1 " << h1->GetName() << " has "
                      << h1->GetBinContent(bx, by, bz) << " in bin " << bx
                      << "," << by << "," << bz << ", h2 " << h2->GetName()
                      << " has " << h2->GetBinContent(bx, by, bz) << std::endl;
            std::cout << "skipping the rest of bins" << std::endl;
            doBreak = 1;
          }
          if (doBreak) {
            break;
          }
        }
        if (doBreak) {
          break;
        }
      }
      if (doBreak) {
        break;
      }
    }
  }

  if (result) {
    *result = 0;
  }

  for (int h = 0; h < hnum; h++) {
    if (!(testOnly & (1 << h))) {
      continue;
    }
    std::cout << hnames[h] << ": " << hresult[hdiff[h]] << std::endl;
    if (result && hdiff[h] > *result) {
      *result = hdiff[h];
    }
  }

  return;
}

int main(int argc, char** argv) {
  if (argc < 3 || argc > 4) {
    std::cout << "Syntax: " << argv[0] << " file1 file2 [testOnly]"
              << std::endl;
    return -1;
  }

  int result = 0;
  int testOnly = (argc > 3 ? std::stoi(argv[3]) : 127);
  if (testOnly > 127) {
    std::cout << "Warning: testOnly should be an integer between 1 and 127, it "
                 "is now "
              << testOnly << ", changing to " << testOnly << "%128 = ";
    testOnly %= 128;
    std::cout << testOnly << std::endl;
  }
  if (testOnly <= 0) {
    std::cout << "Warning: testOnly should be an integer between 1 and 127, it "
                 "is now "
              << testOnly << ", nothing to do here" << std::endl;
    return -1;
  }

  compareHIClusterGeoFiles(argv[1], argv[2], testOnly, &result);
  return result;
}
