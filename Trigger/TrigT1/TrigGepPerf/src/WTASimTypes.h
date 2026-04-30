/*
    Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef WTASimTypes_h
#define WTASimTypes_h

const float fl_ETA_MAX = 5.0;
const float fl_PHI_MAX = 3.14159265;
#ifdef BITWISE_SIMULATION
typedef FixedInt<12> pt_t;
typedef FixedInt<8> eta_t;
typedef FixedInt<7> phi_t;
typedef FixedInt<1> m_t;
typedef FixedInt<8> tech_t; // Need to match eta_t size to support eta_division
typedef FixedInt<6> tobn_t; // Total TobN bits
typedef FixedInt<1> ring0_tobn_t; // TobN per ring bits
typedef FixedInt<4> ring1_tobn_t;
typedef FixedInt<5> ring2_tobn_t;
typedef FixedInt<5> ring3_tobn_t;
typedef FixedInt<3> ring4_tobn_t;
const phi_t PI = 32;
const tech_t R_PAR = 4;
const tech_t LSB = 250; // 250 MeV
const eta_t ETA_MIN = 0;
const eta_t ETA_MAX = 99;
const eta_t ETA_LEN = 100;
const float ETA_WIDTH = (fl_ETA_MAX - (-fl_ETA_MAX)) / ETA_LEN; // Widths are used in f<->i conversion
const phi_t PHI_MIN = 0;
const phi_t PHI_MAX = 63;
const phi_t PHI_LEN = 64;
const phi_t HALF_PHI_LEN = PHI_LEN / 2;
const float PHI_WIDTH = (fl_PHI_MAX - (-fl_PHI_MAX)) / PHI_LEN;
#define CORE_DIST 8
#elif defined(INTEGER_SIMULATION)
typedef long long int pt_t;
typedef long long int eta_t;
typedef long long int phi_t;
typedef long long int m_t;
typedef long long int tech_t;
typedef long long int tobn_t;
typedef long long int ring0_tobn_t;
typedef long long int ring1_tobn_t;
typedef long long int ring2_tobn_t;
typedef long long int ring3_tobn_t;
typedef long long int ring4_tobn_t;
const phi_t PI = 32;
const tech_t R_PAR = 4;
const tech_t LSB = 250; // 250 MeV
const eta_t ETA_MIN = 0;
const eta_t ETA_MAX = 99;
const eta_t ETA_LEN = 100;
const float ETA_WIDTH = (fl_ETA_MAX - (-fl_ETA_MAX)) / ETA_LEN; // Widths are used in f<->i conversion
const phi_t PHI_MIN = 0;
const phi_t PHI_MAX = 63;
const phi_t PHI_LEN = 64;
const phi_t HALF_PHI_LEN = PHI_LEN / 2;
const float PHI_WIDTH = (fl_PHI_MAX - (-fl_PHI_MAX)) / PHI_LEN;
#define CORE_DIST 8
#elif defined(FLOATING_POINT_SIMULATION) || defined(__CPPCHECK__)
typedef float pt_t;
typedef float eta_t;
typedef float phi_t;
typedef float m_t;
typedef float tech_t;
typedef long long int tobn_t;
typedef long long int ring0_tobn_t;
typedef long long int ring1_tobn_t;
typedef long long int ring2_tobn_t;
typedef long long int ring3_tobn_t;
typedef long long int ring4_tobn_t;
const phi_t PI = 3.14159265;
const tech_t R_PAR = 0.4;
const eta_t ETA_MIN = -2.5;
const eta_t ETA_MAX = 2.5;
const tech_t ETA_LEN = 5.0;
const phi_t PHI_MIN = -PI;
const phi_t PHI_MAX = PI;
const phi_t PHI_LEN = 2 * PI;
#define CORE_DIST 0.8
#else
#error "Simulation type not defined. Define either BITWISE_SIMULATION or FLOATING_POINT_SIMULATION."
#endif

#endif
