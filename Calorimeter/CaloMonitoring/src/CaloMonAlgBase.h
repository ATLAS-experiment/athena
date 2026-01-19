/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOMONITORING_CALOMONALGBASE
#define CALOMONITORING_CALOMONALGBASE

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoring/IDQFilterTool.h"
#include "RecBackgroundEvent/BeamBackgroundData.h"
#include "LArRecEvent/LArCollisionTime.h"

class CaloMonAlgBase : public AthMonitorAlgorithm {
 public:
  
   CaloMonAlgBase(const std::string& name, ISvcLocator* pSvcLocator);

   virtual ~CaloMonAlgBase(){};   

   virtual StatusCode initialize();

   StatusCode checkFilters(bool &ifPass, bool &passBeamBackgroundRemoval, const std::string &MonGroupName, const EventContext &ctx) const; 
   
 private:
  // LArCollisionTime name
  SG::ReadHandleKey<LArCollisionTime> m_LArCollisionTimeKey{this,"LArCollisionTimeKey","LArCollisionTime"};
  SG::ReadHandleKey<BeamBackgroundData>  m_beamBackgroundKey{this,"BeamBackgroundKey","CSCBackgroundForCaloMon"};

  // Handles on filtering tools
  bool m_useBadLBTool;
  ToolHandle<IDQFilterTool> m_BadLBTool;
  bool m_useReadyFilterTool;
  ToolHandle<IDQFilterTool> m_ReadyFilterTool;

  bool m_useLArNoisyAlg;
  //bool m_useTriggerFilter;
  bool m_useCollisionFilterTool;
  bool m_useBeamBackgroundRemoval;

protected:

  // Common methods for LArCell-oriented histograms
  const CaloCell_ID* m_calo_id{nullptr};

  void getHistoCoordinates(const CaloDetDescrElement* dde, float& celleta, float& cellphi, unsigned& iLyr, unsigned& iLyrNS) const; 
  //enums to help with the conversion of Layer, partitions and such:
  //Enumerate layers 
  enum LayerEnum{EMBPA=0, EMBPC, EMB1A, EMB1C, EMB2A, EMB2C, EMB3A, EMB3C,
		 HEC0A, HEC0C, HEC1A, HEC1C, HEC2A, HEC2C, HEC3A, HEC3C,
		 EMECPA,EMECPC,EMEC1A,EMEC1C,EMEC2A,EMEC2C,EMEC3A,EMEC3C,
		 FCAL1A,FCAL1C,FCAL2A,FCAL2C,FCAL3A,FCAL3C,MAXLAYER};

  //Enumerate layer-types, ignoring sides. Useful for configuration that is per-definition symmetric 
  enum LayerEnumNoSides{EMBPNS=0, EMB1NS, EMB2NS, EMB3NS, HEC0NS, HEC1NS, HEC2NS, HEC3NS,
			EMECPNS,EMEC1NS,EMEC2NS,EMEC3NS,FCAL1NS,FCAL2NS,FCAL3NS,MAXLYRNS};



  //Mapping of CaloCell nomencature to CaloCellMonitoring nomencature
  const std::map<unsigned,LayerEnumNoSides> m_caloSamplingToLyrNS{ 
    {CaloSampling::PreSamplerB, EMBPNS},{CaloSampling::EMB1,EMB1NS},{CaloSampling::EMB2,EMB2NS},{CaloSampling::EMB3,EMB3NS},         //LAr Barrel
    {CaloSampling::PreSamplerE, EMECPNS},{CaloSampling::EME1,EMEC1NS}, {CaloSampling::EME2,EMEC2NS}, {CaloSampling::EME3,EMEC3NS},   //LAr Endcap     
    {CaloSampling::HEC0,HEC0NS}, {CaloSampling::HEC1,HEC1NS}, {CaloSampling::HEC2,HEC2NS}, {CaloSampling::HEC3,HEC3NS},              //Hadronic endcap
    {CaloSampling::FCAL0,FCAL1NS}, {CaloSampling::FCAL1,FCAL2NS}, {CaloSampling::FCAL2,FCAL3NS}                                      //FCAL
  };




};


#endif 
