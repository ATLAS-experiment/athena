/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RHADRONS_RHADRONSPHYSICSTOOL_H
#define RHADRONS_RHADRONSPHYSICSTOOL_H

// Include files
#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IPhysicsOptionTool.h"
#include "G4VPhysicsConstructor.hh"
#include "CxxUtils/checker_macros.h"

/** @class RHadronsPhysicsTool RHadronsPhysicsTool.h "RHadrons/RHadronsPhysicsTool.h"
 *
 *
 *
 *  @author Edoardo Farina
 *  @date  2015-05-14
 */
class ATLAS_NOT_THREAD_SAFE RHadronsPhysicsTool :  public G4VPhysicsConstructor, public extends<AthAlgTool, IPhysicsOptionTool>
{
public:
  /// Standard constructor
  RHadronsPhysicsTool( const std::string& type , const std::string& name,
                       const IInterface* parent ) ;

  virtual ~RHadronsPhysicsTool( ); ///< Destructor

  /// Initialize method
  virtual StatusCode initialize( ) ;
  virtual void ConstructParticle();
  virtual void ConstructProcess();

  /** Implements
   */

  virtual RHadronsPhysicsTool* GetPhysicsOption();

private:
  // 521   B+  5279.17 (MeV/c) meson B 0
  // -521   B-  5279.17 (MeV/c) meson B 0
  // 511   B0  5279.5 (MeV/c) meson B 0
  // -511   anti_B0  5279.5 (MeV/c) meson B 0
  // 531   Bs0  5366.3 (MeV/c) meson Bs 0
  // -531   anti_Bs0  5366.3 (MeV/c) meson Bs 0
  // 541   Bc+  6277 (MeV/c) meson Bc 0
  // -541   Bc-  6277 (MeV/c) meson Bc 0
  // 553   Upsilon  9460.3 (MeV/c) meson Upsilon 0

  // 411   D+  1869.57 (MeV/c) meson D 0
  // -411   D-  1869.57 (MeV/c) meson D 0
  // 421   D0  1864.8 (MeV/c) meson D 0
  // -421   anti_D0  1864.8 (MeV/c) meson D 0
  // 431   Ds+  1968.45 (MeV/c) meson Ds 0
  // -431   Ds-  1968.45 (MeV/c) meson Ds 0
  // 441   etac  2980.3 (MeV/c) meson etac 0
  // 443   J/psi  3096.92 (MeV/c) meson J/psi 0

  // 4122   lambda_c+  2286.46 (MeV/c) baryon lambda_c 0
  // 4222   sigma_c++  2454.02 (MeV/c) baryon sigma_c 0
  // 4212   sigma_c+  2452.9 (MeV/c) baryon sigma_c 0
  // 4112   sigma_c0  2453.76 (MeV/c) baryon sigma_c 0
  // 4232   xi_c+  2467.8 (MeV/c) baryon xi_c 0
  // 4132   xi_c0  2470.88 (MeV/c) baryon xi_c 0
  // 4332   omega_c0  2695.2 (MeV/c) baryon omega_c 0

  // 5122   lambda_b  5620.2 (MeV/c) baryon lambda_b 0
  // 5222   sigma_b+  5807.8 (MeV/c) baryon sigma_b 0
  // 5212   sigma_b0  5807.8 (MeV/c) baryon sigma_b 0
  // 5112   sigma_b-  5815.2 (MeV/c) baryon sigma_b 0
  // 5232   xi_b0  5790.5 (MeV/c) baryon xi_b 0
  // 5132   xi_b-  5790.5 (MeV/c) baryon xi_b 0
  // 5332   omega_b-  6071 (MeV/c) baryon omega_b 0

  IntegerArrayProperty m_standardpdgidtodecay{this, "StandardParticlesToDecay",
      {
        4122, 4222, 4212, 4112, 4232, 4132, 4332, 5122, 5222, 5212, 5112, 5232, 5132, 5332,
          -4122, -4222, -4212, -4112, -4232, -4132, -4332, -5122, -5222, -5212, -5112, -5232, -5132, -5332,
          411, -411, 421, -421, 431, -431, 441, 443, 521,
          -521, 511, -511, 531, -531, 541, -541, 553
          },
      "Particles which are known to Geant4"};
  // IntegerArrayProperty m_extrapdgidtodecay{this, "ExtraParticlesToDecay",
  //     {
  //       //  Unknown to Geant4, so removed for the time being
  //       // 423, -423, 433, -433, 513, -513, 523, -523, 551, -551
  //         },
  //     "Particles which are added to Geant4 by quasi-stable particle simulation."};

};



#endif
