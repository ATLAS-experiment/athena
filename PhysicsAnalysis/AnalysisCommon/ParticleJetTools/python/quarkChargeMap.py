# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration                                                                                                                                             

hadrons_dict={
    0:0,    #light   
    15:15,   #tau-   
    #Bottom mesons     
    511:-5,   #B0    (d-bbar)  
    513:-5, #B*0 (d bbar)      
    521:-5,   #B+ (u-bbaar)    
    523:-5, #B*+ (u bbar)      
    10511:-5,  #B_0*0 (d-bbar) 
    531:-5,   #Bs0 (s-bbar)    
    541:-5,   #Bc+ (c-bbar)    
    543:-5, #B_c*+ (c bbar)    
    # bbar mesons      
    10551:0,  #χ b0 (1P )(bbar)      #assigned to other    
    100551:0, #η b (2S)  (bbar)      #assigned to other     
    110551:0, #χ b0 (2P )(bbar)      #assigned to other     
    20553:0,  #χ b1 (1P ) (bbar)     #assigned to other     
    100553:0, #Upsilon(2S)(bbar)     #assigned to other     
    100555:0,#chi_b2(2P) (bbar)      #assigned to other      
    200553:0,#Upsilon(3S)(bbar)      #assigned to other      
    120553:0,#chi_b1(2P)(bbar)       #assigned to other      
    551:0,   #eta_b(bbar) #assigned to other       
    553:0,   #Upsilon(bbar)         #assigned to other       
    555:0,   #chi_b0(bbar)#assigned to other       
    431:4,  #Ds+      
    421:4,  #D0       
    411:4,  #D+       
    441:0,   #etac    
    443:0,   #J/psi   
    445:0,   #chi_c0  
    4122:4,  #Lambda_c+         
    4132:4,  #Xi_c0   
    4114:4,  #Sigma_c0*         
    4232:4,  #Xi_c+   
    4332:4,  #Omega_c0
    10441:0,  #chi_c1       
    10443:0,  #h_c(1P)
    20443:0,  #chi_c2 
    100441:0, #chi_c0 
    100443:0, #psi(2S)
    30443:0,  #psi(3770)        
    5122:5,  #Lambda_b0         
    5132:5,  #Xi_b-   
    5232:5,  #Xi_b0   
    5332:5,  #Omega_b-
    # Doubly heavy baryons with cc
    4412:0,    #Xi_cc+ (ccd)    
    4422:0,    #Xi_cc++ (ccu)   
    4414:0,    #Xi_cc*+ [EvtGen decays it into weak channels, although the lifetime set to 0...]     
    4424:0,    #Xi_cc*++ [same] 
    4432:0,    #Omega_cc+ (ccs) 
    4434:0,   #Omega_cc*+ [also decays weakly]      
    4444:0,    #Omega_ccc+ [exists only in Pythia, not EvtGen]         
    4322:0,    #Omega_cc0bar (ccs)        
    # Doubly heavy baryons with b 
    # None ever observed, but technically can be produced by Pythia and decayed weakly (mass differences for * and ' states do not allow strong decays with pions) 
    5142:5,    #Xi_bc0 (bcd)    
    5242:5,    #Xi_bc+ (bcu)    
    5412:5,    #Xi'_bc0         
    5422:5,    #Xi'_bc+         
    5414:5,    #Xi*_bc0         
    5424:5,    #Xi*_bc+         
    5342:5,    #Omega_bc0 (bcs) 
    5432:5,   #Omega'_bc0       
    5434:5,    #Omega*_bc0
    # Triply heavy baryons with b
    # REALLY fat guys below, unlikely can ever by produced, but Pythia knows them and believes they all should decay weakly
    5442:5,    #Omega_bcc+      
    5444:5,    #Omeaga*_bcc+    
    5512:5,    #Xi_bb- (bbd)    
    5522:5,    #Xi_bb0 (bbu)    
    5514:5,    #Xi*_bb-         
    5524:5,    #Xi*_bb0
    5532:5,    #Omega_bb- (bbs) 
    5534:5,    #Omega*_bb-      
    5542:5,    #Omega_bbc0      
    5544:5,    #Omega*_bbc0     
    5554:5,    #Omega_bbb-      
    # Excited states, needed with HadronGhostInitialTruthLabelPdgId      
    20433:0,  #D s1 (2460) +    
    10431:0, #D s0 (2317) +     
    4334:0, #Omega_c0* (css)    
    #Charmed baryons    
    4324:4,  # Ξ c ∗+ 
    4314:4, #Ξ c ∗0   
    4312:4, #Ξ'c 0    
    4224:4, #Σ c ∗++  
    4222:4, #Σ c ++   
    4214:4, #Σ c ∗+   
    4212:4, #Σ c +    
    4112:4, #Σ c 0    
    433:4, #D s ∗+    
    425:4, #D 2 ∗ (2460) 0      
    423:4, #D ∗ (2007) 0        
    413:4, #D ∗ (2010) +        
    14122:0, #?? 
    10433:4,  #D_s1 (2536) +    
    10423:4, #D_1 (2420) 0      
    10421:4, #D 0 ∗ (2400) 0    
    10413:4, #D 1 (2420) +      
    10411:4, #D 0 (2400) +      
    4124:0, #??       
    20413:4, #D 1 (H) +         
    20423:4, #D 1 (2430) 0      
    9010443:0, # ψ(4160)        
    435:4, #D s2(2573) +        
    415:4, #D 2 ∗ (2460) +      
    5334:5, #Omega_b-* (ssb)    
    5324:5,  #Xi_b0* (usb)      
    5322:5, #Xi_b0' (usb)       
    5314:5, #Xi_b-* (dsb)       
    5312:5, #Xi_b'- (dsb)       
    5224:5, #sigma_b*+ (uub)    
    5222:5, #sigma_b+ (uub)     
    5214:5, #sigma_b0* (udb)    
    5212:5, #sigma_b0 (udb)     
    5114:5, #sigma_b*- (ddb)    
    5112:5, #sigma_b- (ddb)     
    533:-5, #B0_s* (s bbar) 
}
