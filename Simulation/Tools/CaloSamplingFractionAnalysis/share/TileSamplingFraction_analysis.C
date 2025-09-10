/*
Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

void TileSamplingFraction_analysis()
{
  const std::vector< std::string > thetalist={"-65","-60","-55","-50","-45","-40","-35","-30","-25","-20","-15","-10","-5",
                                              "+5","+10","+15","+20","+25","+30","+35","+40","+45","+50","+55","+60","+65"};

  const int n = 26;
  Double_t x[n], y[n], ex[n], ey[n];
  int bin_count = 0;
  for(const auto& theta : thetalist) {
    TFile* file=TFile::Open(Form("hist%s.root",theta.c_str()));
    if(!file) continue;
    if(!file->IsOpen()) continue;
    //cout<<theta<<" : Open"<<endl;
    //file->ls();
    TH1* h_ptgen=(TH1*)file->Get("h_ptgen");
    if(!h_ptgen) continue;
    TF1* thefun =(TF1*)file->Get("thefun");
    if(!thefun) continue;

    double invSF=(h_ptgen->GetMean())/(thefun->GetParameter(0));
    if(isnan(invSF)) continue;
    double invSFerr=(h_ptgen->GetMean()/thefun->GetParameter(0))*(thefun->GetParError(0)/thefun->GetParameter(0));
    if(isnan(invSFerr)) continue;

    double th=atof(theta.c_str());

    cout << "1/SF value (theta="<<th<<"): " << (h_ptgen->GetMean())/(thefun->GetParameter(0)) << "+/-" << (h_ptgen->GetMean()/thefun->GetParameter(0))*(thefun->GetParError(0)/thefun->GetParameter(0)) << " dErr % " <<  (thefun->GetParError(0)/thefun->GetParameter(0))*100 << endl;   
    x[bin_count] = th;
    ex[bin_count] = 0;
    y[bin_count] = invSF;
    ey[bin_count] = invSFerr;
    bin_count++;
  }
  TCanvas* c=new TCanvas("TileSamplingFractions","Tile Sampling Fractions");
  double ylow=30;
  double yhigh=34.5;

  TGraphErrors* SF_graph = new TGraphErrors(n,x,y,ex,ey);
  SF_graph->SetTitle(" ");
  SF_graph->GetXaxis()->SetTitle("#it{#theta} [degree]");
  SF_graph->GetYaxis()->SetTitle("Inverted Sampling Fraction");
  SF_graph->GetXaxis()->SetLimits(-72,72);
  SF_graph->GetXaxis()->SetRangeUser(-72,72);
  SF_graph->GetYaxis()->SetRangeUser(ylow,yhigh);
  SF_graph->SetMinimum(ylow);
  SF_graph->SetLineColor(2);
  SF_graph->SetMarkerColor(2);
  SF_graph->SetMarkerStyle(23);
  SF_graph->SetMarkerSize(2);
  SF_graph->Draw("AP");

  TF1 *f1=new TF1("f1","2*TMath::ATan(TMath::Exp(x))",-1.8427300,1.8427300);
  TGaxis *A1 = new TGaxis(-72,yhigh,72,yhigh,"f1",510,"-");
  A1->SetLabelFont(SF_graph->GetXaxis()->GetLabelFont());
  A1->SetTitle("#it{#eta}");
  A1->SetLabelSize(0.035);
  A1->SetTitleSize(0.04);
  A1->SetTitleOffset(1.05);
  A1->Draw();

  c->SaveAs("TileSamplingFractions.png");
  c->SaveAs("TileSamplingFractions.pdf");

  TFile fout("TileSamplingFractions.root","recreate");
  SF_graph->Write();
  fout.Close();
}
