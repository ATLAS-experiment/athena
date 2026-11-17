/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SURVEYCONSTRAINTTOOLS_SURVEYCONSTRAINT_H
#define SURVEYCONSTRAINTTOOLS_SURVEYCONSTRAINT_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetSurveyConstraintTool/ISurveyConstraint.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/ReadCondHandleKey.h"
#include <vector>
#include <map>

class AtlasDetectorID;
class IInDetAlignDBTool;
class PixelID;
class SCT_ID;

class SurveyConstraint : public extends <AthAlgTool, ISurveyConstraint>{
 public:
  using base_class::base_class;
  
  virtual ~SurveyConstraint() = default;
  
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  
  virtual StatusCode computeConstraint(const Identifier& ModuleID,
				       Amg::VectorX& dparams,
				       double& deltachisq,
				       Amg::VectorX& dchisqdparams,
				       Amg::MatrixX& d2chisqdpdp) override;
  virtual void setup_SurveyConstraintModules() override;

  virtual void MMap(std::map<Identifier, SurveyConstraintModule*, std::less<Identifier> >&  ModuleMap) override;

  virtual int getWeightPixEC(//const Identifier& ModuleID,
			    Amg::MatrixX& weight) override;
  virtual int getWeightPixB(//const Identifier& ModuleID,
			    Amg::MatrixX& weight) override;
  virtual int getWeightSCTEC(//const Identifier& ModuleID,
			    Amg::MatrixX& weight) override;
  virtual int getWeightSCTB(//const Identifier& ModuleID,
			    Amg::MatrixX& weight) override;
  virtual void getSurveyCoordsPixEC(//const Identifier& ModuleID,
				    std::vector< Amg::Vector3D > & coords) override;
  virtual void getSurveyCoordsPixB(//const Identifier& ModuleID,
				   std::vector< Amg::Vector3D > & coords) override;
  virtual void getSurveyCoordsSCTEC(//const Identifier& ModuleID,
				    std::vector< Amg::Vector3D > & coords) override;
  virtual void getSurveyCoordsSCTB(//const Identifier& ModuleID,
				   std::vector< Amg::Vector3D > & coords) override;
  virtual void GlobalToLocal(SurveyConstraintModule* mut,std::vector<SurveyConstraintPoint>& points) override;
  virtual int SectorNumber(int phi_module) override;
  virtual double PhiModuleToSector(int phi_module) override;
  virtual void TransformSector(Identifier Pixel_ModuleID,
			       SurveyConstraintModule* mut,
			       Amg::Vector3D& current,
			       Amg::Transform3D CurrentTransRandSect) override;
  
 private :
  const AtlasDetectorID*                 m_idHelper{};
  const PixelID*                         m_pixid{nullptr};
  const SCT_ID*                          m_sctid{nullptr};
  

  IInDetAlignDBTool*                     m_current_IDAlignDBTool{nullptr};
  IInDetAlignDBTool*                     m_survey_IDAlignDBTool{nullptr};

  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey{this, "PixelDetEleCollKey", "PixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_SCTDetEleCollKey{this, "SCTDetEleCollKey", "SCT_DetectorElementCollection", "Key of SiDetectorElementCollection for SCT"};

  // dictionary of module constrain objects, indexed by moduleID
  std::map<Identifier, SurveyConstraintModule*, std::less<Identifier> >  m_ModuleMap;        //!< Map of Wafer objects

  // algorithm parameters, possible to declare at runtime
  Gaudi::Property<double> m_SurveyWeightX{this, "SurveyWeightX", 1.0};              //!< Multiplicative weight, representing systematic unc. of survey 
                                                                       // (e.g. bow), set to 1 for now
  Gaudi::Property<double> m_SurveyWeightY{this, "SurveyWeightY", 1.0};              //!< ""
  Gaudi::Property<double> m_SurveyWeightZ{this, "SurveyWeightZ", 1.0};              //!< ""
  Gaudi::Property<double> m_SurveyWeightPhiX{this, "SurveyWeightPhiX", 1.0};           //!< ""
  Gaudi::Property<double> m_SurveyWeightPhiY{this, "SurveyWeightPhiY", 1.0};           //!< ""
  Gaudi::Property<double> m_SurveyWeightPhiZ{this, "SurveyWeightPhiZ", 1.0};           //!< ""
  Gaudi::Property<double> m_TransX{this, "TransX", 0.0};                     //!< Translation in X of the first current PixEC module 
  Gaudi::Property<double> m_TransY{this, "TransY", 0.0};                     //!< Translation in Y of the first current PixEC module 
  Gaudi::Property<double> m_TransZ{this, "TransZ", 0.0};                     //!< Translation in Z of the first current PixEC module 
  Gaudi::Property<double> m_RotX{this, "RotX", 0.0};                       //!< Rotation in X of the first current PixEC module 
  Gaudi::Property<double> m_RotX2{this, "RotX2", 0.0};                      //!< Rotation in X (after Y & Z) of the first current PixEC module 
  Gaudi::Property<double> m_RotY{this, "RotY", 0.0};                       //!< Rotation in Y of the first current PixEC module 
  Gaudi::Property<double> m_RotZ{this, "RotZ", 0.0};                       //!< Rotation in Z of the first current PixEC module 
  Gaudi::Property<double> m_TransXRand{this, "TransXRand", 0.0046};                 //!< Weight & rand Translation in X of current PixEC modules 
  Gaudi::Property<double> m_TransYRand{this, "TransYRand", 0.0047};                 //!< Weight & rand Translation in Y of current PixEC modules 
  Gaudi::Property<double> m_TransZRand{this, "TransZRand", 0.0127};                 //!< Weight & rand Translation in Z of current PixEC modules 
  Gaudi::Property<double> m_RotXRand{this, "RotXRand", 0.0003};                   //!< Weight & rand Rotation in X of current PixEC modules 
  Gaudi::Property<double> m_RotYRand{this, "RotYRand", 0.0007};                   //!< Weight & rand Rotation in Y of current PixEC modules 
  Gaudi::Property<double> m_RotZRand{this, "RotZRand", 0.00012};                   //!< Weight & rand Rotation in Z of current PixEC modules  
  Gaudi::Property<double> m_TransXRandPixB{this, "TransXRandPixB", 0.01};             //!< Weight & rand Translation in X of current PixB modules 
  Gaudi::Property<double> m_TransYRandPixB{this, "TransYRandPixB", 0.01};             //!< Weight & rand Translation in Y of current PixB modules 
  Gaudi::Property<double> m_TransZRandPixB{this, "TransZRandPixB", 0.1};             //!< Weight & rand Translation in Z of current PixB modules 
  Gaudi::Property<double> m_RotXRandPixB{this, "RotXRandPixB", 0.001};               //!< Weight & rand Rotation in X of current PixB modules 
  Gaudi::Property<double> m_RotYRandPixB{this, "RotYRandPixB", 0.001};               //!< Weight & rand Rotation in Y of current PixB modules 
  Gaudi::Property<double> m_RotZRandPixB{this, "RotZRandPixB", 0.00024};               //!< Weight & rand Rotation in Z of current PixB modules   
  Gaudi::Property<double> m_TransXRandSCTEC{this, "TransXRandSCTEC", 0.03};             //!< Weight & rand Translation in X of current SCTEC modules 
  Gaudi::Property<double> m_TransYRandSCTEC{this, "TransYRandSCTEC", 0.03};             //!< Weight & rand Translation in Y of current SCTEC modules 
  Gaudi::Property<double> m_TransZRandSCTEC{this, "TransZRandSCTEC", 0.03};             //!< Weight & rand Translation in Z of current SCTEC modules 
  Gaudi::Property<double> m_RotXRandSCTEC{this, "RotXRandSCTEC", 0.001};               //!< Weight & rand Rotation in X of current SCTEC modules 
  Gaudi::Property<double> m_RotYRandSCTEC{this, "RotYRandSCTEC", 0.001};               //!< Weight & rand Rotation in Y of current SCTEC modules 
  Gaudi::Property<double> m_RotZRandSCTEC{this, "RotZRandSCTEC", 0.0003};               //!< Weight & rand Rotation in Z of current SCTEC modules    
  Gaudi::Property<double> m_TransXRandSCTB{this, "TransXRandSCTB", 0.1};             //!< Weight & rand Translation in X of current SCTB modules 
  Gaudi::Property<double> m_TransYRandSCTB{this, "TransYRandSCTB", 0.1};             //!< Weight & rand Translation in Y of current SCTB modules 
  Gaudi::Property<double> m_TransZRandSCTB{this, "TransZRandSCTB", 0.1};             //!< Weight & rand Translation in Z of current SCTB modules 
  Gaudi::Property<double> m_RotXRandSCTB{this, "RotXRandSCTB", 0.001};               //!< Weight & rand Rotation in X of current SCTB modules 
  Gaudi::Property<double> m_RotYRandSCTB{this, "RotYRandSCTB", 0.001};               //!< Weight & rand Rotation in Y of current SCTB modules 
  Gaudi::Property<double> m_RotZRandSCTB{this, "RotZRandSCTB", 0.001};               //!< Weight & rand Rotation in Z of current SCTB modules  
  Gaudi::Property<bool> m_gaus{this, "Gaus", false};                       //!< use random (Gaus) rotations and translations   
  Gaudi::Property<double> m_TransXRandSect{this, "TransXRandSect", 0.0};             //!< rand Translation in X of current PixEC sectors 
  Gaudi::Property<double> m_TransYRandSect{this, "TransYRandSect", 0.0};             //!< rand Translation in Y of current PixEC sectors 
  Gaudi::Property<double> m_TransZRandSect{this, "TransZRandSect", 0.0};             //!< rand Translation in Z of current PixEC sectors 
  Gaudi::Property<double> m_RotXRandSect{this, "RotXRandSect", 0.0};               //!< rand Rotation in X of current PixEC sectors 
  Gaudi::Property<double> m_RotYRandSect{this, "RotYRandSect", 0.0};               //!< rand Rotation in Y of current PixEC sectors 
  Gaudi::Property<double> m_RotZRandSect{this, "RotZRandSect", 0.0};               //!< rand Rotation in Z of current PixEC sectors  
  Gaudi::Property<double> m_TransLayerRand{this, "TransLayerRand", -1};             //!< rand Translation in X,Y,Z of all Pixel/SCT EC/B layers  
  Gaudi::Property<int> m_misaligncase{this, "Misaligncase", -1};               //!< misaligncase
  Gaudi::Property<bool> m_gausSect{this, "GausSect", false};                   //!< use random (Gaus) rotations and translations for sectors  
  Gaudi::Property<bool> m_FullDisk{this, "FullDisk", false};                   //!< use Full Disk
  Gaudi::Property<double> m_scaleZ{this, "ScaleZ", 1.0};                     //!< scale Z coordinate to match sensitivity
  Gaudi::Property<double> m_proximity{this, "Proximity", 999999.};                  //!< Proximity of Survey points used for alignment of SOW
  Gaudi::Property<std::string> m_aligndbtoolinst{this, "CurrentDBToolInst", "InDetCurrentDBTool"}; 
  Gaudi::Property<std::string> m_surveydbtoolinst{this, "SurveyDBToolInst", "InDetSurveyDBTool"}; 
  Gaudi::Property<std::string> m_surveywfile{this, "SurveyWfile", ""};
  Gaudi::Property<std::string> m_surveyrfile{this, "SurveyRFile", ""};
  Gaudi::Property<bool> m_ntuple{this, "Ntuple", false};

};

#endif // SURVEYCONSTRAINTTOOLS_SURVEYCONSTRAINT_H
