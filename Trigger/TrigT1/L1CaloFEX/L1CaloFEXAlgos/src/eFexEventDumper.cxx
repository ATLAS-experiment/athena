/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "eFexEventDumper.h"

#include "TFile.h"
#include "TTree.h"
#include "PathResolver/PathResolver.h"
#include "TH2D.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TBox.h"
#include "TLatex.h"


namespace LVL1 {

    eFexEventDumper::eFexEventDumper(const std::string &name, ISvcLocator *pSvcLocator) :
        AthReentrantAlgorithm(name,pSvcLocator) {


    }

    StatusCode eFexEventDumper::initialize() {
        CHECK( m_towerKey.initialize(SG::AllowEmpty) );
        CHECK( m_emTOBKey.initialize(SG::AllowEmpty) );
        CHECK( m_tauTOBKey.initialize(SG::AllowEmpty) );
        CHECK( m_noiseCutsKey.initialize() );
        m_file.reset(TFile::Open(m_outputFileName.value().c_str(),"RECREATE"));
        return StatusCode::SUCCESS;
    }

    StatusCode eFexEventDumper::execute(const EventContext& ctx) const {

        std::map<std::pair<int, int>, int> noiseCutsMap; // key is [eta,layer]
        SG::ReadCondHandle <CondAttrListCollection> noiseCuts{m_noiseCutsKey, ctx};
        if (noiseCuts.isValid()) {
            for (auto itr = noiseCuts->begin(); itr != noiseCuts->end(); ++itr) {
                if (itr->first >= 50) continue;
                noiseCutsMap[std::pair(itr->first, 0)] = itr->second["EmPS"].data<int>();
                noiseCutsMap[std::pair(itr->first, 1)] = itr->second["EmFR"].data<int>();
                noiseCutsMap[std::pair(itr->first, 2)] = itr->second["EmMD"].data<int>();
                noiseCutsMap[std::pair(itr->first, 3)] = itr->second["EmBK"].data<int>();
                noiseCutsMap[std::pair(itr->first, 4)] = (itr->first >= 10 && itr->first < 40)
                                                         ? itr->second["Tile"].data<int>()
                                                         : itr->second["HEC"].data<int>();
            }
        }


        TDirectory* dir = gDirectory;
        m_file->cd();
        TCanvas c;c.Divide(2,3);
        c.SetName(TString::Format("evt%lu", ctx.eventID().event_number()));
        c.SetTitle(TString::Format("Run %u LB %u Event %lu",
                                   ctx.eventID().run_number(),
                                   ctx.eventID().lumi_block(),
                                   ctx.eventID().event_number()));

        TH2D ps("ps", "ps [MeV];#eta;#phi", 50, -2.5, 2.5, 64, -M_PI, M_PI);
        TH2D l1("l1", "l1 [MeV];#eta;#phi", 200, -2.5, 2.5, 64, -M_PI, M_PI);
        TH2D l2("l2", "l2 [MeV];#eta;#phi", 200, -2.5, 2.5, 64, -M_PI, M_PI);
        TH2D l3("l3", "l3 [MeV];#eta;#phi", 50, -2.5, 2.5, 64, -M_PI, M_PI);
        TH2D had("had", "had [MeV];#eta;#phi", 50, -2.5, 2.5, 64, -M_PI, M_PI);
        TH2D tobs("tobs", "Sum [MeV];#eta;#phi", 50, -2.5, 2.5, 64, -M_PI, M_PI);
        std::vector < TH2 * > hists{&ps, &l1, &l2, &l3, &had};
        std::vector < TH2 * > fhists;
        for(auto h : hists) fhists.push_back(dynamic_cast<TH2*>(h->Clone(TString::Format("%s_fail",h->GetName()))));

        if(!m_towerKey.empty()) {
            SG::ReadHandle <xAOD::eFexTowerContainer> towers{m_towerKey, ctx};
            for (const xAOD::eFexTower* tower: *towers) {
                auto counts = tower->et_count();
                if (counts.empty()) continue;
                double tEta = (int((tower->eta()+0.025)*10)-(tower->eta()<0)*1)*0.1; // left edge
                double tPhi = tower->phi() + 0.025; // ~bin centre

                for (size_t layer = 0; layer < 5; layer++) {
                    for (size_t cell = 0; cell < ((layer == 1 || layer == 2) ? 4 : 1); cell++) {

                        auto et = tower->cellEt(layer, cell); //- commented out until added to EDM
                        // only fill each bin once ... when using data towers some locations will have multiple towers
                        if (et && hists.at(layer)->GetBinContent(hists.at(layer)->FindFixBin(tEta + 0.025*cell + 0.0125, tPhi)) == 0) {
                            // check if fails noise cut ... will then also fill into another map
                            bool failedCut = (counts.at(tower->cellIdx(layer,cell)) <= noiseCutsMap[std::pair( int( (tower->eta() + 2.525)/0.1 ), layer)]);

                            hists.at(layer)->Fill(tEta + 0.025*cell + 0.0125, tPhi, et);
                            if(failedCut) fhists.at(layer)->Fill(tEta + 0.025*cell + 0.0125, tPhi, et);
                            // if filling a phi edge bin, fill the over/underflow on the other side, so that energies wrap around
                            int j = hists.at(layer)->GetYaxis()->FindFixBin(tPhi);
                            if(j==1) {
                                hists.at(layer)->Fill(tEta + 0.025*cell + 0.0125, tPhi+2*M_PI,et);
                                if(failedCut) fhists.at(layer)->Fill(tEta + 0.025*cell + 0.0125, tPhi+2*M_PI,et);
                            } else if(j==hists.at(layer)->GetNbinsY()) {
                                hists.at(layer)->Fill(tEta + 0.025*cell + 0.0125, tPhi-2*M_PI,et);
                                if(failedCut) fhists.at(layer)->Fill(tEta + 0.025*cell + 0.0125, tPhi-2*M_PI,et);
                            }
                        }
                    }
                }
            }
        }

        for (size_t i = 0; i < hists.size(); i++) {
            c.GetPad(i + 1)->cd()->SetGrid(1,1);
            hists[i]->SetStats(false);
            hists[i]->SetMarkerSize(2); // controls text size
            hists[i]->GetXaxis()->SetRangeUser(-0.3, 0.3);
            hists[i]->GetYaxis()->SetRangeUser(-0.3, 0.3);
            hists[i]->Draw((hists[i]->GetNbinsX()>50)?"col1text89" : "col1text");
            fhists[i]->SetMarkerSize(2);
            fhists[i]->SetMarkerColor(kRed); // displays supercells failing noise cuts in red text
            fhists[i]->GetXaxis()->SetRangeUser(-0.3, 0.3);
            fhists[i]->GetYaxis()->SetRangeUser(-0.3, 0.3);
            fhists[i]->Draw((hists[i]->GetNbinsX()>50)?"sametext89" : "sametext");
            for(int ii=1;ii<=hists[i]->GetNbinsX();ii++) {
                for(int jj=1;jj<=hists[i]->GetNbinsY();jj++)
                    tobs.Fill(hists[i]->GetXaxis()->GetBinCenter(ii),hists[i]->GetYaxis()->GetBinCenter(jj),hists[i]->GetBinContent(ii,jj));
            }
        }
        auto lastPad = c.GetPad(hists.size() + 1)->cd();
        tobs.SetStats(false);
        tobs.Draw("col1");
        TBox b(-0.3, -0.3, 0.3, 0.3);
        b.SetLineColor(kRed);
        b.SetFillStyle(0);
        b.SetLineWidth(1);
        b.SetBit(TBox::kCannotMove);
        tobs.GetListOfFunctions()->Add(b.Clone());
        lastPad->AddExec("onClick", TString::Format(
                "{ auto pad = gPad->GetCanvas()->GetPad(%lu); if( pad->GetEvent()==kButton1Down ) { double x = pad->PadtoX(pad->AbsPixeltoX(pad->GetEventX())); double y = pad->PadtoY(pad->AbsPixeltoY(pad->GetEventY())); for(int i=1;i<%lu;i++) {for(auto o : *gPad->GetCanvas()->GetPad(i)->GetListOfPrimitives()) {if(auto h = dynamic_cast<TH1*>(o);h) {h->GetXaxis()->SetRangeUser(x-0.3,x+0.3);h->GetYaxis()->SetRangeUser(y-0.3,y+0.3); } } } if(auto b = dynamic_cast<TBox*>(pad->FindObject(\"tobs\")->FindObject(\"TBox\"))) {b->SetX1(x-0.3);b->SetX2(x+0.3);b->SetY1(y-0.3);b->SetY2(y+0.3);} gPad->GetCanvas()->Paint(); gPad->GetCanvas()->Update(); } }",
                hists.size() + 1, hists.size() + 1));

        if(!m_emTOBKey.empty()) {
            SG::ReadHandle <xAOD::eFexEMRoIContainer> tobs1{m_emTOBKey, ctx};
            TLatex l;l.SetTextColor(kMagenta);
            for (auto tob: *tobs1) {
                l.DrawLatex(tob->eta(), tob->phi(), TString::Format("%x", tob->word0()));
            }
        }


        c.Write();
        gDirectory = dir;


        return StatusCode::SUCCESS;
    }




}
