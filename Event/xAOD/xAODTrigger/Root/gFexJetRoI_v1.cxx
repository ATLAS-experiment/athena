
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


// System include(s):
#include <stdexcept>
#include <numbers>

// xAOD include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"

// Local include(s):
#include "xAODTrigger/versions/gFexJetRoI_v1.h"

namespace xAOD {

  /// Constants used in converting to ATLAS units
  using std::numbers::pi;
  const float gFexJetRoI_v1::s_PhiWidth = (2*pi)/32; //In central region, gFex has 32 bins in phi
  const float gFexJetRoI_v1::s_PhiWidthFR = (2*pi)/16; //In central region, gFex has 32 bins in phi
  const std::vector<float> gFexJetRoI_v1::s_EtaEdge     = { -4.9, -4.1, -3.5, -3.25, -3.2, -3.1, 
                                                            -2.9, -2.7, -2.5, -2.2, -2.0, -1.8, -1.6, -1.4, -1.2, -1.0,  
                                                            -0.8, -0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0,                                                 
                                                            1.2, 1.4, 1.6, 1.8, 2.0, 2.2, 2.5, 2.7, 2.9,
                                                            3.1,3.25, 3.3, 3.5, 4.1, 4.9 }; //Indices 0-3 and 37-40 are niot currently used in hardware (don't receive energy info)
  
  const std::vector<float> gFexJetRoI_v1::s_EtaCenter   = { -4.5, -3.8, -3.38, -3.18, -3.15, -3, 
                                                            -2.8, -2.6, -2.35, -2.1, -1.9, -1.7, -1.5, -1.3, -1.1, -0.9,  
                                                            -0.7, -0.5, -0.3, -0.1, 0.1, 0.3, 0.5, 0.7, 0.9, 1.1,                                                 
                                                            1.3, 1.5, 1.7, 1.9, 2.1, 2.35, 2.6, 2.8, 3.0,
                                                            3.15, 3.18, 3.38, 3.8, 4.5};


  //vector<float> gFexJetRoI_v1::s_maxEta = {}; 

  gFexJetRoI_v1::gFexJetRoI_v1()
    : SG::AuxElement() {

  }

  void gFexJetRoI_v1::initialize( uint32_t word, int tobEtScale ) {

    setWord( word );
    setScale( tobEtScale );
    setgFexType(unpackType());
    setTobEt( unpackEt() );
    setEta( unpackEtaIndex() );
    setPhi( unpackPhiIndex() );
    setStatus( unpackStatus());
    setSaturated(unpackSaturated());

  }

    int gFexJetRoI_v1::menuEta() const {
      return static_cast<int>(eta()*40)/4;
    }


  /// Raw data words
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexJetRoI_v1, uint32_t, word,
                                         setWord )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexJetRoI_v1, int, tobEtScale,
                                         setScale )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexJetRoI_v1, uint8_t, saturated,
                                         setSaturated )

  /// Only calculable externally
  ///eFex puts here RetaCore, RetaEnv, RhadEM, RhadHad, WstotNumerator etc (coming from algorithms)
   
  /// Extracted from data words
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexJetRoI_v1, int, gFexType,
                                         setgFexType )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexJetRoI_v1, int16_t, gFexTobEt,
                                         setTobEt )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexJetRoI_v1, uint8_t, iEta,
                                         setEta )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( gFexJetRoI_v1, uint8_t, iPhi,
                                         setPhi )

  uint8_t gFexJetRoI_v1::status() const {
  static const Accessor< uint8_t > acc( "gFexJetStatus" );
  return acc( *this );                                   
  }
  void gFexJetRoI_v1::setStatus ( uint8_t value ) {
     static const Accessor< uint8_t > acc( "gFexJetStatus" );
     acc( *this ) = value;
    return;
  }


  // Methods to decode data from the TOB/RoI and return to the user


  // Object disambiguation ()
  int gFexJetRoI_v1::unpackType() const {
    auto tobID = (word() >> s_tobIDBit) & s_tobIDMask;
    if (tobID == 0 ){
      return gRho;
    }  
    else if (tobID == 1 || tobID == 2) {
      return gBlockLead;
    } 
    else if (tobID == 3 || tobID == 4 ) {
      return gBlockSub;
    }      
    else if (tobID == 5 || tobID == 6) {
      return gJet;
    }  
    else return -999;
  }

  bool gFexJetRoI_v1::isgBlockLead() const {
    return gFexType() == gBlockLead;
  }

  bool gFexJetRoI_v1::isgBlockSub() const {
    return gFexType() == gBlockSub;
  }

  bool gFexJetRoI_v1::isgJet() const {
    return gFexType() == gJet;
  }

  bool gFexJetRoI_v1::isgRho() const {
    return gFexType() == gRho;
  }


  unsigned int gFexJetRoI_v1::unpackStatus() const{
    return (word() >> s_statusBit) & s_statusMask;
  }

  unsigned int gFexJetRoI_v1::unpackSaturated() const{
    return (word() >> s_saturBit) & s_saturMask;
  }
    

  // Raw ET on TOB scale 
  int16_t gFexJetRoI_v1::unpackEt() const {
  // Data content = TOB
    int16_t energy = (word() >> s_etBit) & s_etMask; 
    return energy; 
  }

  // Return an eta index in the range 0-49
  unsigned int gFexJetRoI_v1::unpackEtaIndex() const {
    return (word() >> s_etaBit) & s_etaMask;
  }
   
  // Return an eta index in the range 0-32
  unsigned int gFexJetRoI_v1::unpackPhiIndex() const {
    return (word() >> s_phiBit) & s_phiMask;

  }

  // Methods that require combining results or applying scales
  
  float gFexJetRoI_v1::et() const {
      return gFexTobEt()*tobEtScale();
  }


  // Floating point coordinates. Return the center of Eta. 
  
  float gFexJetRoI_v1::eta() const {
    if (gFexType() != gRho){
      int new_iEtaFR = 0;
      // Remapping in the extreme FR
      if (iEta() < 4 || iEta() > 35){
        if (iEta() == 2){
          new_iEtaFR = (int(iPhi())%2 == 0) ? 0 : 1 ;
        }
        if (iEta() == 3){
          new_iEtaFR = (int(iPhi())%2 == 0) ? 2 : 3 ;
        }
        if (iEta() == 36){
          new_iEtaFR = (int(iPhi())%2 == 0) ? 37 : 36 ;
        }
        if (iEta() == 37){
          new_iEtaFR = (int(iPhi())%2 == 0) ? 39 : 38 ;
        }
        return s_EtaCenter[new_iEtaFR];
      }
      //No remapping for central region |eta| < 3.0
      else{
        return s_EtaCenter[iEta()];
      }
    } 
    return 0;
   }

   /// Floating point coordinates. Return he minimum Eta and he maximum Eta of the Eta range. 
  float gFexJetRoI_v1::etaMin() const {
    if (gFexType() != gRho){
      int new_iEtaFR = 0;
      // Remapping in the extreme FR
      if (iEta() < 4 || iEta() > 35){
        if (iEta() == 2){
          new_iEtaFR = (int(iPhi())%2 == 0) ? 0 : 1 ;
        }
        if (iEta() == 3){
          new_iEtaFR = (int(iPhi())%2 == 0) ? 2 : 3 ;
        }
        if (iEta() == 36){
          new_iEtaFR = (int(iPhi())%2 == 0) ? 37 : 36 ;
        }
        if (iEta() == 37){
          new_iEtaFR = (int(iPhi())%2 == 0) ? 39 : 38 ;
        }
        return s_EtaEdge[new_iEtaFR];
      }
      //No remapping for central region |eta| < 3.0
      else{
        return s_EtaEdge[iEta()];
      }
    }
    return 0;
  }
   
  float gFexJetRoI_v1::etaMax() const {
    if (gFexType() != gRho){
      int new_iEtaFR = 0;
      // Remapping in the extreme FR
      if (iEta() < 4 || iEta() > 35){
        if (iEta() == 2){
          new_iEtaFR = (int(iPhi())%2 == 0) ? 0 : 1 ;
        }
        if (iEta() == 3){
          new_iEtaFR = (int(iPhi())%2 == 0) ? 2 : 3 ;
        }
        if (iEta() == 36){
          new_iEtaFR = (int(iPhi())%2 == 0) ? 37 : 36 ;
        }
        if (iEta() == 37){
          new_iEtaFR = (int(iPhi())%2 == 0) ? 39 : 38 ;
        }
        return s_EtaEdge[new_iEtaFR + 1];
      }
      //No remapping for central region |eta| < 3.0
      else{
        return s_EtaEdge[iEta() + 1];
      }
    }
    return 0;
  }



  // Floating point coordinates using gFex convention [0, 2pi].
  // Returns the centre phi coordinate.
  // Note that for iEta = 0,1,2,3,36,37,38,39 - there are only 16 bins in phi (otherwise 32)
  float gFexJetRoI_v1::phi_gFex() const {
    float phi_out = 0;
    if (gFexType() != gRho){
      float phi_idx;
      if (iEta() < 4 || iEta() > 35){
        // take the even iphi() and divide by 2 to get a continuous range (0-15)
        if (int(iPhi()%2) == 0 ){ 
          phi_idx = iPhi()/2;
          phi_out = (phi_idx * s_PhiWidthFR) + (s_PhiWidthFR/2);
          }
          // take the odd iphi() and get a continuous range (0,15)
        else { 
          phi_idx = (iPhi()-1)/2;
          phi_out = (phi_idx * s_PhiWidthFR) + (s_PhiWidthFR/2);}
      }
      else{
        phi_out = (iPhi() * s_PhiWidth) + (s_PhiWidth/2);
      }
        
    } 
    return phi_out; 
  }


  // Floating point coordinates using gFex convention [0, 2pi]. 
  // Returns the low edge of the phi bin.
  float gFexJetRoI_v1::phiMin_gFex() const {
    float phi_out = 0;
    if (gFexType() != gRho){
      phi_out = iPhi() * s_PhiWidth;
    } 
    return phi_out; 
  }

  // Floating point coordinates using gFex convention [0, 2pi].
  // Returns the high edge of the phi bin.
  float gFexJetRoI_v1::phiMax_gFex() const {
    float phi_out = 0;
    if (gFexType() != gRho){
      phi_out = iPhi() * s_PhiWidth + s_PhiWidth;
    } 
    return phi_out; 
  }

  // Floating point coordinates using ATLAS convention [-pi, pi]. 
  // Returns the centre phi coordinate.
  float gFexJetRoI_v1::phi() const {
    float phi_out = 0;
    if (gFexType() != gRho){
      if (phi_gFex() < pi) phi_out = phi_gFex();
      else phi_out = (phi_gFex() - 2*pi);
    }
    return phi_out; 
  }

  // Floating point coordinates using ATLAS convention [-pi, pi].
  // Returns the high edge of the phi bin. 
  float gFexJetRoI_v1::phiMax() const {
    float phi_out = 0;
    if (gFexType() != gRho){
      if (phiMax_gFex() < pi) phi_out = phiMax_gFex();
      else phi_out = (phiMax_gFex() - 2*pi);
    }
    return phi_out; 
  }

  // Floating point coordinates using ATLAS convention [-pi, pi]. 
  // Returns the low edge of the phi bin.
  float gFexJetRoI_v1::phiMin() const {
    float phi_out = 0;
    if (gFexType() != gRho){
      if (phiMin_gFex() < pi) phi_out = phiMin_gFex();
      else phi_out = (phiMin_gFex() - 2*pi);
    }
    return phi_out; 
  }

  // Returns phi index in the range used by L1Topo (0->127).
  int gFexJetRoI_v1::iPhiTopo() const {
    int phi_out = 0;
    if (gFexType() != gRho){
      if (( iEta() <= 3 ) || ( (iEta() >= 36) )){
        phi_out = ( 8 * iPhi() ) + 4;
      }
      else if ( iEta() >3  && iEta() < 36 ){
        phi_out = ( 4 * iPhi() ) + 2;
      } 
      else return -999; 
    } 
    return phi_out; 
  }


} // namespace xAOD
