// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODCORE_XAODCORESTLDICT_H
#define XAODCORE_XAODCORESTLDICT_H

// System include(s).
#include <bitset>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace {
   struct GCCXML_DUMMY_INSTANTIATION_XAODCORESTL {

      // "Single" vector type(s).
      std::vector< char > stl_v1;
      std::vector< signed char > stl_v2;
      std::vector< unsigned char > stl_v3;
      std::vector< uint32_t > stl_v4;

      // "Double" vector type(s).
      std::vector< std::vector< bool > > stl_vv1;
      std::vector< std::vector< char > > stl_vv2;
      std::vector< std::vector< signed char > > stl_vv3;
      std::vector< std::vector< unsigned char > > stl_vv4;
      std::vector< std::vector< int > > stl_vv5;
      std::vector< std::vector< size_t > > stl_vv6;
      std::vector< std::vector< int16_t > > stl_vv71;
      std::vector< std::vector< uint16_t > > stl_vv72;
      std::vector< std::vector< int32_t > > stl_vv81;
      std::vector< std::vector< uint32_t > > stl_vv82;
      std::vector< std::vector< long > > stl_vv91;
      std::vector< std::vector< unsigned long > > stl_vv92;
      std::vector< std::vector< long long > > stl_vv101;
      std::vector< std::vector< unsigned long long > > stl_vv102;
      std::vector< std::vector< std::string > > stl_vv11;

      // "Triple" vector type(s).
      std::vector< std::vector< std::vector< int > > > stl_vvv1;
      std::vector< std::vector< std::vector< char > > > stl_vvv21;
      std::vector< std::vector< std::vector< signed char > > > stl_vvv22;
      std::vector< std::vector< std::vector< unsigned char > > > stl_vvv23;
      std::vector< std::vector< std::vector< size_t > > > stl_vvv3;
      std::vector< std::vector< std::vector< int16_t > > > stl_vvv41;
      std::vector< std::vector< std::vector< uint16_t > > > stl_vvv42;
      std::vector< std::vector< std::vector< int32_t > > > stl_vvv51;
      std::vector< std::vector< std::vector< uint32_t > > > stl_vvv52;
      std::vector< std::vector< std::vector< long > > > stl_vvv61;
      std::vector< std::vector< std::vector< unsigned long > > > stl_vvv62;
      std::vector< std::vector< std::vector< long long > > > stl_vvv71;
      std::vector< std::vector< std::vector< unsigned long long > > > stl_vvv72;
      std::vector< std::vector< std::vector< float > > >  stl_vvv8;
      std::vector< std::vector< std::vector< std::string > > > stl_vvv9;

      // "Quadruple" vector type(s).
      std::vector< std::vector< std::vector< std::vector< std::string > > > >
         stl_vvvv1;

      // "Special" vector type(s).
      std::vector< std::set< uint32_t > > stl_sv1;
      std::vector< std::vector< std::set< uint32_t > > > stl_sv2;
      std::vector< std::pair< std::string, std::string > > stl_sv3;
      std::vector< std::vector< std::pair< std::string, std::string > > > stl_sv4;
      std::vector< std::vector< std::pair< unsigned int, double > > > stl_sv5;

      // Map type(s).
      std::map< std::string, std::vector< int > > stl_m1;
      std::map< std::string, std::vector< float > > stl_m2;
      std::map< std::string, std::vector< std::vector< int > > > stl_m3;
      std::map< std::string, std::vector< std::vector< float > > > stl_m4;

      // Pair type(s) belonging to that/those map type(s).
#ifdef XAOD_STANDALONE
      std::pair< std::string, std::vector< int > > stl_p1;
#endif // XAOD_STANDALONE
      std::pair< std::string, std::vector< float > > stl_p2;
      std::pair< std::string, std::vector< std::vector< int > > > stl_p3;
      std::pair< std::string, std::vector< std::vector< float > > > stl_p4;

      // "Other" type(s).
      std::bitset< 3 > stl_o1; // 3 == CaloCluster::NSTATES
      std::bitset< 11 > stl_o2; // 11 == xAOD::NumberOfTrackRecoInfo
      std::set< uint32_t > stl_o3;
   };
}

#endif // XAODCORE_XAODCORESTLDICT_H
