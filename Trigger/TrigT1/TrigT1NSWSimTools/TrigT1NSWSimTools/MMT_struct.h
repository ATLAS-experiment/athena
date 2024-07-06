/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MM_STRUCT_H
#define MM_STRUCT_H

#include "AthenaKernel/getMessageSvc.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonDigitContainer/MmDigit.h"
#include "MuonSimEvent/MMSimHitCollection.h"

#include <Math/Vector2D.h>
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>

#include <stdexcept>
#include <utility>
#include <cmath>

class MMT_Parameters : public AthMessaging {
  public:
    MMT_Parameters(const std::string& layerSetup, char wedgeSize, const MuonGM::MuonDetectorManager* detManager);
    std::vector<ROOT::Math::XYZVector> MM_firststrip_positions(const MuonGM::MuonDetectorManager* detManager, const std::string& wedge, int eta) const;

    // start counting at 1 to be consistent with mmIdHelper.
    bool is_x(int plane) const {
      static const std::set<unsigned int> planes_x{1, 2, 7, 8};
      return planes_x.count(plane);
    }
    bool is_u(int plane) const {
      static const std::set<unsigned int> planes_u {3, 5};
      return planes_u.count(plane);
    }
    bool is_v(int plane) const {
      static const std::set<unsigned int> planes_v{4, 6};
      return planes_v.count(plane);
    }

    char getSector() const { return m_sector; }
    double getlWidth() const { return m_lWidth; }
    double getPitch() const { return m_pitch; }
    double getLowerBoundEta1() const { return m_innerRadiusEta1; }
    double getLowerBoundEta2() const { return m_innerRadiusEta2; }
    double getMissedBottomEtaStrips() const { return m_missedBottomEta; }
    double getMissedBottomStereoStrips() const { return m_missedBottomStereo; }
    double getYbase(int plane, int eta) const { return m_ybases[plane][eta]; }

  private:
    std::array<std::array<float, 2>, 8> m_ybases; // Fixed layout: 8 layers and 2 station eta
    char m_sector;
    double m_lWidth, m_pitch, m_innerRadiusEta1, m_innerRadiusEta2;
    double m_missedBottomEta, m_missedBottomStereo;
};

struct evInf_entry{
  evInf_entry(int event=0,int pdg=0,double e=0,double p=0,double ieta=0,double peta=0,double eeta=0,double iphi=0,double pphi=0,double ephi=0,
              double ithe=0,double pthe=0,double ethe=0,double dth=0,int trn=0,int mun=0,const ROOT::Math::XYZVector& tex=ROOT::Math::XYZVector());

  int athena_event,pdg_id;
  double E,pt,eta_ip,eta_pos,eta_ent,phi_ip,phi_pos,phi_ent,theta_ip,theta_pos,theta_ent,dtheta;
  int truth_n,mu_n;
  ROOT::Math::XYZVector vertex;
};

struct hitData_entry{
  hitData_entry(int ev=0, double gt=0, double q=0, int vmm=0, int mmfe=0, int pl=0, int st=0, int est=0, int phi=0, int mult=0, int gg=0, double locX=0, double tr_the=0, double tru_phi=0,
                bool q_tbg=0, int bct=0, const ROOT::Math::XYZVector& tru=ROOT::Math::XYZVector(), const ROOT::Math::XYZVector& rec=ROOT::Math::XYZVector());

  int event;
  double gtime,charge;
  int VMM_chip,MMFE_VMM,plane,strip,station_eta,station_phi,multiplet,gasgap;
  double localX,tru_theta_ip,tru_phi_ip;
  bool truth_nbg;
  int BC_time;
  ROOT::Math::XYZVector truth,recon;
};

struct digitWrapper{
  digitWrapper(const MmDigit* digit=0,
               const std::string& stationName=std::string(),
               double tmpGTime=0,
               const ROOT::Math::XYZVector& truthLPos=ROOT::Math::XYZVector(),
               const ROOT::Math::XYZVector& stripLPos=ROOT::Math::XYZVector(),
               const ROOT::Math::XYZVector& stripGPos=ROOT::Math::XYZVector());

  const MmDigit* digit;
  std::string stName;
  double gTime;

  ROOT::Math::XYZVector truth_lpos;
  ROOT::Math::XYZVector strip_lpos;
  ROOT::Math::XYZVector strip_gpos;

  inline Identifier id() const { return digit->identify(); };
};
#endif
