/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

  This is a subclass of IConstituentsLoader. 
  It is used to load the general Muons from the jet
  and extract their features for the NN evaluation. 
*/

#ifndef MUONS_LOADER_H
#define MUONS_LOADER_H

// local includes
#include "FlavorTagInference/ConstituentsLoader.h"
#include "FlavorTagInference/CustomGetterUtils.h"

// EDM includes
#include "xAODBase/IParticle.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODMuon/Muon.h"

// STL includes
#include <string>
#include <vector>
#include <functional>

namespace FlavorTagInference {

ConstituentsInputConfig createMuonsLoaderConfig(
    std::pair<std::string, std::vector<std::string>> iparticle_names
);
// Subclass for Muons loader inherited from abstract IConstituentsLoader class
class MuonsLoader final : public IConstituentsLoader {
 public:
  MuonsLoader(const ConstituentsInputConfig&, const FTagOptions& options);
  std::tuple<Inputs, std::vector<const xAOD::IParticle*>> getData(
      const xAOD::IParticle& jet ) const override;
  const FTagDataDependencyNames& getDependencies() const override;
  const std::set<std::string>& getUsedRemap() const override;
  const std::string& getName() const override;
  const ConstituentsType& getType() const override;

 protected:
  // typedefs
  typedef xAOD::IParticle Jet;
  typedef std::pair<std::string, double> NamedVar;
  typedef std::pair<std::string, std::vector<double>> NamedSeq;
  // muons typedefs
  typedef std::vector<const xAOD::Muon*> Muons;
  typedef std::function<double(const xAOD::Muon*, const Jet&)> MuonSortVar;

  // filter function
  typedef std::function<bool(const Jet&, const xAOD::Muon*)> MuonFilter;

  // usings for Muon getter
  using AE = SG::AuxElement;
  using IPC = xAOD::IParticleContainer;
  using PartLinks = std::vector<ElementLink<IPC>>;
  using IPV = std::vector<const xAOD::Muon*>;

  MuonSortVar muonSortVar(ConstituentsSortOrder);

  Muons getMuonsFromJet(const xAOD::IParticle& jet) const;
  std::pair<MuonFilter,std::set<std::string>> muonFilter(ConstituentsSelection config);

  MuonSortVar m_muonSortVar;
  MuonFilter m_muonFilter;
  getter_utils::SeqGetter<xAOD::Muon> m_seqGetter;
  std::function<IPV(const Jet&)> m_associator;
};
}  // namespace FlavorTagInference

#endif
