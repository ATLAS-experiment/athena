#ifndef IPARTICLE_WRITER_H
#define IPARTICLE_WRITER_H

#include <memory>
#include <vector>

namespace H5 {
  class Group;
}
namespace H5Utils {
  template <size_t N, typename I> class Writer;
}
namespace xAOD {
  class IParticle;
}
struct IParticleWriterConfig;

namespace details {
  // implementation depends on type of array we're storing
  class IParticleWriterBase;
}

class IParticleWriter {
public:
  IParticleWriter(H5::Group& output_group, const IParticleWriterConfig&);
  ~IParticleWriter();
  void fill(const std::vector<const xAOD::IParticle*>&);
  void flush();
private:
  std::unique_ptr<details::IParticleWriterBase> m_writer;
};

#endif
