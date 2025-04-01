/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// EDM includes(s):
#include "xAODCore/AuxStoreAccessorMacros.h"

// Local include(s):
#include "xAODTrigL1Calo/versions/eFexTower_v1.h"

namespace xAOD{  

  AUXSTORE_OBJECT_SETTER_AND_GETTER( eFexTower_v1 , std::vector<uint16_t> , et_count , setEt_count )
  AUXSTORE_OBJECT_MOVE( eFexTower_v1 , std::vector<uint16_t> , et_count , setEt_count )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( eFexTower_v1, float , eta , setEta )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( eFexTower_v1, float , phi , setPhi )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( eFexTower_v1, uint8_t , module , setModule )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( eFexTower_v1, uint8_t , fpga , setFpga )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( eFexTower_v1, uint32_t , em_status , setEm_status )
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( eFexTower_v1, uint32_t , had_status , setHad_status )


  uint32_t eFexTower_v1::eFEXtowerID() const {
      // Calculate ID by hand from coordinate
      float eta = this->eta(); float phi = this->phi();
      int posneg = (eta >= 0 ? 1 : -1);
      int towereta = std::abs(eta+0.025)/0.1;
      if (phi < 0) phi += 2*M_PI;
      int towerphi = int(32*(phi+0.025)/M_PI);
      unsigned int tower_id = towerphi + 64*towereta;

      if (towereta < 14) {
          tower_id += (posneg > 0 ? 200000 : 100000);
      }
      else if (towereta == 14) {
          tower_id += (posneg > 0 ? 400000 : 300000);
      }
      else {
          tower_id += (posneg > 0 ? 600000 : 500000);
      }

      return tower_id;
  }
  
  
  /// initialize
  void eFexTower_v1::initialize(const float Eta,const float Phi)
  {
    setEta( Eta );
    setPhi( Phi );    
  }
  
  void eFexTower_v1::initialize(const float Eta,const float Phi,
                                   const std::vector<uint16_t>& Et_count,
                                   const uint8_t Module,
                                   const uint8_t Fpga,
                                   const uint32_t Em_status,
				   const uint32_t Had_status)
  {
    setEta( Eta );
    setPhi( Phi );
    setEt_count( Et_count );
    setModule( Module );
    setFpga( Fpga );
    setEm_status( Em_status );
    setHad_status( Had_status );
  }

  int32_t eFexTower_v1::id() const {
      int etaIndex = int( (eta()+0.025)*10 ) + (((eta()+0.025)<0) ? -1 : 1); // runs from -25 to 25 (excluding 0)
      int phiIndex = int( (phi()+0.025)*32./ROOT::Math::Pi() ) + ((phi()+0.025)<0 ? 63 : 0); // runs from 0 to 63
      int modIndex = ( module()>23 ) ? 99 : module(); // module runs from 0 to 23 or otherwise takes value 99
      int fpgaIndex = ( fpga() > 3 ) ? 9 : fpga(); //from runs from 0 to 3 or otherwise takes value 9
      return (std::abs(etaIndex)*100000 + phiIndex*1000 + modIndex*10 +fpgaIndex)*(etaIndex<0 ? -1 : 1);
  }

  bool eFexTower_v1::disconnectedCount(size_t idx) const {
      if(idx>11) return true;
      int mod = eFexTower_v1::module();
      double eta = eFexTower_v1::eta() + 0.025;
      double phi = eFexTower_v1::phi() + 0.025;

      if ( std::abs(eta)>1.8 && idx==0 ) return true; // no PS beyond 1.8
      if ( std::abs(eta)>2.4 && idx>0 && idx<5 ) return idx!=4; // only the 'last' l1 is connected

      if (mod>23) return false; // all emulated towers are connected

      // extremities of the real modules are disconnected
      // em inputs at most extreme phi of module are all disconnected
      // as well as the last two in eta of each module (modules centered at eta = -1.6,0,1.6 ... extent in em inputs is +/- 1.0 (1.1 ish for the A/C modules) )
      // exception is the eta=+/-2.45 inputs at the extreme of efex environment, which are present in the most extreme phi still.
      // meaning |eta|>2.4 is all connected

      double module_central_eta = 1.6*(mod%3-1);

      if( std::abs(eta)<2.4 &&
          idx!=11 &&
          (std::abs(std::remainder(phi - (M_PI/32)*(8*(mod/3) + 6 - (mod>11)*64),2*M_PI)) >  5.*M_PI/32 || std::abs( module_central_eta - (eta) ) > 1.0 )
              ) {
          return true;
      }

      // finally, we also treat any input tower that is too far away from the centre of the module it is feeding as 'disconnected'
      // it shouldn't be in use by any algorithm at least
      double fpga_central_eta = module_central_eta + 0.4*(int(fpga())-2) + 0.2;
      if ( std::abs(fpga_central_eta - eta) > 0.3 ) return true;

      return false;
  };

    size_t eFexTower_v1::cellIdx(uint32_t layer, uint32_t cell) const {

        if(cell > 3 || (layer!=1 && layer!=2 && cell>0)) return et_count().size();
        // for |eta|>2.4 towers, the l1 energy (which is in the last slot) is treated as ps
        if(layer==0 && std::abs(eta()+0.025)>2.4) return 4;
        return (layer>0)*1 + (layer>1)*4 + (layer>2)*4 + (layer>3)*1 + cell;

    }

    int eFexTower_v1::cellEt(uint32_t layer, uint32_t cell) const {

        auto idx = cellIdx(layer,cell);
        if(idx == et_count().size()) return 0;
        if(disconnectedCount(idx)) return 0;
        auto count = et_count().at(idx);
        // convert count to MeV using either LATOME or Tile energy scales
        if (layer==4 && std::abs(eta()+0.025)<1.5) {
            // Tile energy scale (500MeV per count)
            return count*500;
        }

        // Deal with special codes first:
        if (count == 0)  return 0; // NoData
        if (count == 1021 || count == 1022 || count > 1023) return 0; // Reserved || Invalid || OutOfRange
        if (count == 1023) return 65535*25; // Saturated case, return saturated value;

        if (count >= 768) return 44000 + (count-768)*400;
        if (count >= 512) return 18400 + (count-512)*100;
        if (count >= 256) return 5600 + (count-256)*50;
        return -750 + (count-2)*25;
    }

} // namespace xAOD
