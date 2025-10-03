/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//Produce ps files for EM cluster validation
// from work of 
//author: A. Kaczmarska
//date: 27.07.2004
//
//16.12.2004 - AK: displaying of 2d histos
//             corrected
//********************************

void setStyle(){
  cout<<"drawing..."<<endl;
  
  //Set Some default styles for the histogram plots
  gStyle->SetMarkerStyle(8);
  gStyle->SetStatW(0.2);
  gStyle->SetStatH(0.2);
  gStyle->SetLabelSize(0.06);
  gStyle->SetOptStat(0000);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetPadBorderMode(0);
  gStyle->SetPadColor(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetTitleColor(0);
  gStyle->SetStatColor(0);
  gStyle->SetMarkerSize(1.);
}



// Iterate over a hist file to retrieve hists.
// Does not know the number of hists before execution
TObjArray* getHists(char* fname){
  cout<<"getting hists "<<endl;
  
  TFile* f = new TFile(fname);
  TObjArray* hists = new TObjArray(0);
  // if hist file open fails, return an empty collection
  // of TObjects

  if(!f){
    cout<<"Could not open file "<<fname<<endl;
    return hists;
  }

  TIter next(f->GetListOfKeys());
  TKey *key;
  while ((key=(TKey*)next())) {
    char* name = key->GetName();
    TObject* obj = f->Get(name);
    hists->Add(obj);
  }  
  
  return hists;
}

// make a postscript file from hist and ref hist files
// sets up the canvas parameters
void histsToPostScript(char* newFile, char* refFile, char* psFile){
  cout<<"histsToPostScript"<<endl;
  TCanvas* c1 = new TCanvas("c1","cluster validation",200,10,600,400);
  c1->Clear();
  int nx = 2;
  int ny = 2;
  c1->Divide(nx, ny); 
  
  TPostScript *ps = new TPostScript(psFile, 112);

  TObjArray* hListNew = getHists(newFile);
  TObjArray* hListRef = getHists(refFile);
  
  TIter nIter(hListNew);
  TIter rIter(hListRef);
  
  // Display in turn as many hists as there are windows
  // until all the hists have been displayed
  int nWin = nx*ny;
  int count = nWin;
  TH1* nhist;
  TH1* rhist;
  TObjArray* nHistsForAPage = new TObjArray(0);
  TObjArray* rHistsForAPage = new TObjArray(0);
  while( (nhist = (TH1*)nIter.Next())){
    rhist = (TH1*)rIter.Next();

    
    
    if( count == nWin){
      c1->Clear();
      c1->Divide(nx,ny);
      nHistsForAPage->Clear();
      rHistsForAPage->Clear();
    }
    
    
    nHistsForAPage->Add(nhist);
    rHistsForAPage->Add(rhist);
    --count;
    
    if( count == 0){
      displayHists(c1, nHistsForAPage, rHistsForAPage);
      count = nWin;
      c1->Update();
    }
  }
    
  // display the last set of hists
  if(count != nWin){
    displayHists(c1, nHistsForAPage, rHistsForAPage);
    c1->Update();
  }
  
  ps->Close();
}

// display hists on a canvas
void displayHists(TCanvas* c1, TObjArray* newHists, TObjArray* refHists){

  TIter nIter(newHists);
  TIter rIter(refHists);
  int panel = 0;
  TH1* nhist;
  TH1* rhist;
  while( (nhist = static_cast<TH1*>(nIter.Next()))){
    rhist = static_cast<TH1*>(rIter.Next());

    ++panel;
    c1->cd(panel);

    if (nhist->GetDimension()<2) {
      if(nhist){
        nhist->SetMarkerStyle();
	nhist->SetLineColor(1);
	nhist->Draw();
      }

      if(rhist){
        rhist->SetMarkerStyle();
	rhist->SetLineColor(2);
	rhist->Draw("same");
      }
    }else{
      if(nhist){
        nhist->SetMarkerStyle(29);
        nhist->SetMarkerSize(.5);
        nhist->SetMarkerColor(1);
	nhist->Draw();
      }
      
      if(rhist){
        rhist->SetMarkerStyle(25);
        rhist->SetMarkerSize(.5);
        rhist->SetMarkerColor(2);
	rhist->Draw("same");
      }
    }
  }
}

// Entrance point to the macro
// supply names of new and referencce histograms
int egammaPerformanceHistsToPostScript(char* newFile, char* refFile, char* psFile){

  setStyle();
  histsToPostScript(newFile, refFile, psFile);
  
  return 0;
}
