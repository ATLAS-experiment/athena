/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MMLOADVARIABLES_H
#define MMLOADVARIABLES_H

#include "AthenaBaseComps/AthMessaging.h"
#include "AthenaKernel/getMessageSvc.h"
#include "AtlasHepMC/GenEvent.h"
#include "GeneratorObjects/McEventCollection.h"
#include "TrackRecord/TrackRecordCollection.h"
#include <Math/Vector3D.h>
#include "Math/Vector4D.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include <map>
#include <vector>
#include <string>
#include <cmath>
#include <stdexcept>

struct evInf_entry{
  evInf_entry(uint64_t event=0,int pdg=0,double e=0,double p=0,double ieta=0,double peta=0,double eeta=0,double iphi=0,double pphi=0,double ephi=0,
              double ithe=0,double pthe=0,double ethe=0,double dth=0,int trn=0,int mun=0,const ROOT::Math::XYZVector& tex=ROOT::Math::XYZVector());

  uint64_t athena_event;
  int pdg_id;
  double E,pt,eta_ip,eta_pos,eta_ent,phi_ip,phi_pos,phi_ent,theta_ip,theta_pos,theta_ent,dtheta;
  int truth_n,mu_n;
  ROOT::Math::XYZVector vertex;
};

class MMLoadVariables : public AthMessaging {

  public:
    MMLoadVariables();

    StatusCode getTruthInfo(const EventContext& ctx,
                            const McEventCollection *truthContainer,
                            const TrackRecordCollection* trackRecordCollection,
                            std::map<std::pair<uint64_t,unsigned int>,evInf_entry>& Event_Info) const;

  private:
};
#endif
