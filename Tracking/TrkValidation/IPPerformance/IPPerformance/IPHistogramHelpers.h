#ifndef IPHISTOGRAMHELPERS_H
#define IPHISTOGRAMHELPERS_H


#include "TF1.h"
#include "TH3.h"
#include "TH2.h"
#include "TH1.h"
#include <vector>
#include <memory>
#include <string>

namespace IPHistogramHelpers {
    const int fDebug = 0;

    double GaussExpTails_f(double* x, double *par);
    double twoGaussExp_f(double* x, double *par);
    double twoGauss_f(double*x, double* par);
    double singleGauss_f(double*x, double* par);
    
    //double IPHistogramHelpers::GetBSWidth(TH1* hist);
    std::vector<float> CalcRms90(TH1* h);
    
    template <typename ToCall, typename... Args> ToCall return_type_of(ToCall(*)(Args...)); 
    using ReturnTypeOfFitting = double (*)(double*, double*);
    ReturnTypeOfFitting FitOption(TH1* hist, std::string WithFitting, std::vector<double>& initial);
    
    std::unique_ptr<TF1> IterativeGeneralFit(TH1* hist, double(*fcn)(double *, double *), const std::vector<double>& initial, const std::vector<double>& fitRange);
    void profileYwithIterativeGaussFit(TH2* hist, std::vector<TH1*> &histos, std::string WithFitting = "SingleGauss", int num_bins = 1);
    void profileZwithIterativeGaussFit(TH3* hist, std::vector<TH2*> &histos, std::string WithFitting = "SingleGauss", int num_bins = 1);
    void HistogramConditioning (TH1* hist);
    int  IterativeGaussFit(TH1* hist, std::vector<double> &values, std::vector<double> &errors, std::string WithFitting);
    
} 
#endif

