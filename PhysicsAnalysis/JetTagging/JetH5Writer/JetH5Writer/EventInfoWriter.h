#ifndef EVENT_INFO_WRITER
#define EVENT_INFO_WRITER

#include <memory>

namespace H5 {
  class Group;
}
namespace H5Utils {
  template <size_t N, typename I> class Writer;
}
namespace xAOD {
  class EventInfo_v1;
  typedef EventInfo_v1 EventInfo;
}
struct EventInfoWriterConfig;

class EventInfoWriter {
public:
  using Writer_t = H5Utils::Writer<0, const xAOD::EventInfo&>;
  EventInfoWriter(H5::Group& output_group, const EventInfoWriterConfig&);
  ~EventInfoWriter();
  void fill(const xAOD::EventInfo&);
  void flush();
private:
  std::unique_ptr<Writer_t> m_writer;
};

#endif
