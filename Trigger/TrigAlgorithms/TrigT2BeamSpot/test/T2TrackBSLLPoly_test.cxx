/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>

#include "../src/T2TrackBSLLPoly.h"
#include <stdexcept>
using namespace std;
using namespace PESA;


namespace {

void test_idx()
{
    constexpr int maxPower=2;
    cout << "=== Testing T2TrackBSLLPoly::idx method ===\n";
    for (unsigned power_Bx = 0; power_Bx < maxPower; ++ power_Bx) {
        for (unsigned power_By = 0; power_By < maxPower; ++ power_By) {
            for (unsigned power_tx = 0; power_tx < maxPower; ++ power_tx) {
                for (unsigned power_ty = 0; power_ty < maxPower; ++ power_ty) {
                    for (unsigned power_omegax = 0; power_omegax < maxPower; ++ power_omegax) {
                        for (unsigned power_omegay = 0; power_omegay < maxPower; ++ power_omegay) {
                            int idx=-1;
                            try{
                                idx = T2TrackBSLLPoly::idx(power_Bx, power_By, power_tx, power_ty, power_omegax, power_omegay);
                            } catch (std::out_of_range & e){
                                std::cout<<"index out of range for ";
                            }
                            cout << power_Bx << " " << power_By << " "
                                 << power_tx << " " << power_ty << " "
                                 << power_omegax << " " << power_omegay
                                 << " : " << idx << "\n";
                        }
                    }
                }
            }
        }
    }
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

}

int main()
{
  test_idx();
  test_update();
}
