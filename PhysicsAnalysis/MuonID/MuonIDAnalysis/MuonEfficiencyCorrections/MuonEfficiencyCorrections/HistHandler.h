/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef MUONEFFICIENCYCORRECTIONS_HISTOHANDLER_H
#define MUONEFFICIENCYCORRECTIONS_HISTOHANDLER_H

// EDM include(s):
#include "xAODMuon/Muon.h"

// supported SF histogram types
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TH2Poly.h>

#include "CxxUtils/checker_macros.h"
#include "PATInterfaces/CorrectionCode.h"
// further ROOT includes
#include <TFile.h>
#include <TDirectory.h>

#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarMuon/MuonDef.h>
#include <ColumnarCore/ObjectColumn.h>

// STL includes
#include <string>
#include <iostream>
#include <exception>
#include <map>
#include <memory>
#include <cmath>

namespace CP {
    
    ///Forward declarations
    class AxisHandler;
   
    
    class HistHandler {
            /// @class HistHandler
            /// @brief  utility class to avoid having to determine the input histo at every single
            /// call to the tool
            /// Remembers the type and axis binning of a histogram
            /// and provides a common interface independent of these
            /// details.
        public:

            inline  double GetBinContent(int bin) const{
                if (!m_H) return DBL_MAX;
                return m_H->GetBinContent(bin);
            }
            
            void SetBinContent(int bin, float val);
            
            inline  double GetBinError(int bin) const{
                if (!m_H) return DBL_MAX;
                return m_H->GetBinError(bin);
            }
            
            inline TH1* GetHist() const{
                return m_H.get();
            }

            void SetBinError(int bin, float val);
            
            ///Function that changes from Implementation to implementation
            virtual CorrectionCode FindBin(columnar::MuonId muon, int & bin) const = 0;
            /// Return the total number of bins in the histogram -> (GetNbins() +2)**n
            virtual int nBins() const = 0;
            /// Return the number of overflow bins 
            virtual int nOverFlowBins() const = 0;
            /// States whether a bin is overflow or not
            virtual bool isOverFlowBin(int b) const =0;
            /// Translates the bin number into the borders and return them as name
            virtual std::string GetBinName(unsigned int bin) const=0;

            virtual ~HistHandler();
        protected:
            HistHandler(columnar::ColumnarTool<>* parent, TH1* Hist);
            HistHandler(const HistHandler & other) = delete;
            HistHandler& operator =(const HistHandler & other) = delete;
        private:
            std::unique_ptr<TH1> m_H;

    };

    

    class HistHandler_TH1: public HistHandler {

        public:

            HistHandler_TH1(columnar::ColumnarTool<>* parent, TH1* hist);
            virtual ~HistHandler_TH1();

             int nBins() const override;
             
             int nOverFlowBins() const override;
             bool isOverFlowBin(int b) const override;


             std::string GetBinName(unsigned int bin) const override;
             CorrectionCode FindBin(columnar::MuonId muon, int & bin) const override;

        private:
            std::unique_ptr<AxisHandler> m_x_handler;
    };

    class HistHandler_TH2: public HistHandler {

        public:
            /// The HistHandler TH2 handles 2D histograms which are not TH2Poly, since TH2 inhertis from TH1,
            /// a TH1* object is parsed avoiding the dynamic cast. Via the GetDimension() method of the TH1 
            /// the class ensures that the histogram has actual dimension 2. No specific TH2 method is required 
            /// in all considered usecases of TH1.
            HistHandler_TH2(columnar::ColumnarTool<>* parent, TH1* hist);
            virtual ~HistHandler_TH2();

             int nBins() const override;
           
             int nOverFlowBins() const override;
             bool isOverFlowBin(int b) const override;
           
             std::string GetBinName(unsigned int bin) const override;

             CorrectionCode FindBin(columnar::MuonId muon, int & bin) const override;

        private:
            std::unique_ptr<AxisHandler> m_x_handler;
            std::unique_ptr<AxisHandler> m_y_handler;
    };

    class HistHandler_TH3: public HistHandler {

        public:
            /// The HistHandler TH3 handles 3D histograms, since TH3 inhertis from TH1,a TH1* object is parsed avoiding the dynamic cast. 
            /// Via the GetDimension() method of the TH1 the class ensures that the histogram has actual dimension 3. No specific TH3 method
            /// is required in all use-caes of the class.
            
            HistHandler_TH3(columnar::ColumnarTool<>* parent, TH1* hist);
            virtual ~HistHandler_TH3();

             int nBins() const override;
             
             int nOverFlowBins() const override;
             bool isOverFlowBin(int b) const override;


             std::string GetBinName(unsigned int bin) const override;

             CorrectionCode FindBin(columnar::MuonId muon, int & bin) const override;

        private:
            std::unique_ptr<AxisHandler> m_x_handler;
            std::unique_ptr<AxisHandler> m_y_handler;
            std::unique_ptr<AxisHandler> m_z_handler;
    };

    class HistHandler_TH2Poly: public HistHandler {

        public:
            /// The HistHandler_TH2Poly handles the TH2Poly histograms of the scale-factor maps. Since the bin-finding,
            /// algoritihms of the TH2Poly differ from the ones of TH1, TH2 & TH3 explicit methods of TH2Poly are needed
            /// to pick-up the proper bin.
            HistHandler_TH2Poly(columnar::ColumnarTool<>* parent, TH2Poly* hist);
            virtual ~HistHandler_TH2Poly();

             int nOverFlowBins() const override;
             bool isOverFlowBin(int b) const override;

             int nBins() const override;
             std::string GetBinName(unsigned int bin) const override;
             CorrectionCode FindBin(columnar::MuonId muon, int & bin) const override;

        private:
            TH2Poly* m_h;
            std::unique_ptr<AxisHandler> m_x_handler;
            std::unique_ptr<AxisHandler> m_y_handler;
    };

    class AxisHandler : public columnar::ColumnarTool<> {
        public:
            virtual CorrectionCode GetBinningParameter(columnar::MuonId mu, float & value) const =0;
            AxisHandler (columnar::ColumnarTool<> * parent) : columnar::ColumnarTool<>(parent) {}
            virtual ~AxisHandler()= default;
    };
    class AxisHandlerProvider {
        public:
            static std::unique_ptr<AxisHandler> GetAxisHandler(columnar::ColumnarTool<> * parent, const TAxis* axis);
    };

    class PtAxisHandler: public AxisHandler {
        public:
            CorrectionCode GetBinningParameter(columnar::MuonId mu, float & value) const override;
            PtAxisHandler (columnar::ColumnarTool<> * parent) : AxisHandler(parent) {}
            virtual ~PtAxisHandler() = default;
            columnar::MuonAccessor<float> ptAcc {*this, "pt"};
    };

    class ChargeAxisHandler: public AxisHandler {
        public:
            CorrectionCode GetBinningParameter(columnar::MuonId mu, float & value) const override;
            ChargeAxisHandler (columnar::ColumnarTool<> * parent) : AxisHandler(parent) {}
            virtual ~ChargeAxisHandler() = default;
            columnar::MuonAccessor<float> chargeAcc {*this, "charge"};
    };
    
    class EtaAxisHandler: public AxisHandler {
        public:
            CorrectionCode GetBinningParameter(columnar::MuonId mu, float & value) const override;
            EtaAxisHandler (columnar::ColumnarTool<> * parent) : AxisHandler(parent) {}
            virtual ~EtaAxisHandler() = default;
            columnar::MuonAccessor<float> etaAcc {*this, "eta"};
    };
    class AbsEtaAxisHandler: public AxisHandler {
        public:
            CorrectionCode GetBinningParameter(columnar::MuonId mu, float & value) const override; 
            AbsEtaAxisHandler (columnar::ColumnarTool<> * parent) : AxisHandler(parent) {}
            virtual ~AbsEtaAxisHandler() = default;
            columnar::MuonAccessor<float> etaAcc {*this, "eta"};
    };
    class PhiAxisHandler: public AxisHandler {
        public:
            CorrectionCode GetBinningParameter(columnar::MuonId mu, float & value) const override;
            PhiAxisHandler (columnar::ColumnarTool<> * parent) : AxisHandler(parent) {}
            virtual ~PhiAxisHandler() = default;
            columnar::MuonAccessor<float> phiAcc {*this, "phi"};
    };
    class dRJetAxisHandler: public AxisHandler {
        public:
            dRJetAxisHandler(columnar::ColumnarTool<> * parent);
            
            CorrectionCode GetBinningParameter(columnar::MuonId mu, float & value) const override;
            virtual ~dRJetAxisHandler() = default;
            
            //this method must be called only before any object creation, since it sets the static variable s_close_jet_decor, that is used only to initialize the m_acc member variable, and for nothing else; using it after object creation will not change the behaviour of the class
            static void set_close_jet_decorator(const std::string& decor_name);
            //this method must be called only before any object creation, since it sets the static variable s_use_2D_sf, that is used only to initialize the m_use_2D_sf member variable, and for nothing else; using it after object creation will not change the behaviour of the class
            static void set_use_2D_sf(const bool);
            
        private:
            //this static variable is safe because it is only used during object creation (to set the value of m_acc) and avoiding it would require a significant rewrite
            static std::string s_close_jet_decor ATLAS_THREAD_SAFE;
            //this static variable is safe because it is only used during object creation (to set the value of m_use_2D_sf) and avoiding it would require a significant rewrite
            static bool s_use_2D_sf ATLAS_THREAD_SAFE;
            
            bool m_use_2D_sf;

        public:
            columnar::MuonAccessor<float> m_acc;
            columnar::MuonAccessor<float> acc_dR_deriv {*this, "DFCommonJetDr"};
    };
    class UndefinedAxisHandler: public AxisHandler {
        public:
            CorrectionCode GetBinningParameter(columnar::MuonId, float &) const override;
            UndefinedAxisHandler (columnar::ColumnarTool<> * parent) : AxisHandler(parent) {}
            virtual ~UndefinedAxisHandler() = default;
    };

} // namespace CP
#endif
