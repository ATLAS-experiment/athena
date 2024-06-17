# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# a function for resampling  meson in the lhe file 
# for specific HeavyN_meson model decay to meson+lepton (only) 
# the meson by default is generated as an on shell particle 
# with this adhoc solution a Breit-Wigner width in introduce to meson particle and 
# the meson+lepton 4 momentum has to be replace with new consistent values
# for more information mbahmani@cern.ch  

import sys
import math
import ROOT


def calculate_momentum_magnitude(E, Q2):
    return math.sqrt(E*E - Q2)

# Function to generate random virtuality according to Breit-Wigner distribution
def generate_virtuality(mass, width):
        Q = ROOT.gRandom.BreitWigner(mass, width)
        return Q*Q

def build_four_momentum(Energy, p3_magnitude, theta, phi):
    # Calculate the components of the new four-momentum vector
    px = p3_magnitude * math.sin(theta) * math.cos(phi)
    py = p3_magnitude * math.sin(theta) * math.sin(phi)
    pz = p3_magnitude * math.cos(theta)
    # Return the new four-momentum vector
    return [Energy, px, py, pz]

def build_TLorentz(energy, px,py,pz):
    # Build the new four-momentum using the energy and momentum components
    return ROOT.TLorentzVector(px, py, pz, energy)

# Function to resample meson particles in LHE file
def resample_meson(input_filename, output_filename, meson_id, meson_width ):
    event = False
    findW=False
    
    with open(input_filename, 'r') as input_file:
        with open(output_filename, 'w') as output_file:
            for line in input_file:
                    Q2=0
                    if not event and '<event>' not in line:
                        output_file.write(line)
                        continue
                    # Check if we are just starting an event
                    if not event and '<event>' in line:
                        output_file.write(line)
                        event = True
                        findW = False
                        continue
                    # Check if we have finished and are doing something else
                    if '<' in line or '>' in line:
                        event = False
                    if line.startswith('<mgrwt>') or line.startswith('</mgrwt>'):
                        output_file.write(line)
                        continue
                    
                    if abs(int(line.split()[0])) == 24:
                        #find W boson
                        findW=True
                    
                        
                      
                    if  (abs(int(line.split()[0])) == abs(int(meson_id)) and abs(int(line.split()[2])) == 4) or (not findW and abs(int(line.split()[0])) == abs(int(meson_id)) and abs(int(line.split()[2]))==3 ): # Check for meson
                            #check the charge
                            
                            line_parts = line.split()
                            line_parts[0] = str(int(line.split()[0]))
                            
                        
                            # Resample meson mass using Breit-Wigner distribution
                           
                            momentum_x = float(line.split()[6])
                            momentum_y = float(line.split()[7])
                            momentum_z= float(line.split()[8])
                            Energy = float(line.split()[9])
                            
                            meson=build_TLorentz(Energy,momentum_x, momentum_y, momentum_z) 
                            
                            continue
                       
                    if  ((abs(int(line.split()[0])) == 13 or abs(int(line.split()[0])) ==11) and int(line.split()[2])==4) or ( not findW and (abs(int(line.split()[0])) == 13 or abs(int(line.split()[0])) ==11) and int(line.split()[2])==3):
                           #check for the displace lepton
                            E_l = float(line.split()[9])
                            px_l= float(line.split()[6])
                            py_l= float(line.split()[7])
                            pz_l= float(line.split()[8])
                           
                            lep1=build_TLorentz(E_l,px_l, py_l, pz_l) 

                            # Boost into the rest frame of meson+lep1
                            combined = meson+lep1
                           
                            beta=combined.BoostVector()
                            meson.Boost(-beta)
                            lep1.Boost(-beta)
                            
                            

                            combined_B =meson+lep1
                            
                            meson_massp =meson.M()
                            
                            # Calculate theta
                            theta = math.acos(meson.Pz() / math.sqrt(meson.Px()**2 + meson.Py()**2 + meson.Pz()**2))
                            # Calculate phi
                            phi = math.atan2(meson.Py(), meson.Px())
                            # Generate random virtuality
                            Q2 = generate_virtuality(meson_massp, float(meson_width))
                            
                            while (Q2 > meson.E()*meson.E()+meson.Px()**2 + meson.Py()**2 + meson.Pz()**2 or not (0.3 < math.sqrt(Q2) < 1.5)):
                                # Generate another virtuality until it's smaller
                                Q2 = generate_virtuality(meson_massp, float(meson_width))
                            # build the meson energy
                            meson_energy = combined_B.E()/2 +((Q2-lep1.M()**2)/(2*combined_B.E()))

                            p3_magnitude=calculate_momentum_magnitude(meson_energy, Q2)
                            four_momentum = build_four_momentum(meson_energy, p3_magnitude, theta, phi)
                            meson.SetPxPyPzE(four_momentum[1],four_momentum[2],four_momentum[3],four_momentum[0])
                            
                            px_lep= combined_B.Px()-meson.Px()
                            py_lep= combined_B.Py()-meson.Py()
                            pz_lep= combined_B.Pz()-meson.Pz()
                            # Ensure momentum and energy conservation for mu1
                            lep1_energy = math.sqrt(px_lep**2+py_lep**2+pz_lep**2+lep1.M()**2)
                            lep1.SetPxPyPzE(px_lep,py_lep,pz_lep,lep1_energy)
                            
                            # Boost meson and mu1 particles back to the lab frame
                            meson.Boost(beta)
                            lep1.Boost(beta)
                            
                            line_parts[9] = str("{:+.10e}".format(meson.E()))
                            line_parts[6] =str("{:+.10e}".format(meson.Px()))
                            line_parts[7] =str("{:+.10e}".format(meson.Py()))
                            line_parts[8] =str("{:+.10e}".format(meson.Pz()))
                            line_parts[10] =str("{:+.10e}".format(meson.M()))
                
                            updated_line = '  '.join(line_parts) + '\n'   
                            output_file.write('      ' +''.join(updated_line) )
                                  
                    if  ((abs(int(line.split()[0])) == 13 or abs(int(line.split()[0])) ==11) and int(line.split()[2])==4) or (not findW and (abs(int(line.split()[0])) == 13 or abs(int(line.split()[0])) ==11) and int(line.split()[2])==3):
                            line_parts_l = line.split()
                            line_parts_l[9] = str("{:+.10e}".format(lep1.E()))
                            line_parts_l[6] =str("{:+.10e}".format(lep1.Px()))
                            line_parts_l[7] =str("{:+.10e}".format(lep1.Py()))
                            line_parts_l[8] =str("{:+.10e}".format(lep1.Pz()))
                            line_parts_l[10] =str("{:+.10e}".format(lep1.M()))
                            
                            updated_line_l = '  '.join(line_parts_l) + '\n'   
                            output_file.write('      ' +''.join(updated_line_l) + '\n')
                                 
                    
                    else:
                        output_file.write(line)
                           
            

if __name__ == "__main__":
    print("len(sys.argv)", len(sys.argv))
    if len(sys.argv) !=5:
        print("Usage: python resample_meson.py input_file.lhe output_file.lhe meson_id meson_width")
    else:
        input_file = sys.argv[1]
        output_file = sys.argv[2]
        meson_id = sys.argv[3]
        meson_width = sys.argv[4]

        resample_meson(input_file, output_file, meson_id, meson_width)

