/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimObjects/FPGATrackSimFunctions.h"


/**
 * Given a list of sizes (of arrays), generates a list of all combinations of indices to
 * index one element from each array.
 *
 * For example, given [2 3], generates [(0 0) (1 0) (0 1) (1 1) (0 2) (1 2)].
 *
 * This basically amounts to a positional number system of where each digit has its own base.
 * The number of digits is sizes.size(), and the base of digit i is sizes[i]. Then all combinations
 * can be uniquely represented just by counting from [0, nCombs).
 *
 * For a decimal number like 1357, you get the thousands digit with n / 1000 = n / (10 * 10 * 10).
 * So here, you get the 0th digit with n / (base_1 * base_2 * base_3);
 */
std::vector<std::vector<int>> getComboIndices(std::vector<size_t> const & sizes)
{
    size_t nCombs = 1;
    std::vector<size_t> nCombs_prior(sizes.size());
    std::vector<int> temp(sizes.size(), 0);

    for (size_t i = 0; i < sizes.size(); i++)
    {
        if (sizes[i] > 0)
        {
            nCombs_prior[i] = nCombs;
            nCombs *= sizes[i];
        }
        else temp[i] = -1;
    }

    std::vector<std::vector<int>> combos(nCombs, temp);

    for (size_t icomb = 0; icomb < nCombs; icomb++)
    {
        size_t index = icomb;
        for (size_t isize = sizes.size() - 1; isize < sizes.size(); isize--)
        {
            if (sizes[isize] == 0) continue;
            combos[icomb][isize] = static_cast<int>(index / nCombs_prior[isize]);
            index = index % nCombs_prior[isize];
        }
    }

    return combos;
}


/**
 * This function is used to calculate RMS95 value for 1D histograms.
 * It was ported from 
 * https://gitlab.cern.ch:8443/fpastore/l1tracksim/-/blob/master/PatRec/macros/FraMacros/Functions.cxx#L253
 * See more details there.
 */
double rms95(TH1 const * h)
{
    double const frac = 0.95;
    double entries = h->Integral(0, h->GetNbinsX() + 1);

    // Not enough entries for this fraction, i.e. we want 0.95 then need 5% to be 1 events, so need at least 20 events.
    if ((1.0 - frac) * entries < 1 || entries == 0) return h->GetRMS();

    TH1* h_tmp = dynamic_cast<TH1*>(h->Clone());
    if (not h_tmp){
      throw "dynamic_cast failure in FPGATrackSimFunctions rms95(TH1*)";
    }
    h_tmp->GetXaxis()->SetRange(1, h_tmp->GetNbinsX());

    int meanbin  = h->GetXaxis()->FindBin(h_tmp->GetMean());
    int lowerbin = meanbin;
    int upperbin = meanbin;

    double sum = h->GetBinContent(meanbin);
    double lowerfrac = 0;
    double upperfrac = 0;

    int i = 1;
    while (true) {
        int this_lowerbin = meanbin - i;
        int this_upperbin = meanbin + i;
        if (this_lowerbin < 1 || this_upperbin > h->GetNbinsX()) break;

        sum += h_tmp->GetBinContent(this_lowerbin) + h_tmp->GetBinContent(this_upperbin);
        if (sum >= entries * frac) break;

        lowerfrac = sum / entries;
        lowerbin = this_lowerbin;
        upperbin = this_upperbin;

        i++;
    }
    upperfrac = sum / entries;

    if (upperfrac == lowerfrac) return h->GetRMS();

    double rms_lower = 0;
    double rms_upper = 0;

    h_tmp->GetXaxis()->SetRange(lowerbin, upperbin);
    rms_lower = h_tmp->GetRMS();

    h_tmp->GetXaxis()->SetRange(lowerbin - 1, upperbin + 1);
    rms_upper = h_tmp->GetRMS();

    double rms = rms_lower + (frac - lowerfrac) * (rms_upper - rms_lower) / (upperfrac - lowerfrac);

    return rms * 1.1479538518;
}


std::vector<float> computeIdealCoords(const FPGATrackSimHit &hit, const double hough_x, const double hough_y, const double target_r, const bool doDeltaGPhis, const TrackCorrType trackCorrType) {
  
  std::vector<float> idealized_coordinates;

  float hitGPhi = (float) hit.getGPhi();
  float hitZ = (float) hit.getZ();

  // rho = 0.33 m * (pT / GeV) / (B/T)
  // B = 2 T so
  // rho = 0.33 m * (pT / GeV) / (2)
  double houghRho = 0.0003 * hough_y; //A*q/pT

  hitGPhi += (hit.getR() - target_r) * houghRho; //first order
  
  if (trackCorrType == TrackCorrType::Second) {
    hitGPhi += (pow(hit.getR() * houghRho, 3.0) / 6.0); //higher order
  }

  if (hit.getR() > 1e-8) {
    hitZ -= hit.getGCotTheta() * (hit.getR() - target_r); //first order
    if (trackCorrType == TrackCorrType::Second)
      hitZ -= (hit.getGCotTheta() * std::pow(hit.getR(), 3.0) * houghRho * houghRho) / 6.0; //higher order
  }
  
  idealized_coordinates.push_back(hitZ);

  if (doDeltaGPhis) {
    double expectedGPhi = hough_x;

    expectedGPhi -= target_r * houghRho; //first order

    if (trackCorrType == TrackCorrType::Second) {
      expectedGPhi -= (std::pow(target_r * houghRho, 3.0) / 6.0); //higher order
    }

    idealized_coordinates.push_back(hitGPhi - expectedGPhi);
  }
  else {
    idealized_coordinates.push_back(hitGPhi);
  }


  return idealized_coordinates;

}
// This is the magnetic field correction for the Hough transform tools
// and related algorithms, parametrized by region number.
double fieldCorrection(unsigned region, double qoverpt, double r)
{
  double corr = 0;
  if (region == 34) {
    corr = 0.0248 + -0.0005727f*r + 0.000000781*r*r;
  }
  else if (region == 98) {
    corr = 0.0465 + -0.0008291f*r + 0.000000951*r*r;
  }
  else if (region == 162) {
    corr = -0.0106 + -0.0003101f*r + 0.000000253*r*r;
  }
  else if (region == 226) {
    corr = -0.0283 + -0.0000953f*r + -0.000001034*r*r;
  }
  else if (region == 290) {
    corr = -0.0308 + 0.0001405f*r + -0.000002490*r*r;
  }
  else if (region == 354) {
    corr = -0.0458 + 0.0010403f*r + -0.000005134*r*r;
  }
  else if (region == 418) {
    corr = -0.2827 + 0.0037327f*r + -0.000011028*r*r;
  }
  else if (region == 482) {
    corr = -0.5366 + 0.0071298f*r + -0.000019784*r*r;
  }
  else if (region == 546) {
    corr = -1.2843 + 0.0158170f*r + -0.000039258*r*r;
  }
  else if (region == 610) {
    corr = -1.5723 + 0.0218720f*r + -0.000058057*r*r;
  }
  else if (region == 674) {
    corr = -1.6054 + 0.0255374f*r + -0.000075211*r*r;
  }
  else if (region == 738) {
    corr = -1.1620 + 0.0218955f*r + -0.000082656*r*r;
  }
  else if (region == 802) {
    corr = -1.0069 + 0.0232949f*r + -0.000108384*r*r;
  }
  else if (region == 866) {
    corr = -0.6938 + 0.0183421f*r + -0.000105477*r*r;
  }
  else if (region == 930) {
    corr = -0.5990 + 0.0176694f*r + -0.000122848*r*r;
  }
  else if (region == 994) {
    corr = -0.6198 + 0.0185393f*r + -0.000151144*r*r;
  }
  else if (region == 1058) {
    corr = -0.6539 + 0.0218236f*r + -0.000196043*r*r;
  }
  else if (region == 1122) {
    corr = -0.8929 + 0.0316982f*r + -0.000308742*r*r;
  }
  else if (region == 1186) {
    corr = -0.7069 + 0.0283101f*r + -0.000328576*r*r;
  }
  else if (region == 1250) {
    corr = -1.2240 + 0.0472546f*r + -0.000538216*r*r;
  }
  return -corr*qoverpt;
}
