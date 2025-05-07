/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>

#include <TNtuple.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TLatex.h>
#include <TError.h>
#include <TPad.h>


void print(char *figname, TCanvas *c1)
{

    char filename[1000];
    sprintf(filename, "output/%s", figname);

    c1->Print(Form("%s.png", filename));
    c1->Print(Form("%s.pdf", filename));
    c1->Print(Form("%s.eps", filename));
    c1->SaveAs("allPlots.pdf","pdf");
    c1->Write(Form("%s", filename), TObject::kWriteDelete);
    printf("Info in <TCanvas::Print>: %s has been saved and added to allPlots.pdf file\n", filename);
}

void TRTCalib_StrawStatusReport(){

    // read the content from a default txt file name into an ntuple
    int run = 0;
    TNtuple *ntuple = new TNtuple("ntuple", "data", "side:phi:straw:status:hits:occ:htocc:eff:lay");
    gErrorIgnoreLevel = kError;

    int var[15];
    double par[5];
    int count = 0;

    std::ifstream in;
    in.open("TRT_StrawStatusReport.txt");

    while (1)
    {

        for (int i = 0; i < 5; i++){
            in >> var[i];
        }
            
        for (int i = 0; i < 3; i++){
            in >> par[i];
        }  
        in >> var[5];
        if (!in.good()){
            break;
        }  
        count++;

        if (var[0] == 0)
        {
            run = var[4];
            continue;
        }
        ntuple->Fill(var[0], var[1], var[2], var[3], var[4], par[0], par[1], par[2] * 100, var[5]);
    }

    printf("read %d lines from file\n", count);
    in.close();

    // make the efficiency distribution plot
    TFile* myfile = new TFile("test.HIST.root","RECREATE");

    TCanvas *c1 = new TCanvas("c1", "This is the canvas test", 1200,800);
    c1->SetLogy(1);
    c1->SetTicks(1, 1);

    // set stat box size
    gStyle->SetOptStat(10); 
	gStyle->SetStatY(0.98);
	gStyle->SetStatX(0.9);
	gStyle->SetStatW(0.15);
	gStyle->SetStatH(0.1);    

    c1->SaveAs("allPlots.pdf[");

    std::unique_ptr<TH1F> h1 = std::make_unique<TH1F>("h1", "", 101, 0., 101);
    ntuple->Project("h1", "eff");
    h1->GetXaxis()->SetTitle("Straw hit efficiency [%]");
    h1->GetXaxis()->SetTitleSize(0.04);
    h1->GetYaxis()->SetTitle("Number of straws");
    h1->GetYaxis()->SetTitleSize(0.04);
    h1->Draw();

    std::unique_ptr<TH1F> h1a = std::make_unique<TH1F>("h1a", "", 101, 0., 101);
    ntuple->Project("h1a", "eff", "status>0");
    h1a->SetFillStyle(1001);
    h1a->SetFillColor(kOrange);
    h1a->Draw("same");

    std::unique_ptr<TH1F> h1b = std::make_unique<TH1F>("h1b", "", 101, 0., 101);
    ntuple->Project("h1b", "eff", "status==42");
    h1b->SetFillStyle(1001);
    h1b->SetFillColor(2);
    h1b->Draw("same");

    gPad->RedrawAxis();


    TLatex mytext;
    mytext.SetTextAlign(21);
    mytext.SetNDC();
    mytext.SetTextSize(0.04);
    mytext.SetTextFont(42);
    mytext.DrawLatex(0.10, 0.02, Form("Run %d", run));

    print(Form("strawHitEfficiency_%d", run), c1);

    h1.reset();
    h1a.reset();
    h1b.reset();

    // make the HT occupancy distribution plot
    double htrange = 0.1;
    std::unique_ptr<TH1F> hh1 = std::make_unique<TH1F>("hh1", "", 100, 0., htrange);
    ntuple->Project("hh1", "htocc");

    hh1->GetXaxis()->SetTitle("Straw HT occupancy");
    hh1->GetXaxis()->SetTitleSize(0.04);

    hh1->GetYaxis()->SetTitle("Number of straws");
    hh1->GetYaxis()->SetTitleSize(0.04);

    hh1->SetBinContent(100, hh1->GetBinContent(100) + hh1->GetBinContent(101));
    hh1->Draw();

    std::unique_ptr<TH1F> hh1a = std::make_unique<TH1F>("hh1a", "", 100, 0., htrange);
    ntuple->Project("hh1a", "htocc", "status>0");
    hh1a->SetFillStyle(1001);
    hh1a->SetFillColor(kOrange);
    hh1a->SetBinContent(100, hh1a->GetBinContent(100) + hh1a->GetBinContent(101));
    hh1a->Draw("same");

    std::unique_ptr<TH1F> hh1b = std::make_unique<TH1F>("hh1b", "", 100, 0., htrange);
    ntuple->Project("hh1b", "htocc", "status==52");
    hh1b->SetFillStyle(1001);
    hh1b->SetFillColor(2);
    hh1b->SetBinContent(100, hh1b->GetBinContent(100) + hh1b->GetBinContent(101));
    hh1b->Draw("same");

    gPad->RedrawAxis();

    mytext.DrawLatex(0.10, 0.02, Form("Run %d", run));

    print(Form("strawHToccupancy_%d", run), c1);

    hh1.reset();
    hh1a.reset();
    hh1b.reset();

    gStyle->SetPalette(1);
    c1->SetRightMargin(0.2);
    c1->SetLogy(0);

    std::unique_ptr<TH1F> h7 = std::make_unique<TH1F>("h7", "", 100, 0, 10000);
    ntuple->Draw("hits>>h7", "straw>=147&&straw<200&&eff>0.7");
    double range = h7->GetMean();
    h7.reset();

    std::unique_ptr<TH2F> h2 = std::make_unique<TH2F>("h2", "", 110, -0.5, 5481.5, 100, 0., range * 1.2);
    ntuple->Project("h2", "hits:straw", "status==0");

    h2->GetXaxis()->SetTitle("Straw index");
    h2->GetXaxis()->SetTitleSize(0.04);

    h2->GetYaxis()->SetTitleOffset(1.2);
    h2->GetYaxis()->SetTitle("Number of hits on track");
    h2->GetYaxis()->SetTitleSize(0.04);

    h2->GetZaxis()->SetTitleOffset(1.2);
    h2->GetZaxis()->SetTitle("Number of straws");
    h2->GetZaxis()->SetTitleSize(0.04);

    h2->Draw("colz");
    ntuple->SetMarkerStyle(20);
    ntuple->SetMarkerSize(0.8);
    ntuple->SetMarkerColor(1);
    ntuple->Draw("hits:straw", "status==52||status==51", "same");

    TLatex l;
    l.SetTextSize(0.03);
    l.DrawLatex(750, range * 1.1, "black points: straws excluded due to low / high HT occupancy");
    mytext.DrawLatex(0.10, 0.02, Form("Run %d", run));

    print(Form("numberOfHitsOnTrack_%d", run), c1);
    h2.reset();


    
    
    std::unique_ptr<TH2F> hh22 = std::make_unique<TH2F>("hh22", "", 5, -2.5, 2.5, 32, -0.5, 31.5);
    ntuple->Project("hh22", "phi:side", "status>1");

    hh22->GetXaxis()->SetTitle("Detector side (athena numbering) Z");
    hh22->GetXaxis()->SetTitleSize(0.04);

    hh22->GetYaxis()->SetTitleOffset(0.8);
    hh22->GetYaxis()->SetTitle("Detector #phi (athena 0-31 range)");
    hh22->GetYaxis()->SetTitleSize(0.04);

    hh22->GetZaxis()->SetTitle("Number of additional excluded straws");
    hh22->GetZaxis()->SetTitleSize(0.04);

    hh22->Draw("colz");

    mytext.DrawLatex(0.10, 0.02, Form("Run %d", run));
    mytext.DrawLatex(0.23, 0.91, "Endcap  ||  Barrel");
    mytext.DrawLatex(0.45, 0.91, "#leftarrow C || A #rightarrow");
    mytext.DrawLatex(0.67, 0.91, "Barrel  ||  Endcap");

    print(Form("additionalExcludedStraws_%d", run), c1);
    hh22.reset();

    std::unique_ptr<TH2F> h3 = std::make_unique<TH2F>("h3", "", 7, -3.5, 3.5, 32, -0.5, 31.5);
    ntuple->Project("h3", "phi:side*(lay+1)/abs(side)", "status>1 && abs(side)==1");
    h3->GetXaxis()->SetTitle("Barrel Module (athena numbering) Z");
    h3->GetXaxis()->SetTitleSize(0.04);

    h3->GetYaxis()->SetTitleOffset(0.8);
    h3->GetYaxis()->SetTitle("Detector #phi (athena 0-31 range)");
    h3->GetYaxis()->SetTitleSize(0.04);

    h3->GetZaxis()->SetTitle("Number of additional excluded straws");
    h3->GetZaxis()->SetTitleSize(0.04);

    h3->Draw("colz text");

    mytext.DrawLatex(0.10, 0.02, Form("Run %d", run));
    mytext.DrawLatex(0.45, 0.91, "#leftarrow C || A #rightarrow");

    print(Form("additionalExcludedStrawsBarrel_%d", run), c1);
    h3.reset();

    std::unique_ptr<TH2F> h4 = std::make_unique<TH2F>("h4", "", 29, -14.5, 14.5, 32, -0.5, 31.5);
    ntuple->Project("h4", "phi:side*(lay+1)/abs(side)", "status>1 && abs(side)==2");

    h4->GetXaxis()->SetTitle("Endcap Wheel (athena numbering) Z");
    h4->GetXaxis()->SetTitleSize(0.04);

    h4->GetYaxis()->SetTitleOffset(0.8);
    h4->GetYaxis()->SetTitle("Detector #phi (athena 0-31 range)");
    h4->GetYaxis()->SetTitleSize(0.04);

    h4->GetZaxis()->SetTitle("Number of additional excluded straws");
    h4->GetZaxis()->SetTitleSize(0.04);

    h4->Draw("colz text");

    mytext.DrawLatex(0.10, 0.02, Form("Run %d", run));
    mytext.DrawLatex(0.45, 0.91, "#leftarrow C || A #rightarrow");

    print(Form("additionalExcludedStrawsEndcap_%d", run), c1);
    h4.reset();

    std::unique_ptr<TH2F> h5 = std::make_unique<TH2F>("h5", "", 7, -3.5, 3.5, 32, -0.5, 31.5);
    ntuple->Project("h5", "phi:side*(lay+1)/abs(side)", "status == 12 && abs(side)==1");
    if (h5->GetEntries() != 0)
    {

        h5->GetXaxis()->SetTitle("Barrel Module (athena numbering) Z");
        h5->GetXaxis()->SetTitleSize(0.04);

        h5->GetYaxis()->SetTitleOffset(0.8);
        h5->GetYaxis()->SetTitle("Detector #phi (athena 0-31 range)");
        h5->GetYaxis()->SetTitleSize(0.04);

        h5->GetZaxis()->SetTitle("Number of additional DEAD excluded straws");
        h5->GetZaxis()->SetTitleSize(0.04);

        h5->Draw("colz text");

        mytext.DrawLatex(0.10, 0.02, Form("Run %d", run));
        mytext.DrawLatex(0.45, 0.91, "#leftarrow C || A #rightarrow");

        print(Form("additionalExcludedDEADStrawsBarrel_%d", run), c1);
    }
    h5.reset();

    std::unique_ptr<TH2F> h6 = std::make_unique<TH2F>("h6", "", 29, -14.5, 14.5, 32, -0.5, 31.5);
    ntuple->Project("h6", "phi:side*(lay+1)/abs(side)", "status == 12 && abs(side)==2");
    if (h6->GetEntries() != 0)
    {

        h6->GetXaxis()->SetTitle("Endcap Module (athena numbering) Z");
        h6->GetXaxis()->SetTitleSize(0.04);

        h6->GetYaxis()->SetTitleOffset(0.8);
        h6->GetYaxis()->SetTitle("Detector #phi (athena 0-31 range)");
        h6->GetYaxis()->SetTitleSize(0.04);

        h6->GetZaxis()->SetTitle("Number of additional DEAD excluded straws");
        h6->GetZaxis()->SetTitleSize(0.04);

        h6->Draw("colz text");
        
        mytext.DrawLatex(0.10, 0.02, Form("Run %d", run));
        mytext.DrawLatex(0.45, 0.91, "#leftarrow C || A #rightarrow");

        print(Form("additionalExcludedDEADStrawsEndcap_%d", run), c1);
    }
    h6.reset();

    std::unique_ptr<TH2F> h55 = std::make_unique<TH2F>("h55", "", 7, -3.5, 3.5, 32, -0.5, 31.5);
    ntuple->Project("h55", "phi:side*(lay+1)/abs(side)", "status == 11 && abs(side)==1");
    if (h55->GetEntries() != 0)
    {

        h55->GetXaxis()->SetTitle("Barrel Module (athena numbering) Z");
        h55->GetXaxis()->SetTitleSize(0.04);

        h55->GetYaxis()->SetTitleOffset(0.8);
        h55->GetYaxis()->SetTitle("Detector #phi (athena 0-31 range)");
        h55->GetYaxis()->SetTitleSize(0.04);

        h55->GetZaxis()->SetTitle("Number of additional NOISY excluded straws");
        h55->GetZaxis()->SetTitleSize(0.04);

        h55->Draw("colz text");

        mytext.DrawLatex(0.10, 0.02, Form("Run %d", run));
        mytext.DrawLatex(0.45, 0.91, "#leftarrow C || A #rightarrow");

        print(Form("additionalExcludedNOISYStrawsBarrel_%d", run), c1);
    }
    h55.reset();


    std::unique_ptr<TH2F> h66 = std::make_unique<TH2F>("h66", "", 29, -14.5, 14.5, 32, -0.5, 31.5);
    ntuple->Project("h66", "phi:side*(lay+1)/abs(side)", "status == 11 && abs(side)==2");
    if (h66->GetEntries() != 0)
    {

        h66->GetXaxis()->SetTitle("Endcap Module (athena numbering) Z");
        h66->GetXaxis()->SetTitleSize(0.04);

        h66->GetYaxis()->SetTitleOffset(0.8);
        h66->GetYaxis()->SetTitle("Detector #phi (athena 0-31 range)");
        h66->GetYaxis()->SetTitleSize(0.04);

        h66->GetZaxis()->SetTitle("Number of additional NOISY excluded straws");
        h66->GetZaxis()->SetTitleSize(0.04);

        h66->Draw("colz text");

        mytext.DrawLatex(0.10, 0.02, Form("Run %d", run));
        mytext.DrawLatex(0.45, 0.91, "#leftarrow C || A #rightarrow");

        print(Form("additionalExcludedNOISYStrawsEndcap_%d", run), c1);
    }
    h66.reset();

    std::unique_ptr<TH2F> h555 = std::make_unique<TH2F>("h555", "", 7, -3.5, 3.5, 32, -0.5, 31.5);
    ntuple->Project("h555", "phi:side*(lay+1)/abs(side)", "status == 42 && abs(side)==1");
    if (h555->GetEntries() != 0)
    {

        h555->GetXaxis()->SetTitle("Barrel Module (athena numbering) Z");
        h555->GetXaxis()->SetTitleSize(0.04);

        h555->GetYaxis()->SetTitleOffset(0.8);
        h555->GetYaxis()->SetTitle("Detector #phi (athena 0-31 range)");
        h555->GetYaxis()->SetTitleSize(0.04);

        h555->GetZaxis()->SetTitle("Number of additional LowEff excluded straws");
        h555->GetZaxis()->SetTitleSize(0.04);

        h555->Draw("colz text");

        mytext.DrawLatex(0.10, 0.02, Form("Run %d", run));
        mytext.DrawLatex(0.45, 0.91, "#leftarrow C || A #rightarrow");

        print(Form("additionalExcludedLowEffStrawsBarrel_%d", run), c1);
    }
    h555.reset();

    std::unique_ptr<TH2F> h666 = std::make_unique<TH2F>("h666", "", 29, -14.5, 14.5, 32, -0.5, 31.5);
    ntuple->Project("h666", "phi:side*(lay+1)/abs(side)", "status == 42 && abs(side)==2");
    if (h666->GetEntries() != 0)
    {

        h666->GetXaxis()->SetTitle("Endcap Module (athena numbering) Z");
        h666->GetXaxis()->SetTitleSize(0.04);

        h666->GetYaxis()->SetTitleOffset(0.8);
        h666->GetYaxis()->SetTitle("Detector #phi (athena 0-31 range)");
        h666->GetYaxis()->SetTitleSize(0.04);

        h666->GetZaxis()->SetTitle("Number of additional LowEff excluded straws");
        h666->GetZaxis()->SetTitleSize(0.04);

        h666->Draw("colz text");

        mytext.DrawLatex(0.10, 0.02, Form("Run %d", run));
        mytext.DrawLatex(0.45, 0.91, "#leftarrow C || A #rightarrow");

        print(Form("additionalExcludedLowEffStrawsEndcap_%d", run), c1);
    }
    h666.reset();

    std::unique_ptr<TH2F> h5555 = std::make_unique<TH2F>("h5555", "", 7, -3.5, 3.5, 32, -0.5, 31.5);
    ntuple->Project("h5555", "phi:side*(lay+1)/abs(side)", "status > 50 && abs(side)==1");
    if (h5555->GetEntries() != 0)
    {

        h5555->GetXaxis()->SetTitle("Barrel Module (athena numbering) Z");
        h5555->GetXaxis()->SetTitleSize(0.04);

        h5555->GetYaxis()->SetTitleOffset(0.8);
        h5555->GetYaxis()->SetTitle("Detector #phi (athena 0-31 range)");
        h5555->GetYaxis()->SetTitleSize(0.04);

        h5555->GetZaxis()->SetTitle("Number of additional BadHT excluded straws");
        h5555->GetZaxis()->SetTitleSize(0.04);

        h5555->Draw("colz text");

        mytext.DrawLatex(0.10, 0.02, Form("Run %d", run));
        mytext.DrawLatex(0.45, 0.91, "#leftarrow C || A #rightarrow");

        print(Form("additionalExcludedBadHTStrawsBarrel_%d", run), c1);
    }
    h5555.reset();

    std::unique_ptr<TH2F> h6666 = std::make_unique<TH2F>("h6666", "", 29, -14.5, 14.5, 32, -0.5, 31.5);
    ntuple->Project("h6666", "phi:side*(lay+1)/abs(side)", "status > 50 && abs(side)==2");
    if (h6666->GetEntries() != 0)
    {

        h6666->GetXaxis()->SetTitle("Endcap Module (athena numbering) Z");
        h6666->GetXaxis()->SetTitleSize(0.04);

        h6666->GetYaxis()->SetTitleOffset(0.8);
        h6666->GetYaxis()->SetTitle("Detector #phi (athena 0-31 range)");
        h6666->GetYaxis()->SetTitleSize(0.04);

        h6666->GetZaxis()->SetTitle("Number of additional BadHT excluded straws");
        h6666->GetZaxis()->SetTitleSize(0.04);

        h6666->Draw("colz text");

        mytext.DrawLatex(0.10, 0.02, Form("Run %d", run));
        mytext.DrawLatex(0.45, 0.91, "#leftarrow C || A #rightarrow");

        print(Form("additionalExcludedBadHTStrawsEndcap_%d", run), c1);
    }
    h6666.reset();

    c1->SaveAs("allPlots.pdf]");
    myfile->Write(0,TObject::kOverwrite);
    myfile->Close();
    
    //delete new'ed objects
    delete c1;
    delete myfile;
    return;

}

int main(){
    TRTCalib_StrawStatusReport();
}
