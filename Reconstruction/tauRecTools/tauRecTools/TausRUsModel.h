/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUSRUSMODEL_H
#define TAURECTOOLS_TAUSRUSMODEL_H

#include "xAODCaloEvent/CaloVertexedTopoCluster.h"
#include "xAODTau/TauJet.h"
#include "xAODTau/TauTrack.h"
#include "xAODTracking/Vertex.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

/**
 * @brief Inputs and outputs of the TausRUs model.
 */
namespace TausRUsModel {

/// Which EDM collection a tensor is built from.
enum class Source { Cluster, TauTrack, Vertex, Scalar };

/// Transform applied to a raw variable before it enters the tensor.
enum class Transform { Identity, Log, Log10, Log1p, Abs, AbsLog };

struct Normalisation {
  Transform transform{Transform::Identity};
  float offset{0.f};
  float scale{1.f};

  float apply(float raw) const;
};

using ClusterGetter = std::function<bool(const xAOD::TauJet&, const xAOD::CaloVertexedTopoCluster&, float&)>;
using TrackGetter   = std::function<bool(const xAOD::TauJet&, const xAOD::TauTrack&, float&)>;
using VertexGetter  = std::function<bool(const xAOD::TauJet&, const xAOD::Vertex&, float&)>;
using ScalarGetter  = std::function<bool(const xAOD::TauJet&, float&)>;

/// One column of an input tensor: where the value comes from and how it is
/// normalised on the way in. A getter returning false leaves the slot at its
/// zero-padded default.
template <class Getter>
struct Variable {
  std::string name;              ///< reference-dumper branch name, for logging
  Getter get;
  Normalisation norm;
};

using ClusterVariable = Variable<ClusterGetter>;
using TrackVariable   = Variable<TrackGetter>;
using VertexVariable  = Variable<VertexGetter>;
using ScalarVariable  = Variable<ScalarGetter>;

/// One input tensor. Exactly one of the variable lists is filled, selected by
/// @c source; its length is the trailing tensor dimension and its order is the
/// model's feature order.
struct Input {
  std::string name;                     ///< model input node name
  Source source{Source::Scalar};
  size_t maxConstituents{0};            ///< unused for Source::Scalar
  std::vector<ClusterVariable> clusterVariables;
  std::vector<TrackVariable> trackVariables;
  std::vector<VertexVariable> vertexVariables;
  std::vector<ScalarVariable> scalarVariables;

  /// Exactly one list is non-empty, so the sum is that list's length.
  size_t nVariables() const {
    return clusterVariables.size() + trackVariables.size()
         + vertexVariables.size() + scalarVariables.size();
  }
};

struct Output {
  std::string name;                     ///< model output node name
  std::vector<int64_t> dims;
  size_t size{0};                       ///< product of dims
};

/// Class order of the 'primary' head, which is three raw logits.
enum PrimaryClass : size_t { QCD = 0, Tau = 1, Electron = 2 };

/// Number of classes of the 'decay_mode' head, which is raw logits.
constexpr size_t N_DECAY_MODES = 5;

/// Number of classes of the 'tautrack_class' head, which is raw logits per track slot.
constexpr size_t N_TRACK_CLASSES = 4;

/// The pt heads ('tes', 'charged_pion_pt', 'neutral_pion_pt') regress five
/// quantiles of the log response, log(pt_seedjet / pt_true). This is the index
/// of the median, the one to take as the point estimate; the pt then follows as
///   pt = exp(log(ptJetSeed) - quantile[PT_MEDIAN_QUANTILE]).
constexpr size_t PT_MEDIAN_QUANTILE = 2;

/// The two components of a phi head, which regresses the angle as a point on
/// the unit circle: phi = atan2(sin, cos).
enum PhiComponent : size_t { PHI_SIN = 0, PHI_COS = 1 };

std::vector<Input> inputs();
std::vector<Output> outputs();

/// The output node called @p name, or nullptr if the model has no such node.
const Output* findOutput(const std::vector<Output>& outputs, const std::string& name);

} // namespace TausRUsModel

#endif // TAURECTOOLS_TAUSRUSMODEL_H
