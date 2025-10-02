/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODTrigL1Calo/eFexTowerContainer.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"

#include "L1CaloFEXSim/eTower.h"
#include "L1CaloFEXSim/eTowerContainer.h"
#include "./eTowerMakerFromEfexTowers.h"
#include "L1CaloFEXSim/eFEXCompression.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"

#undef R__HAS_VDT
#include "ROOT/RVec.hxx"

#include "TFile.h"
#include "TTree.h"
#include "PathResolver/PathResolver.h"
#include "TH2D.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TBox.h"

namespace LVL1 {

  eTowerMakerFromEfexTowers::eTowerMakerFromEfexTowers(const std::string& name, ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  { 
  
  }


StatusCode eTowerMakerFromEfexTowers::initialize()
{
  ATH_CHECK( m_eTowerBuilderTool.retrieve() );
  ATH_CHECK( m_eFexTowerContainerSGKey.initialize() );
  ATH_CHECK( m_eFexTowerContainer2SGKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_eTowerContainerSGKey.initialize() );
  ATH_CHECK( m_noiseCutsKey.initialize(SG::AllowEmpty) );

  if (!m_noiseCutsKey.empty()) {
      // disable built-in noise cuts because will be loading them from conditions db instead
      eFEXCompression::s_disableNoiseCuts = true;
  }

  return StatusCode::SUCCESS;
}


  StatusCode eTowerMakerFromEfexTowers::execute(const EventContext& ctx) const
{
  // load noise cuts .. should really only need to do this at start of runs, not every event!
  std::map<std::pair<int, int>, int> noiseCutsMap; // key is [eta,layer]
  bool useHardcodedCuts = false;
  if(!m_noiseCutsKey.empty()) {
      // check timestamp of event is not *before* date when started using database
      if (ctx.eventID().time_stamp() < m_noiseCutBeginTimestamp) {
        useHardcodedCuts = true;
      } else {
        SG::ReadCondHandle <CondAttrListCollection> noiseCuts{m_noiseCutsKey, ctx};
        if (noiseCuts.isValid()) {
            if(msgLvl(MSG::DEBUG) && !m_printedNoiseCuts) {
                m_printedNoiseCuts = true;
                ATH_MSG_DEBUG("DB Noise cuts are:");
                noiseCuts->dump();
            }
            if(noiseCuts->size()==0) {
              ATH_MSG_ERROR("No noise cuts loaded from conditions db for event with timestamp" << ctx.eventID().time_stamp());
              return StatusCode::FAILURE;
            }
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
      }
  }

  // STEP 0 - Make a fresh local eTowerContainer
  std::unique_ptr<eTowerContainer> local_eTowerContainerRaw = std::make_unique<eTowerContainer>();

  // STEP 1 - Make some eTowers and fill the local container
  m_eTowerBuilderTool->init(local_eTowerContainerRaw);
  local_eTowerContainerRaw->clearContainerMap();
  local_eTowerContainerRaw->fillContainerMap();

  SG::ReadHandle<xAOD::eFexTowerContainer> eFexTowers(m_eFexTowerContainerSGKey, ctx);
  // we may have DataTowers because of error statuses ... so don't use the primary towers if
  // they all have errors
  size_t badTowers = 0;
  if(eFexTowers.isValid()) {
      for(auto eFexTower : *eFexTowers) {
          if(eFexTower->em_status()||eFexTower->had_status()) badTowers++;
      }
  }
  if((!eFexTowers.isValid() || eFexTowers->size() < m_minTowersRequired || eFexTowers->size()==badTowers) && !m_eFexTowerContainer2SGKey.empty()) {
    eFexTowers = SG::ReadHandle<xAOD::eFexTowerContainer>(m_eFexTowerContainer2SGKey, ctx);
      // removing this to avoid breaking frozen tier0 policy
      // bug keeping commented out until sure we've replaced with a good alternative
      //const xAOD::EventInfo* ei = nullptr;
      //CHECK( evtStore()->retrieve(ei) );
      //ei->auxdecor<bool>("eTowerMakerFromEfexTowers_usedSecondary") = true;
  }

  // STEP 2 - Do the efexTower-tower mapping - put this information into the eTowerContainer
  for(auto eFexTower : *eFexTowers) {
      // need to ensure this eFexTower is a "core" tower in a module ... so that there aren't disconnected inputs
      // and also need to only do one tower per location, of course
      auto tower = local_eTowerContainerRaw->findTower(eFexTower->eFEXtowerID());
      auto counts = eFexTower->et_count();
      for(size_t i=0;i<counts.size();i++) {
          if(i<10 && eFexTower->em_status()) continue; // bad status bits have their energy zerod by the firmware
          if(i==10 && eFexTower->had_status()) continue;
          if (eFexTower->disconnectedCount(i)) continue;
          if (counts.at(i)==0 || (counts.at(i)>1020 && counts.at(i)!=1023)) continue; // absent (1025 from BS decoder), invalid (1022), empty (0) or masked (0) channel
          // special case logic for reordering |eta|=2.5 and overlap
          // and l1 1.8-2.0 ... need to put the merged sc counts into slots that wont be split
          int layer; int cell=i;
          if(i<1 || (i==4 && std::abs(eFexTower->eta()+0.025)>2.4)) {layer = 0;cell=0;}
          else if(i<5) layer = 1;
          else if(i<9) layer = 2;
          else if(i<10) layer = 3;
          else layer = 4;

          // apply noise cut ... for runs up to 14th April 2023 was killing with <, then from run 449180 onwards kills with <=
          // since long-term behaviour is latter, will use that
          //if(useHardcodedCuts && !eFEXCompression::noiseCut(counts.at(i),layer,true)) continue;
          if(!useHardcodedCuts && counts.at(i) <= noiseCutsMap[std::pair( int( (eFexTower->eta() + 2.525)/0.1 ), layer)]) continue;

          // checking we haven't already filled this tower (happens when using efexDataTowers ... multiple towers per loc for different modules)
          // this is ugly as ...
          if(!tower->getET_float(layer,cell-(layer==1)*1-(layer==2)*5-(layer==3)*9-(layer==4)*10)) {
              // if tile then count is in steps of 500 MeV, not in latome multilinear encoding
              bool isTile = (std::abs(eFexTower->eta()+0.025)<1.5 && layer==4);
              tower->setET(cell,isTile ? (counts.at(i)*500.) : eFEXCompression::expand(counts.at(i)),layer + isTile, useHardcodedCuts);
          }
      }
  }

    if(msgLvl(MSG::DEBUG)) {
        std::scoped_lock lock(m_debugMutex);
        // dump towers to histograms
        // counts are the "codes" of multi-scale latome, or tile count

        TFile *debugFile = dynamic_cast<TFile *>(gROOT->GetListOfFiles()->FindObject("debug_eFexTowerMakerFromEfexTowers.root"));
        if (!debugFile) debugFile = TFile::Open("debug_eFexTowerMakerFromEfexTowers.root", "RECREATE");
        if (debugFile->GetListOfKeys()->GetEntries() < 20) {
            TDirectory *dir = gDirectory;
            debugFile->cd();
            TH2D ps("ps", "ps [code];#eta;#phi", 50, -2.5, 2.5, 64, -M_PI, M_PI);
            TH2D l1("l1", "l1 [code];#eta;#phi", 200, -2.5, 2.5, 64, -M_PI, M_PI);
            TH2D l2("l2", "l2 [code];#eta;#phi", 200, -2.5, 2.5, 64, -M_PI, M_PI);
            TH2D l3("l3", "l3 [code];#eta;#phi", 50, -2.5, 2.5, 64, -M_PI, M_PI);
            TH2D had("had", "had [code~25MeV or 500MeV for tile];#eta;#phi", 50, -2.5, 2.5, 64, -M_PI, M_PI);
            std::vector < TH1 * > hists{&ps, &l1, &l2, &l3, &had};
            for(auto eFexTower : *eFexTowers) {
                auto counts = eFexTower->et_count();
                if (counts.empty()) continue;
                int etaIndex = int( (eFexTower->eta()+0.025)*10 ) + (((eFexTower->eta()+0.025)<0) ? -1 : 1); // runs from -25 to 25 (excluding 0)
                int phiIndex = int( (eFexTower->phi()+0.025)*32./M_PI ) + ((eFexTower->phi()+0.025)<0 ? -1 : 1); // runs from -32 to 32 (excluding 0)
                double tEta = ((etaIndex < 0 ? 0.5 : -0.5) + etaIndex - 0.5) * 0.1; // left edge
                double tPhi = ((phiIndex < 0 ? 0.5 : -0.5) + phiIndex) * M_PI / 32; // centre
                for(size_t i=0;i<counts.size();i++) {
                    if(i<10 && eFexTower->em_status()) continue; // bad status bits have their energy zerod by the firmware
                    if(i==10 && eFexTower->had_status()) continue;
                    if (eFexTower->disconnectedCount(i)) continue;
                    int layer; int cell=i;
                    if(i<1 || (i==4 && std::abs(eFexTower->eta()+0.025)>2.4)) {layer = 0;cell=0;}
                    else if(i<5) {layer = 1;cell = i-1;}
                    else if(i<9) {layer = 2;cell = i-5;}
                    else if(i<10) {layer = 3;cell=0;}
                    else {layer = 4;cell=0;}
                    if(!useHardcodedCuts && counts.at(i) <= noiseCutsMap[std::pair( int( (eFexTower->eta() + 2.525)/0.1 ), layer)]) continue;
                    hists.at(layer)->SetBinContent(hists.at(layer)->FindFixBin(tEta + 0.025 * cell + 0.0125, tPhi),counts.at(i));

                }
            }

            TCanvas c;
            c.SetName(TString::Format("evt%lu", ctx.eventID().event_number()));
            c.SetTitle(TString::Format("Run %u LB %u Event %lu", ctx.eventID().run_number(), ctx.eventID().lumi_block(),
                                       ctx.eventID().event_number()));
            c.Divide(2, 3);
            TH2D tobs("tobs", "Sum [MeV];#eta;#phi", 50, -2.5, 2.5, 64, -M_PI, M_PI);
            for (size_t i = 0; i < hists.size(); i++) {
                c.GetPad(i + 1)->cd();gPad->SetGrid(1,1);
                hists[i]->SetStats(false);
                hists[i]->SetMarkerSize(2); // controls text size
                hists[i]->GetXaxis()->SetRangeUser(-0.3, 0.3);
                hists[i]->GetYaxis()->SetRangeUser(-0.3, 0.3);
                hists[i]->Draw((hists[i]->GetNbinsX() > 50) ? "coltext89" : "coltext");
                for (int ii = 1; ii <= hists[i]->GetNbinsX(); ii++) {
                    bool isTile = (i==4 && std::abs(hists[i]->GetXaxis()->GetBinCenter(ii))<1.5);
                    for (int jj = 1; jj <= hists[i]->GetNbinsY(); jj++)
                        tobs.Fill(hists[i]->GetXaxis()->GetBinCenter(ii), hists[i]->GetYaxis()->GetBinCenter(jj),
                                  isTile ? (hists[i]->GetBinContent(ii, jj)*500.) : eFEXCompression::expand(hists[i]->GetBinContent(ii, jj)));
                }
            }
            c.GetPad(hists.size() + 1)->cd();
            tobs.SetStats(false);
            tobs.Draw("col");
            TBox b(-0.3, -0.3, 0.3, 0.3);
            b.SetLineColor(kRed);
            b.SetFillStyle(0);
            b.SetLineWidth(1);
            b.SetBit(TBox::kCannotMove);
            tobs.GetListOfFunctions()->Add(b.Clone());
            gPad->AddExec("onClick", TString::Format(
                    "{ auto pad = gPad->GetCanvas()->GetPad(%lu); if( pad->GetEvent()==kButton1Down ) { double x = pad->PadtoX(pad->AbsPixeltoX(pad->GetEventX())); double y = pad->PadtoY(pad->AbsPixeltoY(pad->GetEventY())); for(int i=1;i<%lu;i++) {auto h = dynamic_cast<TH1*>(gPad->GetCanvas()->GetPad(i)->GetListOfPrimitives()->At(1)); if(h) {h->GetXaxis()->SetRangeUser(x-0.3,x+0.3);h->GetYaxis()->SetRangeUser(y-0.3,y+0.3); } } if(auto b = dynamic_cast<TBox*>(pad->FindObject(\"tobs\")->FindObject(\"TBox\"))) {b->SetX1(x-0.3);b->SetX2(x+0.3);b->SetY1(y-0.3);b->SetY2(y+0.3);} gPad->GetCanvas()->Paint(); gPad->GetCanvas()->Update(); } }",
                    hists.size() + 1, hists.size() + 1));
            c.Write();
            gDirectory = dir;
        }
    }



    // STEP 3 - Write the completed eTowerContainer into StoreGate (move the local copy in memory)
  SG::WriteHandle<LVL1::eTowerContainer> eTowerContainerSG(m_eTowerContainerSGKey, ctx);
  ATH_CHECK(eTowerContainerSG.record(std::move(/*my_eTowerContainerRaw*/local_eTowerContainerRaw)));

  // STEP 4 - Close and clean the event
  m_eTowerBuilderTool->reset();

  return StatusCode::SUCCESS;
}

  

} // end of LVL1 namespace
