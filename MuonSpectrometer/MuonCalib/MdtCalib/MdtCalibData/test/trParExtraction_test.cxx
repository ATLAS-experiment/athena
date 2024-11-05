/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <stdlib.h>
#include <limits>
#include <format>
#include <MuonCalibMath/BaseFunctionFitter.h>
#include <MuonCalibMath/ChebyshevPolynomial.h>
#include <MuonCalibMath/LegendrePolynomial.h>

#include <MdtCalibData/RtChebyshev.h>
#include <MdtCalibData/RtLegendre.h>


#include <TGraph.h>
#include <TMultiGraph.h>
#include <TCanvas.h>
#include <TLegend.h>
using namespace MuonCalib;

inline double resolution(const double r) {
    return 0.23 * std::exp(-std::abs(r) / 6.06) + 0.0362;
}

inline std::ostream& operator<<(std::ostream& ostr, const std::vector<double>& v){
    ostr<<"[";
    for (unsigned int k =0; k < v.size(); ++k) {
        ostr<<v[k];
        if (k + 1 != v.size()) ostr<<", ";
    }
    ostr<<"]";
    return ostr;
}



int main (){
    /// Interval & step size
    constexpr double tMin = 10.;
    constexpr double tMax = 1000.;
    constexpr double stepSize = 1.e-3;


    constexpr double maxR = 15.;
    constexpr unsigned int maxPolyGrade = 7;

    for (unsigned int maxOrder = 1; maxOrder <= maxPolyGrade; ++maxOrder) {
        std::vector<double> rtParsCheby{tMin, tMax}, rtParsLegendre{tMin, tMax};
        double rMax{-std::numeric_limits<double>::max()}, rMin{std::numeric_limits<double>::max()};
        
        std::vector<double> rtPars{1};
        for (unsigned int o =1; o <= maxOrder; ++o) {
            rtPars.push_back( maxR *o*o/ maxOrder / std::pow(tMax, o));
        }

        auto rFromT = [&rtPars](const double t) {
            double r{0.};
            for (unsigned int p = 0; p < rtPars.size(); ++p) {
                r += rtPars[p] * std::pow(t, p);
            }
            return r;
        };

        {
            std::cout<<"rt-parameters to test: "<<rtPars<<std::endl;
            std::vector<SamplePoint> samplePointsRt{};
            for (double t = tMin;  std::abs(tMax -t) > stepSize;t+=stepSize ){
                const double r = rFromT(t);
                rMax = std::max(rMax, r);
                rMin = std::min(rMin, r);
                const double cheb_t = 2.*(t - 0.5*(tMin + tMax)) / (tMax - tMin);
                samplePointsRt.emplace_back(cheb_t, r , 1.);
            }
            std::cout<<" rmin: "<<rMin<<" rMax: "<<rMax<<std::endl;

       
            BaseFunctionFitter chebyFitter{maxOrder};
            chebyFitter.fit_parameters(samplePointsRt, 1, samplePointsRt.size(), ChebyshevPolynomial{});
            for (const double c : chebyFitter.coefficients()){
                rtParsCheby.push_back(c);
            }

            BaseFunctionFitter legendreFitter{maxOrder};
            legendreFitter.fit_parameters(samplePointsRt, 1, samplePointsRt.size(), LegendrePolynomial{});
            for (const double c : legendreFitter.coefficients()){
                rtParsLegendre.push_back(c);
            }
        }
        std::cout<<"r-t relation (cheby): "<<rtParsCheby<<std::endl;
        std::cout<<"r-t relation (legendre): "<<rtParsLegendre<<std::endl;

        RtChebyshev chebyRt{std::move(rtParsCheby)};
        RtLegendre legendreRt{std::move(rtParsLegendre)};

        std::vector<SamplePoint> samplePoints{}, samplePointsCheby;
        for (double t = tMin;  std::abs(tMax -t) > stepSize;t+=stepSize ){
            const double r = rFromT(t);
            const double cheb_r =  2.* (r - 0.5*(rMin + rMax)) / (rMax - rMin);
            samplePointsCheby.emplace_back(cheb_r, t, 1.);

        }

        std::vector<double> trParsCheby{rMin, rMax}, trParsLegendre{rMin, rMax};
        {
            BaseFunctionFitter fitter{maxOrder};
            fitter.fit_parameters(samplePointsCheby, 1, samplePointsCheby.size(), ChebyshevPolynomial{});
            for (const double c : fitter.coefficients()){
               trParsCheby.push_back(c);
            }
        }
        {
            BaseFunctionFitter legendreFitter{maxOrder};
            legendreFitter.fit_parameters(samplePointsCheby, 1, samplePointsCheby.size(), LegendrePolynomial{});
            for (const double c : legendreFitter.coefficients()){
               trParsLegendre.push_back(c);
            }
            std::cout<<"Legendre t-r parameters: "<<trParsLegendre<<std::endl;
        }
        RtChebyshev trCheby{std::move(trParsCheby)};
        RtLegendre trLegendre{std::move(trParsLegendre)};

        auto canvas = std::make_unique<TCanvas>("PolyTest", "test", 800, 600 );

        auto inData{std::make_unique<TGraph>()}, chebyHisto{std::make_unique<TGraph>()}, 
             legendreHisto{std::make_unique<TGraph>()};
        inData->SetLineColor(kBlack);
        chebyHisto->SetLineColor(kRed);
        legendreHisto->SetLineColor(kBlue);


        auto fitGraphCheby{std::make_unique<TGraph>()}, fitGraphLegendre{std::make_unique<TGraph>()};
        fitGraphCheby->SetLineColor(kOrange +2);
        fitGraphLegendre->SetLineColor(kGreen-1);

        for (double t = tMin;  std::abs(tMax -t) > stepSize;t+=1 ){
            const double r = rFromT(t);
            inData->SetPoint(inData->GetN(), r, t);
            chebyHisto->SetPoint(chebyHisto->GetN(), chebyRt.radius(t), t + 2);  
            legendreHisto->SetPoint(legendreHisto->GetN(), legendreRt.radius(t), t);

            fitGraphCheby->SetPoint(fitGraphCheby->GetN(), r, trCheby.radius(r));
            fitGraphLegendre->SetPoint(legendreHisto->GetN(), r, trLegendre.radius(r));  
        }
        auto legend = std::make_unique<TLegend>(0.7,0.1,0.8,0.4);
        auto multiGraph = std::make_unique<TMultiGraph>();
        legend->AddEntry(inData.get(), "data points");
   
        legend->AddEntry(chebyHisto.get(), "chebyFit r(t)");
        legend->AddEntry(legendreHisto.get(), "legendre r(t)");

        legend->AddEntry(fitGraphCheby.get(), "chebyFit t(r)");
        legend->AddEntry(fitGraphLegendre.get(), "legendreFit t(r)");

        multiGraph->Add(inData.release());
        multiGraph->Add(chebyHisto.release());
        multiGraph->Add(legendreHisto.release());
   
        multiGraph->Add(fitGraphCheby.release());
        multiGraph->Add(fitGraphLegendre.release());

        canvas->cd();
        multiGraph->Draw("AC");
        legend->Draw();
        canvas->SaveAs(std::format("RtTest_rank{:}.pdf", maxOrder).c_str());
        
        multiGraph.reset();
        inData.reset();
        chebyHisto.reset();
        fitGraphCheby.reset();

    }
    return EXIT_SUCCESS;
}