#include <stdexcept>
#include <vector>

#include "ZdcUtils/RPDUtils.h"

namespace RPDUtils {
  unsigned int ZDCSideToSideIndex(int const ZDCSide) {
    switch (ZDCSide) {
      case -1:
        return sideC;
      case 1:
        return sideA;
      default:
        throw std::runtime_error("Invalid ZDC side: " + std::to_string(ZDCSide));
    }
    return 0;
  };
  template<typename T>
  std::ostream & operator <<(std::ostream & os, const std::vector<T> & v) {
    os << "{ ";
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(os, ", "));
    os << "}";
    return os;
  }
  template std::ostream & operator << <>(std::ostream &, const std::vector<float> &);
  template<class T> OptionalToolProperty<T>::OptionalToolProperty() = default;
  template<class T> OptionalToolProperty<T>::OptionalToolProperty(T value) : m_value(std::move(value)) {}
  template<class T> bool OptionalToolProperty<T>::has_value() const {
    return m_hasValue;
  }
  template<class T> T const& OptionalToolProperty<T>::value() const {
    if (!m_hasValue) {
      throw std::runtime_error("accessed null OptionalToolProperty!");
    }
    return m_value;
  }
  template<class T> std::ostream & operator<<(std::ostream& os, OptionalToolProperty<T> const& obj) {
    if (obj.has_value()) {
      return os << obj.value();
    }
    return os << "None";
  }
  template std::ostream & operator<<(std::ostream&, OptionalToolProperty<bool> const&);
  template std::ostream & operator<<(std::ostream&, OptionalToolProperty<unsigned int> const&);
  template std::ostream & operator<<(std::ostream&, OptionalToolProperty<float> const&);
  template std::ostream & operator<<(std::ostream&, OptionalToolProperty<std::vector<float>> const&);
  template<class T> OptionalToolProperty<T>::operator T&() {
    // this method is called when the value is set from python, so this now has a value
    m_hasValue = true;
    return m_value;
  }
  template class OptionalToolProperty<bool>;
  template class OptionalToolProperty<unsigned int>;
  template class OptionalToolProperty<float>;
  template class OptionalToolProperty<std::vector<float>>;
  template <typename T>
  std::string vecToString(std::vector<T> const& v) {
    std::ostringstream oss;
    oss << "{ ";
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(oss, ", "));
    oss << "}";
    return oss.str();
  }
  template std::string vecToString(std::vector<float> const& v);
} // namespace RPDUtils
