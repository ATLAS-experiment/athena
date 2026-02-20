#ifndef MUONINFERENCE_GRAPHBUCKETFILTERTOOL_H
#define MUONINFERENCE_GRAPHBUCKETFILTERTOOL_H

#include "BucketInferenceToolBase.h"
#include "StoreGate/WriteHandleKey.h"
#include "MuonSpacePoint/SpacePointContainer.h"

namespace MuonML {

class GraphBucketFilterTool : public BucketInferenceToolBase {
public:
  using BucketInferenceToolBase::BucketInferenceToolBase;
  ~GraphBucketFilterTool() override = default;

  StatusCode initialize() override final;
  StatusCode runGraphInference(const EventContext& ctx, GraphRawData& graphData) const override final;

private:
  /// Input: buckets to filter
  SG::ReadHandleKey<MuonR4::SpacePointContainer> m_readKey{
      this, "ReadSpacePointKey", "MuonSpacePoints"};

  /// Output: buckets that pass the class selection
  SG::WriteHandleKey<MuonR4::SpacePointContainer> m_writeKey{
      this, "WriteSpacePointKey", "FilteredMlBuckets"};

  /// Keep these classes (argmax ∈ AcceptClasses)
  Gaudi::Property<std::vector<int>> m_acceptClasses{this, "AcceptClasses", {1, 2}};

  /// Bias applied to class 0: logits += [-BiasClass0, 0, 0]
  Gaudi::Property<double> m_biasClass0{this, "BiasClass0", 2.71484375};

};

} // namespace MuonML

#endif
