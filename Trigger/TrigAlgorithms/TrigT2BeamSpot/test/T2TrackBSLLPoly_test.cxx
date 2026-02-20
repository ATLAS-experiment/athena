/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>

#include "../src/T2TrackBSLLPoly.h"
#include "../src/idx.h"
#include <array>

using namespace std;
using namespace PESA;


template<unsigned Bx, unsigned By, unsigned tx, unsigned ty, unsigned ox, unsigned oy>
consteval int idx_or_neg1()
{   
  if constexpr(Bx<=2 and By<=2 and tx<=2 and ty<=2 and ox<=2 and oy<=2){
    if constexpr(g_order[Bx][By][tx][ty]>=0 and g_order2[ox][oy]>=0){
      return idx<Bx, By, tx, ty, ox, oy>();
    } else return -1;
  } else return -1;
}

// Build a compile-time array over all combinations
template<unsigned PBx, unsigned PBy, unsigned Ptx, unsigned Pty, unsigned Pox, unsigned Poy>
struct idx_table6 {
    static constexpr std::size_t total = 1ull * PBx * PBy * Ptx * Pty * Pox * Poy;
  template<std::size_t K>
  static consteval int value() {
      // decode K in mixed radix, least-significant axis first
      constexpr std::size_t k0 = K;
      constexpr unsigned oy = static_cast<unsigned>(k0 % Poy);
      constexpr std::size_t k1 = k0 / Poy;

      constexpr unsigned ox = static_cast<unsigned>(k1 % Pox);
      constexpr std::size_t k2 = k1 / Pox;

      constexpr unsigned ty = static_cast<unsigned>(k2 % Pty);
      constexpr std::size_t k3 = k2 / Pty;

      constexpr unsigned tx = static_cast<unsigned>(k3 % Ptx);
      constexpr std::size_t k4 = k3 / Ptx;

      constexpr unsigned By = static_cast<unsigned>(k4 % PBy);
      constexpr std::size_t k5 = k4 / PBy;

      constexpr unsigned Bx = static_cast<unsigned>(k5 % PBx);

      return idx_or_neg1<Bx, By, tx, ty, ox, oy>();
  }

    template<std::size_t... Is>
    static consteval auto make_impl(std::index_sequence<Is...>) {
        return std::array<int, total>{ value<Is>()... };
    }

    static consteval auto make() {
        return make_impl(std::make_index_sequence<total>{});
    }
};

inline int 
idx_runtime(unsigned power_Bx, unsigned power_By, unsigned power_tx, unsigned power_ty,
    unsigned power_omegax, unsigned power_omegay){
    if (power_Bx > 2 or power_By > 2
            or power_tx > 2 or power_ty > 2
            or power_omegax > 2 or power_omegay > 2) {
        return -1;
    }
    int idx = g_order[power_Bx][power_By][power_tx][power_ty];
    if (idx < 0) return -1;
    int idx2 = g_order2[power_omegax][power_omegay];
    if (idx2 < 0) return -1;
    return idx*g_size2 + idx2;
}



bool test_idx(){
  bool result(true);
  constexpr auto all = idx_table6<3,3,3,3,3,3>::make();
  cout << "=== Testing T2TrackBSLLPoly::idx method ===\n";
  cout << "=== Size  = " <<all.size()<<" ===\n";
  for (std::size_t k = 0; k < all.size(); ++k) {
    std::size_t t = k;
    auto step = [](std::size_t& x, unsigned base){ unsigned d = x % base; x /= base; return d; };
    unsigned oy = step(t, 3), ox = step(t, 3), ty = step(t, 3),
             tx = step(t, 3), By = step(t, 3), Bx = step(t, 3);

    int v = all[k];
    int v2 = idx_runtime(Bx, By, tx, ty, ox, oy);
    std::cout << Bx << ' ' << By << ' ' << tx << ' ' << ty << ' '
              << ox << ' ' << oy << " : ";
    std::cout << v << '\n';
    if (v2 != v){
      result = false;
      break;
    }
  }
  return result;
}

void test_update()
{
    cout << "=== Testing T2TrackBSLLPoly::update method ===\n";
    T2TrackBSLLPoly llpoly(0.01);
    std::vector<double> coeff;

    double z_0 = 3;
    double d_0 = .8;
    double phi = 1.5;
    double var_d0 = 0.05*0.05;
    llpoly.update(z_0, d_0, phi, var_d0, coeff);

    cout << "coeff size: " << coeff.size() << "\n";
    for (unsigned i = 0; i < coeff.size(); ++i) {
        cout << "coeff[" << i << "]: " << coeff[i] << "\n";
    }

    z_0 = -3;
    d_0 = .2;
    phi = .1;
    var_d0 = 0.08*0.08;
    llpoly.update(z_0, d_0, phi, var_d0, coeff);

    z_0 = 0;
    d_0 = -0.8;
    phi = -1.7;
    var_d0 = 0.1*0.1;
    llpoly.update(z_0, d_0, phi, var_d0, coeff);

    cout << "coeff size: " << coeff.size() << "\n";
    for (unsigned i = 0; i < coeff.size(); ++i) {
        cout << "coeff[" << i << "]: " << coeff[i] << "\n";
    }
}



int main()
{
  bool ok = test_idx();
  test_update();
  if (not ok) return 1;
  return 0;
}
