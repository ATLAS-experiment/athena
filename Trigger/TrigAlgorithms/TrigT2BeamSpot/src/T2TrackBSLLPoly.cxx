/*
Copyright (C) 2019, 2025 CERN for the benefit of the ATLAS collaboration
*/
#include "T2TrackBSLLPoly.h"
#include "idx.h"

#include <cmath>
#include <cstdint>




using namespace PESA;



void
T2TrackBSLLPoly::update(double z0, double d0, double phi, double var_d0, std::vector<double>& coeff)
{
    if (coeff.empty()) {
        coeff.resize(nbins, 0.);
    }

    double cos_phi = std::cos(phi);
    double sin_phi = std::sin(phi);
    double var_b = m_beam_size*m_beam_size;
    double var_bd = var_d0 + var_b;
    double pow_cos_phi[7]{};
    double pow_sin_phi[7]{};
    double pow_var_bd[4]{};
    for (size_t i=2; i<7; ++i) {
      pow_cos_phi[i] = std::pow(cos_phi, i);
      pow_sin_phi[i] = std::pow(sin_phi, i);
    }
    for (size_t i=2; i<4; ++i) {
      pow_var_bd[i] = std::pow(var_bd, i);
    }
    double pow2_d0 = std::pow(d0, 2);
    double pow2_z0 = std::pow(z0, 2);

    // z0 and its square
    coeff[g_size*g_size2] += z0;
    coeff[g_size*g_size2+1] += pow2_z0;

    // store number of tracks and beam_size*n_tracks in last two bins
    coeff[g_size*g_size2+2] += 1;
    coeff[g_size*g_size2+3] += m_beam_size;

    coeff[idx<0, 0, 0, 0, 0, 0>()] += -pow2_d0/(2*var_bd);
    coeff[idx<0, 0, 0, 0, 0, 1>()] += pow_cos_phi[2]*pow2_d0/(2*pow_var_bd[2]);
    coeff[idx<0, 0, 0, 0, 0, 2>()] += -pow_cos_phi[4]*pow2_d0/(2*pow_var_bd[3]);
    coeff[idx<0, 0, 0, 0, 1, 0>()] += pow2_d0*pow_sin_phi[2]/(2*pow_var_bd[2]);
    coeff[idx<0, 0, 0, 0, 1, 1>()] += -pow_cos_phi[2]*pow2_d0*pow_sin_phi[2]/pow_var_bd[3];
    coeff[idx<0, 0, 0, 0, 2, 0>()] += -pow2_d0*pow_sin_phi[4]/(2*pow_var_bd[3]);
    coeff[idx<0, 0, 0, 1, 0, 0>()] += cos_phi*d0*z0/var_bd;
    coeff[idx<0, 0, 0, 1, 0, 1>()] += -pow_cos_phi[3]*d0*z0/pow_var_bd[2];
    coeff[idx<0, 0, 0, 1, 0, 2>()] += pow_cos_phi[5]*d0*z0/pow_var_bd[3];
    coeff[idx<0, 0, 0, 1, 1, 0>()] += -cos_phi*d0*pow_sin_phi[2]*z0/pow_var_bd[2];
    coeff[idx<0, 0, 0, 1, 1, 1>()] += 2*pow_cos_phi[3]*d0*pow_sin_phi[2]*z0/pow_var_bd[3];
    coeff[idx<0, 0, 0, 1, 2, 0>()] += cos_phi*d0*pow_sin_phi[4]*z0/pow_var_bd[3];
    coeff[idx<0, 0, 0, 2, 0, 0>()] += -pow_cos_phi[2]*pow2_z0/(2*var_bd);
    coeff[idx<0, 0, 0, 2, 0, 1>()] += pow_cos_phi[4]*pow2_z0/(2*pow_var_bd[2]);
    coeff[idx<0, 0, 0, 2, 0, 2>()] += -pow_cos_phi[6]*pow2_z0/(2*pow_var_bd[3]);
    coeff[idx<0, 0, 0, 2, 1, 0>()] += pow_cos_phi[2]*pow_sin_phi[2]*pow2_z0/(2*pow_var_bd[2]);
    coeff[idx<0, 0, 0, 2, 1, 1>()] += -pow_cos_phi[4]*pow_sin_phi[2]*pow2_z0/pow_var_bd[3];
    coeff[idx<0, 0, 0, 2, 2, 0>()] += -pow_cos_phi[2]*pow_sin_phi[4]*pow2_z0/(2*pow_var_bd[3]);
    coeff[idx<0, 0, 1, 0, 0, 0>()] += -d0*sin_phi*z0/var_bd;
    coeff[idx<0, 0, 1, 0, 0, 1>()] += pow_cos_phi[2]*d0*sin_phi*z0/pow_var_bd[2];
    coeff[idx<0, 0, 1, 0, 0, 2>()] += -pow_cos_phi[4]*d0*sin_phi*z0/pow_var_bd[3];
    coeff[idx<0, 0, 1, 0, 1, 0>()] += d0*pow_sin_phi[3]*z0/pow_var_bd[2];
    coeff[idx<0, 0, 1, 0, 1, 1>()] += -2*pow_cos_phi[2]*d0*pow_sin_phi[3]*z0/pow_var_bd[3];
    coeff[idx<0, 0, 1, 0, 2, 0>()] += -d0*pow_sin_phi[5]*z0/pow_var_bd[3];
    coeff[idx<0, 0, 1, 1, 0, 0>()] += cos_phi*sin_phi*pow2_z0/var_bd;
    coeff[idx<0, 0, 1, 1, 0, 1>()] += -pow_cos_phi[3]*sin_phi*pow2_z0/pow_var_bd[2];
    coeff[idx<0, 0, 1, 1, 0, 2>()] += pow_cos_phi[5]*sin_phi*pow2_z0/pow_var_bd[3];
    coeff[idx<0, 0, 1, 1, 1, 0>()] += -cos_phi*pow_sin_phi[3]*pow2_z0/pow_var_bd[2];
    coeff[idx<0, 0, 1, 1, 1, 1>()] += 2*pow_cos_phi[3]*pow_sin_phi[3]*pow2_z0/pow_var_bd[3];
    coeff[idx<0, 0, 1, 1, 2, 0>()] += cos_phi*pow_sin_phi[5]*pow2_z0/pow_var_bd[3];
    coeff[idx<0, 0, 2, 0, 0, 0>()] += -pow_sin_phi[2]*pow2_z0/(2*var_bd);
    coeff[idx<0, 0, 2, 0, 0, 1>()] += pow_cos_phi[2]*pow_sin_phi[2]*pow2_z0/(2*pow_var_bd[2]);
    coeff[idx<0, 0, 2, 0, 0, 2>()] += -pow_cos_phi[4]*pow_sin_phi[2]*pow2_z0/(2*pow_var_bd[3]);
    coeff[idx<0, 0, 2, 0, 1, 0>()] += pow_sin_phi[4]*pow2_z0/(2*pow_var_bd[2]);
    coeff[idx<0, 0, 2, 0, 1, 1>()] += -pow_cos_phi[2]*pow_sin_phi[4]*pow2_z0/pow_var_bd[3];
    coeff[idx<0, 0, 2, 0, 2, 0>()] += -pow_sin_phi[6]*pow2_z0/(2*pow_var_bd[3]);
    coeff[idx<0, 1, 0, 0, 0, 0>()] += cos_phi*d0/var_bd;
    coeff[idx<0, 1, 0, 0, 0, 1>()] += -pow_cos_phi[3]*d0/pow_var_bd[2];
    coeff[idx<0, 1, 0, 0, 0, 2>()] += pow_cos_phi[5]*d0/pow_var_bd[3];
    coeff[idx<0, 1, 0, 0, 1, 0>()] += -cos_phi*d0*pow_sin_phi[2]/pow_var_bd[2];
    coeff[idx<0, 1, 0, 0, 1, 1>()] += 2*pow_cos_phi[3]*d0*pow_sin_phi[2]/pow_var_bd[3];
    coeff[idx<0, 1, 0, 0, 2, 0>()] += cos_phi*d0*pow_sin_phi[4]/pow_var_bd[3];
    coeff[idx<0, 1, 0, 1, 0, 0>()] += -pow_cos_phi[2]*z0/var_bd;
    coeff[idx<0, 1, 0, 1, 0, 1>()] += pow_cos_phi[4]*z0/pow_var_bd[2];
    coeff[idx<0, 1, 0, 1, 0, 2>()] += -pow_cos_phi[6]*z0/pow_var_bd[3];
    coeff[idx<0, 1, 0, 1, 1, 0>()] += pow_cos_phi[2]*pow_sin_phi[2]*z0/pow_var_bd[2];
    coeff[idx<0, 1, 0, 1, 1, 1>()] += -2*pow_cos_phi[4]*pow_sin_phi[2]*z0/pow_var_bd[3];
    coeff[idx<0, 1, 0, 1, 2, 0>()] += -pow_cos_phi[2]*pow_sin_phi[4]*z0/pow_var_bd[3];
    coeff[idx<0, 1, 1, 0, 0, 0>()] += cos_phi*sin_phi*z0/var_bd;
    coeff[idx<0, 1, 1, 0, 0, 1>()] += -pow_cos_phi[3]*sin_phi*z0/pow_var_bd[2];
    coeff[idx<0, 1, 1, 0, 0, 2>()] += pow_cos_phi[5]*sin_phi*z0/pow_var_bd[3];
    coeff[idx<0, 1, 1, 0, 1, 0>()] += -cos_phi*pow_sin_phi[3]*z0/pow_var_bd[2];
    coeff[idx<0, 1, 1, 0, 1, 1>()] += 2*pow_cos_phi[3]*pow_sin_phi[3]*z0/pow_var_bd[3];
    coeff[idx<0, 1, 1, 0, 2, 0>()] += cos_phi*pow_sin_phi[5]*z0/pow_var_bd[3];
    coeff[idx<0, 2, 0, 0, 0, 0>()] += -pow_cos_phi[2]/(2*var_bd);
    coeff[idx<0, 2, 0, 0, 0, 1>()] += pow_cos_phi[4]/(2*pow_var_bd[2]);
    coeff[idx<0, 2, 0, 0, 0, 2>()] += -pow_cos_phi[6]/(2*pow_var_bd[3]);
    coeff[idx<0, 2, 0, 0, 1, 0>()] += pow_cos_phi[2]*pow_sin_phi[2]/(2*pow_var_bd[2]);
    coeff[idx<0, 2, 0, 0, 1, 1>()] += -pow_cos_phi[4]*pow_sin_phi[2]/pow_var_bd[3];
    coeff[idx<0, 2, 0, 0, 2, 0>()] += -pow_cos_phi[2]*pow_sin_phi[4]/(2*pow_var_bd[3]);
    coeff[idx<1, 0, 0, 0, 0, 0>()] += -d0*sin_phi/var_bd;
    coeff[idx<1, 0, 0, 0, 0, 1>()] += pow_cos_phi[2]*d0*sin_phi/pow_var_bd[2];
    coeff[idx<1, 0, 0, 0, 0, 2>()] += -pow_cos_phi[4]*d0*sin_phi/pow_var_bd[3];
    coeff[idx<1, 0, 0, 0, 1, 0>()] += d0*pow_sin_phi[3]/pow_var_bd[2];
    coeff[idx<1, 0, 0, 0, 1, 1>()] += -2*pow_cos_phi[2]*d0*pow_sin_phi[3]/pow_var_bd[3];
    coeff[idx<1, 0, 0, 0, 2, 0>()] += -d0*pow_sin_phi[5]/pow_var_bd[3];
    coeff[idx<1, 0, 0, 1, 0, 0>()] += cos_phi*sin_phi*z0/var_bd;
    coeff[idx<1, 0, 0, 1, 0, 1>()] += -pow_cos_phi[3]*sin_phi*z0/pow_var_bd[2];
    coeff[idx<1, 0, 0, 1, 0, 2>()] += pow_cos_phi[5]*sin_phi*z0/pow_var_bd[3];
    coeff[idx<1, 0, 0, 1, 1, 0>()] += -cos_phi*pow_sin_phi[3]*z0/pow_var_bd[2];
    coeff[idx<1, 0, 0, 1, 1, 1>()] += 2*pow_cos_phi[3]*pow_sin_phi[3]*z0/pow_var_bd[3];
    coeff[idx<1, 0, 0, 1, 2, 0>()] += cos_phi*pow_sin_phi[5]*z0/pow_var_bd[3];
    coeff[idx<1, 0, 1, 0, 0, 0>()] += -pow_sin_phi[2]*z0/var_bd;
    coeff[idx<1, 0, 1, 0, 0, 1>()] += pow_cos_phi[2]*pow_sin_phi[2]*z0/pow_var_bd[2];
    coeff[idx<1, 0, 1, 0, 0, 2>()] += -pow_cos_phi[4]*pow_sin_phi[2]*z0/pow_var_bd[3];
    coeff[idx<1, 0, 1, 0, 1, 0>()] += pow_sin_phi[4]*z0/pow_var_bd[2];
    coeff[idx<1, 0, 1, 0, 1, 1>()] += -2*pow_cos_phi[2]*pow_sin_phi[4]*z0/pow_var_bd[3];
    coeff[idx<1, 0, 1, 0, 2, 0>()] += -pow_sin_phi[6]*z0/pow_var_bd[3];
    coeff[idx<1, 1, 0, 0, 0, 0>()] += cos_phi*sin_phi/var_bd;
    coeff[idx<1, 1, 0, 0, 0, 1>()] += -pow_cos_phi[3]*sin_phi/pow_var_bd[2];
    coeff[idx<1, 1, 0, 0, 0, 2>()] += pow_cos_phi[5]*sin_phi/pow_var_bd[3];
    coeff[idx<1, 1, 0, 0, 1, 0>()] += -cos_phi*pow_sin_phi[3]/pow_var_bd[2];
    coeff[idx<1, 1, 0, 0, 1, 1>()] += 2*pow_cos_phi[3]*pow_sin_phi[3]/pow_var_bd[3];
    coeff[idx<1, 1, 0, 0, 2, 0>()] += cos_phi*pow_sin_phi[5]/pow_var_bd[3];
    coeff[idx<2, 0, 0, 0, 0, 0>()] += -pow_sin_phi[2]/(2*var_bd);
    coeff[idx<2, 0, 0, 0, 0, 1>()] += pow_cos_phi[2]*pow_sin_phi[2]/(2*pow_var_bd[2]);
    coeff[idx<2, 0, 0, 0, 0, 2>()] += -pow_cos_phi[4]*pow_sin_phi[2]/(2*pow_var_bd[3]);
    coeff[idx<2, 0, 0, 0, 1, 0>()] += pow_sin_phi[4]/(2*pow_var_bd[2]);
    coeff[idx<2, 0, 0, 0, 1, 1>()] += -pow_cos_phi[2]*pow_sin_phi[4]/pow_var_bd[3];
    coeff[idx<2, 0, 0, 0, 2, 0>()] += -pow_sin_phi[6]/(2*pow_var_bd[3]);
    coeff[idx<0, 0, 0, 0, 0, 0>() +90] += -std::log(var_bd)/2;
    coeff[idx<0, 0, 0, 0, 0, 1>() +90] += -pow_cos_phi[2]/(2*var_bd);
    coeff[idx<0, 0, 0, 0, 0, 2>() +90] += pow_cos_phi[4]/(4*pow_var_bd[2]);
    coeff[idx<0, 0, 0, 0, 1, 0>() +90] += -pow_sin_phi[2]/(2*var_bd);
    coeff[idx<0, 0, 0, 0, 1, 1>() +90] += pow_cos_phi[2]*pow_sin_phi[2]/(2*pow_var_bd[2]);
    coeff[idx<0, 0, 0, 0, 2, 0>() +90] += pow_sin_phi[4]/(4*pow_var_bd[2]);
}
