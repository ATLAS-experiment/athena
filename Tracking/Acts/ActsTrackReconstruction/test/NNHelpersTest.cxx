#undef NDEBUG
#include <cmath>
#include <iostream>

#include "TestTools/expect.h"
#include "src/detail/NNPixelClusterCalibratorHelpers.h"

void settersTest() {
  using namespace ActsTrk;
  NNinput i0;
  for (auto el : i0.payload()) {
    VALUE(el) EXPECTED(0);
  }

  VALUE(static_cast<std::size_t>(NNinput::Index::totalSize)) EXPECTED(67);
  VALUE(i0.payload().size()) EXPECTED(67);

  VALUE(static_cast<std::size_t>(NNinput::Index::theta)) EXPECTED(66);
  VALUE(static_cast<std::size_t>(NNinput::Index::phi)) EXPECTED(65);

  i0.set(NNinput::Index::phi, 0.7);
  i0.set(NNinput::Index::theta, 0.8);

  auto v_at = [&i0](int i) -> float { return i0.payload()[i]; };

  VALUE(v_at(66)) EXPECTED(0.8f);
  VALUE(v_at(65)) EXPECTED(0.7f);
  try {
    i0.setPixelYPitch(0, 0.25);
    i0.setPixelYPitch(1, 0.25);
    i0.setPixelYPitch(2, 0.5);

    i0.setPixelXPitch(0, 0.25);
    i0.setPixelXPitch(1, 0.25);
    i0.setPixelXPitch(2, 0.5);

    for (int i = 49; i < 49 + 7; ++i) {
      std::cout << "index " << i << " y pitch  " << v_at(i) << std::endl;
    }

    VALUE(v_at(48)) EXPECTED(0.f);
    VALUE(v_at(49)) EXPECTED(0.25f);
    VALUE(v_at(50)) EXPECTED(0.25f);
    VALUE(v_at(51)) EXPECTED(0.5f);

    VALUE(v_at(55)) EXPECTED(0.f);
    VALUE(v_at(56)) EXPECTED(0.25f);
    VALUE(v_at(57)) EXPECTED(0.25f);
    VALUE(v_at(58)) EXPECTED(0.5f);
    VALUE(v_at(59)) EXPECTED(0.f);

    // no change to other variables
    VALUE(v_at(66)) EXPECTED(0.8f);
    VALUE(v_at(65)) EXPECTED(0.7f);

    // finally fill the cluster charge
    int counter = 0;
    for (std::size_t x = 0; x < 7; ++x) {
      for (std::size_t y = 0; y < 7; ++y) {
        i0.setPixelCharge(x, y, static_cast<NNinput::payload_t>(counter));
        counter++;
      }
    }
    for (std::size_t i = 0; i < 49; ++i) {
      VALUE(v_at(i)) EXPECTED(static_cast<NNinput::payload_t>(i));
    }
  } catch (const std::exception& e) {
    std::cout << "exception while filling  " << e.what() << std::endl;
    throw;
  }
}

void indexCalcTest() {
  using namespace ActsTrk;
  const int center = 23;
  VALUE(NNinput::toNNinputIndex(20, center)) EXPECTED(0);
  VALUE(NNinput::toNNinputIndex(21, center)) EXPECTED(1);
  VALUE(NNinput::toNNinputIndex(22, center)) EXPECTED(2);
  VALUE(NNinput::toNNinputIndex(23, center)) EXPECTED(3);
  VALUE(NNinput::toNNinputIndex(24, center)) EXPECTED(4);
  VALUE(NNinput::toNNinputIndex(25, center)) EXPECTED(5);
  VALUE(NNinput::toNNinputIndex(26, center)) EXPECTED(6);

}

int main() {
  try {
    settersTest();
  } catch (const std::exception&) {
    return -1;
  }
  indexCalcTest();
}