/*
Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

void TileSamplingFraction_analysis(std::string args="")
{
  std::vector<std::string> thetalist;
  std::stringstream ss(args);
  std::string token;
  while (ss >> token) {
      thetalist.push_back(token);
  }

  std::sort(thetalist.begin(), thetalist.end(), [](const std::string& a, const std::string& b) {
    return std::stof(a) < std::stof(b);
  });

  std::vector<Double_t> x, y, ex, ey;
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

    x.push_back(th);
    ex.push_back(0);
    y.push_back(invSF);
    ey.push_back(invSFerr);
  }



  TCanvas* c=new TCanvas("TileSamplingFractions","Tile Sampling Fractions");
  double ylow=30;
  double yhigh=34.5;

  int n = x.size();
  TGraphErrors* SF_graph = new TGraphErrors(n,&x[0], &y[0], &ex[0], &ey[0]);
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
