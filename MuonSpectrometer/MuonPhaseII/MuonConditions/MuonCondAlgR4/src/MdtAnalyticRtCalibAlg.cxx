/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtAnalyticRtCalibAlg.h"

#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/WriteCondHandle.h"

#include "MdtCalibData/CalibParamSorter.h"
#include "MdtCalibData/RtFromPoints.h"
#include "MdtCalibData/SamplePointUtils.h"

#include "MuonVisualizationHelpersR4/VisualizationHelpers.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TROOT.h"

#include <format>

using namespace MuonCalib;
namespace MuonCalibR4{

    StatusCode MdtAnalyticRtCalibAlg::initialize() {
       ATH_CHECK(m_idHelperSvc.retrieve());
       ATH_CHECK(m_readKey.initialize());
       ATH_CHECK(m_writeKey.initialize());
       if (m_saveDiagnostic) {
            gROOT->SetStyle("ATLAS");
       }
       return StatusCode::SUCCESS;
    } 
    StatusCode MdtAnalyticRtCalibAlg::execute() {
      const EventContext& ctx{Gaudi::Hive::currentContext()};
      SG::WriteCondHandle writeHandle{m_writeKey, ctx};
      if(writeHandle.isValid()) {
          ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid.");
          return StatusCode::SUCCESS;
      }
      /// Setup the conditions handle to convert the calibration constants
      SG::ReadCondHandle readHandle{m_readKey, ctx};
      ATH_CHECK(readHandle.isValid());
      writeHandle.addDependency(readHandle);
      /// Output conditions object
      auto writeCdo = std::make_unique<MdtCalibDataContainer>(m_idHelperSvc.get(), readHandle->granularity());
      /// Start the loop
      const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
      std::map<RtRelationPtr, RtRelationPtr, CalibParamSorter> translatedRts{CalibParamSorter{std::pow(0.1, m_precCutOff)}};
      
      RtRelationPtr dummyFillerRt{};
      MdtFullCalibData::CorrectionPtr dummyFillerCorr{};
      std::unordered_set<Identifier> missedElements{};
      for (auto itr = idHelper.detectorElement_begin(); itr != idHelper.detectorElement_end(); ++itr){
          const Identifier& detId{*itr};
          if (!readHandle->hasDataForChannel(detId, msgStream())){
              ATH_MSG_VERBOSE("There's no calibration data available for "<<m_idHelperSvc->toStringDetEl(detId));
              missedElements.insert(detId);
              continue;
          }
          const MdtFullCalibData* copyMe = readHandle->getCalibData(detId, msgStream());
          
          if (copyMe->corrections && !writeCdo->storeData(detId, copyMe->corrections, msgStream())) {
            return StatusCode::FAILURE;
          }
          /// Check whether all tubes are complete 
          const std::vector<Identifier> tubes = tubeIds(detId);
          if (std::ranges::find_if(tubes, [copyMe](const Identifier& tubeId){
                return !copyMe->tubeCalib->getCalib(tubeId);
          }) == tubes.end()) {
           /// Copy the calibration constants for T0 & the corrections
            if (!writeCdo->storeData(detId, copyMe->tubeCalib, msgStream())) {
                return StatusCode::FAILURE;
            }
          } else {
            ATH_MSG_VERBOSE("Tube calibration constants invalid for: "<<m_idHelperSvc->toStringDetEl(detId));
            missedElements.insert(detId);
          }

          RtRelationPtr& translated = translatedRts[copyMe->rtRelation];
          if (!translated) {
              translated = translateRt(ctx, detId, *copyMe->rtRelation);
          }
          if (!dummyFillerRt) {
            dummyFillerRt = translated;
            dummyFillerCorr = copyMe->corrections;
          }
          if (!writeCdo->storeData(detId, translated, msgStream())) {
            ATH_MSG_FATAL("Failed to store rt relation for "<<m_idHelperSvc->toStringDetEl(detId));
            return StatusCode::FAILURE;
          }
      }
      if (!m_fillMissingCh) {
        missedElements.clear();
      }
      for (const Identifier& detId: missedElements) {
        ATH_MSG_WARNING("Initial container did not contain any constants for "<<m_idHelperSvc->toString(detId)
                        <<". Insert instead the first translated rt-relation for those channels");
        const bool fillRt = !writeCdo->hasDataForChannel(detId, msgStream()) || 
                            !writeCdo->getCalibData(detId, msgStream())->rtRelation;
        if(fillRt) {

            if (!writeCdo->storeData(detId, dummyFillerRt, msgStream())) {
                ATH_MSG_FATAL("Failed to store rt relation for "<<m_idHelperSvc->toStringDetEl(detId));
                return StatusCode::FAILURE;
            }
            if (dummyFillerCorr && !writeCdo->storeData(detId, dummyFillerCorr, msgStream())) {
                return StatusCode::FAILURE;
            }
        }
        if (writeCdo->getCalibData(detId, msgStream())->tubeCalib) {
            continue;
        }
        MdtCalibDataContainer::TubeContainerPtr dummyT0cont = std::make_unique<MdtTubeCalibContainer>(m_idHelperSvc.get(), detId);
        MdtTubeCalibContainer::SingleTubeCalibPtr dummyT0Calib = std::make_unique<MdtTubeCalibContainer::SingleTubeCalib>();
        dummyT0Calib->t0 = m_missingT0;
        for (const Identifier& tubeId : tubeIds(detId)) {
            if (!dummyT0cont->setCalib(dummyT0Calib, tubeId, msgStream())) {
                return StatusCode::FAILURE;
            } 
        }
        if (!writeCdo->storeData(detId, std::move(dummyT0cont), msgStream())) {
            return StatusCode::FAILURE;
        }
      }
      ATH_CHECK(writeHandle.record(std::move(writeCdo)));
      return StatusCode::SUCCESS;
    }
    std::vector<Identifier> MdtAnalyticRtCalibAlg::tubeIds(const Identifier& chId) const {
        std::vector<Identifier> tubes{};
         const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
        for (int ml = 1; ml <= idHelper.multilayerMax(chId); ++ml) {
            const Identifier detId = idHelper.multilayerID(chId, ml);
            for (int layer = 1; layer <= idHelper.tubeLayerMax(detId); ++layer) {
                for (int tube = 1; tube <= idHelper.tubeMax(detId); ++tube) {
                    tubes.emplace_back(idHelper.channelID(detId, ml, layer, tube));
                }
            }
        }
        return tubes;
    }

    MdtAnalyticRtCalibAlg::RtRelationPtr 
        MdtAnalyticRtCalibAlg::translateRt(const EventContext& ctx,
                                           const Identifier& detId,
                                           const MdtRtRelation& inRel) const {
        
        
        const std::vector<SamplePoint> rtPoints = fetchDataPoints(*inRel.rt(), *inRel.rtRes());
        IRtRelationPtr rt{};
        ITrRelationPtr tr{};
        unsigned int order{0};
        while (order++ <= m_maxOrder && !rt) {
            ATH_MSG_DEBUG("Attempt to fit rt relation for "<<m_idHelperSvc->toStringDetEl(detId)
                        <<" using a polynomial of order  "<<order);
            switch(m_polyTypeRt) {
                case static_cast<int>(PolyType::ChebyChev):
                    rt = RtFromPoints::getRtChebyshev(rtPoints, order);
                    break;
                case static_cast<int>(PolyType::Legendre): 
                    rt = RtFromPoints::getRtLegendre(rtPoints, order);
                    break;
                case static_cast<int>(PolyType::Simple): 
                    rt = RtFromPoints::getRtSimplePoly(rtPoints, order);
                    break;
                default:
                    break;
            }
            const unsigned rtNdoF = rtPoints.size() - rt->nDoF();
            const double rtChi2 = calculateChi2(rtPoints, *rt) / rtNdoF;
            ATH_MSG_VERBOSE("Fit has a chi2 of "<<rtChi2<<". Cut off at "<<m_chiCutOff);
            if (rtChi2  > m_chiCutOff) {
                rt.reset();
            }
        }
        if (!rt) {
            ATH_MSG_ERROR("Failed to fit rt relation for "<<m_idHelperSvc->toStringDetEl(detId));
            return nullptr;
        }
        const std::vector<SamplePoint> trPoints = swapCoordinates(rtPoints, *rt);
        order = 0;
        while (order++ <= m_maxOrder && !tr) {
            ATH_MSG_DEBUG("Now continue with tr fit for "<<m_idHelperSvc->toStringDetEl(detId)
                        <<" using a polynomial of order  "<<order);
            switch(m_polyTypeTr) {
                case static_cast<int>(PolyType::Legendre): 
                    tr = RtFromPoints::getTrLegendre(trPoints, order);
                    break;
                case static_cast<int>(PolyType::ChebyChev):
                    tr = RtFromPoints::getTrChebyshev(trPoints, order);
                    break;
                case static_cast<int>(PolyType::Simple): 
                    tr = RtFromPoints::getTrSimplePoly(trPoints, order);
                    break;
                default:
                    break;
            }
            const unsigned trNdoF = rtPoints.size() - tr->nDoF();
            const double trChi2 = calculateChi2(trPoints, *tr) / trNdoF; 
            /// Reject bad fits
            ATH_MSG_VERBOSE("T-R fit resulted in a chi2/nDoF for tr: "<<trChi2<<" ("<<trNdoF<<")");
            if (trChi2 > m_chiCutOff) {
                tr.reset();
            }
        }        
        if (!tr) {
            ATH_MSG_FATAL("Failed to fit tr relation for "<<m_idHelperSvc->toStringDetEl(detId));
            return nullptr;
        }
        auto rtReso = inRel.smartReso();
        if (m_fitRtReso) {
            std::unique_ptr<IRtResolution> fittedReso{};
            std::vector<SamplePoint> rtResoPoints = fetchResolution(rtPoints, m_relUncReso);
            order = 0;
            while (order++ <= m_maxOrderReso && !fittedReso) {
                ATH_MSG_VERBOSE("Finally fit the resolution function "<<m_idHelperSvc->toStringDetEl(detId)
                        <<" using a polynomial of order  "<<order);
                fittedReso = RtFromPoints::getResoChebyshev(rtPoints, rt, m_relUncReso, order);
                const unsigned nDoF = rtResoPoints.size() - fittedReso->nDoF();
                const double chi2 = calculateChi2(rtResoPoints, *fittedReso) / nDoF;
                ATH_MSG_VERBOSE("Fit has a chi2 of "<<chi2<<". Cut off at "<<m_chiCutOff);
                if (chi2  > m_chiCutOff) {
                    if (order == m_maxOrderReso) {
                        drawResoFunc(ctx, detId, rtResoPoints, *fittedReso);
                    }
                    fittedReso.reset();
                }
            }

            if (fittedReso) {
                drawResoFunc(ctx, detId, rtResoPoints, *fittedReso);
                rtReso = std::move(fittedReso);
            } else {
                ATH_MSG_WARNING("Despite of having a "<<m_maxOrderReso
                            <<" no rt resolution function could be fitted for "<<m_idHelperSvc->toStringDetEl(detId));
            }
        }
        auto finalRt = std::make_unique<MdtRtRelation>(std::move(rt), rtReso, std::move(tr)); 
        drawRt(ctx, detId, rtPoints, *finalRt);
        return finalRt;
    }
    void MdtAnalyticRtCalibAlg::drawRt(const EventContext& ctx, 
                                       const Identifier& detId,
                                       const std::vector<SamplePoint>& rtPoints,
                                       const MdtRtRelation& inRel) const {
        if (!m_saveDiagnostic) {
            return;
        }
        auto dataGraph = std::make_unique<TGraphErrors>();
        for (const SamplePoint& dataPoint : rtPoints) {
            dataGraph->SetPoint(dataGraph->GetN(), dataPoint.x1(), dataPoint.x2());
            dataGraph->SetPointError(dataGraph->GetN()-1, 0., dataPoint.error());
        }

        /// Populate the rt & tr relation graphs
        auto rtGraph = std::make_unique<TGraph>();
        auto trGraph = std::make_unique<TGraph>();

        double driftTime{inRel.rt()->tLower()};
        while (driftTime <= inRel.rt()->tUpper()) {
            const double radius = inRel.rt()->radius(driftTime);
            const double backRadius = std::clamp(radius,inRel.tr()->minRadius(), inRel.tr()->maxRadius());
            const double backTime = inRel.tr()->driftTime(backRadius).value_or(-666.);
            rtGraph->SetPoint(rtGraph->GetN(), driftTime, radius);
            trGraph->SetPoint(trGraph->GetN(), backTime, radius);
            driftTime+=1.;
        }
        auto canvas = std::make_unique<TCanvas>("can","can", 800, 600);
        canvas->cd();
        auto refHisto = std::make_unique<TH1F>("canvasHisto", "canvasHisto;drift time [ns]; drift radius [mm]", 1, 
                                               inRel.rt()->tLower() - 2, inRel.rt()->tUpper());

        refHisto->SetMinimum(0.);
        refHisto->SetMaximum(inRel.rt()->radius(refHisto->GetXaxis()->GetBinUpEdge(1))*1.3);
        refHisto->Draw("AXIS");
        const std::string chName = std::format("{:}{:d}{:}{:d}M{:1d}",
                                               m_idHelperSvc->stationNameString(detId),
                                               std::abs(m_idHelperSvc->stationEta(detId)),
                                               m_idHelperSvc->stationEta(detId)> 0? 'A' : 'C',
                                               m_idHelperSvc->stationPhi(detId),
                                               m_idHelperSvc->mdtIdHelper().multilayer(detId));
       
        dataGraph->SetMarkerSize(0);
        dataGraph->Draw("P");
        rtGraph->SetLineColor(kRed);
        rtGraph->Draw("C");
        trGraph->SetLineColor(kBlue);
        trGraph->Draw("C");

        const unsigned rtNDoF= rtPoints.size() - inRel.rt()->nDoF();
        const unsigned trNDoF= rtPoints.size() - inRel.tr()->nDoF();
        const double rtChi2 = calculateChi2(rtPoints, *inRel.rt())/ rtNDoF;
        const double trChi2 = calculateChi2(swapCoordinates(rtPoints, *inRel.rt()), *inRel.tr())/ trNDoF;

        auto legend = std::make_unique<TLegend>(0.2,0.7,0.6, 0.9,chName.c_str());
        legend->SetFillStyle(0);
        legend->SetBorderSize(0);
        legend->AddEntry(rtGraph.get(),std::format("{:}, order: {:d}, #chi^{{2}}: {:.3f}({:d})",
                                                        inRel.rt()->name(), inRel.rt()->nDoF(), rtChi2, rtNDoF).c_str(),"L");

        legend->AddEntry(trGraph.get(),std::format("{:}, order: {:d}, #chi^{{2}}: {:.3f}({:d})",
                                                        inRel.tr()->name(), inRel.tr()->nDoF(), trChi2, trNDoF).c_str(),"L");

        legend->Draw();
        canvas->SaveAs(std::format("MdtAnalyticRt_Evt{:d}_{:}.pdf", ctx.eventID().event_number(), chName).c_str());
        
        /// Save the histogram to the outFile
        saveGraph(std::format("/{:}/Evt{:d}/Data_{:}", m_outStream.value(), ctx.evt(), chName ), std::move(dataGraph));
        saveGraph(std::format("/{:}/Evt{:d}/Rt_{:}", m_outStream.value(), ctx.evt(), chName ), std::move(rtGraph));
        saveGraph(std::format("/{:}/Evt{:d}/Tr_{:}", m_outStream.value(), ctx.evt(), chName ), std::move(trGraph));
    }
    void MdtAnalyticRtCalibAlg::saveGraph(const std::string& path, std::unique_ptr<TGraph>&& graph) const {
        graph->SetName(path.substr(path.rfind("/")+1).c_str());
        histSvc()->regGraph(path, std::move(graph)).ignore();
    }
    void MdtAnalyticRtCalibAlg::drawResoFunc(const EventContext& ctx,
                                             const Identifier& detId,
                                             const std::vector<SamplePoint>& resoPoints,
                                             const IRtResolution& inReso) const{
        auto dataGraph = std::make_unique<TGraphErrors>();
        for (const SamplePoint& dataPoint : resoPoints) {
            dataGraph->SetPoint(dataGraph->GetN(), dataPoint.x1(), dataPoint.x2());
            dataGraph->SetPointError(dataGraph->GetN()-1, 0., dataPoint.error());
        }
        auto resoGraph = std::make_unique<TGraph>();
        const auto [tLow, tHigh] = interval(resoPoints);
        const auto [resoLow, resoHigh] = minMax(resoPoints);
        double driftTime{tLow};
        while (driftTime <= tHigh) {
            const double evalReso = inReso.resolution(driftTime);
            resoGraph->SetPoint(resoGraph->GetN(), driftTime, evalReso);
            driftTime+=0.5;
        }
        auto canvas = std::make_unique<TCanvas>("can","can", 800, 600);
        canvas->cd();
        auto refHisto = std::make_unique<TH1F>("canvasHisto", "canvasHisto;drift time [ns];  #sigma(r_{drift}) [mm]", 1, 
                                               tLow - 2, tHigh + 2);

        refHisto->SetMinimum(resoLow*0.7);
        refHisto->SetMaximum(resoHigh*1.3);
        refHisto->Draw("AXIS");

        dataGraph->Draw("P");
        resoGraph->SetLineColor(kRed);
        resoGraph->Draw("C");

        const std::string chName = std::format("{:}{:d}{:}{:d}M{:1d}",
                                               m_idHelperSvc->stationNameString(detId),
                                               std::abs(m_idHelperSvc->stationEta(detId)),
                                               m_idHelperSvc->stationEta(detId)> 0? 'A' : 'C',
                                               m_idHelperSvc->stationPhi(detId),
                                               m_idHelperSvc->mdtIdHelper().multilayer(detId));

        const double chi2 = calculateChi2(resoPoints, inReso) / (resoPoints.size() - inReso.nDoF());
        auto label = MuonValR4::drawLabel(std::format("{:}, {:}, order: {:1d}  #chi^{{2}}: {:.3f}", chName, inReso.name(), 
                                                        inReso.nDoF(), chi2), 0.2, 0.8);
        label->Draw();
        canvas->SaveAs(std::format("MdtAnalyticReso_Evt{:d}_{:}.pdf", ctx.eventID().event_number(), chName).c_str());
    } 
}