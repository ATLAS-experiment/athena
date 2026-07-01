/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////////////
// Utils for the main MdtRawDataMonAlg.cxx
// Part of MdtRawDataMonAlg.h
// Authors
//    see MdtRawDataMonAlg.cxx
///////////////////////////////////////////////////////////////////////////

#include <TLine.h>
#include <TList.h>
#include <TString.h>

#include <string>

#include "MdtRawDataMonAlg.h"
#include "MuonDQAUtils/MuonChamberNameConverter.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"

using std::string;

StatusCode MdtRawDataMonAlg::binMdtGlobal(TH2* h, char ecap) {
    // Set x-axis labels
    int LowerEta = 0;
    int UpperEta = 0;
    if (ecap == 'B') {
        LowerEta = -8;
        UpperEta = 8;
    } else {
        LowerEta = -6;
        UpperEta = 6;
    }
    TString eta_s;
    for (int ieta = LowerEta; ieta != UpperEta + 1; ++ieta) {
        if (ieta == 0 && ecap == 'E') continue;
        eta_s = ecap;
        (ieta < 0) ? eta_s += "C" : ((ieta > 0) ? eta_s += "A" : eta_s += "B");
        eta_s += std::to_string(std::abs(ieta));
        h->Fill(eta_s, 1, 0);
    }

    // Set y-axis labels
    TString phi_s;
    for (int iphi = 1; iphi <= 16; iphi++) {
        if (iphi < 10)
            phi_s = "0" + std::to_string(iphi);
        else
            phi_s = std::to_string(iphi);
        if (ecap == 'B' && (iphi == 11 || iphi == 15)) {
            h->Fill(eta_s, "I," + phi_s + ",R", 0);
            h->Fill(eta_s, "I," + phi_s + ",M", 0);
            h->Fill(eta_s, "M," + phi_s, 0);
            h->Fill(eta_s, "O," + phi_s, 0);
        } else {
            h->Fill(eta_s, "I," + phi_s, 0);
            h->Fill(eta_s, "M," + phi_s, 0);
            h->Fill(eta_s, "O," + phi_s, 0);
            if (iphi % 2 == 0 && ecap == 'B')
                h->Fill(eta_s, "E," + phi_s, 0);
            else if (ecap == 'E')
                h->Fill(eta_s, "E," + phi_s, 0);
        }
    }

    h->LabelsDeflate("X");
    h->LabelsDeflate("Y");
    h->LabelsOption("a", "Y");
    h->Reset();

    return StatusCode::SUCCESS;
}

StatusCode MdtRawDataMonAlg::binMdtRegional(TH2* h, std::string_view xAxis) {
    /// SET Ghost Entries
    int LowerEta = -0;
    int UpperEta = 0;
    if (xAxis.substr(0, 2) == "BO" || xAxis.substr(0, 2) == "BI") {  // Barrel Hit Mutltiplicities
        LowerEta = -8;
        UpperEta = 8;
    } else if (xAxis.substr(0, 2) == "BM" || xAxis.substr(0, 2) == "EO") {
        LowerEta = -6;
        UpperEta = 6;
    } else if (xAxis.substr(0, 2) == "EM" || xAxis.substr(0, 2) == "EI") {
        LowerEta = -5;
        UpperEta = 5;
    } else {
        LowerEta = -2;
        UpperEta = 2;
    }

    // Set x-axis labels
    TString eta_s;
    for (int ieta = LowerEta; ieta != UpperEta + 1; ++ieta) {
        if (xAxis.substr(2, 1) == "A" && ieta < 0) continue;  // A side goes from 1-...
        if (xAxis.substr(2, 1) == "C" && ieta == 0) break;    // C side goes from ...-1
        if (ieta == 0 && xAxis.substr(0, 2) == "BO")
            eta_s = "BOB";
        else if (ieta == 0)
            continue;
        else
            eta_s = xAxis.substr(0, 3);
        eta_s += std::to_string(std::abs(ieta));
        h->Fill(eta_s, 1, 0);
    }

    bool barrelExtra = (xAxis == "BEA" || xAxis == "BEC");
    // Set y-axis labels
    TString phi_s;
    for (int iphi = 1; iphi <= 16; iphi++) {
        if (iphi < 10)
            phi_s = "0" + std::to_string(iphi);
        else
            phi_s = std::to_string(iphi);
        if (!barrelExtra) {
            // Also uncomment lines in MdtRawDataValAlg.cxx in fillMDTSummaryHistograms() that actually fill these values
            if (xAxis.substr(0, 2) == "BI" && (iphi == 11 || iphi == 15)) {
                h->Fill(eta_s, phi_s + ",1,R", 0);
                h->Fill(eta_s, phi_s + ",2,R", 0);
                h->Fill(eta_s, phi_s + ",1,M", 0);
                h->Fill(eta_s, phi_s + ",2,M", 0);
            } else {
                h->Fill(eta_s, phi_s + ",1", 0);
                h->Fill(eta_s, phi_s + ",2", 0);
            }
        } else if (iphi % 2 == 0)
            h->Fill(eta_s, phi_s + ",1", 0);
    }  // loop over phi

    h->LabelsDeflate("X");
    h->LabelsDeflate("Y");
    h->Reset();

    return StatusCode::SUCCESS;
}

StatusCode MdtRawDataMonAlg::binMdtGlobal_byLayer(TH2* nHits_In, TH2* nHits_Mid, TH2* nHits_Out) {
    for (int iPhi = 1; iPhi != 17; ++iPhi) {
        TString phiString = "";
        if (iPhi < 10)
            phiString = "0" + std::to_string(iPhi);
        else
            phiString = std::to_string(iPhi);
        TString phiString_ml1 = phiString + ",1";
        TString phiString_ml2 = phiString + ",2";
        if (iPhi == 11 || iPhi == 15) {
            TString phiString_ml1_BIR = phiString + ",1,R";
            TString phiString_ml2_BIR = phiString + ",2,R";
            nHits_In->Fill("EIC5", phiString_ml1_BIR, 0.);
            nHits_In->Fill("EIC5", phiString_ml2_BIR, 0.);
            TString phiString_ml1_BIM = phiString + ",1,M";
            TString phiString_ml2_BIM = phiString + ",2,M";
            nHits_In->Fill("EIC5", phiString_ml1_BIM, 0.);
            nHits_In->Fill("EIC5", phiString_ml2_BIM, 0.);
        } else {
            nHits_In->Fill("EIC5", phiString_ml1, 0.);
            nHits_In->Fill("EIC5", phiString_ml2, 0.);
        }
        nHits_Mid->Fill("EMC5", phiString_ml1, 0.);
        nHits_Mid->Fill("EMC5", phiString_ml2, 0.);
        nHits_Out->Fill("EOC6", phiString_ml1, 0.);
        nHits_Out->Fill("EOC6", phiString_ml2, 0.);
    }
    for (int iEta = 6; iEta != -2; --iEta) {
        TString etaIn = "EIC" + std::to_string(iEta);
        TString etaMid = "EMC" + std::to_string(iEta);
        TString etaOut = "EOC" + std::to_string(iEta);
        if (iEta > 0) {
            if (iEta < 6) {
                nHits_In->Fill(etaIn, "01,1", 0);
                nHits_Mid->Fill(etaMid, "01,1", 0);
            }
            nHits_Out->Fill(etaOut, "01,1", 0);
        } else {
            nHits_In->Fill("", "01,1", 0);
            nHits_Mid->Fill("", "01,1", 0);
            nHits_Out->Fill("", "01,1", 0);
        }
    }
    // Add BIR11/15 separately at a higher eta station

    // BEE, EE chambers on inner plots
    for (int iEta = -4; iEta != 1; ++iEta) {
        TString etaIn = "";
        if (iEta < -2) {
            etaIn = "EEC" + std::to_string(std::abs(iEta + 2));
            nHits_In->Fill(etaIn, "01,1", 0);
        } else if (iEta < 0) {
            etaIn = "BEC" + std::to_string(std::abs(iEta));
            nHits_In->Fill(etaIn, "01,1", 0);
        } else
            nHits_In->Fill("  ", "01,1", 0);
    }

    for (int iEta = -8; iEta != 11; ++iEta) {
        TString etaIn = "";
        TString etaMid = "";
        TString etaOut = "";
        if (iEta < 0) {
            etaIn = "BIC" + std::to_string(std::abs(iEta));
            etaOut = "BOC" + std::to_string(std::abs(iEta));
            if (iEta > -7) { etaMid = "BMC" + std::to_string(std::abs(iEta)); }
        } else if (iEta == 0) {
            etaOut = "BOB" + std::to_string(iEta);
        } else if (iEta < 9) {
            etaIn = "BIA" + std::to_string(iEta);
            etaOut = "BOA" + std::to_string(iEta);
            if (iEta < 7) { etaMid = "BMA" + std::to_string(iEta); }
        }
        if (iEta < 9) {
            nHits_In->Fill(etaIn, "01,1", 0);
            nHits_Mid->Fill(etaMid, "01,1", 0);
            nHits_Out->Fill(etaOut, "01,1", 0);
        } else {
            nHits_In->Fill(" ", "01,1", 0);
            nHits_Mid->Fill(" ", "01,1", 0);
            nHits_Out->Fill(" ", "01,1", 0);
        }
    }

    // BEE, EE chambers on inner plots
    for (int iEta = 1; iEta != 6; ++iEta) {
        TString etaIn = "";
        if (iEta < 3) {
            etaIn = "BEA" + std::to_string(std::abs(iEta));
            nHits_In->Fill(etaIn, "01,1", 0);
        } else if (iEta < 5) {
            etaIn = "EEA" + std::to_string(std::abs(iEta - 2));
            nHits_In->Fill(etaIn, "01,1", 0);
        } else
            nHits_In->Fill("   ", "01,1", 0);
    }

    for (int iEta = 1; iEta != 7; ++iEta) {
        TString etaIn = "EIA" + std::to_string(iEta);
        TString etaMid = "EMA" + std::to_string(iEta);
        TString etaOut = "EOA" + std::to_string(iEta);
        if (iEta < 6) {
            nHits_In->Fill(etaIn, "01,1", 0);
            nHits_Mid->Fill(etaMid, "01,1", 0);
        }
        nHits_Out->Fill(etaOut, "01,1", 0);
    }

    nHits_In->LabelsDeflate("X");
    nHits_In->LabelsDeflate("Y");
    nHits_In->LabelsOption("v", "x");
    nHits_In->Reset();
    nHits_Mid->LabelsDeflate("X");
    nHits_Mid->LabelsDeflate("Y");
    nHits_Mid->LabelsOption("v", "x");
    nHits_Mid->Reset();
    nHits_Out->LabelsDeflate("X");
    nHits_Out->LabelsDeflate("Y");
    nHits_Out->LabelsOption("v", "x");
    nHits_Out->Reset();

    return StatusCode::SUCCESS;
}

// Correct for CutOuts
void MdtRawDataMonAlg::ChamberTubeNumberCorrection(int& tubeNum, std::string_view hardware_name, int tubePos, int numLayers) {
    // numLayers should be mdt_layer-1 so numLayers = 0 implies layer 1 ML 1 or mdt_layer==1
    if (hardware_name.substr(0, 4) == "BMS4" || hardware_name.substr(0, 4) == "BMS6") {  // layer 1-4 tubeId 41-48 cut out
        if (numLayers <= 2) tubeNum = tubePos + numLayers * 48;
    }
    if (hardware_name.substr(0, 3) == "BIR" && numLayers <= 3) {
        if (hardware_name.substr(5, 2) == "11" || hardware_name.substr(5, 2) == "15") {
            if (hardware_name.substr(3, 1) == "1") tubeNum = tubePos + 6 + numLayers * 30;  // layer 1-4 tube id 1-6 cut out
            if (hardware_name.substr(3, 1) == "2") tubeNum = tubePos + numLayers * 30;      // layer 1-4 tube id 28-30 cut out
            if (hardware_name.substr(3, 1) == "4") tubeNum = tubePos + 3 + numLayers * 30;  // layer 1-4 tube id 1-3 cut out
            if (hardware_name.substr(3, 1) == "5") tubeNum = tubePos + numLayers * 24;      // layer 1-4 tube id 22-24 cut out
        }
    }
    if (hardware_name.substr(0, 3) == "BIR" && hardware_name.substr(3, 1) == "3") tubeNum = tubePos + numLayers * 36;  // cut out on both ML
    if (hardware_name == "EEL1A05" || hardware_name == "EEL1C05")
        tubeNum = tubePos + numLayers * 48;  // mdtIdHelper gives wrong #tubes/layer (incidentally also wrong #layers)
}

// Correct for F@#!ing mdtIdHelper
void MdtRawDataMonAlg::CorrectTubeMax(const std::string& hardware_name, int& numTubes) {
    if (hardware_name == "EEL1A05" || hardware_name == "EEL1C05") numTubes = 48;
}

// Correct for F@#!ing mdtIdHelper
void MdtRawDataMonAlg::CorrectLayerMax(const std::string& hardware_name, int& numLayers) {
    if (hardware_name == "EEL1A05" || hardware_name == "EEL1C05") numLayers = 3;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// Private function to select mdt chambersId of the spectrometer

void MdtRawDataMonAlg::mdtchamberId() {
    ATH_MSG_DEBUG("in MDT ChambersIDvector");

    std::vector<Identifier>::const_iterator idfirst = m_idHelperSvc->mdtIdHelper().module_begin();
    std::vector<Identifier>::const_iterator idlast = m_idHelperSvc->mdtIdHelper().module_end();

    IdContext mdtModuleContext = m_idHelperSvc->mdtIdHelper().module_context();
    Identifier Id;
    IdentifierHash Idhash;
    for (std::vector<Identifier>::const_iterator i = idfirst; i != idlast; ++i) {
        Id = *i;
        int gethash_code = m_idHelperSvc->mdtIdHelper().get_hash(Id, Idhash, &mdtModuleContext);
        m_chambersId.push_back(Id);
        m_chambersIdHash.push_back(Idhash);

        std::string extid = m_idHelperSvc->mdtIdHelper().show_to_string(Id);
        ATH_MSG_DEBUG("Adding the chamber Identifier: " << extid);
        if (gethash_code == 0) {
            ATH_MSG_VERBOSE(" its hash Id is " << Idhash);
        } else {
            ATH_MSG_VERBOSE("  hash Id NOT computed " << Idhash);
        }
    }
    m_hist_hash_list.resize(m_chambersIdHash.size());
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
int MdtRawDataMonAlg::mezzmdt(const Identifier& digcoll_id) const {
    int TotmezzTubes = 8;
    if (cachedTubeLayerMax(digcoll_id) == 4) TotmezzTubes = 6;
    int Imezz = (int)((m_idHelperSvc->mdtIdHelper().tube(digcoll_id) - 1) / TotmezzTubes) +
                (int)((m_idHelperSvc->mdtIdHelper().multilayer(digcoll_id) - 1) * ((cachedTubeMax(digcoll_id)) / TotmezzTubes));

    return Imezz;
}

// Get the Maximum # of tubes in the chamber
// the 'if' statements are for chambers with ML1 != ML2
// except for BIS8 -- mdtIdHelper gets the # layers wrong in this instance
int MdtRawDataMonAlg::GetTubeMax(const Identifier& digcoll_id, std::string_view hardware_name) {
    int tubeMax(0);
    if (hardware_name.substr(0, 4) == "BIS8") {  // Why does mdtIdHelper get this one wrong?
        tubeMax = 16 * 3;
    } else if (hardware_name.substr(0, 4) == "BIR5") {
        //     tubeMax = 21*4 + 24*4;
        tubeMax = 24 * 4 + 24 * 4;
    } else if (hardware_name.substr(0, 4) == "BIR2" || hardware_name.substr(0, 4) == "BIR4") {
        //     tubeMax = 27*4 + 30*4;
        tubeMax = 30 * 4 + 30 * 4;
    } else if (hardware_name.substr(0, 4) == "BIR3") {
        tubeMax = 36 * 4 + 36 * 4;
    } else if (hardware_name.substr(0, 4) == "BIR1") {
        //     tubeMax = 24*4 + 30*4;
        tubeMax = 30 * 4 + 30 * 4;
    } else if (hardware_name.substr(0, 4) == "BMS4" || hardware_name.substr(0, 4) == "BMS6") {
        //     tubeMax = 40*3 + 48*3;
        tubeMax = 48 * 3 + 48 * 3;
    } else if (hardware_name == "EEL1A05" || hardware_name == "EEL1C05") {
        tubeMax = 48 * 3 + 48 * 3;
    } else if (hardware_name.substr(0, 3) == "BME") {
        tubeMax = 546;
    } else {
        int numtubes = m_idHelperSvc->mdtIdHelper().tubeMax(digcoll_id);
        int numlayers = m_idHelperSvc->mdtIdHelper().tubeLayerMax(digcoll_id);
        int numML = m_idHelperSvc->mdtIdHelper().numberOfMultilayers(digcoll_id);
        tubeMax = numtubes * numlayers * numML;
    }
    return tubeMax;
}

bool MdtRawDataMonAlg::AinB(int A, std::vector<int>& B) {
    for (int i : B) {
        if (i == A) return true;
    }
    return false;
}

// StatusCode MdtRawDataMonAlg::GetTimingInfo() {
//   m_time = -1;

//   SG::ReadHandle<xAOD::EventInfo> evt(m_eventInfo);
//   m_time = (1000000000L*(uint64_t)evt->timeStamp()+evt->timeStampNSOffset()) * 0.000000001;

//   // protection against simulated cosmics when the trigger_info() of the event_info is not filled and returns a null pointer.
//   //trigtype = eventInfo->level1TriggerType();

void MdtRawDataMonAlg::setIsATLASReady(const EventContext& ctx) {
    bool filterresult(true);
    if (!m_DQFilterTools.empty()) {
        ToolHandleArray<IDQFilterTool>::const_iterator ifilter(m_DQFilterTools.begin()), filterend(m_DQFilterTools.end());
        for (; filterresult && (ifilter != filterend); ++ifilter) { filterresult = (filterresult && (*ifilter)->accept(ctx)); }
    }
    m_atlas_ready = filterresult;
}

std::string MdtRawDataMonAlg::getChamberName(const Muon::MdtPrepData* hit) const { return getChamberName(hit->identify()); }

std::string MdtRawDataMonAlg::getChamberName(const Identifier& id) const {
    IdentifierHash idHash{0};
    const MdtIdHelper& id_helper = m_idHelperSvc->mdtIdHelper();
    id_helper.get_module_hash(id, idHash);
    MDTChamber* chamber{nullptr};
    if (getChamber(idHash, chamber).isSuccess()) return chamber->getName();
    return convertChamberName(id_helper.stationName(id), id_helper.stationEta(id), id_helper.stationPhi(id), "MDT");
}

StatusCode MdtRawDataMonAlg::getChamber(const IdentifierHash& id, MDTChamber*& chamber) const {
    if (id >= m_hist_hash_list.size()) return StatusCode::FAILURE;
    chamber = m_hist_hash_list[id].get();
    if (!chamber) return StatusCode::FAILURE;

    return StatusCode::SUCCESS;
}

int MdtRawDataMonAlg::get_bin_for_LB_hist(int region, int layer, int phi, int eta, bool isBIM) const {
    if (region == 0 || region == 1) {  // Barrel

        if (layer == 0) {  // Inner
            if (eta < 6) {
                if (phi < 11 && !isBIM)
                    return 18 * (eta - 1) + phi;
                else if ((phi == 10 && isBIM) || (phi < 15 && !isBIM))
                    return 18 * (eta - 1) + phi + 1;
                else
                    return 18 * (eta - 1) + phi + 2;
            } else if (eta == 6)
                return 90 + phi;
            else if (eta == 7)
                return 106 + (phi / 2);
            else if (eta == 8)
                return 114 + (phi / 2);
        }

        else if (layer == 1) {  // Middle
            // 95 = 1 + 16 + 16 + 16 + 16 + 16 + 14   total number of phi sectors (+1)
            // in the last eta-sector (6) there is no phi-sector 13; ie there are no chambers BML6A13 and BML6C13, so there are only 14 phi
            // sectos we move the bin of phi=14 directly above phi=12 so there is no white line in the histogram
            if (eta == 6 && phi > 11)
                return 16 * (eta - 1) + phi - 1;
            else
                return 16 * (eta - 1) + phi;

        } else if (layer == 2 && region == 0) {  // Outer, side A (must be separated due to presence of eta=0 chambers)
            if (eta == 0 && phi == 11)
                return 0;
            else if (eta == 0 && phi == 13)
                return 1;
            else if (eta < 7)
                return 16 * (eta - 1) + phi + 2;
            else if (eta == 7 && phi == 11)
                return 98;
            else if (eta == 7 && phi == 13)
                return 99;
            else if (eta == 8 && phi == 11)
                return 100;
            else if (eta == 8 && phi == 13)
                return 101;
        } else if (layer == 3 && region == 0) {  // Extra: put with outer
            return 102 + 8 * (eta - 1) + (phi - 1) / 2;
        } else if (layer == 2 && region == 1) {  // Outer, side C
            if (eta < 7)
                return 16 * (eta - 1) + phi;
            else if (eta == 7 && phi == 11)
                return 96;
            else if (eta == 7 && phi == 13)
                return 97;
            else if (eta == 8 && phi == 11)
                return 98;
            else if (eta == 8 && phi == 13)
                return 99;
        } else if (layer == 3 && region == 1) {  // Extra: put with outer
            return 100 + 8 * (eta - 1) + (phi - 1) / 2;
        }

    }

    else {                 // Endcap
        if (layer == 0) {  // Inner
            int run3_offset = 0;
            if (m_do_run3Geometry) run3_offset = 40;
            if (eta < 3)
                return 16 * (eta - 1) + phi - run3_offset;
            else if (eta == 3)
                return 32 + phi / 2 - run3_offset;
            else if (eta == 4)
                return 40 + phi / 2 - run3_offset;
            else if (eta == 5)
                return 48 + phi / 8 - run3_offset;

        } else if (layer == 1) {  // Middle
            return 16 * (eta - 1) + phi;
        } else if (layer == 2) {  // Outer
            return 16 * (eta - 1) + phi;
        } else if (layer == 3 && region == 2) {  // Extra A: put with outer. Asymmetry between A and C
            if (eta == 1 && phi == 2) return 96;
            if (eta == 1 && phi == 4) return 97;
            if (eta == 1 && phi == 10) return 98;
            if (eta == 1 && phi == 12) return 99;
            if (eta == 2 && phi == 2) return 100;
            if (eta == 2 && phi == 10) return 101;
            if (eta == 2 && phi == 12) return 102;
        } else if (layer == 3 && region == 3) {  // Extra C: put with outer.
            if (eta == 1 || phi < 5)
                return 80 + 16 * eta + phi;
            else
                return 79 + 16 * eta + phi;  //(missing eta = 2, phi = 5)
        }
    }

    return -1;
}

StatusCode MdtRawDataMonAlg::binMdtOccVsLB(TH2*& h, int region, int layer) {
    if (region == 0 || region == 1) {  // Barrel
        if (layer == 0) {              // Inner
            // Add Labels
            h->SetBins(834, 1, 2502, 122, 0, 122);
            h->GetYaxis()->SetBinLabel(1, "BI1");
            h->GetYaxis()->SetBinLabel(19, "BI2");
            h->GetYaxis()->SetBinLabel(37, "BI3");
            h->GetYaxis()->SetBinLabel(55, "BI4");
            h->GetYaxis()->SetBinLabel(73, "BI5");
            h->GetYaxis()->SetBinLabel(91, "BI6");
            h->GetYaxis()->SetBinLabel(107, "BI7");
            h->GetYaxis()->SetBinLabel(115, "BI8");
            // Add lines
            h->GetYaxis()->SetTickLength(0);
            for (int i = 0; i < 6; i++) {
                TLine* l = new TLine(1, 18 * i, 50, 18 * i);
                h->GetListOfFunctions()->Add(l);
            }
            TLine* l2 = new TLine(1, 106, 50, 106);
            h->GetListOfFunctions()->Add(l2);
            TLine* l3 = new TLine(1, 114, 50, 114);
            h->GetListOfFunctions()->Add(l3);
            for (int i = 1; i < 122; i++) {
                if (i < 90 && (i % 18 == 11 || i % 18 == 16)) {
                    TLine* l = new TLine(1, i, 10, i);
                    h->GetListOfFunctions()->Add(l);
                } else {
                    TLine* l = new TLine(1, i, 20, i);
                    h->GetListOfFunctions()->Add(l);
                }
            }
        }

        else if (layer == 1) {                    // Middle
            h->SetBins(834, 1, 2502, 95, 0, 95);  // 95 = 1 + 16 + 16 + 16 + 16 + 16 + 14   total number of phi sectors (+1)
                                                  // in the last eta-sector (6) there is no phi-sector 13; ie there arent chambers BML6A13
                                                  // and BML6C13 so there are only 14 phi sectors
            // Add Labels
            h->GetYaxis()->SetBinLabel(1, "BM1");
            h->GetYaxis()->SetBinLabel(17, "BM2");
            h->GetYaxis()->SetBinLabel(33, "BM3");
            h->GetYaxis()->SetBinLabel(49, "BM4");
            h->GetYaxis()->SetBinLabel(65, "BM5");
            h->GetYaxis()->SetBinLabel(81, "BM6");
            // Add lines
            h->GetYaxis()->SetTickLength(0);
            for (int i = 1; i < 95; i++) {
                TLine* l = new TLine(1, i, 20, i);
                h->GetListOfFunctions()->Add(l);
            }
            TLine* l1 = new TLine(1, 16, 50, 16);
            h->GetListOfFunctions()->Add(l1);
            TLine* l2 = new TLine(1, 32, 50, 32);
            h->GetListOfFunctions()->Add(l2);
            TLine* l3 = new TLine(1, 48, 50, 48);
            h->GetListOfFunctions()->Add(l3);
            TLine* l4 = new TLine(1, 64, 50, 64);
            h->GetListOfFunctions()->Add(l4);
            TLine* l5 = new TLine(1, 80, 50, 80);
            h->GetListOfFunctions()->Add(l5);
        }

        else if (layer == 2 && region == 0) {  // Outer, side A
            h->SetBins(834, 1, 2502, 118, 0, 118);
            // Add labels
            h->GetYaxis()->SetBinLabel(1, "BO0");
            h->GetYaxis()->SetBinLabel(4, "BO1");
            h->GetYaxis()->SetBinLabel(19, "BO2");
            h->GetYaxis()->SetBinLabel(35, "BO3");
            h->GetYaxis()->SetBinLabel(51, "BO4");
            h->GetYaxis()->SetBinLabel(67, "BO5");
            h->GetYaxis()->SetBinLabel(83, "BO6");
            h->GetYaxis()->SetBinLabel(99, "BO7,8");
            // h->GetYaxis()->SetBinLabel(101,"BO8");
            h->GetYaxis()->SetBinLabel(103, "BE1");
            h->GetYaxis()->SetBinLabel(111, "BE2");
            // Add lines
            h->GetYaxis()->SetTickLength(0);
            for (int i = 1; i < 118; i++) {
                TLine* l = new TLine(1, i, 20, i);
                h->GetListOfFunctions()->Add(l);
            }
            TLine* l1 = new TLine(1, 2, 50, 2);
            h->GetListOfFunctions()->Add(l1);
            TLine* l2 = new TLine(1, 18, 50, 18);
            h->GetListOfFunctions()->Add(l2);
            TLine* l3 = new TLine(1, 34, 50, 34);
            h->GetListOfFunctions()->Add(l3);
            TLine* l4 = new TLine(1, 50, 50, 50);
            h->GetListOfFunctions()->Add(l4);
            TLine* l5 = new TLine(1, 66, 50, 66);
            h->GetListOfFunctions()->Add(l5);
            TLine* l6 = new TLine(1, 82, 50, 82);
            h->GetListOfFunctions()->Add(l6);
            TLine* l7 = new TLine(1, 98, 50, 98);
            h->GetListOfFunctions()->Add(l7);
            TLine* l8 = new TLine(1, 100, 50, 100);
            h->GetListOfFunctions()->Add(l8);
            TLine* l9 = new TLine(1, 102, 50, 102);
            h->GetListOfFunctions()->Add(l9);
            TLine* l10 = new TLine(1, 110, 50, 110);
            h->GetListOfFunctions()->Add(l10);
        }

        else if (layer == 2 && region == 1) {  // Outer, side C (no eta = 0)
            h->SetBins(834, 1, 2502, 116, 0, 116);
            // Add labels
            h->GetYaxis()->SetBinLabel(1, "BO1");
            h->GetYaxis()->SetBinLabel(17, "BO2");
            h->GetYaxis()->SetBinLabel(33, "BO3");
            h->GetYaxis()->SetBinLabel(49, "BO4");
            h->GetYaxis()->SetBinLabel(65, "BO5");
            h->GetYaxis()->SetBinLabel(81, "BO6");
            h->GetYaxis()->SetBinLabel(97, "BO7,8");
            //      h->GetYaxis()->SetBinLabel(101,"BO8");
            h->GetYaxis()->SetBinLabel(101, "BE1");
            h->GetYaxis()->SetBinLabel(109, "BE2");
            // Add lines
            h->GetYaxis()->SetTickLength(0);
            for (int i = 1; i < 116; i++) {
                TLine* l = new TLine(1, i, 20, i);
                h->GetListOfFunctions()->Add(l);
            }
            TLine* l1 = new TLine(1, 16, 50, 16);
            h->GetListOfFunctions()->Add(l1);
            TLine* l2 = new TLine(1, 32, 50, 32);
            h->GetListOfFunctions()->Add(l2);
            TLine* l3 = new TLine(1, 48, 50, 48);
            h->GetListOfFunctions()->Add(l3);
            TLine* l4 = new TLine(1, 64, 50, 64);
            h->GetListOfFunctions()->Add(l4);
            TLine* l5 = new TLine(1, 80, 50, 80);
            h->GetListOfFunctions()->Add(l5);
            TLine* l6 = new TLine(1, 96, 50, 96);
            h->GetListOfFunctions()->Add(l6);
            TLine* l7 = new TLine(1, 98, 50, 98);
            h->GetListOfFunctions()->Add(l7);
            TLine* l8 = new TLine(1, 100, 50, 100);
            h->GetListOfFunctions()->Add(l8);
            TLine* l9 = new TLine(1, 108, 50, 108);
            h->GetListOfFunctions()->Add(l9);
        }
    }

    else {                 // Endcap
        if (layer == 0) {  // Inner
            h->SetBins(834, 1, 2502, 50, 0, 50);
            // Add labels
            h->GetYaxis()->SetBinLabel(1, "EI1");
            h->GetYaxis()->SetBinLabel(17, "EI2");
            h->GetYaxis()->SetBinLabel(33, "EI3");
            h->GetYaxis()->SetBinLabel(41, "EI4");
            h->GetYaxis()->SetBinLabel(49, "EI5");
            // Add lines
            h->GetYaxis()->SetTickLength(0);
            for (int i = 1; i < 50; i++) {
                TLine* l = new TLine(1, i, 20, i);
                h->GetListOfFunctions()->Add(l);
            }
            TLine* l1 = new TLine(1, 16, 50, 16);
            h->GetListOfFunctions()->Add(l1);
            TLine* l2 = new TLine(1, 32, 50, 32);
            h->GetListOfFunctions()->Add(l2);
            TLine* l3 = new TLine(1, 40, 50, 40);
            h->GetListOfFunctions()->Add(l3);
            TLine* l4 = new TLine(1, 48, 50, 48);
            h->GetListOfFunctions()->Add(l4);
        }

        else if (layer == 1) {  // Middle
            h->SetBins(834, 1, 2502, 80, 0, 80);
            // Add labels
            h->GetYaxis()->SetBinLabel(1, "EM1");
            h->GetYaxis()->SetBinLabel(17, "EM2");
            h->GetYaxis()->SetBinLabel(33, "EM3");
            h->GetYaxis()->SetBinLabel(49, "EM4");
            h->GetYaxis()->SetBinLabel(65, "EM5");
            // Add lines
            h->GetYaxis()->SetTickLength(0);
            for (int i = 1; i < 80; i++) {
                TLine* l = new TLine(1, i, 20, i);
                h->GetListOfFunctions()->Add(l);
            }
            TLine* l1 = new TLine(1, 16, 50, 16);
            h->GetListOfFunctions()->Add(l1);
            TLine* l2 = new TLine(1, 32, 50, 32);
            h->GetListOfFunctions()->Add(l2);
            TLine* l3 = new TLine(1, 48, 50, 48);
            h->GetListOfFunctions()->Add(l3);
            TLine* l4 = new TLine(1, 64, 50, 64);
            h->GetListOfFunctions()->Add(l4);
        }

        else if (layer == 2) {  // Outer + extra

            if (region == 2) {  // side A
                h->SetBins(834, 1, 2502, 103, 0, 103);
                // Add labels
                h->GetYaxis()->SetBinLabel(1, "EO1");
                h->GetYaxis()->SetBinLabel(17, "EO2");
                h->GetYaxis()->SetBinLabel(33, "EO3");
                h->GetYaxis()->SetBinLabel(49, "EO4");
                h->GetYaxis()->SetBinLabel(65, "EO5");
                h->GetYaxis()->SetBinLabel(81, "EO6");
                h->GetYaxis()->SetBinLabel(97, "EE1,2");
                // Add lines
                h->GetYaxis()->SetTickLength(0);
                for (int i = 1; i < 103; i++) {
                    TLine* l = new TLine(1, i, 20, i);
                    h->GetListOfFunctions()->Add(l);
                }
                TLine* l1 = new TLine(1, 16, 50, 16);
                h->GetListOfFunctions()->Add(l1);
                TLine* l2 = new TLine(1, 32, 50, 32);
                h->GetListOfFunctions()->Add(l2);
                TLine* l3 = new TLine(1, 48, 50, 48);
                h->GetListOfFunctions()->Add(l3);
                TLine* l4 = new TLine(1, 64, 50, 64);
                h->GetListOfFunctions()->Add(l4);
                TLine* l5 = new TLine(1, 80, 50, 80);
                h->GetListOfFunctions()->Add(l5);
                TLine* l6 = new TLine(1, 96, 50, 96);
                h->GetListOfFunctions()->Add(l6);
                TLine* l7 = new TLine(1, 100, 50, 100);
                h->GetListOfFunctions()->Add(l7);
            } else if (region == 3) {  // side C
                h->SetBins(834, 1, 2502, 127, 0, 127);
                // Add labels
                h->GetYaxis()->SetBinLabel(1, "EO1");
                h->GetYaxis()->SetBinLabel(17, "EO2");
                h->GetYaxis()->SetBinLabel(33, "EO3");
                h->GetYaxis()->SetBinLabel(49, "EO4");
                h->GetYaxis()->SetBinLabel(65, "EO5");
                h->GetYaxis()->SetBinLabel(81, "EO6");
                h->GetYaxis()->SetBinLabel(97, "EE1");
                h->GetYaxis()->SetBinLabel(113, "EE2");
                // Add lines
                h->GetYaxis()->SetTickLength(0);
                for (int i = 1; i < 127; i++) {
                    TLine* l = new TLine(1, i, 20, i);
                    h->GetListOfFunctions()->Add(l);
                }
                TLine* l1 = new TLine(1, 16, 50, 16);
                h->GetListOfFunctions()->Add(l1);
                TLine* l2 = new TLine(1, 32, 50, 32);
                h->GetListOfFunctions()->Add(l2);
                TLine* l3 = new TLine(1, 48, 50, 48);
                h->GetListOfFunctions()->Add(l3);
                TLine* l4 = new TLine(1, 64, 50, 64);
                h->GetListOfFunctions()->Add(l4);
                TLine* l5 = new TLine(1, 80, 50, 80);
                h->GetListOfFunctions()->Add(l5);
                TLine* l6 = new TLine(1, 96, 50, 96);
                h->GetListOfFunctions()->Add(l6);
                TLine* l7 = new TLine(1, 112, 50, 112);
                h->GetListOfFunctions()->Add(l7);
            }
        }
    }

    return StatusCode::SUCCESS;
}

StatusCode MdtRawDataMonAlg::binMdtOccVsLB_Crate(TH2*& h, int region, int crate) {
    if (region == 0 || region == 1) {  // Barrel
        if (crate == 0) {              // Crate BA01 or BC01
            // Add Labels
            h->SetBins(834, 1, 2502, 73, 0, 73);
            h->GetYaxis()->SetBinLabel(1, "BIL");
            h->GetYaxis()->SetBinLabel(13, "BIS");
            h->GetYaxis()->SetBinLabel(25, "BME");
            h->GetYaxis()->SetBinLabel(26, "BML");
            h->GetYaxis()->SetBinLabel(38, "BMS");
            h->GetYaxis()->SetBinLabel(50, "BOL");
            h->GetYaxis()->SetBinLabel(62, "BOS");
            // Add lines
            h->GetYaxis()->SetTickLength(0);
            for (int i = 0; i < 73; i++) {
                TLine* l = new TLine(1, i, 20, i);
                h->GetListOfFunctions()->Add(l);
            }
            TLine* l2 = new TLine(1, 0, 50, 0);
            h->GetListOfFunctions()->Add(l2);
            TLine* l3 = new TLine(1, 12, 50, 12);
            h->GetListOfFunctions()->Add(l3);
            TLine* l4 = new TLine(1, 24, 50, 24);
            h->GetListOfFunctions()->Add(l4);
            TLine* l5 = new TLine(1, 25, 50, 25);
            h->GetListOfFunctions()->Add(l5);
            TLine* l6 = new TLine(1, 37, 50, 37);
            h->GetListOfFunctions()->Add(l6);
            TLine* l8 = new TLine(1, 49, 50, 49);
            h->GetListOfFunctions()->Add(l8);
            TLine* l9 = new TLine(1, 61, 50, 61);
            h->GetListOfFunctions()->Add(l9);
        }

        else if (crate == 1) {  // BA02, BC02
            h->SetBins(834, 1, 2502, 73, 0, 73);
            // Add Labels
            h->GetYaxis()->SetBinLabel(1, "BIL");
            h->GetYaxis()->SetBinLabel(13, "BIS");
            h->GetYaxis()->SetBinLabel(25, "BME");
            h->GetYaxis()->SetBinLabel(37, "BMS");
            h->GetYaxis()->SetBinLabel(49, "BOL");
            h->GetYaxis()->SetBinLabel(61, "BOS");
            // Add lines
            h->GetYaxis()->SetTickLength(0);
            for (int i = 1; i < 73; i++) {
                TLine* l = new TLine(1, i, 20, i);
                h->GetListOfFunctions()->Add(l);
            }
            TLine* l1 = new TLine(1, 12, 50, 12);
            h->GetListOfFunctions()->Add(l1);
            TLine* l2 = new TLine(1, 24, 50, 24);
            h->GetListOfFunctions()->Add(l2);
            TLine* l3 = new TLine(1, 36, 50, 36);
            h->GetListOfFunctions()->Add(l3);
            TLine* l4 = new TLine(1, 48, 50, 48);
            h->GetListOfFunctions()->Add(l4);
            TLine* l6 = new TLine(1, 60, 50, 60);
            h->GetListOfFunctions()->Add(l6);

        } else if (crate == 2) {  // BA03, BC03
            // Add Labels
            h->SetBins(834, 1, 2502, 80, 0, 80);
            h->GetYaxis()->SetBinLabel(1, "BIL");
            h->GetYaxis()->SetBinLabel(7, "BIM");
            h->GetYaxis()->SetBinLabel(12, "BIR");
            h->GetYaxis()->SetBinLabel(18, "BIS");
            h->GetYaxis()->SetBinLabel(30, "BMF");
            h->GetYaxis()->SetBinLabel(33, "BMG");
            h->GetYaxis()->SetBinLabel(36, "BML");
            h->GetYaxis()->SetBinLabel(48, "BMS");
            h->GetYaxis()->SetBinLabel(54, "BOF");
            h->GetYaxis()->SetBinLabel(58, "BOG");
            if (region == 0) {
                h->GetYaxis()->SetBinLabel(63, "BOL");
                h->GetYaxis()->SetBinLabel(75, "BOS");
            } else if (region == 1) {
                h->GetYaxis()->SetBinLabel(62, "BOL");
                h->GetYaxis()->SetBinLabel(74, "BOS");
            }
            // Add lines
            h->GetYaxis()->SetTickLength(0);
            for (int i = 0; i < 80; i++) {
                TLine* l = new TLine(1, i, 20, i);
                h->GetListOfFunctions()->Add(l);
            }  ////change line positions!
            TLine* l2 = new TLine(1, 6, 50, 6);
            h->GetListOfFunctions()->Add(l2);
            TLine* l3 = new TLine(1, 11, 50, 11);
            h->GetListOfFunctions()->Add(l3);
            TLine* l3b = new TLine(1, 17, 50, 17);
            h->GetListOfFunctions()->Add(l3b);
            // TLine* l4 = new TLine(1,26,50,26);  h->GetListOfFunctions()->Add(l4); //removed this line because it doesnt correspond to
            // anything
            TLine* l5 = new TLine(1, 29, 50, 29);
            h->GetListOfFunctions()->Add(l5);
            TLine* l6 = new TLine(1, 32, 50, 32);
            h->GetListOfFunctions()->Add(l6);
            TLine* l7 = new TLine(1, 35, 50, 35);
            h->GetListOfFunctions()->Add(l7);
            TLine* l8 = new TLine(1, 47, 50, 47);
            h->GetListOfFunctions()->Add(l8);
            TLine* l9 = new TLine(1, 53, 50, 53);
            h->GetListOfFunctions()->Add(l9);
            TLine* l9b = new TLine(1, 57, 50, 57);
            h->GetListOfFunctions()->Add(l9b);
            if (region == 0) {
                TLine* l10 = new TLine(1, 62, 50, 62);
                h->GetListOfFunctions()->Add(l10);
                TLine* l11 = new TLine(1, 74, 50, 74);
                h->GetListOfFunctions()->Add(l11);
            } else if (region == 1) {
                TLine* l10 = new TLine(1, 61, 50, 61);
                h->GetListOfFunctions()->Add(l10);
                TLine* l11 = new TLine(1, 73, 50, 73);
                h->GetListOfFunctions()->Add(l11);
            }
        }

        else if (crate == 3) {  // BA04, BC04
            // Add Labels
            h->SetBins(834, 1, 2502, 79, 0, 79);
            h->GetYaxis()->SetBinLabel(1, "BIL");
            h->GetYaxis()->SetBinLabel(7, "BIM");
            h->GetYaxis()->SetBinLabel(12, "BIR");
            h->GetYaxis()->SetBinLabel(18, "BIS");
            h->GetYaxis()->SetBinLabel(30, "BMF");
            h->GetYaxis()->SetBinLabel(33, "BMG");
            h->GetYaxis()->SetBinLabel(36, "BML");
            h->GetYaxis()->SetBinLabel(47, "BMS");
            h->GetYaxis()->SetBinLabel(53, "BOF");
            h->GetYaxis()->SetBinLabel(57, "BOG");
            if (region == 0) {
                h->GetYaxis()->SetBinLabel(62, "BOL");
                h->GetYaxis()->SetBinLabel(75, "BOS");
            } else if (region == 1) {
                h->GetYaxis()->SetBinLabel(61, "BOL");
                h->GetYaxis()->SetBinLabel(74, "BOS");
            }

            // Add lines
            h->GetYaxis()->SetTickLength(0);
            for (int i = 0; i < 79; i++) {
                TLine* l = new TLine(1, i, 20, i);
                h->GetListOfFunctions()->Add(l);
            }
            TLine* l2 = new TLine(1, 6, 50, 6);
            h->GetListOfFunctions()->Add(l2);
            TLine* l3 = new TLine(1, 11, 50, 11);
            h->GetListOfFunctions()->Add(l3);
            TLine* l3b = new TLine(1, 17, 50, 17);
            h->GetListOfFunctions()->Add(l3b);
            // TLine* l4 = new TLine(1,26,50,26);  h->GetListOfFunctions()->Add(l4);//removed this line because it doesnt correspond to
            // anything
            TLine* l5 = new TLine(1, 29, 50, 29);
            h->GetListOfFunctions()->Add(l5);
            TLine* l6 = new TLine(1, 32, 50, 32);
            h->GetListOfFunctions()->Add(l6);
            TLine* l7 = new TLine(1, 35, 50, 35);
            h->GetListOfFunctions()->Add(l7);
            TLine* l8 = new TLine(1, 46, 50, 46);
            h->GetListOfFunctions()->Add(l8);
            TLine* l9 = new TLine(1, 52, 50, 52);
            h->GetListOfFunctions()->Add(l9);
            TLine* l9b = new TLine(1, 56, 50, 56);
            h->GetListOfFunctions()->Add(l9b);
            if (region == 0) {
                TLine* l10 = new TLine(1, 61, 50, 61);
                h->GetListOfFunctions()->Add(l10);
                TLine* l11 = new TLine(1, 74, 50, 74);
                h->GetListOfFunctions()->Add(l11);
            } else if (region == 1) {
                TLine* l10 = new TLine(1, 60, 50, 60);
                h->GetListOfFunctions()->Add(l10);
                TLine* l11 = new TLine(1, 73, 50, 73);
                h->GetListOfFunctions()->Add(l11);
            }
        }
    }

    else {                               // Endcap
        if (crate == 0 || crate == 2) {  // EA01, EC01 and EA03, EC03 are the same
            h->SetBins(834, 1, 2502, 73, 0, 73);
            // Add labels
            h->GetYaxis()->SetBinLabel(1, "BEE");
            h->GetYaxis()->SetBinLabel(5, "BIS");
            h->GetYaxis()->SetBinLabel(9, "EEL");
            h->GetYaxis()->SetBinLabel(13, "EES");
            h->GetYaxis()->SetBinLabel(17, "EIL");
            h->GetYaxis()->SetBinLabel(26, "EIS");
            h->GetYaxis()->SetBinLabel(30, "EML");
            h->GetYaxis()->SetBinLabel(40, "EMS");
            h->GetYaxis()->SetBinLabel(50, "EOL");
            h->GetYaxis()->SetBinLabel(62, "EOS");
            // Add lines
            h->GetYaxis()->SetTickLength(0);
            for (int i = 1; i < 73; i++) {
                TLine* l = new TLine(1, i, 20, i);
                h->GetListOfFunctions()->Add(l);
            }
            TLine* l2 = new TLine(1, 4, 50, 4);
            h->GetListOfFunctions()->Add(l2);
            TLine* l3 = new TLine(1, 8, 50, 8);
            h->GetListOfFunctions()->Add(l3);
            TLine* l4 = new TLine(1, 12, 50, 12);
            h->GetListOfFunctions()->Add(l4);
            TLine* l5 = new TLine(1, 16, 50, 16);
            h->GetListOfFunctions()->Add(l5);
            TLine* l6 = new TLine(1, 25, 50, 25);
            h->GetListOfFunctions()->Add(l6);
            TLine* l7 = new TLine(1, 29, 50, 29);
            h->GetListOfFunctions()->Add(l7);
            TLine* l8 = new TLine(1, 39, 50, 39);
            h->GetListOfFunctions()->Add(l8);
            TLine* l9 = new TLine(1, 49, 50, 49);
            h->GetListOfFunctions()->Add(l9);
            TLine* l10 = new TLine(1, 61, 50, 61);
            h->GetListOfFunctions()->Add(l10);
        } else if (crate == 1) {  // EA02, EC02
            h->SetBins(834, 1, 2502, 71, 0, 71);
            // Add labels
            h->GetYaxis()->SetBinLabel(1, "BEE");
            h->GetYaxis()->SetBinLabel(5, "BIS");
            h->GetYaxis()->SetBinLabel(9, "EEL");
            h->GetYaxis()->SetBinLabel(12, "EES");
            h->GetYaxis()->SetBinLabel(16, "EIL");
            h->GetYaxis()->SetBinLabel(24, "EIS");
            h->GetYaxis()->SetBinLabel(28, "EML");
            h->GetYaxis()->SetBinLabel(38, "EMS");
            h->GetYaxis()->SetBinLabel(48, "EOL");
            h->GetYaxis()->SetBinLabel(60, "EOS");
            // Add lines
            h->GetYaxis()->SetTickLength(0);
            for (int i = 1; i < 71; i++) {
                TLine* l = new TLine(1, i, 20, i);
                h->GetListOfFunctions()->Add(l);
            }
            TLine* l2 = new TLine(1, 4, 50, 4);
            h->GetListOfFunctions()->Add(l2);
            TLine* l3 = new TLine(1, 8, 50, 8);
            h->GetListOfFunctions()->Add(l3);
            TLine* l4 = new TLine(1, 11, 50, 11);
            h->GetListOfFunctions()->Add(l4);
            TLine* l5 = new TLine(1, 15, 50, 15);
            h->GetListOfFunctions()->Add(l5);
            TLine* l6 = new TLine(1, 23, 50, 23);
            h->GetListOfFunctions()->Add(l6);
            TLine* l7 = new TLine(1, 27, 50, 27);
            h->GetListOfFunctions()->Add(l7);
            TLine* l8 = new TLine(1, 37, 50, 37);
            h->GetListOfFunctions()->Add(l8);
            TLine* l9 = new TLine(1, 47, 50, 47);
            h->GetListOfFunctions()->Add(l9);
            TLine* l10 = new TLine(1, 59, 50, 59);
            h->GetListOfFunctions()->Add(l10);
        } else if (crate == 3) {  // EA04, EC04
            h->SetBins(834, 1, 2502, 72, 0, 72);
            // Add labels
            h->GetYaxis()->SetBinLabel(1, "BEE");
            h->GetYaxis()->SetBinLabel(5, "BIS");
            h->GetYaxis()->SetBinLabel(9, "EEL");
            h->GetYaxis()->SetBinLabel(13, "EES");
            h->GetYaxis()->SetBinLabel(17, "EIL");
            h->GetYaxis()->SetBinLabel(25, "EIS");
            h->GetYaxis()->SetBinLabel(29, "EML");
            h->GetYaxis()->SetBinLabel(39, "EMS");
            h->GetYaxis()->SetBinLabel(49, "EOL");
            h->GetYaxis()->SetBinLabel(61, "EOS");
            // Add lines
            h->GetYaxis()->SetTickLength(0);
            for (int i = 1; i < 72; i++) {
                TLine* l = new TLine(1, i, 20, i);
                h->GetListOfFunctions()->Add(l);
            }
            TLine* l2 = new TLine(1, 4, 50, 4);
            h->GetListOfFunctions()->Add(l2);
            TLine* l3 = new TLine(1, 8, 50, 8);
            h->GetListOfFunctions()->Add(l3);
            TLine* l4 = new TLine(1, 12, 50, 12);
            h->GetListOfFunctions()->Add(l4);
            TLine* l5 = new TLine(1, 16, 50, 16);
            h->GetListOfFunctions()->Add(l5);
            TLine* l6 = new TLine(1, 24, 50, 24);
            h->GetListOfFunctions()->Add(l6);
            TLine* l7 = new TLine(1, 28, 50, 28);
            h->GetListOfFunctions()->Add(l7);
            TLine* l8 = new TLine(1, 38, 50, 38);
            h->GetListOfFunctions()->Add(l8);
            TLine* l9 = new TLine(1, 48, 50, 48);
            h->GetListOfFunctions()->Add(l9);
            TLine* l10 = new TLine(1, 60, 50, 60);
            h->GetListOfFunctions()->Add(l10);
        }
    }

    return StatusCode::SUCCESS;
}

int MdtRawDataMonAlg::get_bin_for_LB_crate_hist(int region, int crate, int phi, int eta, std::string_view chamber) const {
    int binNum = 999;
    std::string_view stName = chamber.substr(0, 3);
    if (region == 0 || region == 1) {  // Barrel
        if (crate == 1) {              // BA01, BC01
            if (stName == "BIL")
                binNum = 2 * eta + (phi - 1) / 2 - 1;
            else if (stName == "BIS")
                binNum = 12 + 2 * eta + (phi - 2) / 2 - 1;
            else if (stName == "BME")
                binNum = 25;
            else if (stName == "BML")
                binNum = 25 + 2 * eta + (phi - 1) / 2 - 1;
            else if (stName == "BMS")
                binNum = 37 + 2 * eta + (phi - 2) / 2 - 1;
            else if (stName == "BOL")
                binNum = 49 + 2 * eta + (phi - 1) / 2 - 1;
            else if (stName == "BOS")
                binNum = 61 + 2 * eta + (phi - 2) / 2 - 1;
            else
                ATH_MSG_INFO("chamber " << chamber << " didn't exist in crate Bx01");
            return binNum - 1;
        } else if (crate == 2) {  // BA02, BC02

            if (stName == "BIL")
                binNum = 2 * eta + (phi - 5) / 2 - 1;
            else if (stName == "BIS")
                binNum = 12 + 2 * eta + (phi - 6) / 2 - 1;
            else if (stName == "BML")
                binNum = 24 + 2 * eta + (phi - 5) / 2 - 1;
            else if (stName == "BMS")
                binNum = 36 + 2 * eta + (phi - 6) / 2 - 1;
            else if (stName == "BOL")
                binNum = 48 + 2 * eta + (phi - 5) / 2 - 1;
            else if (stName == "BOS")
                binNum = 60 + 2 * eta + (phi - 6) / 2 - 1;
            else
                ATH_MSG_INFO("chamber " << chamber << " didn't exist in crate Bx02");
            return binNum - 1;
        } else if (crate == 3) {  // BA03,BC03
            if (stName == "BIL")
                binNum = eta;
            else if (stName == "BIM")
                binNum = 6 + eta;
            else if (stName == "BIR")
                binNum = 11 + eta;
            else if (stName == "BIS")
                binNum = 17 + 2 * eta + (phi - 10) / 2 - 1;
            else if (stName == "BMF")
                binNum = 29 + (eta + 1) / 2;
            else if (stName == "BMG")
                binNum = 32 + eta / 2;
            else if (stName == "BML")
                binNum = 35 + 2 * eta + (phi - 9) / 2 - 1;
            else if (stName == "BMS")
                binNum = 47 + eta;
            else if (stName == "BOF")
                binNum = 53 + (eta + 1) / 2;
            else if (chamber.substr(0, 4) == "BOG0" && region == 0)
                binNum = 58;
            else if (stName == "BOG")
                binNum = 58 + eta / 2 - region;
            else if (stName == "BOL")
                binNum = 62 + 2 * eta + (phi - 9) / 2 - 1 - region;
            else if (stName == "BOS")
                binNum = 74 + eta - region;
            else
                ATH_MSG_INFO("chamber " << chamber << " didn't exist in crate Bx03");
            return binNum - 1;
        } else if (crate == 4) {  // BA04, BC04

            if (stName == "BIL")
                binNum = eta;
            else if (stName == "BIM")
                binNum = 6 + eta;
            else if (stName == "BIR")
                binNum = 11 + eta;
            else if (stName == "BIS")
                binNum = 17 + 2 * eta + (phi - 14) / 2 - 1;
            else if (stName == "BMF")
                binNum = 29 + (eta + 1) / 2;
            else if (stName == "BMG")
                binNum = 32 + eta / 2;
            else if (stName == "BML" && eta < 6)
                binNum = 35 + 2 * eta + (phi - 13) / 2 - 1;
            else if (chamber.substr(0, 7) == "BML6A15" || chamber.substr(0, 7) == "BML6C15")
                binNum = 46;
            else if (stName == "BMS")
                binNum = 46 + eta;
            else if (stName == "BOF")
                binNum = 52 + (eta + 1) / 2;
            else if (chamber.substr(0, 4) == "BOG0" && region == 0)
                binNum = 57;
            else if (stName == "BOG")
                binNum = 57 + eta / 2 - region;
            else if (stName == "BOL")
                binNum = 61 + 2 * eta + (phi - 13) / 2 - 1 - region;
            else if (stName == "BOS")
                binNum = 74 + eta - region;
            else
                ATH_MSG_INFO("chamber " << chamber << " didn't exist in crate Bx04");
            return binNum - 1;
        }
    } else if (region == 2 || region == 3) {
        if (crate == 1) {  // EA01, EC01
            if (stName == "BEE")
                binNum = 2 * eta + (phi - 2) / 2 - 1;
            else if (stName == "BIS")
                binNum = 4 + 2 * (eta - 6) + (phi - 2) / 2 - 1;
            else if (stName == "EEL")
                binNum = 8 + 2 * eta + (phi - 1) / 2 - 1;
            else if (stName == "EES")
                binNum = 12 + 2 * eta + (phi - 2) / 2 - 1;
            else if (stName == "EIL" && eta < 5)
                binNum = 16 + 2 * eta + (phi - 1) / 2 - 1;
            else if (stName == "EIL" && eta == 5)
                binNum = 25;
            else if (stName == "EIS")
                binNum = 25 + 2 * eta + (phi - 2) / 2 - 1;
            else if (stName == "EML")
                binNum = 29 + 2 * eta + (phi - 1) / 2 - 1;
            else if (stName == "EMS")
                binNum = 39 + 2 * eta + (phi - 2) / 2 - 1;
            else if (stName == "EOL")
                binNum = 49 + 2 * eta + (phi - 1) / 2 - 1;
            else if (stName == "EOS")
                binNum = 61 + 2 * eta + (phi - 2) / 2 - 1;
            else
                ATH_MSG_INFO("chamber " << chamber << " didn't exist in crate Ex01");
            if (m_do_run3Geometry) {
                if (region == 2) {
                    if (binNum > 8 && binNum < 23)
                        binNum = binNum - 2;
                    else if (binNum > 22 && binNum < 30)
                        binNum = binNum - 8;
                    else if (binNum > 29)
                        binNum = binNum - 12;
                } else if (region == 3) {
                    if (binNum > 22 && binNum < 30)
                        binNum = binNum - 6;
                    else if (binNum > 29)
                        binNum = binNum - 10;
                }
            }
            return binNum - 1;
        } else if (crate == 2) {  // EA02, EC02
            if (stName == "BEE")
                binNum = 2 * eta + (phi - 6) / 2 - 1;
            else if (stName == "BIS")
                binNum = 4 + 2 * (eta - 6) + (phi - 6) / 2 - 1;
            else if (chamber.substr(0, 4) == "EEL1")
                binNum = 9 + (phi - 3) / 2 - 1;
            else if (chamber.substr(0, 4) == "EEL2")
                binNum = 11;
            else if (stName == "EES")
                binNum = 11 + 2 * eta + (phi - 6) / 2 - 1;
            else if (stName == "EIL")
                binNum = 15 + 2 * eta + (phi - 5) / 2 - 1;
            else if (stName == "EIS")
                binNum = 23 + 2 * eta + (phi - 6) / 2 - 1;
            else if (stName == "EML")
                binNum = 27 + 2 * eta + (phi - 5) / 2 - 1;
            else if (stName == "EMS")
                binNum = 37 + 2 * eta + (phi - 6) / 2 - 1;
            else if (stName == "EOL")
                binNum = 47 + 2 * eta + (phi - 5) / 2 - 1;
            else if (stName == "EOS")
                binNum = 59 + 2 * eta + (phi - 6) / 2 - 1;
            else
                ATH_MSG_INFO("chamber " << chamber << " didn't exist in crate Ex02");
            if (m_do_run3Geometry) {
                if (region == 2) {
                    if (binNum > 8 && binNum < 22)
                        binNum = binNum - 2;
                    else if (binNum > 21 && binNum < 28)
                        binNum = binNum - 8;
                    else if (binNum > 27)
                        binNum = binNum - 12;
                } else if (region == 3) {
                    if (binNum > 21 && binNum < 28)
                        binNum = binNum - 6;
                    else if (binNum > 27)
                        binNum = binNum - 10;
                }
            }
            return binNum - 1;
        } else if (crate == 3) {  // EA03, EC03

            if (stName == "BEE")
                binNum = 2 * eta + (phi - 10) / 2 - 1;
            else if (stName == "BIS")
                binNum = 4 + 2 * (eta - 6) + (phi - 10) / 2 - 1;
            else if (stName == "EEL")
                binNum = 8 + 2 * eta + (phi - 9) / 2 - 1;
            else if (stName == "EES")
                binNum = 12 + 2 * eta + (phi - 10) / 2 - 1;
            else if (stName == "EIL" && eta < 5)
                binNum = 16 + 2 * eta + (phi - 9) / 2 - 1;
            else if (chamber.substr(0, 4) == "EIL5")
                binNum = 25;
            else if (stName == "EIS")
                binNum = 25 + 2 * eta + (phi - 10) / 2 - 1;
            else if (stName == "EML")
                binNum = 29 + 2 * eta + (phi - 9) / 2 - 1;
            else if (stName == "EMS")
                binNum = 39 + 2 * eta + (phi - 10) / 2 - 1;
            else if (stName == "EOL")
                binNum = 49 + 2 * eta + (phi - 9) / 2 - 1;
            else if (stName == "EOS")
                binNum = 61 + 2 * eta + (phi - 10) / 2 - 1;
            else
                ATH_MSG_INFO("chamber " << chamber << " didn't exist in crate Ex03");
            if (m_do_run3Geometry) {
                if (region == 2) {
                    if (binNum > 8 && binNum < 23)
                        binNum = binNum - 2;
                    else if (binNum > 22 && binNum < 30)
                        binNum = binNum - 8;
                    else if (binNum > 29)
                        binNum = binNum - 12;
                }
                if (region == 3) {
                    if (binNum > 22 && binNum < 30)
                        binNum = binNum - 6;
                    else if (binNum > 29)
                        binNum = binNum - 10;
                }
            }
            return binNum - 1;
        } else if (crate == 4) {  // EA04, EC04
            if (stName == "BEE")
                binNum = 2 * eta + (phi - 14) / 2 - 1;
            else if (stName == "BIS")
                binNum = 4 + 2 * (eta - 6) + (phi - 14) / 2 - 1;
            else if (stName == "EEL")
                binNum = 8 + 2 * eta + (phi - 13) / 2 - 1;
            else if (stName == "EES")
                binNum = 12 + 2 * eta + (phi - 14) / 2 - 1;
            else if (stName == "EIL")
                binNum = 16 + 2 * eta + (phi - 13) / 2 - 1;
            else if (stName == "EIS")
                binNum = 24 + 2 * eta + (phi - 14) / 2 - 1;
            else if (stName == "EML")
                binNum = 28 + 2 * eta + (phi - 13) / 2 - 1;
            else if (stName == "EMS")
                binNum = 38 + 2 * eta + (phi - 14) / 2 - 1;
            else if (stName == "EOL")
                binNum = 48 + 2 * eta + (phi - 13) / 2 - 1;
            else if (stName == "EOS")
                binNum = 60 + 2 * eta + (phi - 14) / 2 - 1;
            else
                ATH_MSG_INFO("chamber " << chamber << " didn't exist in crate Ex04");
            if (m_do_run3Geometry) {
                if (region == 2) {
                    if (binNum > 8 && binNum < 23)
                        binNum = binNum - 2;
                    else if (binNum > 22 && binNum < 29)
                        binNum = binNum - 8;
                    else if (binNum > 28)
                        binNum = binNum - 12;
                }
                if (region == 3) {
                    if (binNum > 22 && binNum < 28)
                        binNum = binNum - 6;
                    else if (binNum > 28)
                        binNum = binNum - 10;
                }
            }

            return binNum - 1;
        }
    }
    return -1;
}
int MdtRawDataMonAlg::cachedTubeMax(const Identifier& id) const {
    const MuonGM::MdtReadoutElement* roe = m_detMgr->getMdtReadoutElement(id);
    return roe ? roe->getNtubesperlayer() : -1;
}
int MdtRawDataMonAlg::cachedTubeLayerMax(const Identifier& id) const {
    const MuonGM::MdtReadoutElement* roe = m_detMgr->getMdtReadoutElement(id);
    return roe ? roe->getNLayers() : -1;
}
