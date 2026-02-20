#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TList.h>
#include <TMath.h>
#include <iostream>
#include <vector>
#include <string>

void combine_all(const char* inputDir,
                     const char* outputFile,
                     const std::vector<std::string>& matrixNames,
                     const char* eventHistName = "bcid") {
    // Get all files in the directory
    TSystemDirectory dir(inputDir, inputDir);
    TList* filesList = dir.GetListOfFiles();
    if (!filesList) {
        std::cerr << "Error: cannot open directory " << inputDir << "\n";
        return;
    }

    std::vector<std::string> inputFiles;
    TIter next(filesList);
    while (TSystemFile* f = (TSystemFile*) next()) {
        TString fname = f->GetName();
    if (!f->IsDirectory() && fname.EndsWith(".root") && fname.BeginsWith("RatesHistograms_data24_13p6TeV.")) {
            inputFiles.push_back(std::string(inputDir) + "/" + fname.Data());
        }
    }

    if (inputFiles.size() < 2) {
        std::cerr << "Error: no valid files found in " << inputDir << "\n";
        return;
    }
    
    std::cout << "Input files to be combined:\n";
    for (const auto& f : inputFiles) {
    	std::cout << "  " << f << "\n";
    }
    std::cout << "Found " << inputFiles.size() << " files to combine.\n";

    // Open files and count events
    std::vector<TFile*> files;
    std::vector<double> nEvents;
    double totalEvents = 0.0;

    for (const auto& filename : inputFiles) {
	TFile* f = TFile::Open(("file:" + std::string(filename)).c_str());
        if (!f || f->IsZombie()) {
            std::cerr << "Error: cannot open " << filename << "\n";
            return;
        }
        files.push_back(f);

        TH1* h_event = dynamic_cast<TH1*>(f->Get(eventHistName));
        if (!h_event) {
            std::cerr << "Error: histogram \"" << eventHistName << "\" not found in " << filename << "\n";
            return;
        }

        double nev = h_event->GetEntries();
        if (nev <= 0) {
            std::cerr << "Error: zero events in " << filename << "\n";
            return;
        }

        nEvents.push_back(nev);
        totalEvents += nev;
    }

    // Print total number of events processed
    std::cout << "Total number of events processed (bcid sum): " << totalEvents << "\n";

    // Create output file
    TFile* fout = TFile::Open(outputFile, "RECREATE");

    // Process each matrix
    for (const auto& matrixName : matrixNames) {
        std::vector<TH2D*> matrices;

        // Retrieve the matrix from each file
        for (size_t i = 0; i < files.size(); ++i) {
            TH2D* h_matrix = dynamic_cast<TH2D*>(files[i]->Get(matrixName.c_str()));
	    if (!h_matrix) {
                std::cerr << "Error: matrix \"" << matrixName << "\" not found in " 
                          << files[i]->GetName() << "\n";
                return;
            }
            matrices.push_back(h_matrix);
        }

        // Create empty combined matrix
        TH2D* h_combined = (TH2D*)matrices[0]->Clone((matrixName + "_combined").c_str());
        h_combined->Reset();
        h_combined->SetDirectory(0);

        int nbx = h_combined->GetNbinsX();
        int nby = h_combined->GetNbinsY();

	bool isCounts = (matrixName == "counts_matrix");
	// Combine bin by bin
	for (int x = 1; x <= nbx; ++x) {
    		for (int y = 1; y <= nby; ++y) {
        		double sum = 0.0;
        		double sumErr2 = 0.0;
        		for (size_t i = 0; i < matrices.size(); ++i) {
            			double val = matrices[i]->GetBinContent(x, y);
            			double err = matrices[i]->GetBinError(x, y);
	
            			if (isCounts) {
                			sum += val;               // sum counts directly
                			sumErr2 += err * err;     // errors in quadrature
            			} else {
                			double w = nEvents[i] / totalEvents; // weighted average for rates
					sum += w * val;
                			sumErr2 += w * w * err * err;
            			}
        		}
        		h_combined->SetBinContent(x, y, sum);
        		h_combined->SetBinError(x, y, std::sqrt(sumErr2));
    		}
	}

        // Save combined matrix
        h_combined->Write();
        std::cout << "Combined matrix \"" << matrixName << "\" written.\n";
    }

    fout->Close();
    for (auto* f : files) f->Close();
    std::cout << "Output file written to: " << outputFile << "\n";
}

void combine_RCmatrices() {
    // Directory containing all .root files
    const char* inputDir = "./athena24/run_grid/user.$.l1topoRates.EB_fullRun_00482596_1Oct25_RatesHistograms_XYZ.root.tgz/";  
    // Output file
    const char* outputFile = "CombinedMatrices.root";

    // Matrices to combine
    std::vector<std::string> matrices = {
        "rates_matrix",
        "counts_matrix",
    };

    combine_all(inputDir, outputFile, matrices);
	
    // ---- Calculate TopoScore matrix ----
    TFile* fout = TFile::Open(outputFile, "UPDATE");
    if (!fout || fout->IsZombie()) {
        std::cerr << "Error: cannot open combined file to add L1TopoScore_matrix.\n";
        return;
    }

    TH2D* h_rates = (TH2D*) fout->Get("rates_matrix_combined");
    if (!h_rates) {
        std::cerr << "Error: rates_matrix_combined not found!\n";
        fout->Close();
        return;
    }

    int nbx = h_rates->GetNbinsX();
    int nby = h_rates->GetNbinsY();

    TH2D* h_topo = (TH2D*)h_rates->Clone("L1TopoScore_matrix_combined");
    h_topo->Reset();

    // Loop over bins and compute TS and its error by propagation
    for (int ix = 1; ix <= nbx; ++ix) {
        for (int jy = 1; jy <= nby; ++jy) {
            // Diagonal treatment: TS(i,i) = 1 (A==B and O==A typically), set error 0
            if (ix == jy) {
                h_topo->SetBinContent(ix, jy, 1.0);
                h_topo->SetBinError(ix, jy, 0.0);
                continue;
            }

            double A = h_rates->GetBinContent(ix, ix);      // diagonal A
            double sigmaA = h_rates->GetBinError(ix, ix);
            double B = h_rates->GetBinContent(jy, jy);      // diagonal B
            double sigmaB = h_rates->GetBinError(jy, jy);
            double O = h_rates->GetBinContent(ix, jy);      // overlap
            double sigmaO = h_rates->GetBinError(ix, jy);

            // Safety checks
            if (O <= 0.0) {
                h_topo->SetBinContent(ix, jy, 0.0);
                h_topo->SetBinError(ix, jy, 0.0);
                continue;
            }
            double denom = O * (A + B - O);
            if (denom <= 0.0) {
                h_topo->SetBinContent(ix, jy, 0.0);
                h_topo->SetBinError(ix, jy, 0.0);
                continue;
            }

            double TS = (A * B) / denom;

            // Derivatives
            // D = denom
            double D = denom;
            double dD_dA = O;
            double dD_dB = O;
            double dD_dO = (A + B - 2.0 * O); // derivative of O*(A+B-O) w.r.t O

            // Using TS = N / D, with N = A*B
            double N = A * B;

            // dTS/dA = (dN/dA * D - N * dD/dA) / D^2 = (B*D - N*O)/D^2
            double dTS_dA = (B * D - N * dD_dA) / (D * D);
            // dTS/dB = (A*D - N*O)/D^2
            double dTS_dB = (A * D - N * dD_dB) / (D * D);
            // dTS/dO = - N * dD/dO / D^2
            double dTS_dO = - N * dD_dO / (D * D);

            // Propagate errors (assume A,B,O independent)
            double varTS = 0.0;
            if (sigmaA > 0.0) varTS += (dTS_dA * dTS_dA) * (sigmaA * sigmaA);
            if (sigmaB > 0.0) varTS += (dTS_dB * dTS_dB) * (sigmaB * sigmaB);
            if (sigmaO > 0.0) varTS += (dTS_dO * dTS_dO) * (sigmaO * sigmaO);

            double sigmaTS = (varTS > 0.0) ? std::sqrt(varTS) : 0.0;

            h_topo->SetBinContent(ix, jy, TS);
            h_topo->SetBinError(ix, jy, sigmaTS);
        }
    }

    fout->cd();
    h_topo->Write();
    fout->Close();
    std::cout << "L1TopoScore_matrix_combined written successfully.\n";
}

