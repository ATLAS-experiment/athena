/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include <MuonEfficiencyCorrections/HistHandler.h>
#include <MuonEfficiencyCorrections/UtilFunctions.h>

#include <iostream>
#include <cmath>
#include <cstdint>
#include <cstring>

#include <TH1.h>
#include <TH2Poly.h>

namespace CP {
    //###########################################################################################################
    //                                                   AxisHandlerProvider
    //###########################################################################################################
    std::unique_ptr<AxisHandler> AxisHandlerProvider::GetAxisHandler(columnar::ColumnarTool<> * parent, const TAxis * axisptr) {
        if (axisptr != nullptr) {
            std::string axis = axisptr->GetTitle();
            axis = EraseWhiteSpaces(axis);
            size_t Abs1 = axis.find("|");
            size_t Abs2(0);
            if (Abs1 != std::string::npos) Abs2 = axis.find("|", Abs1 + 1);
            bool AbsAxis = (Abs2 != std::string::npos) && (Abs2 != 0);
            if (axis.find("pt") != std::string::npos || axis.find("pT") != std::string::npos || axis.find("p_{T}") != std::string::npos) {
                return std::make_unique<PtAxisHandler>(parent);
            } else if (axis.find("phi") != std::string::npos) {
                return std::make_unique<PhiAxisHandler>(parent);
            } else if (axis.find("q") != std::string::npos || axis.find("charge") != std::string::npos) {
                return std::make_unique<ChargeAxisHandler>(parent);
            } else if (axis.find("eta") != std::string::npos) {
                if (AbsAxis) return std::make_unique<AbsEtaAxisHandler>(parent);
                return std::make_unique<EtaAxisHandler>(parent);
            } else if (axis.find("dRJet") != std::string::npos || axis.find("#DeltaR (jet, #mu)") != std::string::npos) {
                return std::make_unique<dRJetAxisHandler>(parent);
            }

            Error("AxisHandlerProvider", "Can not interpret axis title '%s'", axis.c_str());
        } else {
            Error("AxisHandlerProvider", "nullptr pointer passed");
        }
        return std::make_unique<UndefinedAxisHandler>(parent);
    }
  
  
    CorrectionCode PtAxisHandler::GetBinningParameter(columnar::MuonId mu, float & value) const {
        value = ptAcc(mu) / 1000.;
        return CorrectionCode::Ok;
    }
    CorrectionCode ChargeAxisHandler::GetBinningParameter(columnar::MuonId mu, float & value) const {
        value = chargeAcc (mu);
        return CorrectionCode::Ok;
    }
    CorrectionCode EtaAxisHandler::GetBinningParameter(columnar::MuonId mu, float & value) const {
        value = etaAcc(mu);
        return CorrectionCode::Ok;
    }
    CorrectionCode AbsEtaAxisHandler::GetBinningParameter(columnar::MuonId mu, float & value) const {
        value = std::abs(etaAcc(mu));
        return CorrectionCode::Ok;
    }
    CorrectionCode PhiAxisHandler::GetBinningParameter(columnar::MuonId mu, float & value) const {
        value = phiAcc(mu);
        return CorrectionCode::Ok;
    }
    void dRJetAxisHandler::set_close_jet_decorator(const std::string& decor_name){
        s_close_jet_decor = decor_name;
    }
    
    void dRJetAxisHandler::set_use_2D_sf(const bool in)
    {
        s_use_2D_sf=in;
    }
    
    std::string dRJetAxisHandler::s_close_jet_decor = "dRJet";
    bool dRJetAxisHandler::s_use_2D_sf = false;
    dRJetAxisHandler::dRJetAxisHandler(columnar::ColumnarTool<> * parent):
            AxisHandler(parent),
            m_use_2D_sf(s_use_2D_sf),
            m_acc(*this,s_close_jet_decor){}
    
    CorrectionCode dRJetAxisHandler::GetBinningParameter(columnar::MuonId mu, float & value) const {
        
        static std::atomic<unsigned int> warned = {0};
        
        if(!m_use_2D_sf) value = -2.;
        else if (acc_dR_deriv.isAvailable(mu)){
            value = acc_dR_deriv(mu);
        }else if( m_acc.isAvailable(mu) ) {
            // decoration available in DxAOD
            value = m_acc(mu);
            if (warned < 5){
                Warning("MuonEfficiencyCorrections::dRJetAxisHandler()", "The DFCommonJetDr jet decoration is not available in the derivation will fall back to %s",s_close_jet_decor.c_str());
                ++warned;
            }
        } else {
            // decoration not available 
            value = -2.; 
            // We want these warnings to be printed few times per job, so that they're visible, then stop before log file's size blows up 
            if (warned<5){
                Warning("MuonEfficiencyCorrections::dRJetAxisHandler()", "The %s decoration has not been found for the Muon. Isolation scale-factors are now also binned in #Delta R(jet,#mu)", s_close_jet_decor.c_str());
                Warning("MuonEfficiencyCorrections::dRJetAxisHandler()", "using the closest calibrated AntiKt4EMTopo jet with p_{T}>20~GeV and surving the standard OR criteria.");
                Warning("MuonEfficiencyCorrections::dRJetAxisHandler()", "You should decorate your muon appropiately before passing to the tool, and use dRJet = -1 in case there is no jet in an event.");
                Warning("MuonEfficiencyCorrections::dRJetAxisHandler()", "For the time being the inclusive scale-factor is going to be returned.");
                Warning("MuonEfficiencyCorrections::dRJetAxisHandler()", "In future derivations, muons will also be decorated centrally with DFCommonJetDr, for your benefit.");
                Warning("MuonEfficiencyCorrections::dRJetAxisHandler()", "You can define custom jet decorations via the 'CloseJetDRDecorator' property of the MuonEfficiencyCorrections tool");
                ++warned;
            }
        }
        return CorrectionCode::Ok;
    }
    
    CorrectionCode UndefinedAxisHandler::GetBinningParameter(columnar::MuonId, float &) const  {
        return CorrectionCode::Error;
    }
    //###########################################################################################################
    //                                                   HistHandler
    //###########################################################################################################
    HistHandler::HistHandler(columnar::ColumnarTool<>* /*parent*/, TH1* Hist) :
                m_H(clone(Hist)) {       

    }
    // void HistHandler::Copy(const HistHandler & other) {
    //     if (this == &other) {
    //         return;
    //     }
    //     if (other.m_H) {
    //         m_H = clone(other.GetHist());
    //     }
    // }
    // HistHandler::HistHandler(const HistHandler & other) :
    //             m_H() {
    //     Copy(other);
    // }
    HistHandler::~HistHandler() {
    }    
    void HistHandler::SetBinContent(int bin, float val) {
        if (m_H) {
            m_H->SetBinContent(bin, val);
        }
    }    
    void HistHandler::SetBinError(int bin, float val) {
        if (m_H) {
            m_H->SetBinError(bin, val);
        }
    }
    //###########################################################################################################
    //                                                   HistHandler_TH1
    //###########################################################################################################
    HistHandler_TH1::HistHandler_TH1(columnar::ColumnarTool<>* parent, TH1* h) :
                HistHandler(parent, h),
                m_x_handler(h == nullptr ? 0 : AxisHandlerProvider::GetAxisHandler(parent, h->GetXaxis())) {
    }

    HistHandler_TH1::~HistHandler_TH1() {
    }
    int HistHandler_TH1::nBins() const {return GetHist()->GetNbinsX() + 2;}
    int HistHandler_TH1::nOverFlowBins() const {return 2; }
    bool HistHandler_TH1::isOverFlowBin(int b) const { return b == 0 || b >= nBins() -1; }
    std::string HistHandler_TH1::GetBinName(unsigned int bin) const {
        TAxis* xAx = GetHist()->GetXaxis();
        return Form("%s_%.2f_to_%.2f", xAx->GetTitle(), xAx->GetBinLowEdge(bin), xAx->GetBinUpEdge(bin));
    }
    CorrectionCode HistHandler_TH1::FindBin(columnar::MuonId muon, int & bin) const {
        if (!GetHist()) {
            bin = -1;
            return CorrectionCode::Error;
        }
        float par = 0;
        if (m_x_handler->GetBinningParameter(muon, par) == CorrectionCode::Error) {
            return CorrectionCode::Error;
        } else {
            bin = GetHist()->FindBin(par);
            if (bin < 1 || bin > GetHist()->GetNbinsX()) {
                return CorrectionCode::OutOfValidityRange;
            }
        }
        return CorrectionCode::Ok;
    }

    //###########################################################################################################
    //                                                   HistHandler_TH2
    //###########################################################################################################
    HistHandler_TH2::HistHandler_TH2(columnar::ColumnarTool<>* parent, TH1*  h) :
                HistHandler(parent, h),
                m_x_handler(h == nullptr ? 0 : AxisHandlerProvider::GetAxisHandler(parent, h->GetXaxis())),
                m_y_handler(h == nullptr ? 0 : AxisHandlerProvider::GetAxisHandler(parent, h->GetYaxis())) {

    }
    int HistHandler_TH2::nBins() const {
        return (GetHist()->GetNbinsX() + 2) * (GetHist()->GetNbinsY() + 2);
    }
    int HistHandler_TH2::nOverFlowBins() const {return 2*GetHist()->GetNbinsX() +  2*GetHist()->GetNbinsY() + 4; }
    bool HistHandler_TH2::isOverFlowBin(int b) const { 
        int x(-1),y(-1), z(-1);
        GetHist()->GetBinXYZ(b,x,y,z);
        return  x == 0 ||  x == GetHist()->GetXaxis()->GetNbins() + 1 ||  y == 0 ||  y == GetHist()->GetYaxis()->GetNbins() + 1; 
    }
    CorrectionCode HistHandler_TH2::FindBin(columnar::MuonId muon, int & bin) const {
        if (!GetHist()) return CorrectionCode::Error;
        float parx = 0;
        float pary = 0;
        if (m_x_handler->GetBinningParameter(muon, parx) == CorrectionCode::Error || 
            m_y_handler->GetBinningParameter(muon, pary) == CorrectionCode::Error) {
            return CorrectionCode::Error;
        } else {
            int binx = GetHist()->GetXaxis()->FindBin(parx);
            int biny = GetHist()->GetYaxis()->FindBin(pary);
            if (binx < 1 || binx > GetHist()->GetNbinsX() || biny < 1 || biny > GetHist()->GetNbinsY()) {
                return CorrectionCode::OutOfValidityRange;
            }
            bin = GetHist()->GetBin(binx, biny);
        }
        return CorrectionCode::Ok;
    }
    HistHandler_TH2::~HistHandler_TH2() {
    }
    std::string HistHandler_TH2::GetBinName(unsigned int bin) const {
        int x(0), y(0), z(0);
        GetHist()->GetBinXYZ(bin, x, y, z);
        TAxis* xAx = GetHist()->GetXaxis();
        TAxis* yAx = GetHist()->GetYaxis();
        return Form("%s_%.2f_to_%.2f_times_%s_%.2f_to_%.2f",
                //xAxis
                xAx->GetTitle(), xAx->GetBinLowEdge(x), xAx->GetBinUpEdge(x),
                //yAxis
                yAx->GetTitle(), yAx->GetBinLowEdge(y), yAx->GetBinUpEdge(y));
    }
    //###########################################################################################################
    //                                                   HistHandler_TH3
    //###########################################################################################################

    HistHandler_TH3::HistHandler_TH3(columnar::ColumnarTool<>* parent, TH1* h) :
                HistHandler(parent,h),
                m_x_handler(GetHist() == nullptr ? 0 : AxisHandlerProvider::GetAxisHandler(parent, h->GetXaxis())),
                m_y_handler(GetHist() == nullptr ? 0 : AxisHandlerProvider::GetAxisHandler(parent, h->GetYaxis())),
                m_z_handler(GetHist() == nullptr ? 0 : AxisHandlerProvider::GetAxisHandler(parent, h->GetZaxis())) {

    }
    HistHandler_TH3::~HistHandler_TH3() {
    }
    int HistHandler_TH3::nBins() const {
        return (GetHist()->GetNbinsX() + 2) * (GetHist()->GetNbinsY() + 2) * (GetHist()->GetNbinsZ() + 2);
    }
    int HistHandler_TH3::nOverFlowBins() const {return 2*(GetHist()->GetNbinsX()*GetHist()->GetNbinsY() + 
                                                          GetHist()->GetNbinsX()*GetHist()->GetNbinsZ() +
                                                          GetHist()->GetNbinsY()*GetHist()->GetNbinsZ())  + 8; }
    bool HistHandler_TH3::isOverFlowBin(int b) const { 
        int x(-1),y(-1), z(-1);
        GetHist()->GetBinXYZ(b,x,y,z);
        return  x == 0 ||  x == GetHist()->GetXaxis()->GetNbins() + 1 ||  
                y == 0 ||  y == GetHist()->GetYaxis()->GetNbins() + 1 || 
                z == 0 ||  z == GetHist()->GetZaxis()->GetNbins() + 1; 
    }   
    CorrectionCode HistHandler_TH3::FindBin(columnar::MuonId muon, int & bin) const {
        if (!GetHist()) return CorrectionCode::Error;
        float parx = 0;
        float pary = 0;
        float parz = 0;
        if (m_x_handler->GetBinningParameter(muon, parx) == CorrectionCode::Error || 
            m_y_handler->GetBinningParameter(muon, pary) == CorrectionCode::Error || 
            m_z_handler->GetBinningParameter(muon, parz) == CorrectionCode::Error) return CorrectionCode::Error;
        else {
            int binx = GetHist()->GetXaxis()->FindBin(parx);
            int biny = GetHist()->GetYaxis()->FindBin(pary);
            int binz = GetHist()->GetZaxis()->FindBin(parz);
            if (binx < 1 || binx > GetHist()->GetNbinsX() || biny < 1 || biny > GetHist()->GetNbinsY() || binz < 1 || binz > GetHist()->GetNbinsZ()) {
                return CorrectionCode::OutOfValidityRange;
            }
            bin = GetHist()->GetBin(binx, biny, binz);
        }
        return CorrectionCode::Ok;
    }

    std::string HistHandler_TH3::GetBinName(unsigned int bin) const {
        int x(0), y(0), z(0);
        GetHist()->GetBinXYZ(bin, x, y, z);
        TAxis* xAx = GetHist()->GetXaxis();
        TAxis* yAx = GetHist()->GetYaxis();
        TAxis* zAx = GetHist()->GetZaxis();
        return Form("%s_%.2f_to_%.2f_times_%s_%.2f_to_%.2f_times_%s_%.2f_to_%.2f",
        //xAxis
                xAx->GetTitle(), xAx->GetBinLowEdge(x), xAx->GetBinUpEdge(x),
                //yAxis
                yAx->GetTitle(), yAx->GetBinLowEdge(y), yAx->GetBinUpEdge(y),
                //zAxis
                zAx->GetTitle(), zAx->GetBinLowEdge(z), zAx->GetBinUpEdge(z));
    }

    //###########################################################################################################
    //                                                   HistHandler_TH2Poly
    //###########################################################################################################

    HistHandler_TH2Poly::HistHandler_TH2Poly(columnar::ColumnarTool<>* parent, TH2Poly * h) :
                HistHandler(parent, h),
                m_h(h),
                m_x_handler(GetHist() == nullptr ? 0 : AxisHandlerProvider::GetAxisHandler(parent, h->GetXaxis())),
                m_y_handler(GetHist() == nullptr ? 0 : AxisHandlerProvider::GetAxisHandler(parent, h->GetYaxis())) {

    }

    HistHandler_TH2Poly::~HistHandler_TH2Poly() {
    }
    int HistHandler_TH2Poly::nBins() const {
        return m_h->GetNumberOfBins() + 1;
    }
    
    int HistHandler_TH2Poly::nOverFlowBins() const {return 10;}
    bool HistHandler_TH2Poly::isOverFlowBin(int b) const {return b < 1;}

    CorrectionCode HistHandler_TH2Poly::FindBin(columnar::MuonId muon, int & bin) const {
        if (!m_h) return CorrectionCode::Error;
        float parx = 0;
        float pary = 0;
        if (m_x_handler->GetBinningParameter(muon, parx) == CorrectionCode::Error || 
            m_y_handler->GetBinningParameter(muon, pary) == CorrectionCode::Error) return CorrectionCode::Error;
        else {
            bin = GetHist()->FindBin(parx, pary);
            if (bin < 0) {
                return CorrectionCode::OutOfValidityRange;
            }
        }
        return CorrectionCode::Ok;
    }
    std::string HistHandler_TH2Poly::GetBinName(unsigned int bin) const {
        int x(0), y(0), z(0);
        GetHist()->GetBinXYZ(bin, x, y, z);
        TAxis* xAx = GetHist()->GetXaxis();
        TAxis* yAx = GetHist()->GetYaxis();
        return Form("%s_%.2f_to_%.2f__times_%s_%.2f_to_%.2f",
                xAx->GetTitle(), xAx->GetBinLowEdge(x), xAx->GetBinUpEdge(x),
                yAx->GetTitle(), yAx->GetBinLowEdge(y), yAx->GetBinUpEdge(y));
    }
} // namespace CP
