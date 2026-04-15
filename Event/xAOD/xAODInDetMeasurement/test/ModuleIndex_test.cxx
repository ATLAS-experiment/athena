// #pragma GCC optimize ("O0")
// #undef NDEBUG // to enable asserts
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/ModuleIndex.h"

#include <array>
#include <limits>
#include <cstdint>
#include <cstdint>
#include <random>
#include <vector>
#include <cstdlib>
#include <iostream>
#define assert_always(a) if (!(a)) {std::cerr << ("assert failed:" #a) << std::endl; std::abort(); } do {} while (0)

struct MyRange {
   static constexpr unsigned int INVALID =  std::numeric_limits<unsigned int>::max();
   unsigned int begin = INVALID;
   unsigned int end = INVALID;
   unsigned int idx = INVALID;
   unsigned int hash = INVALID;
};


const PhaseII::DataRange &getRangeAlt(const std::vector<PhaseII::DataRange> &index, unsigned int hash) {
   return index[hash];
}

void registerRangeAlt(std::vector<PhaseII::DataRange> &index,
                   unsigned int identifier_hash,
                   unsigned int begin_index,
                   unsigned int end_index,
                   unsigned int container_index) {
   //   m_range[identifier_hash]=PhaseII::DataRange(begin_index, end_index - begin_index, container_index).makeCompact();
   //   index[identifier_hash]=PhaseII::DataRange::makeDataRange(begin_index,end_index,container_index);
   index[identifier_hash]=PhaseII::DataRange(begin_index, end_index - begin_index, container_index);
}

PhaseII::DataRange getRange(const ModuleIndex<xAOD::PixelClusterContainer> &index, unsigned int hash) {
   return index.range(hash);
}

void registerRangeAlt2(ModuleIndex<xAOD::PixelClusterContainer> &index,
                   unsigned int identifier_hash,
                   unsigned int begin_index,
                   unsigned int end_index,
                   unsigned int container_index) {
   //   m_range[identifier_hash]=PhaseII::DataRange(begin_index, end_index - begin_index, container_index).makeCompact();
   assert(identifier_hash < index.m_range.size());
   reinterpret_cast<PhaseII::DataRange &>(index.m_range[identifier_hash])=PhaseII::DataRange::makeDataRange(begin_index,end_index,container_index);
}

void registerRange(ModuleIndex<xAOD::PixelClusterContainer> &index,
                   unsigned int identifier_hash,
                   unsigned int begin_index,
                   unsigned int end_index,
                   unsigned int container_index) {
   //   m_range[identifier_hash]=PhaseII::DataRange(begin_index, end_index - begin_index, container_index).makeCompact();
   index.registerRange(identifier_hash,begin_index,end_index,container_index);
}



int main() {
   ModuleIndex<xAOD::PixelClusterContainer> index;
   index.m_range.resize(100);
   std::array<std::unique_ptr<xAOD::PixelClusterContainer>,3> cont{
      std::make_unique<xAOD::PixelClusterContainer>(),
      std::make_unique<xAOD::PixelClusterContainer>(),
      std::make_unique<xAOD::PixelClusterContainer>()};
      
   static constexpr unsigned int INVALID_IDX=std::numeric_limits<unsigned int>::max();
   std::array<unsigned int,3> cont_idx{INVALID_IDX, INVALID_IDX, INVALID_IDX};
   std::array<unsigned int,3> cont_sz{0u,0u,0u};
   std::vector<MyRange> range;
   std::vector<unsigned int> hash;
   for (unsigned int i=0; i<100; ++i) {
      hash.push_back(i);
   }
   std::random_device rd;
   std::mt19937 g(rd());
   std::shuffle(hash.begin(), hash.end(), g);
   
   for (unsigned int i : hash) {
      unsigned int idx=rand() % cont.size();
      unsigned int n=rand() % 100;
      if (i==hash.size()/3) {
         n=0u;
      }
      unsigned int index_cont_idx = index.containerIndex(*cont[idx]);
      if (cont_idx[idx] == INVALID_IDX) {
         cont_idx[idx]=index_cont_idx;
      }
      else {
         assert_always( cont_idx[idx] == index_cont_idx );
      }
      range.push_back(MyRange{.begin=cont_sz[idx], .end=cont_sz[idx]+n, .idx=index_cont_idx, .hash=i} );
      registerRange(index, i, cont_sz[idx], cont_sz[idx]+n, index_cont_idx);
   }
   for(const MyRange &a_range : range) {
      PhaseII::DataRange reg_range = getRange(index, a_range.hash);
      assert_always(   reg_range.beginIndex() == a_range.begin
                    && reg_range.endIndex() == a_range.end
                    && reg_range.containerIndex() == a_range.idx);
      assert_always( reg_range.beginIndex() != reg_range.endIndex() || reg_range.empty());
      assert_always( reg_range.size()>0 || reg_range.empty());
      assert_always( reg_range.empty() || index.elementIndex(reg_range.beginIndex())==reg_range.beginIndex());
   }
   return 0;
}
