/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
  @class TElectronTestAlg
  @brief Dual use Test Alg for TElectronEfficiencyTool
  @author Christos Anastopoulos
  */

#ifndef TElectronTestAlg_H
#define TElectronTestAlg_H

#include <memory>
//
#include "AnaAlgorithm/AnaReentrantAlgorithm.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "AsgTools/PropertyWrapper.h"
#include "ColumnarMode/ColumnarMode.h"
//
#include "ElectronEfficiencyCorrection/TElectronEfficiencyCorrectionTool.h"
#include "xAODEgamma/ElectronContainer.h"
namespace ADD_CMODE(CP) {
class TElectronTestAlg : public EL::AnaReentrantAlgorithm {

 public:
  TElectronTestAlg(const std::string& n, ISvcLocator* l)
      : EL::AnaReentrantAlgorithm(n, l) {}
  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;

 private:
  bool m_doDetail{false};
  bool m_doToys{false};
  int m_numberCorr{0};   /// Number of Correlated syst
  double m_lowestEt{0};  /// Lowest Et for the reccomendations

  /// This is not a tool but more of an utility
  std::unique_ptr<Root::TElectronEfficiencyCorrectionTool> m_pimpl = nullptr;
  /// Input collection name.
  SG::ReadHandleKey<xAOD::ElectronContainer> m_electronContainer{
      this, "ElectronContainer", "Electrons", "The input Electrons container"};

  /// Label for the collection.
  Gaudi::Property<std::string> m_mapFile{
      this, "MapFilePath",
      "ElectronEfficiencyCorrection/2015_2025/rel22.2/"
      "2022_Summer_Prerecom_v1/map2.txt",
      "Full path to the map file"};
  Gaudi::Property<std::string> m_recoKey{this, "RecoKey", "",
                                         "Key associated with reconstruction"};
  Gaudi::Property<std::string> m_idKey{
      this, "IdKey", "", "Key associated with identification working point"};

  Gaudi::Property<std::string> m_isoKey{
      this, "IsoKey", "", "Key associated with isolation working point"};

  Gaudi::Property<std::string> m_triggerKey{
      this, "TriggerKey", "", "Key associated with trigger working point"};

  Gaudi::Property<int> m_dataType{this, "dataType",
                                  PATCore::ParticleDataType::Full,
                                  "Particle Data Type Full/Fast"};

  enum mode { All = 0, Full = 1, Total = 2, Toys = 3 };
  Gaudi::Property<int> m_mode{this, "mode", All, "Mode (All/Full/Total/Toys"};
  Gaudi::Property<int> m_number_of_toys{this, "number_of_toys", 40,
                                        "Number of toys in the ensemble"};

  Gaudi::Property<bool> m_decorate{this, "doDecorate", true, "Do decorations"};

  SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_SF{this, "DoNotSet_SF",
                                                        "Electrons.SF", ""};
  SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_TotalUp{
      this, "DoNotSet_TotalUp", "Electrons.TotalUp", ""};
  SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_TotalDown{
      this, "DoNotSet_TotalDown", "Electrons.TotalDown", ""};
  SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_uncorrUp{
      this, "DoNotSet_UnCorrUp", "Electrons.UnCorrUp", ""};
  SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_uncorrDown{
      this, "DoNotSet_UnCorrDown", "Electrons.UnCorrDown", ""};
  SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_HistIndex{
      this, "DoNotSet_HistIndex", "Electrons.HistIndex", ""};
  SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_HistBin{
      this, "DoNotSet_HistBin", "Electrons.HistBin", ""};
  SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_corrUp{
      this, "DoNotSet_CorrUp", "Electrons.CorrUp", ""};
  SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_corrDown{
      this, "DoNotSet_CorrDown", "Electrons.CorrDown", ""};
  SG::WriteDecorHandleKey<xAOD::ElectronContainer> m_toys{this, "DoNotSet_",
                                                          "Electrons.toys", ""};
};
}  // namespace ADD_CMODE(CP)
#endif

