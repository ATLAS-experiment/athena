//======================================================================
// PixelCalibIBL : Read calibration file, redo the fit.
//
// Note : this program only runs on IBL.
//
//
// Mapping convention for FE in the calibration.
//
//     ^
//  phi|
//     |
//  336|   sFE0  sFE1
//     |
//    0+------------------------------>
//     0       80     160
//                                  eta
//======================================================================

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;


//  ignore the "demonstrate Xcheck" lines unless necessary
// #define DEMOXCHECK true
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <utility>
#include <map>
#include <array>
#include "TFile.h"
#include "TDirectory.h"
#include "TDirectoryFile.h"
#include "TKey.h"
#include "TH2D.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TMath.h"
#include "TF1.h"
#include "TStyle.h"
#include "TVectorT.h"
#include "THStack.h"
#include "TRandom3.h"
#include "ChargeCalibration/common/PixelMapping.h"
#include "PathResolver/PathResolver.h"

using namespace std;
using pix::PixelMapping;

//  Lianyou: simplely capacitor charging curve
double funcDisp(double *x, double *par)
{
    double ret = 9.9e10;
    ret = par[0] * 0.1 * (1. - exp(-(x[0] - 1400.) * (x[0] - 1400.) / (1000. * par[1])) + par[2] * 0.0001 * sqrt(x[0]));
    return ret;
}

double ChrgfromToT(float tot, double par[10])
{
    int lr = (tot <= 6. ? 0 : 5);
    double chrg = 1400.;

    double num = par[0 + lr] + tot * (par[1 + lr] + tot * (par[2 + lr] + tot * par[3 + lr]));
    double denom = 1. + tot * par[4 + lr];
    if (denom != 0.)
        chrg = num / denom;

    return chrg;
}

double ToTfromChrg(float chrg, double par[5])
{
    double tot = 0.;
    double num = par[0] + chrg * (par[1] + chrg * (par[2] + chrg * par[3]));
    double denom = 1. + chrg * par[4];
    if (denom != 0.0)
        tot = num / denom;
    return tot;
}

double funcRation5(double *x, double *par)
{
    double ret = 9.9e10;
    double num = par[0] + x[0] * (par[1] + x[0] * (par[2] + x[0] * par[3]));
    double denom = 1. + x[0] * par[4];
    if (denom != 0.0)
        ret = num / denom;
    return ret;
}

double funcDispConvoluted(double *x, double *par)
{
    double ret = 9.9e09;
    ret = -par[0] * (x[0] - par[1]) + par[2];
    return ret;
}

int iblCalib(const std::string& InDir, const std::string& THRscan, const std::string& scan, const std::string& scanLowCharge)
{
    // creating the object for the pixel mapping 
    PixelMapping pixmap(PathResolver::find_file("PixelCalibAlgs/mapping.csv", "DATAPATH"));

    const bool run3 = true; // new series of injection charges (22) and new format
    const float badRDfracCut = 0.2; // Cut on the fraction of bad read out per frontend
    float HDCshift = 2;

    // Shall we try this later ?
    const int finerToT = 1; //  smaller bin size for ToT if doing fit
    constexpr bool moreFE = false;
    constexpr int npsFEs = moreFE ? 8 : 2;

    gStyle->SetOptFit(1100);

#if defined(DEMOXCHECK)

    std::cout << " DEMOXCHECK will run ... " << std::endl;

    //  For debugging a small area of pixels will print their readout and injection
    int XcheckCharge[2] = {13, 14}, XcheckToT = 10;
    int XcheckPhi[2] = {57, 63}, XcheckEta[2] = {23, 29};
    TString XcheckModule = "LI_S11_A_M3_A6";

    //  fill some histograms for monitoring
    int ToTfill = 3 * finerToT, CHRGfill = 3;
    const int HDC = 2; // specifying output

#endif

    Double_t tmpRCf_H[4] = {0.868295, 0.67523, 0.732279, 0.678458}; // initial values from history calibration
    Double_t tmpRCf_T[4] = {1.02844, 1.10788, 1.07204, 1.11015};    // initial values from history calibration
    Double_t reverseCF_H[4], reverseCF_T[4];
    for (int t = 0; t < 4; t++)
    {
        reverseCF_H[t] = tmpRCf_H[t];
        reverseCF_T[t] = tmpRCf_T[t];
    }

    Double_t RDentries[33] = {0, 10494.8, 14605.6, 62952.6, 8663.57, 17320.2, 20417, 49508.9,
                              21646.8, 31503.9, 14955.6, 16290.7, 11617.5, 16461.2, 11980.5, 17210.5,
                              12773.7, 18574, 13930.9, 19777, 14629.4, 20619.5, 15191.6, 20883,
                              15411.1, 19198, 12097.4, 12109.7, 8881.96, 17235.1, 20802.3, 11265.8,
                              0.};

    std::vector<int> reversedModules = {182, 211, 213, 229, 223, 231, 246, 280, 295, 350, 357, 360, 371, 403, 416, 423};

    // outputs : something@somewhere as one like ...
    TString Outdir = "./";
    string spec = "ToTbin" + to_string(finerToT) + "_FrtEnd" + to_string(npsFEs) + "_";

#if defined(DEMOXCHECK)
    spec += "DEMOXCHECK_";
#endif

    TString StrFileName = spec.c_str() + scan;
    TString rootFileName = Outdir + "/TotChargeCalib_" + StrFileName + ".root";
    TString logFileName = Outdir + "/ChargeCalib_" + StrFileName + ".log";
    TString dbFileName = Outdir + "/ChargeCalib_" + StrFileName + ".TXT";
    if (!run3)
    {
        rootFileName = Outdir + "/TotChargeCalib_run2_" + StrFileName + ".root";
        logFileName = Outdir + "/ChargeCalib_run2_" + StrFileName + ".log";
        dbFileName = Outdir + "/ChargeCalib_run2_" + StrFileName + ".TXT";
    }

    // fetch pictures from directory :   ToT/ROD_L1_Sxx/LI_Sy_AC_Mz_ACk
    ofstream logout(logFileName);
    ofstream txtDB(dbFileName);

    TString inThrFile = "";
    TString inTotFile = "";
    TString rodPath = "";
    TString inTotFileAux = "";
    TString rodPathAux = "";

    std::cout << " Running IBL calibration analysis ... " << endl;

    inThrFile = InDir + THRscan + ".root";
    inTotFile = InDir + scan + ".root";
    rodPath = inTotFile + ":/" + scan.substr(scan.find("_")+1);
    inTotFileAux = InDir + scanLowCharge + ".root";
    rodPathAux = inTotFileAux + ":/" + scanLowCharge.substr(scanLowCharge.find("_")+1);

    // historical record for run2 fomat
    if (!run3)
    {
        inThrFile = "calib2018/IBL/THR_SCAN_S000083686.root";
        inTotFile = "calib2018/IBL/TOT_SCAN_S000083690.root";
        rodPath = inTotFile + ":/S000083690";
    }

    // selecting Q threshold
    int nrow = 336, ncol = 160; // y-axis, x-axis respectively

    const Int_t numChrgs = 22;
    Double_t IBLchrgs[numChrgs] = {1400., 1500., 1750., 2000., 2500., 3000., 3500., 4000., 5000., 6000., 8000., 10000.,
                                   12000., 14000., 16000., 18000., 20000., 22000., 24000., 26000., 28000., 30000.};

    const Int_t nchargeIBL = (run3 ? 22 : 19);

    int skip = numChrgs - nchargeIBL;
    Double_t chrgAbaciIBL[nchargeIBL], chargeErrArrIBL[nchargeIBL];
    for (int c = 0; c < nchargeIBL; c++)
    {
        chrgAbaciIBL[c] = IBLchrgs[c + skip];
        chargeErrArrIBL[c] = 0.;
    }

    Double_t chrgsbins[nchargeIBL + 1];
    for (int c = 1; c < nchargeIBL; c++)
        chrgsbins[c] = 0.5 * (chrgAbaciIBL[c - 1] + chrgAbaciIBL[c]);

    chrgsbins[0] = chrgAbaciIBL[0] - 0.5 * (chrgAbaciIBL[1] - chrgAbaciIBL[0]);
    chrgsbins[nchargeIBL] = chrgAbaciIBL[nchargeIBL - 1] + 0.5 * (chrgAbaciIBL[nchargeIBL - 1] - chrgAbaciIBL[nchargeIBL - 2]);

    //  please note extra +1  for ending bins.
    const Int_t nToTibl = 16 * finerToT + 1;

    Double_t totAbaci[nToTibl], totbins[nToTibl + 1];

    totbins[0] = -0.25;
    totbins[1] = 0.5;
    totAbaci[0] = 0.0;
    totAbaci[1] = 1.;
    for (int t = 2; t < nToTibl; t++)
    {
        totAbaci[t] = totAbaci[1] + (t - 1) / (1. * finerToT);
        totbins[t] = 0.5 * (totAbaci[t] + totAbaci[t - 1]);
    }
    totbins[nToTibl] = totAbaci[nToTibl - 1] + 0.5 * (totbins[nToTibl - 1] - totbins[nToTibl - 2]);

    Double_t totErrArr[nToTibl];
    for (int t = 0; t < nToTibl; t++)
        totErrArr[t] = 0.;

#if defined(DEMOXCHECK)
    for (int t = 0; t < nToTibl; t++)
        logout << " totAbaci : " << t << " " << totbins[t] << " < " << totAbaci[t] << " > " << totbins[t + 1] << endl;

    if (finerToT == 1)
    {
        for (int t = 1; t < nToTibl; t++)
        {
            RDentries[t] = RDentries[2 * t - 1] + RDentries[2 * t];
            logout << " finer RDentries : " << t << " " << 2 * t - 1 << " " << 2 * t << " " << RDentries[t] << endl;
        }
    }
#endif

    map<string, map<string, map<string, float>>> pcdMap;

    TFile roFile(rootFileName, "RECREATE");
    TDirectory *roThrDir = roFile.mkdir("Threshold");
    TDirectory *roTotDir = roFile.mkdir("ToT");

    // ****************************** Threshold IBL ******************************

    Double_t THR_avg[2][npsFEs], ThrSig_avg[2][npsFEs];
    array<std::unique_ptr<TH2D> , npsFEs> h2_Thr{};
    array<std::unique_ptr<TH2D> , npsFEs> h2_ThrSig{};

    std::multimap<float, TString, std::greater<float>> badThr_Order;

    for (int sfe = 0; sfe < npsFEs; sfe++)
    {
        stringstream ss;
        ss << sfe;

        string knowModule = "LI_S06_C_M1_C1";
        string idx = "I" + ss.str();

        pcdMap[knowModule][idx]["ThrNorm"] = -42.;
        pcdMap[knowModule][idx]["ThrRmsNorm"] = -42.;
        pcdMap[knowModule][idx]["ThrSigNorm"] = -42.;
        pcdMap[knowModule][idx]["ThrLong"] = -42.;
        pcdMap[knowModule][idx]["ThrRmsLong"] = -42.;
        pcdMap[knowModule][idx]["ThrSigLong"] = -42.;

        for (int nl = 0; nl < 2; nl++)
            THR_avg[nl][sfe] = ThrSig_avg[nl][sfe] = -99.;

        idx = "Threshold" + ss.str();
        h2_Thr[sfe] = std::make_unique<TH2D>(idx.c_str(), " ", 2, 0, 2, 200, 0, 5000);
        idx = "ThresholdSig" + ss.str();
        h2_ThrSig[sfe] =  std::make_unique<TH2D>(idx.c_str(), " ", 2, 0, 2, 200, 0, 500);
    }

    if (inThrFile.Length() > 0)
    {
        std::cout << endl
                  << "INFO =>> [IBL] threshold scan analysis..." << endl;

        TFile riThrFile(inThrFile, "READ");
        TString chi2HistName = "SCURVE_CHI2";
        TString thrHistName = "SCURVE_MEAN";
        TString sigHistName = "SCURVE_SIGMA";

        std::unique_ptr<TH1F> h1dChi2 = std::make_unique<TH1F>("h1dChi2", "", 200, 0, 1);
        std::unique_ptr<TH1F> h1dThr  = std::make_unique<TH1F>("h1dThr" , "", 200, 0, 5000);
        std::unique_ptr<TH1F> h1dSig  = std::make_unique<TH1F>("h1dSig" , "", 200, 0, 500);

        TDirectoryFile *scanDir = (TDirectoryFile *)((TKey *)riThrFile.GetListOfKeys()->First())->ReadObj();
        TList *rodKeyList = (TList *)scanDir->GetListOfKeys();
        TIter rodItr(rodKeyList);
        TKey *rodKey;

        while ((rodKey = (TKey *)rodItr()))
        {   
            TString rodName(rodKey->GetName());
            TDirectoryFile *rodDir = (TDirectoryFile *)rodKey->ReadObj();
            TList *modKeyList = (TList *)rodDir->GetListOfKeys();
            TIter modItr(modKeyList);
            TKey *modKey;

            TDirectory *dirRod = roThrDir->mkdir(rodName);

            while ((modKey = (TKey *)modItr()))
            {
                TString modName(modKey->GetName());
                string modStr(modKey->GetName());
                TDirectory *dirMod = dirRod->mkdir(modName);

                TString chi2HistDirPath = modName + "/" + chi2HistName + "/A0/B0";

                TDirectory *chi2HistDir = (TDirectory *)rodDir->Get(chi2HistDirPath);
                if (chi2HistDir == NULL)
                {
                    cout << " Warning : NULL dir " << endl;
                    continue;
                }

                std::unique_ptr<TH2D> h2dChi2 (static_cast<TH2D*>(static_cast<TKey*>(chi2HistDir->GetListOfKeys()->First())->ReadObj()));
                if (h2dChi2 == NULL)
                {
                    cout << " Warning : NULL dir " << endl;
                    continue;
                }
                TString thrHistDirPath = modName + "/" + thrHistName + "/A0/B0";
                TDirectoryFile *thrHistDir = (TDirectoryFile *)rodDir->Get(thrHistDirPath);
                std::unique_ptr<TH2D> h2dThr(static_cast<TH2D*>(static_cast<TKey*>(thrHistDir->GetListOfKeys()->First())->ReadObj()));
                TString sigHistDirPath = modName + "/" + sigHistName + "/A0/B0";

                TDirectoryFile *sigHistDir = (TDirectoryFile *)rodDir->Get(sigHistDirPath);
                std::unique_ptr<TH2D> h2dSig (static_cast<TH2D*>(static_cast<TKey*>(sigHistDir->GetListOfKeys()->First())->ReadObj()));

                array<std::unique_ptr<TH1F>, npsFEs> h1_ThrNorm{};
                array<std::unique_ptr<TH1F>, npsFEs> h1_ThrSigNorm{};
                array<std::unique_ptr<TH1F>, npsFEs> h1_ThrLong{};
                array<std::unique_ptr<TH1F>, npsFEs> h1_ThrSigLong{};
                array<float, npsFEs> IlledThr{};

                for (int sfe = 0; sfe < npsFEs; sfe++)
                {
                    stringstream ss;
                    ss << sfe;

                    string hname = modStr + "ThrNorm" + ss.str();
                    h1_ThrNorm[sfe] = std::make_unique<TH1F>(hname.c_str(), "", 200, 0, 5000);
                    hname = modStr + "ThrSigNorm" + ss.str();
                    h1_ThrSigNorm[sfe] = std::make_unique<TH1F>(hname.c_str(), "", 200, 0, 500);
                    hname = modStr + "ThrLong" + ss.str();
                    h1_ThrLong[sfe] = std::make_unique<TH1F>(hname.c_str(), "", 200, 0, 5000);
                    hname = modStr + "ThrSigLong" + ss.str();
                    h1_ThrSigLong[sfe] = std::make_unique<TH1F>(hname.c_str(), "", 200, 0, 500);
                }

                for (int col = 1; col <= ncol; col++)
                {
                    for (int row = 1; row <= nrow; row++)
                    {
                        float chi2 = h2dChi2->GetBinContent(col, row);
                        float thr = h2dThr->GetBinContent(col, row);
                        float sig = h2dSig->GetBinContent(col, row);

                        h1dChi2->Fill(chi2);
                        h1dThr->Fill(thr);
                        h1dSig->Fill(sig);
                        //  Shall we measure/count the goodness of THR scan here ?
                        bool filled = true;
                        if (thr == 0 || thr > 10000 || sig == 0 || sig > 1000 || chi2 > 0.5 || chi2 <= 0)
                        {
                            filled = false;
                        }
                        int circ = -1;
                        if (filled)
                        {
                            if (col == 1 || col == ncol / 2 || col == ncol / 2 + 1 || col == ncol)
                            {
                                if (moreFE)
                                {
                                    circ = (int)(row / 84.);
                                    if (col > 80)
                                    {
                                        circ = 7 - circ;
                                    }
                                }
                                else
                                {
                                    if (col <= ncol / 2)
                                        circ = 0;
                                    else
                                        circ = 1;
                                }

                                h1_ThrLong[circ]->Fill(thr);
                                h1_ThrSigLong[circ]->Fill(sig);
                            }
                            else
                            {
                                if (moreFE)
                                {
                                    circ = (int)(row / 84.);
                                    if (col > 80)
                                    {
                                        circ = 7 - circ;
                                    }
                                }
                                else
                                {
                                    if (col <= ncol / 2)
                                        circ = 0;
                                    else
                                        circ = 1;
                                }

                                h1_ThrNorm[circ]->Fill(thr);
                                h1_ThrSigNorm[circ]->Fill(sig);
                            }
                        }
                        else
                        {
                            // Note: what is the intention? circ is -1 here
                            continue;
                        }
                    }
                }
                h2dSig.reset();
                h2dThr.reset();
                h2dChi2.reset();

                for (int sfe = 0; sfe < npsFEs; sfe++)
                {
                    stringstream ss;
                    ss << sfe;
                    string feName = "I" + ss.str();

                    float THRnorm = h1_ThrNorm[sfe]->GetMean();
                    bool valid = THRnorm > 100.;
                    pcdMap[modStr][feName]["ThrNorm"] = THRnorm;
                    pcdMap[modStr][feName]["ThrRmsNorm"] = h1_ThrNorm[sfe]->GetRMS();
                    float THRnormSig = h1_ThrSigNorm[sfe]->GetMean();
                    pcdMap[modStr][feName]["ThrSigNorm"] = THRnormSig;
                    float THRlong = h1_ThrLong[sfe]->GetMean();
                    pcdMap[modStr][feName]["ThrLong"] = THRlong;
                    pcdMap[modStr][feName]["ThrRmsLong"] = h1_ThrLong[sfe]->GetRMS();
                    float THRlongSig = h1_ThrSigLong[sfe]->GetMean();
                    pcdMap[modStr][feName]["ThrSigLong"] = THRlongSig;

                    float blank = 100. * IlledThr[sfe] * npsFEs / (1. * ncol * nrow);

                    if (blank > 0.01)
                        badThr_Order.insert(std::pair<float, TString>(blank, (TString)(modName + " : " + blank)));

                    if (valid)
                    {
                        h2_Thr[sfe]->Fill(0.5, THRnorm, 1);
                        h2_Thr[sfe]->Fill(1.5, THRlong, 1);
                        h2_ThrSig[sfe]->Fill(0.5, THRnormSig, 1);
                        h2_ThrSig[sfe]->Fill(1.5, THRlongSig, 1);
                    }
                    dirMod->WriteTObject(h1_ThrNorm[sfe].get());
                    dirMod->WriteTObject(h1_ThrLong[sfe].get());
                    
                    h1_ThrNorm[sfe].reset();
                    h1_ThrSigNorm[sfe].reset();
                    h1_ThrLong[sfe].reset();
                    h1_ThrSigLong[sfe].reset();
                }
            }
        }
        roThrDir->WriteTObject(h1dChi2.get());
        h1dChi2.reset();
        roThrDir->WriteTObject(h1dThr.get());
        h1dThr.reset();
        roThrDir->WriteTObject(h1dSig.get());
        h1dSig.reset();
    }

    for (int sfe = 0; sfe < npsFEs; sfe++)
    {
        stringstream ss;
        ss << sfe;
        string hName = "thrNorm" + ss.str();
        THR_avg[0][sfe] = h2_Thr[sfe]->ProjectionY(hName.c_str(), 1, 1)->GetMean();
        hName = "thrLong" + ss.str();
        THR_avg[1][sfe] = h2_Thr[sfe]->ProjectionY(hName.c_str(), 2, 2)->GetMean();
        hName = "thrSigNorm" + ss.str();
        ThrSig_avg[0][sfe] = h2_ThrSig[sfe]->ProjectionY(hName.c_str(), 1, 1)->GetMean();
        hName = "thrSigLong" + ss.str();
        ThrSig_avg[1][sfe] = h2_ThrSig[sfe]->ProjectionY(hName.c_str(), 2, 2)->GetMean();
        h2_Thr[sfe].reset();
        h2_ThrSig[sfe].reset();
    }
    // ****************************** End - Threshold IBL ******************************

    /////////////////////////////////////////////////////////////////////////////////
    /*************************     IBL  ToT ***************************/
    /////////////////////////////////////////////////////////////////////////////////

    if (inTotFile.Length() == 0 || inTotFileAux.Length() == 0)
    {
        logout << " Missing ToT file from calib scan as input " << endl;
        return 0;
    }

    std::cout << endl
              << "INFO =>> [IBL] tot calib analysis..." << endl;

    TFile riTotFile(inTotFile, "READ");
    TFile riTotFileAux(inTotFileAux, "READ");

    TString totHistName = "TOT_MEAN";
    TString totSigHistName = "TOT_SIGMA";

    TDirectoryFile *scanDir = (TDirectoryFile *)((TKey *)riTotFile.GetListOfKeys()->First())->ReadObj();
    TList *rodKeyList = (TList *)scanDir->GetListOfKeys();
    TIter rodItr(rodKeyList);
    TKey *rodKey;

    std::map<float, std::pair<vector<TString>, vector<Double_t>>> ModuDataToPrint;
    int totModNum = 0;

#if defined(DEMOXCHECK)
    vector<TH1F *> h1_ChrgEntry;
    h1_ChrgEntry.reserve(nToTibl);

    vector<TH1F *> h1d_totSprdAll;
    h1d_totSprdAll.reserve(nToTibl - 1);

    for (int t = 0; t < nToTibl; t++)
    {
        stringstream tt;
        tt << t;

        string hname = "ChrgEntries_ToT_" + tt.str();
        h1_ChrgEntry[t] = new TH1F(hname.c_str(), "RD entries ", 100, 0., 100000);

        hname = "totSprdsFll_ToT_" + tt.str();
        if (t < nToTibl - 1)
            h1d_totSprdAll[t] = new TH1F(hname.c_str(), "ToT spread ", 36, 0.2, 1.1);
    }
#endif

    const Int_t totFE = 14 * 16 * npsFEs; //  16 modules on each of 14 ReadOutDisk
    Double_t TotArray[totFE][nchargeIBL], TotErrArray[totFE][nchargeIBL];
    Double_t TotSigArray[totFE][nchargeIBL], TotSigErrArray[totFE][nchargeIBL];
    Double_t ChrgArray[totFE][nToTibl], ChrgErrArray[totFE][nToTibl];

    gRandom = new TRandom3(2203);

    float occuPhiEta[nchargeIBL][totFE];

    Int_t cntRod = 0;
    std::map<float, TString> devChrg_Order;
    std::map<float, TString> devToT_Order;

    std::multimap<float, TString, std::greater<float>> badModules_Order;
    std::multimap<float, TString, std::greater<float>> badModules_Order_detailed;
    
    while ((rodKey = (TKey *)rodItr()))
    {
        TString rodName(rodKey->GetName());
        string rodStr(rodKey->GetName());
        TDirectoryFile *rodDir = (TDirectoryFile *)rodKey->ReadObj();

        TString path = rodDir->GetPath();
        TString pathAux = path.ReplaceAll(rodPath, rodPathAux);

        TDirectoryFile *rodDirAux = (TDirectoryFile *)riTotFileAux.Get(pathAux);
        if (rodDirAux == NULL)
        {
            std::cout<< " Fail to get the rodPath in Aux: "<< pathAux << endl;
            logout << " Fail to get the rodPath in Aux " << endl;
            continue;
        }

        TDirectory *dirRod = roTotDir->mkdir(rodName);

        TList *modKeyList = (TList *)rodDir->GetListOfKeys();
        TIter modItr(modKeyList);

        TKey *modKey;

        int cntMod = 0;
        string modNames[16 * npsFEs];

        float occuChrgs[nToTibl][16 * npsFEs];

#if defined(DEMOXCHECK)
        TString feName_maxDevChrg = "", feName_maxDevToT = "";
        float maxDevChrg = -9., maxDevToT = -9., avgDevChrg = 0., avgDevToT = 0.;

        vector<TH1F *> h1d_totSprd;
        h1d_totSprd.reserve(nToTibl - 1);

        for (int t = 0; t < nToTibl - 1; t++)
        {
            stringstream tt;
            tt << t + 1;

            string prfmodname = "ToT_Sprd_" + tt.str();
            h1d_totSprd[t] = new TH1F(prfmodname.c_str(), "ToT spread ", 28, 0.3, 1.0);
            prfmodname = rodStr + "ToT spread @ ToT_" + tt.str();
            h1d_totSprd[t]->SetTitle(prfmodname.c_str());
        }
#endif
        
        while ((modKey = (TKey *)modItr()))
        {
            TString modName(modKey->GetName());
            string modStr(modKey->GetName());

            TDirectory *dirMod = dirRod->mkdir(modName);
            bool ibl3Dfe0 = false, ibl3Dfe1 = false;

            int hashID = -1, hashIDL = -1, hashIDR = -1;
            int bec = -1;
            int layer = -1;
            int phi_module = -1;
            int eta_module = -1;
            int eta_moduleL = -1, eta_moduleR = -1;
            pixmap.mapping(modStr, &hashID, &bec, &layer, &phi_module, &eta_module);
            if (hashID == -1)
            {
                pixmap.mapping(modStr + "_1", &hashID, &bec, &layer, &phi_module, &eta_module);
                if (hashID != -1)
                {
                    ibl3Dfe0 = true;
                    hashIDL = hashID;
                    eta_moduleL = eta_module;
                }

                pixmap.mapping(modStr + "_2", &hashID, &bec, &layer, &phi_module, &eta_module);
                if (hashID != -1)
                {
                    ibl3Dfe1 = true;
                    hashIDR = hashID;
                    eta_moduleR = eta_module;
                }
            }
            if (hashID == -1){
                std::cout<< "negative hash ID\n";
                continue;
            }
                

            if (pcdMap.find(modStr) == pcdMap.end()){
                std::cout<< "MOD not found "<< modStr << "\n";
                continue;
            }
                

            //  a stack of Charge-ToT data for ToT-charge converting
            array<std::unique_ptr<TH2D>, npsFEs> h2d_XchrgYtot{};
            array<std::unique_ptr<TH2D>, npsFEs> h2d_XchrgYToTSig{};

            for (int sfe = 0; sfe < npsFEs; sfe++)
            {
                stringstream ss;
                ss << sfe;

                modNames[cntMod * npsFEs + sfe] = modStr + "-" + ss.str();

                string prfmodname = modStr + "ToTvsChrg_FE" + ss.str();
                h2d_XchrgYtot[sfe] = std::make_unique<TH2D>(prfmodname.c_str(), "ToT vs Charge", nchargeIBL, chrgsbins, nToTibl, totbins);

                prfmodname = modStr + "ToT_Sig_vsChrg_FE" + ss.str();
                h2d_XchrgYToTSig[sfe] = std::make_unique<TH2D>(prfmodname.c_str(), "ToT Sig vs Charge", nchargeIBL, chrgsbins, 100, 0., 1.);
            }

            // Fill ToT-Chrg for pixels per module
            for (int c = 0; c < nchargeIBL; c++)
            {
                TString totHistDirPath = modName + "/" + totHistName + "/A0/B0/C";
                totHistDirPath += std::to_string(c);
                
                TDirectoryFile* totHistDir(static_cast<TDirectoryFile*> (rodDir->Get(totHistDirPath)));
                if(!totHistDir){
                    std::cout<<" Missing totHistDir in : " << totHistDirPath << endl;
                }
                
                std::unique_ptr<TH2F> h2dTot(static_cast<TH2F*> ((static_cast<TKey*>(totHistDir->GetListOfKeys()->First()))->ReadObj()));
                h2dTot->SetDirectory(0);
                
                TDirectoryFile* totHistDirAux(static_cast<TDirectoryFile*>(rodDirAux->Get(totHistDirPath)));
                if (!totHistDirAux)
                {
                    std::cout<<" Missing totHistDir in : " << totHistDirPath << endl;
                    logout << " Missing totHistDir in : " << totHistDirPath << endl;
                }
                
                unique_ptr<TH2F> h2dTotAux (static_cast<TH2F*> ((static_cast<TKey*>(totHistDirAux->GetListOfKeys()->First())->ReadObj())));    
                h2dTotAux->SetDirectory(0);

                
                TString totSigHistDirPath = modName + "/" + totSigHistName + "/A0/B0/C";
                totSigHistDirPath += std::to_string(c);
                TDirectoryFile* totSigHistDir (static_cast<TDirectoryFile*>(rodDir->Get(totSigHistDirPath)));
                unique_ptr<TH2F> h2dTotSig (static_cast<TH2F*> ((static_cast<TKey*>(totSigHistDir->GetListOfKeys()->First()))->ReadObj()));
                h2dTotSig->SetDirectory(0);
                
                // Now one fill the calib-data from all pixles as basics.
                for (int ieta = 0; ieta < ncol; ieta++)
                {
                    for (int iphi = 0; iphi < nrow; iphi++)
                    {
                        int circ = -1;
                        if (moreFE)
                        {
                            //  virtual FE with 80*84 pixels, so that sFE0, sFE1, sFE7, sFE6 will make up the physical 1'st FE
                            // while sFE2, sFE3, sFE5, sFE4 make up the physical 2'nd.
                            circ = (int)(iphi / 84.); // sFE0, sFE1, sFE2, sFE3
                            if (ieta > 80)
                            {
                                circ = 7 - circ;
                            } // sFE7, sFE6, sFE5, sFE4
                        }
                        else
                        {
                            if (ieta < ncol / 2)
                                circ = 0;
                            else
                                circ = 1;
                        }

                        float tot = h2dTot->GetBinContent(ieta + 1, iphi + 1) + HDCshift;
                        float totAux = h2dTotAux->GetBinContent(ieta + 1, iphi + 1);

                        //  use ToT from higher HDC as default
                        float filltot = tot;

                        //  use ToT from lower HDC for SMALL charges, dirty in studying
                        //  May 4, 2022, please keep below strategy, it give reasonable merging till now

                        if (tot <= 5 || c <= 5)
                        {
                            if ((c == 0 && tot - totAux > 0.8) || (c >= 1 && c <= 3 && tot - totAux > 0.5) || (c > 4 && tot - totAux > 0.3))
                                filltot = totAux;
                            else
                                filltot = 0.5 * (totAux + tot);
                            if (c == 0 && totAux > 2.)
                                filltot = 1.;
                        }

                        if (c > 19)
                            filltot = 0.5 * (totAux + tot + 2.);

                        if (filltot > 0.){
                            h2d_XchrgYtot[circ]->Fill(chrgAbaciIBL[c], filltot, 1.);
                        }
                            

                        float err = h2dTotSig->GetBinContent(ieta + 1, iphi + 1);
                        h2d_XchrgYToTSig[circ]->Fill(chrgAbaciIBL[c], err, 1.);
                        if (err == 0.)
                            err = 0.05;

#if defined(DEMOXCHECK)
                        if (modName == XcheckModule && (iphi >= XcheckPhi[0] && iphi <= XcheckPhi[1]) && (ieta >= XcheckEta[0] && ieta <= XcheckEta[1]) && (c == XcheckCharge[0] || c == XcheckCharge[1] || (int)(tot + 0.5) == XcheckToT || (int)(totAux + 0.5) == XcheckToT))
                            logout << " tot = " << tot << ", totAux = " << totAux << " @injection " << chrgAbaciIBL[c] << " " << err << endl;
#endif
                    }
                } //  all pixels done over a module

                h2dTotSig.reset();
                h2dTotAux.reset();
                h2dTot.reset();

            } //  end looping over charges
            float modHash = -1;
            if (!(ibl3Dfe0 || ibl3Dfe1))
            {
                modHash = hashID;
                totModNum++;
            }

            array<TDirectory *, npsFEs> dirFE;

            Int_t idxMod = cntRod * 16 * npsFEs + cntMod * npsFEs;

            for (int sfe = 0; sfe < npsFEs; sfe++)
            {   
                stringstream ss;
                ss << sfe;

                if (ibl3Dfe0 && sfe < npsFEs / 2)
                {
                    modHash = hashIDL;
                    totModNum++;
                }
                else if (ibl3Dfe1 && sfe >= npsFEs / 2)
                {
                    modHash = hashIDR;
                    totModNum++;
                }
                else
                    modHash += (sfe < npsFEs / 2 ? 0 : 1) * 0.8;

                string prfmodname = "DirFE_" + ss.str();
                dirFE[sfe] = dirMod->mkdir(prfmodname.c_str());

                //  standard deviation will be adopted for ERROR
                prfmodname = modStr + "_profile_Tot_FE" + ss.str();
                std::unique_ptr<TProfile> prfl_TotsFE(h2d_XchrgYtot[sfe]->ProfileX(prfmodname.c_str(), 1, -1, "s"));
                prfl_TotsFE->SetTitle(prfmodname.c_str());
                prfmodname = modStr + "_profile_Chrg_FE" + ss.str();
                // shanly : seeming free from ROOT-7770 issue.  2021 Sep 17
                std::unique_ptr<TProfile> prfl_ChrgsFE(h2d_XchrgYtot[sfe]->ProfileY(prfmodname.c_str(), 1, -1, "s"));
                prfl_ChrgsFE->SetTitle(prfmodname.c_str());

                Double_t TotArr[nchargeIBL], TotErrArr[nchargeIBL];
                Double_t ChrgArr[nToTibl], ChrgErrArr[nToTibl];

                Int_t idxFE = idxMod + sfe;
                //   ToT distribution along nchargeIBL bins charges
                int dumbFE = 0;
                for (int c = 0; c < nchargeIBL; c++)
                {
                    TotArr[c] = TotArray[idxFE][c] = prfl_TotsFE->GetBinContent(c + 1);
                    TotErrArr[c] = TotErrArray[idxFE][c] = prfl_TotsFE->GetBinError(c + 1);

                    occuPhiEta[c][idxFE] = prfl_TotsFE->GetBinEntries(c + 1) * npsFEs / (1. * ncol * nrow);

                    if ((1. - occuPhiEta[c][idxFE]) > badRDfracCut)
                        dumbFE++;

                    stringstream sc;
                    sc << c;
                    prfmodname = modStr + "_ToTsig_FE" + ss.str() + "Chrg" + sc.str();

                    std::unique_ptr<TH1D> h_ToTsig(h2d_XchrgYToTSig[sfe]->ProjectionY(prfmodname.c_str(), c + 1, c + 1));
                    TotSigArray[idxFE][c] = TMath::Sqrt(h_ToTsig->GetMean() * h_ToTsig->GetMean() + h_ToTsig->GetRMS() * h_ToTsig->GetRMS());
                    TotSigErrArray[idxFE][c] = TMath::Sqrt(h_ToTsig->GetMeanError() * h_ToTsig->GetMeanError() + h_ToTsig->GetRMSError() * h_ToTsig->GetRMSError());

                    h_ToTsig.reset();
                }
                prfl_TotsFE.reset();

                if (dumbFE > 5)
                {
                    for (int c = 0; c < nchargeIBL; c++)
                    {
                        TotArr[c] = TotArray[idxFE][c] = 0.5 * (TotArray[idxFE - 1][c] + TotArray[idxFE - 2][c]);
                        TotErrArr[c] = TotErrArray[idxFE][c] = 0.5 * (TotErrArray[idxFE - 1][c] + TotErrArray[idxFE - 2][c]);

                        TotSigArray[idxFE][c] = 0.5 * (TotSigArray[idxFE - 1][c] + TotSigArray[idxFE - 2][c]);
                        TotSigErrArray[idxFE][c] = 0.5 * (TotSigErrArray[idxFE - 1][c] + TotSigErrArray[idxFE - 2][c]);

                    }
                }
                // Charges distribution along 16 bins ToT
                Double_t errToT_overChrg[nToTibl];
                for (int t = 0; t < nToTibl; t++)
                {
                    ChrgArr[t] = ChrgArray[idxFE][t] = prfl_ChrgsFE->GetBinContent(t + 1);
                    ChrgErrArr[t] = ChrgErrArray[idxFE][t] = prfl_ChrgsFE->GetBinError(t + 1);

                    occuChrgs[t][cntMod * npsFEs + sfe] = prfl_ChrgsFE->GetBinEntries(t + 1) / RDentries[t];

                    stringstream st;
                    st << t;
                    prfmodname = modStr + "_Chrg_FE" + ss.str() + "ToT" + st.str();
                    std::unique_ptr<TH1D> h_chrg(h2d_XchrgYtot[sfe]->ProjectionX(prfmodname.c_str(), t + 1, t + 1));
                    st.str("");
                    st << totAbaci[t];
                    prfmodname = modStr + " Charge_FE" + ss.str() + " @ ToT : " + st.str();
                    h_chrg->SetTitle(prfmodname.c_str());

                    Double_t scl = h_chrg->Integral();
                    errToT_overChrg[t] = 0.;
                    if (scl > 0.)
                        for (int b = 0; b < nchargeIBL; b++)
                        {
                            errToT_overChrg[t] += TotErrArr[b] * (h_chrg->GetBinContent(b + 1)) / scl;
                        }
                    if (errToT_overChrg[t] == 0.)
                        errToT_overChrg[t] = 1.1;

                    h_chrg.reset();
                }
                prfl_ChrgsFE.reset();
#if defined(DEMOXCHECK)
                for (int b = 1; b < nToTibl; b++)
                {
                    double err = errToT_overChrg[b];
                    h1d_totSprd[b - 1]->Fill(err);
                    h1d_totSprdAll[b - 1]->Fill(err);
                }
#endif

                //  try a correction upon dumb FE
                if (dumbFE > 5)
                {
                    for (int t = 0; t < nToTibl; t++)
                    {
                        ChrgArr[t] = ChrgArray[idxFE][t] = 0.5 * (ChrgArray[idxFE - 1][t] + ChrgArray[idxFE - 2][t]);
                        ChrgErrArr[t] = ChrgErrArray[idxFE][t] = 0.5 * (ChrgErrArray[idxFE - 1][t] + ChrgErrArray[idxFE - 2][t]);
                    }
                }
                bool xoticMod = std::find(reversedModules.begin(), reversedModules.end(), int(modHash)) != reversedModules.end();

                //  when charges' order are distorted along ToTs
                bool reverseH = false, reverseT = false;
                for (int t = 4; t >= 1; t--)
                    if (ChrgArr[t] > ChrgArr[t + 1])
                    {
                        reverseH = true;
                        break;
                    }
                for (int t = 4; t >= 1; t--)
                {
                    int tl = nToTibl - t;
                    if (ChrgArr[tl] < ChrgArr[tl - 1])
                    {
                        reverseT = true;
                        break;
                    }
                }

                if (reverseH)
                {
#if defined(DEMOXCHECK)
                    logout << " Head reversed : " << int(modHash) << " " << modName << " " << sfe << " " << ChrgArr[1] << " "
                           << ChrgArr[2] << " " << ChrgArr[3] << " " << ChrgArr[4] << " " << ChrgArr[5] << std::endl;
#endif
                    //  try a correction with some averaged factors
                    for (int t = 4; t >= 1; t--)
                        if (ChrgArr[t] > ChrgArr[t + 1] && (occuChrgs[t][cntMod * npsFEs + sfe] < 0.5 || xoticMod))
                            ChrgArr[t] = ChrgArray[idxFE][t] = reverseCF_H[t - 1] * ChrgArr[t + 1];
                }
                else // prepare the average factors along ToTs
                {
                    for (int t = 4; t >= 1; t--)
                        if (ChrgArr[t + 1] != 0.)
                            reverseCF_H[t - 1] = (reverseCF_H[t - 1] == tmpRCf_H[t - 1] ? ChrgArr[t] / ChrgArr[t + 1] : 0.5 * (reverseCF_H[t - 1] + ChrgArr[t] / ChrgArr[t + 1]));
                }

                if (reverseT)
                {
#if defined(DEMOXCHECK)
                    logout << " Tail reversed : " << int(modHash) << " " << modName << " " << sfe << " " << ChrgArr[nToTibl - 5] << " "
                           << ChrgArr[nToTibl - 4] << " " << ChrgArr[nToTibl - 3] << " " << ChrgArr[nToTibl - 2] << " " << ChrgArr[nToTibl - 1] << std::endl;
#endif
                    //  try a correction with some averaged factors
                    for (int t = 4; t >= 1; t--)
                    {
                        int tl = nToTibl - t;
                        if (ChrgArr[tl] < ChrgArr[tl - 1] && (occuChrgs[tl][cntMod * npsFEs + sfe] < 0.5 || xoticMod))
                            ChrgArr[tl] = ChrgArray[idxFE][tl] = reverseCF_T[t - 1] * ChrgArr[tl - 1];
                    }
                }
                else // prepare the average factors along ToTs
                {
                    for (int t = 4; t >= 1; t--)
                    {
                        int tl = nToTibl - t;
                        if (ChrgArr[tl - 1] != 0.)
                            reverseCF_T[t - 1] = (reverseCF_T[t - 1] == tmpRCf_T[t - 1] ? ChrgArr[tl] / ChrgArr[tl - 1] : 0.5 * (reverseCF_T[t - 1] + ChrgArr[tl] / ChrgArr[tl - 1]));
                    }
                }

                string gername = modStr + "_grToTsig_FE_" + ss.str();
                std::unique_ptr<TGraphErrors> grTotSig = std::make_unique<TGraphErrors>(nchargeIBL, chrgAbaciIBL, TotSigArray[sfe], chargeErrArrIBL, TotSigErrArray[sfe]);
                grTotSig->SetTitle(gername.c_str());
                grTotSig->SetName(gername.c_str());

                std::unique_ptr<TGraph> grTotSprd = std::make_unique<TGraph>(nToTibl, totAbaci, errToT_overChrg);
                gername = "ToTsprd";
                grTotSprd->SetName(gername.c_str());
                gername = modStr + " ToT spread over FrontEnd @ FE " + ss.str();
                grTotSprd->SetTitle(gername.c_str());
                grTotSprd->SetLineColor(4);
                grTotSprd->SetMarkerStyle(4);
                grTotSprd->SetMarkerSize(1.);

                TF1 ToTres("ToTres", funcDispConvoluted, 0.8, 16.5, 3);
                ToTres.SetParameter(1,13.);
                ToTres.SetParameter(2,0.65);
                ToTres.SetParLimits(1, 10., 16.);
                ToTres.SetParLimits(2, 0.3, 1.0);

                grTotSprd->Fit(&ToTres, "MRQ");
                Double_t parP0 = ToTres.GetParameter(0);
                Double_t parP1 = ToTres.GetParameter(1);

                dirFE[sfe]->WriteTObject(grTotSprd.get());
                grTotSprd.reset();
                //  Lets' keep the bad 2-para fit untill the dataBase will be updated someday
                /**
                        TF1 f1Disp( "f1Disp", funcDisp_old, chrgAbaciIBL[1], chrgAbaciIBL[nchargeIBL-1], 2 );
                        grTotSig->Fit( &f1Disp,"MRQ" );
                        Double_t parP0 = 1.*f1Disp.GetParameter(0);
                        Double_t parP1 = 1.*f1Disp.GetParameter(1);
                        Double_t parP2 = 0. ;

                        TF1 f1Disp( "f1Disp", funcDisp, chrgAbaciIBL[1], chrgAbaciIBL[nchargeIBL-1], 3 );
                        f1Disp.SetParLimits( 0, 3, 7 ) ;
                        f1Disp.SetParLimits( 1, 300., 1000. ) ;
                        f1Disp.SetParLimits( 2, -5, 5 ) ;

                        grTotSig->Fit( &f1Disp,"MRQ" );
                        Double_t parP0 = 1.*f1Disp.GetParameter(0);
                        Double_t parP1 = 1.*f1Disp.GetParameter(1);
                        Double_t parP2 = 1.*f1Disp.GetParameter(2);
                
                paraToTRes[sfe][0] = parP0;
                paraToTRes[sfe][1] = parP1;
                paraToTRes[sfe][2] = parP2;
                **/

                gername = modStr + "_grToT_FE_" + ss.str();
                std::unique_ptr<TGraphErrors> grTot = std::make_unique<TGraphErrors>(nchargeIBL, chrgAbaciIBL, TotArr, chargeErrArrIBL, TotErrArr);
                grTot->SetTitle(gername.c_str());
                grTot->SetName(gername.c_str());
                TF1 *f1ToTfromCharge = new TF1("ToTfromCharge", funcRation5, chrgAbaciIBL[0] - 100., chrgAbaciIBL[nchargeIBL - 1] + 300., 5);

                grTot->Fit(f1ToTfromCharge, "MRQ");

                std::unique_ptr<TGraphErrors> grChrg = std::make_unique<TGraphErrors>(nToTibl, totAbaci, ChrgArr, totErrArr, ChrgErrArr);

                gername = modStr + "_grChrg_FE_" + ss.str();
                grChrg->SetName(gername.c_str());
                grChrg->SetTitle(gername.c_str());
                grChrg->SetLineColor(3);
                grChrg->SetLineWidth(3);

                TF1 *f1ChargefromToTLeft = new TF1("ChargefromToTL", funcRation5, 1., 7.5, 5);
                f1ChargefromToTLeft->SetLineColor(2);
                grChrg->Fit(f1ChargefromToTLeft, "MRQ");
                if (f1ChargefromToTLeft->GetChisquare() / 4. > 2.){
                    logout << "bad fit Left... " << std::endl;
                }

                TF1 *f1ChargefromToTRight = new TF1("ChargefromToTR", funcRation5, 5.5, 16., 5);
                f1ChargefromToTRight->SetLineColor(4);
                grChrg->Fit(f1ChargefromToTRight, "MRQ+");

                dirFE[sfe]->WriteTObject(grTot.get());
                dirFE[sfe]->WriteTObject(grTotSig.get());
                dirFE[sfe]->WriteTObject(h2d_XchrgYtot[sfe].get());

                h2d_XchrgYtot[sfe].reset();
                h2d_XchrgYToTSig[sfe].reset();
                grTot.reset();
                grTotSig.reset();
                grChrg.reset();

                //   prepare a map sor sorting
                string Idx = "I" + ss.str();
                vector<TString> modName2prt;
                modName2prt.push_back(modName);
                modName2prt.push_back((TString)(Idx));
                //  unfortunately some tuning is necessary to satisfy dataBase reauirements
                //  5 for module ID, 4 for threshold, nToTibl - 1 for charges at ToT, 3 for ResToT
                std::vector<Double_t> prtAux;
                prtAux.reserve(5 + 4 + nToTibl - 1 + 3); // skip ToT = 0
                //  @  0 ... 4
                prtAux.push_back(modHash);
                prtAux.push_back(bec);
                prtAux.push_back(layer);
                prtAux.push_back(phi_module);
                prtAux.push_back(eta_module);

                modName2prt[0] = (modStr + "_" + ss.str()).c_str();
                if (ibl3Dfe0 && sfe < npsFEs / 2)
                {
                    if (!moreFE)
                        modName2prt[0] = modName + "_0";
                    prtAux[0] = hashIDL;
                    prtAux[4] = eta_moduleL;
                }
                if (ibl3Dfe1 && sfe >= npsFEs / 2)
                {
                    if (!moreFE)
                        modName2prt[0] = modName + "_1";
                    prtAux[0] = hashIDR;
                    prtAux[4] = eta_moduleR;
                }

                //  @   5 ...  8
                float ThrNorm = pcdMap[modStr][Idx]["ThrNorm"];
                if (ThrNorm == 0. || ThrNorm == -42.)
                {
                    //  retrieve something from average when it's missing in calibration
                    prtAux.push_back(THR_avg[0][sfe]);
                    prtAux.push_back(ThrSig_avg[0][sfe]);
                    prtAux.push_back(THR_avg[1][sfe]);
                    prtAux.push_back(ThrSig_avg[1][sfe]);
                }
                else
                {
                    prtAux.push_back(pcdMap[modStr][Idx]["ThrNorm"]);
                    prtAux.push_back(pcdMap[modStr][Idx]["ThrSigNorm"]);
                    prtAux.push_back(pcdMap[modStr][Idx]["ThrLong"]);
                    prtAux.push_back(pcdMap[modStr][Idx]["ThrSigLong"]);
                }
                //  since this version, the goodness will be skipped
                //  @ 10 ... nToTibl + 8,  with ToT = 0 skipped
                for (int t = 1; t < nToTibl; t++)
                    prtAux.push_back(ChrgArr[t]);

                // @  nToTibl + 9, nToTibl + 10
                prtAux.push_back(parP0);
                prtAux.push_back(parP1);

                std::pair<vector<TString>, vector<Double_t>> payloadDB = std::pair<vector<TString>, vector<Double_t>>(modName2prt, prtAux);
                ModuDataToPrint.insert(std::pair<float, std::pair<vector<TString>, vector<Double_t>>>(modHash, payloadDB));
            } // end 1'st loop over FrontEnds

            cntMod++;
        } // end of loop over Mods

#if defined(DEMOXCHECK)

        TDirectory *dirToTSprd = dirRod->mkdir("ToT_Spreads");
        for (int b = 0; b < nToTibl - 1; b++)
        {
            Int_t upper = h1d_totSprd[b]->FindLastBinAbove(1, 1);
            h1d_totSprd[b]->SetAxisRange(h1d_totSprd[b]->GetBinLowEdge(
                                             h1d_totSprd[b]->FindFirstBinAbove(1, 1)),
                                         h1d_totSprd[b]->GetBinLowEdge(upper) + h1d_totSprd[b]->GetBinWidth(upper), "X");

            dirToTSprd->WriteTObject(h1d_totSprd[b]);
            delete h1d_totSprd[b];
        }
        h1d_totSprd.clear();

        TString hn = "";

        TH2F *h2_badChrgs = new TH2F(rodName + "ChargeOccupancy", "Occupancy ", nchargeIBL, 0, nchargeIBL, 16 * npsFEs, 0, 16 * npsFEs);
        hn = "Occupancy along charges @ " + rodName;
        h2_badChrgs->SetTitle(hn);

        TH2F *h2_badToT = new TH2F(rodName + "ToTOccupancy", "Occupancy ", nToTibl - 1, 0.5, nToTibl - 0.5, 16 * npsFEs, 0, 16 * npsFEs);
        hn = "Occupancy along ToT @ " + rodName;
        h2_badToT->SetTitle(hn);

        bool fillChrg = false, fillToT = false;

        for (int sfe = 0; sfe < 16 * npsFEs; sfe++)
        {
            float PhiEta = 0.;
            for (int c = 0; c < nchargeIBL; c++)
            {
                float occu = occuPhiEta[c][cntRod * 16 * npsFEs + sfe];
                float nt = 10. - floor(10. * occu + 0.5);
                PhiEta += 0.1 * nt;
                TString modchrg = (TString)(modNames[sfe] + "_Chrg_" + c);
                if (nt > 0.01)
                    badModules_Order_detailed.insert(std::pair<float, TString>(1. - occu, modchrg));
                if (nt > 5.)
                    continue;
                if (nt < 0.1)
                    nt = 0.01;

                h2_badChrgs->SetBinContent(c + 1, sfe + 1, nt);
                fillChrg = true;
            }
            PhiEta /= nchargeIBL;
            if (PhiEta > 0.01)
                badModules_Order.insert(std::pair<float, TString>(PhiEta,
                                                                  (TString)(modNames[sfe])));

            for (int t = 1; t < nToTibl; t++)
            {
                float nt = floor(10. * occuChrgs[t][sfe] + 0.5);
                if (nt > 5.)
                    continue;
                h2_badToT->SetBinContent(t + 1, sfe + 1, nt);
                fillToT = true;
            }

            TString shortName = modNames[sfe].substr(9, 13).c_str();
            h2_badChrgs->GetYaxis()->SetBinLabel(sfe + 1, shortName);
            h2_badToT->GetYaxis()->SetBinLabel(sfe + 1, shortName);
            h2_badChrgs->SetStats(kFALSE);
            h2_badToT->SetStats(kFALSE);
            h2_badChrgs->SetTickLength(0.01, "Y");
            h2_badToT->SetTickLength(0.01, "Y");
        }

        h2_badChrgs->SetOption("TEXT");
        h2_badToT->SetOption("TEXT");

        if (fillChrg)
        {
            dirRod->WriteTObject(h2_badChrgs, "", "Overwrite");
        }

        if (fillToT)
        {
            dirRod->WriteTObject(h2_badToT, "", "WriteDelete");
        }

        delete h2_badChrgs;
        delete h2_badToT;

#endif

        cntRod++;
    } // end of loop over Rods

#if defined(DEMOXCHECK)

    for (int t = 0; t < 4; t++)
    {
        if (t == 0)
            logout << " correction factors for reversed charges : ";
        logout << reverseCF_H[t] << ", ";
        if (t == 3)
            logout << endl;
    }
    for (int t = 0; t < 4; t++)
    {
        if (t == 0)
            logout << " correction factors for reversed charges : ";
        logout << reverseCF_T[t] << ", ";
        if (t == 3)
            logout << endl;
    }

    for (int t = 0; t < nToTibl; t++)
    {
        if (t == 0)
            logout << "RD Entries per ToT bin ";
        logout << h1_ChrgEntry[t]->GetMean() << ",  ";
        if (t == nToTibl - 1)
            logout << std::endl;
        roTotDir->WriteTObject(h1_ChrgEntry[t]);
        delete h1_ChrgEntry[t];
    }
    h1_ChrgEntry.clear();

    for (std::map<float, TString>::const_iterator itr = devChrg_Order.begin();
         itr != devChrg_Order.end(); ++itr)
        logout << " Charge dev order : " << itr->second << " : " << itr->first << endl;

    for (std::map<float, TString>::const_iterator itr = devToT_Order.begin();
         itr != devToT_Order.end(); ++itr)
        logout << " ToT dev order : " << itr->second << " : " << itr->first << endl;

    logout << "ToTRes (spread) & its RMS  @ ToT from 1 to 16 : " << endl;
    for (int t = 0; t < nToTibl - 1; t++)
    {
        if (t == 0)
            logout << "[ ";
        logout << "[ " << h1d_totSprdAll[t]->GetMean() << " , " << h1d_totSprdAll[t]->GetStdDev() << " ] ";
        if (t < nToTibl - 2)
            logout << " , " << endl;
        if (t == nToTibl - 2)
            logout << " ] " << endl;
        delete h1d_totSprdAll[t];
    }
    h1d_totSprdAll.clear();
#endif

    logout << "  goto print " << std::endl;

    // Comment out for information only
    // txtDB << "####################################################################################################" << endl;
    // txtDB << "===========                   physics meaning of each column                   ====================" << endl;
    // txtDB << "hashId  " << "Threshold " << "ThresholdRMS " << "ThresholdLong " << "ThresholdRMSLong " << "Chrg_ToT1 "
    //       << "Chrg_ToT2 " << "Chrg_ToT3 " << "Chrg_ToT4 " << "Chrg_ToT5 " << "Chrg_ToT6 " << "Chrg_ToT7 " << "Chrg_ToT8 " << "Chrg_ToT9 "
    //       << "Chrg_ToT10 " << "Chrg_ToT11 " << "Chrg_ToT12 " << "Chrg_ToT13 " << "Chrg_ToT14 " << "Chrg_ToT15 " << "Chrg_ToT16 "
    //       << "ToT_sprd " << " ToT_sprd_var" << endl;

    // txtDB << "########################  TXT for IBL calibration  ____Start : _>        ###########################" << endl;

    //  now print the dataBase payloads in a TXT format
    for (std::map<float, std::pair<vector<TString>, vector<Double_t>>>::const_iterator itr = ModuDataToPrint.begin();
         itr != ModuDataToPrint.end(); ++itr)
    {
        int hash = floor(itr->first);
        std::pair<vector<TString>, vector<Double_t>> payload = itr->second;
        vector<Double_t> fe = payload.second;

        txtDB << hash << "  ";

        for (unsigned int t = 5; t < fe.size(); t++)
        {
            if (t < fe.size() - 2)
            {
                txtDB << (int)(fe[t]) << " ";
#if defined(DEMOXCHECK)
                if (fe[t] > fe[t + 1] && t >= 9 && t <= 23)
                    logout << " Reversed charge ! " << t - 5 << " " << fe[t] << " " << fe[t + 1] << std::endl;
#endif
            }
            else
            {
                float p01 = abs(fe[t]);
                if (t < fe.size() - 1)
                    txtDB << 0.001 * (int)(p01 * 1000.) << " ";
                else
                    txtDB << 0.000000001 * (int)(p01 * 1000000000.) << " ";
            }
        }
        txtDB << endl;
    }
    // txtDB << "####################################################################################################" << endl;
    // txtDB << "########################  TXT for IBL calibration  <___ : End _____      ###########################" << endl;
    // txtDB << "####################################################################################################" << endl;

    std::cout << " Please find the file  : " << dbFileName << " for dataBase payload " << endl;

    // statistics for BAD frontends, only for ELOG reports
    logout << " modules lacking in RD during Threshold scan : " << std::endl;
    for (std::multimap<float, TString, std::greater<float>>::const_iterator itr = badThr_Order.begin(); itr != badThr_Order.end(); ++itr)
        logout << " " << itr->second << "   " << (itr->first) * 100. << "%" << endl;

    logout << " modules lacking in RD during ToT scan : " << std::endl;
    for (std::multimap<float, TString, std::greater<float>>::const_iterator itr = badModules_Order.begin(); itr != badModules_Order.end(); ++itr)
        logout << " " << itr->second << "   " << (itr->first) * 100. << "%" << endl;

    logout << " modules lacking in RD at certain charges during ToT scan : " << std::endl;
    for (std::multimap<float, TString, std::greater<float>>::const_iterator itr = badModules_Order_detailed.begin();
         itr != badModules_Order_detailed.end(); ++itr)
        logout << " " << itr->second << "   " << (itr->first) * 100. << "%" << endl;

    roFile.Close();
    txtDB.close();
    logout.close();

    return 0;
}

void printError(){
    printf("ERROR - Argument not expected or wrongly set:\n\n");
    printf("Valid format is: ./IBLCalibration THR=SCAN_Sxxxxxxxxx TOT_HISDIS=SCAN_Sxxxxxxxxx TOT_LOWQ=SCAN_Sxxxxxxxxx directory_path=path/to/file/\n");
    printf("\n\t i.e: ./IBLCalibration THR=SCAN_S000087719 TOT_HISDIS=SCAN_S000087717 TOT_LOWQ=SCAN_S000087710 directory_path=/eos/user/x/xxxx/\n");
}

int main(int argc, char *argv[]) {
    
    bool saveInfo          = false;
    std::string THR        = "THR";
    std::string TOT_HISDIS = "TOT_HISDIS";
    std::string TOT_LOWQ   = "TOT_LOWQ";
    std::string dpath      = "directory_path";
    
    for(int i=1; i<argc; i++){
        std::string aux(argv[i]);
        if     (THR.compare(aux.substr(0,aux.find("="))) == 0)          THR = aux.substr(aux.find("=")+1);
        else if(TOT_HISDIS.compare(aux.substr(0,aux.find("="))) == 0)   TOT_HISDIS = aux.substr(aux.find("=")+1);
        else if(TOT_LOWQ.compare(aux.substr(0,aux.find("="))) == 0)     TOT_LOWQ = aux.substr(aux.find("=")+1);
        else if(dpath.compare(aux.substr(0,aux.find("="))) == 0)        dpath = aux.substr(aux.find("=")+1);
        else if(aux.compare("saveInfo") == 0)   saveInfo = true;
        else{
            printError();
            return 1;
        } 
    }
    
    printf("%-14s = %s\n","Directory path",dpath.c_str());
    printf("%-14s = %s.root\n","THR",THR.c_str());
    printf("%-14s = %s.root\n","TOT_HISDIS",TOT_HISDIS.c_str());
    printf("%-14s = %s.root\n","TOT_LOWQ",TOT_LOWQ.c_str());    
    printf("%-14s = %s\n\n\n" ,"Save root file",saveInfo ? "True" : "False" );    
    
    bool correctArgc = (THR.compare("THR") == 0) or (TOT_HISDIS.compare("TOT_HISDIS") == 0) or (TOT_LOWQ.compare("TOT_LOWQ") == 0) or (dpath.compare("directory_path") == 0);
    
    if(correctArgc){
        printf("Cannot continue, one arguments is incorrect or not filled correctly...\n");
        printf("Helper below:\n**********************\n\n");
        printError(); 
        return 1;
    }
    
    iblCalib(dpath, THR, TOT_HISDIS, TOT_LOWQ);
    return 0;
    
}

