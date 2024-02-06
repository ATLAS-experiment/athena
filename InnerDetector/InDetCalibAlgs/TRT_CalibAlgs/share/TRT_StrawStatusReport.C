void print(char *figname, TCanvas *c1){

	char filename[1000]; 
	sprintf(filename, "output/%s", figname);
	
	c1->Print( Form("%s.png", filename) );
	c1->Print( Form("%s.pdf", filename) );
	c1->Print( Form("%s.eps", filename));				
	c1->Print( "allPlots.pdf"); 
}


void TRT_StrawStatusReport(){		
	
	// read the content from a default txt file name into an ntuple
	
	int run = 0;
	TNtuple *ntuple = new TNtuple("ntuple", "data", "side:phi:straw:status:hits:occ:htocc:eff:lay");;
	
	int var[15];
	double par[5];
	int count = 0;
	
	ifstream in;
	in.open( "TRT_StrawStatusReport.txt" );
		
	while(1) {
		
		for (int i=0; i<5; i++) in >> var[i];		   
		for (int i=0; i<3; i++) in >> par[i];		   
		in >> var[5];
		if (!in.good()) break;	   
		count++;
		
		if (var[0]==0) {
			run = var[4];
			continue;
		}
		ntuple->Fill( var[0], var[1], var[2], var[3], var[4], par[0], par[1], par[2], var[5] );
	}
	
	printf("read %d lines from file\n", count);
	in.close();		
	
	// make the efficiency distribution plot
	
	TCanvas *c1 = new TCanvas();
	c1->SetLogy(1);

	// set stat box size 
	gStyle->SetStatY(1);
	gStyle->SetStatX(0.9);
	gStyle->SetStatW(0.15);
	gStyle->SetStatH(0.1);

	c1->Print( "allPlots.pdf["); 
	
	TH1F *h1 = new TH1F("h1", "", 100, 0., 1.);
	ntuple->Project("h1", "eff");
	h1->GetXaxis()->SetTitle( "Straw hit efficiency" );
	h1->GetYaxis()->SetTitle( Form("Number of straws (run %d)", run) );
	h1->Draw();
	
	TH1F *h1a = new TH1F("h1a", "", 100, 0., 1.);
	ntuple->Project("h1a", "eff", "status>0");	
	h1a->SetFillStyle(1001);
	h1a->SetFillColor(kOrange);
	h1a->Draw("same");
	
	TH1F *h1b = new TH1F("h1b", "", 100, 0., 1.);
	ntuple->Project("h1b", "eff", "status==42");	
	h1b->SetFillStyle(1001);
	h1b->SetFillColor(2);
	h1b->Draw("same");
	
	gPad->RedrawAxis();
	
	print( Form("strawHitEfficiency_%d", run), c1);
//	return;		
	
	delete h1; 
	delete h1a;
	delete h1b;	
	
	// make the HT occupancy distribution plot
	
	double htrange = 0.1;
	TH1F *hh1 = new TH1F("hh1", "", 100, 0., htrange);
	ntuple->Project("hh1", "htocc");
	hh1->GetXaxis()->SetTitle( "Straw HT occupancy" );
	hh1->GetYaxis()->SetTitle( Form("Number of straws (run %d)", run) );
	hh1->SetBinContent(100, hh1->GetBinContent(100)+hh1->GetBinContent(101) );
	hh1->Draw();
	
	TH1F *hh1a = new TH1F("hh1a", "", 100, 0., htrange);
	ntuple->Project("hh1a", "htocc", "status>0");	
	hh1a->SetFillStyle(1001);
	hh1a->SetFillColor(kOrange);
	hh1a->SetBinContent(100, hh1a->GetBinContent(100)+hh1a->GetBinContent(101) );
	hh1a->Draw("same");
	
	TH1F *hh1b = new TH1F("hh1b", "", 100, 0., htrange);
	ntuple->Project("hh1b", "htocc", "status==52");	
	hh1b->SetFillStyle(1001);
	hh1b->SetFillColor(2);
	hh1b->SetBinContent(100, hh1b->GetBinContent(100)+hh1b->GetBinContent(101) );
	hh1b->Draw("same");
	
	gPad->RedrawAxis();
	
	print( Form("strawHToccupancy_%d", run), c1);

	delete hh1; 
	delete hh1a;
	delete hh1b;	

	
	gStyle->SetPalette(1);	
	c1->SetRightMargin(0.2);
	c1->SetLogy(0);
		
	TH1F *h7 = new TH1F("h7", "", 100, 0, 10000); 
	ntuple->Draw("hits>>h7", "straw>=147&&straw<200&&eff>0.7");
	double range = h7->GetMean();
	TH2F *h2 = new TH2F("h2", "", 110, -0.5, 5481.5, 100, 0., range*1.2);
	ntuple->Project("h2", "hits:straw", "status==0");
	h2->GetXaxis()->SetTitle("Straw index");
	h2->GetYaxis()->SetTitle("Number of hits on track");
	h2->GetZaxis()->SetTitleOffset(1.4);
	h2->GetZaxis()->SetTitle( Form("Number of straws (run %d)", run) );
	h2->Draw("colz");
	ntuple->SetMarkerStyle(20);
	ntuple->SetMarkerSize(0.8);
	ntuple->SetMarkerColor(1);
	ntuple->Draw("hits:straw", "status==52||status==51", "same");
	
	TLatex l; 
	l.SetTextSize(0.03);		
	l.DrawLatex(800, range*1.1, "black points: straws excluded due to low / high HT occupancy");				
	
	print( Form("numberOfHitsOnTrack_%d", run), c1);
	
	delete h2;
	
	
	TH2F *hh22 = new TH2F("hh22", "", 5, -2.5, 2.5, 32, -0.5, 31.5);
	ntuple->Project("hh22", "phi:side", "status>1");
	hh22->GetXaxis()->SetTitle( Form("Run %d              Detector side (athena numbering)", run) );
	hh22->GetYaxis()->SetTitle("Detector phi (athena 0-31 range)");
	hh22->GetZaxis()->SetTitleOffset(1.4);
	hh22->GetZaxis()->SetTitle("Number of additional excluded straws");
	hh22->Draw("colz");
	print( Form("additionalExcludedStraws_%d", run), c1);
	
	TH2F *h3 = new TH2F("h3", "", 7, -3.5, 3.5, 32, -0.5, 31.5);
	ntuple->Project("h3", "phi:side*(lay+1)/abs(side)", "status>1 && abs(side)==1");
	h3->GetXaxis()->SetTitle( Form("Run %d              Barrel Layer (athena numbering)", run) );
	h3->GetYaxis()->SetTitle("Detector phi (athena 0-31 range)");
	h3->GetZaxis()->SetTitleOffset(1.4);
	h3->GetZaxis()->SetTitle("Number of additional excluded straws");
	h3->Draw("colz text");
	print( Form("additionalExcludedStrawsBarrel_%d", run), c1);


	TH2F *h4 = new TH2F("h4", "", 29, -14.5, 14.5, 32, -0.5, 31.5);
	ntuple->Project("h4", "phi:side*(lay+1)/abs(side)", "status>1 && abs(side)==2");
	h4->GetXaxis()->SetTitle( Form("Run %d              Endcap Layer (athena numbering)", run) );
	h4->GetYaxis()->SetTitle("Detector phi (athena 0-31 range)");
	h4->GetZaxis()->SetTitleOffset(1.4);
	h4->GetZaxis()->SetTitle("Number of additional excluded straws");
	h4->Draw("colz text");
	print( Form("additionalExcludedStrawsEndcap_%d", run), c1);



	TH2F *h5 = new TH2F("h5", "", 7, -3.5, 3.5, 32, -0.5, 31.5);
	ntuple->Project("h5", "phi:side*(lay+1)/abs(side)", "status == 12 && abs(side)==1");
        if(h5->GetEntries()!=0) {
	h5->GetXaxis()->SetTitle( Form("Run %d              Barrel Layer (athena numbering)", run) );
	h5->GetYaxis()->SetTitle("Detector phi (athena 0-31 range)");
	h5->GetZaxis()->SetTitleOffset(1.4);
	h5->GetZaxis()->SetTitle("Number of additional DEAD excluded straws");
	h5->Draw("colz text");
	print( Form("additionalExcludedDEADStrawsBarrel_%d", run), c1);
	}

	TH2F *h6 = new TH2F("h6", "", 29, -14.5, 14.5, 32, -0.5, 31.5);
	ntuple->Project("h6", "phi:side*(lay+1)/abs(side)", "status == 12 && abs(side)==2");
        if(h6->GetEntries()!=0){
	h6->GetXaxis()->SetTitle( Form("Run %d              Endcap Layer (athena numbering)", run) );
	h6->GetYaxis()->SetTitle("Detector phi (athena 0-31 range)");
	h6->GetZaxis()->SetTitleOffset(1.4);
	h6->GetZaxis()->SetTitle("Number of additional DEAD excluded straws");
	h6->Draw("colz text");
	print( Form("additionalExcludedDEADStrawsEndcap_%d", run), c1);
	}

	TH2F *h55 = new TH2F("h55", "", 7, -3.5, 3.5, 32, -0.5, 31.5);
	ntuple->Project("h55", "phi:side*(lay+1)/abs(side)", "status == 11 && abs(side)==1");
        if(h55->GetEntries()!=0){
	h55->GetXaxis()->SetTitle( Form("Run %d              Barrel Layer (athena numbering)", run) );
	h55->GetYaxis()->SetTitle("Detector phi (athena 0-31 range)");
	h55->GetZaxis()->SetTitleOffset(1.4);
	h55->GetZaxis()->SetTitle("Number of additional NOISY excluded straws");
	h55->Draw("colz text");
	print( Form("additionalExcludedNOISYStrawsBarrel_%d", run), c1);
	}

	TH2F *h66 = new TH2F("h66", "", 29, -14.5, 14.5, 32, -0.5, 31.5);
        
	ntuple->Project("h66", "phi:side*(lay+1)/abs(side)", "status == 11 && abs(side)==2");
        if(h66->GetEntries()!=0){
	h66->GetXaxis()->SetTitle( Form("Run %d              Endcap Layer (athena numbering)", run) );
	h66->GetYaxis()->SetTitle("Detector phi (athena 0-31 range)");
	h66->GetZaxis()->SetTitleOffset(1.4);
	h66->GetZaxis()->SetTitle("Number of additional NOISY excluded straws");
	h66->Draw("colz text");
	print( Form("additionalExcludedNOISYStrawsEndcap_%d", run), c1);
	}

	TH2F *h555 = new TH2F("h555", "", 7, -3.5, 3.5, 32, -0.5, 31.5);
	ntuple->Project("h555", "phi:side*(lay+1)/abs(side)", "status == 42 && abs(side)==1");
        if(h555->GetEntries()!=0){
	h555->GetXaxis()->SetTitle( Form("Run %d              Barrel Layer (athena numbering)", run) );
	h555->GetYaxis()->SetTitle("Detector phi (athena 0-31 range)");
	h555->GetZaxis()->SetTitleOffset(1.4);
	h555->GetZaxis()->SetTitle("Number of additional LowEff excluded straws");
	h555->Draw("colz text");
	print( Form("additionalExcludedLowEffStrawsBarrel_%d", run), c1);
	}



	TH2F *h666 = new TH2F("h666", "", 29, -14.5, 14.5, 32, -0.5, 31.5);
	ntuple->Project("h666", "phi:side*(lay+1)/abs(side)", "status == 42 && abs(side)==2");
        if(h666->GetEntries()!=0){
	h666->GetXaxis()->SetTitle( Form("Run %d              Endcap Layer (athena numbering)", run) );
	h666->GetYaxis()->SetTitle("Detector phi (athena 0-31 range)");
	h666->GetZaxis()->SetTitleOffset(1.4);
	h666->GetZaxis()->SetTitle("Number of additional LowEff excluded straws");
	h666->Draw("colz text");
	print( Form("additionalExcludedLowEffStrawsEndcap_%d", run), c1);
	}

	TH2F *h5555 = new TH2F("h5555", "", 7, -3.5, 3.5, 32, -0.5, 31.5);
	ntuple->Project("h5555", "phi:side*(lay+1)/abs(side)", "status > 50 && abs(side)==1");
        if(h5555->GetEntries()!=0){
	h5555->GetXaxis()->SetTitle( Form("Run %d              Barrel Layer (athena numbering)", run) );
	h5555->GetYaxis()->SetTitle("Detector phi (athena 0-31 range)");
	h5555->GetZaxis()->SetTitleOffset(1.4);
	h5555->GetZaxis()->SetTitle("Number of additional BadHT excluded straws");
	h5555->Draw("colz text");
	print( Form("additionalExcludedBadHTStrawsBarrel_%d", run), c1);
	}

	TH2F *h6666 = new TH2F("h6666", "", 29, -14.5, 14.5, 32, -0.5, 31.5);
	ntuple->Project("h6666", "phi:side*(lay+1)/abs(side)", "status > 50 && abs(side)==2");
        if(h6666->GetEntries()!=0){
	h6666->GetXaxis()->SetTitle( Form("Run %d              Endcap Layer (athena numbering)", run) );
	h6666->GetYaxis()->SetTitle("Detector phi (athena 0-31 range)");
	h6666->GetZaxis()->SetTitleOffset(1.4);
	h6666->GetZaxis()->SetTitle("Number of additional BadHT excluded straws");
	h6666->Draw("colz text");
	print( Form("additionalExcludedBadHTStrawsEndcap_%d", run), c1);
	}




	c1->Print( "allPlots.pdf]"); 





	return;	
}
