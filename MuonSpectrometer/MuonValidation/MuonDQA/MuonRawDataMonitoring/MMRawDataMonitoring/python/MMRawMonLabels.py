#                                                           
#Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

#x-y labels for occupancy plots

label_empty = ['']

x_lab_occ = ['LAYER 1']+label_empty*15+['LAYER 2']+label_empty*15+['LAYER 3']+label_empty*15+['LAYER 4']+label_empty*15+['LAYER 5']+label_empty*15+['LAYER 6']+label_empty*15+['LAYER 7']+label_empty*15+['LAYER 8']+label_empty*15

x_lab_occ_etaminus1=['ml1 gp1 PCB 1','ml1 gp1 PCB 2','ml1 gp1 PCB 3','ml1 gp1 PCB 4','ml1 gp1 PCB 5', 'ml1 gp2 PCB 1','ml1 gp2 PCB 2','ml1 gp2 PCB 3','ml1 gp2 PCB 4','ml1 gp2 PCB5', 'ml1 gp3 PCB 1','ml1 gp3 PCB 2','ml1 gp3 PCB 3','ml1 gp3 PCB 4', 'ml1 gp3 PCB 5', 'ml1 gp4 PCB 1','ml1 gp4 PCB 2','ml1 gp4 PCB 3','ml1 gp4 PCB 4','ml1 gp4 PCB 5','ml2 gp1 PCB 1','ml2 gp1 PCB 2','ml2 gp1 PCB 3','ml2 gp1 PCB 4','ml2 gp1 PCB 5','ml2 gp2 PCB 1','ml2 gp2 PCB 2','ml2 gp2 PCB 3','ml2 gp2 PCB 4','ml2 gp2 PCB 5','ml2 gp3 PCB 1','ml2 gp3 PCB 2','ml2 gp3 PCB 3','ml2 gp3 PCB 4','ml2 gp3 PCB 5','ml2 gp4 PCB 1','ml2 gp4 PCB 2','ml2 gp4 PCB 3','ml2 gp4 PCB 4','ml2 gp4 PCB 5']

x_lab_occ_etaminus2=['ml1 gp1 PCB 1','ml1 gp1 PCB 2','ml1 gp1 PCB 3', 'ml1 gp2 PCB 1','ml1 gp2 PCB 2','ml1 gp2 PCB 3', 'ml1 gp3 PCB 1','ml1 gp3 PCB 2','ml1 gp3 PCB 3', 'ml1 gp4 PCB 1','ml1 gp4 PCB 2','ml1 gp4 PCB 3','ml2 gp1 PCB 1','ml2 gp1 PCB 2','ml2 gp1 PCB 3','ml2 gp2 PCB 1','ml2 gp2 PCB 2','ml2 gp2 PCB 3','ml2 gp3 PCB 1','ml2 gp3 PCB 2','ml2 gp3 PCB 3','ml2 gp4 PCB 1','ml2 gp4 PCB 2','ml2 gp4 PCB 3']

x_lab_occ_ASide=['#eta1 ml1 gp1','#eta1 ml1 gp2', '#eta1 ml1 gp3', '#eta1 ml1 gp4', '#eta1 ml2 gp1','#eta1 ml2 gp2','#eta1 ml2 gp3','#eta1 ml2 gp4','#eta2 ml1 gp1','#eta2 ml1 gp2','#eta2 ml1 gp3','#eta2 ml1 gp4','#eta2 ml2 gp1','#eta2 ml2 gp2','#eta2 ml2 gp3','#eta2 ml2 gp4']

y_lab_occ_ASide=['#phi1 MMS','#phi1 MML','#phi2 MMS','#phi2 MML','#phi3 MMS','#phi3 MML','#phi4 MMS','#phi4 MML','#phi5 MMS','#phi5 MML','#phi6 MMS','#phi6 MML','#phi7 MMS','#phi7 MML','#phi8 MMS','#phi8 MML']

x_lab_occ_CSide=['#eta-2 ml1 gp1', '#eta-2 ml1 gp2', '#eta-2 ml1 gp3', '#eta-2 ml1 gp4','#eta-2 ml2 gp1','#eta-2 ml2gp2','#eta-2 ml2 gp3','#eta-2 ml2gp4','#eta-1 ml1 gp1','#eta-1 ml1 gp2','#eta-1 ml1 gp3', '#eta-1 ml1 gp4','#eta-1 ml2 gp1','#eta-1 ml2 gp2','#eta-1 ml2 gp3','#eta-1 ml2 gp4']

y_lab_occ_CSide=['#phi1 MMS','#phi1 MML','#phi2 MMS','#phi2 MML','#phi3 MMS','#phi3 MML','#phi4 MMS','#phi4 MML','#phi5 MMS','#phi5 MML','#phi6 MMS','#phi6 MML','#phi7 MMS','#phi7 MML','#phi8 MMS','#phi8 MML']


y_lab_occ_lb=['ml1 gap1']+label_empty*7+['ml1 gap2']+label_empty*7+['ml1 gap3']+label_empty*7+['ml1 gap4']+label_empty*7+['ml2 gap1']+label_empty*7+['ml2 gap2']+label_empty*7+['ml2 gap3']+label_empty*7+['ml2 gap4']+label_empty*7

x_lab_mpv = ['ml1 gp1', 'ml1 gp2', 'ml1 gp3', 'ml1 gp4', 'ml2 gp1', 'ml2 gp2', 'ml2 gp3', 'ml2 gp4']

x_layer_in_sector = ['Sector0']+label_empty*7+['Sector1']+label_empty*7+['Sector2']+label_empty*7+['Sector3']+label_empty*7+['Sector4']+label_empty*7+['Sector5']+label_empty*7+['Sector6']+label_empty*7+['Sector7']+label_empty*7+['Sector8']+label_empty*7+['Sector9']+label_empty*7+['Sector10']+label_empty*7+['Sector11']+label_empty*7+['Sector12']+label_empty*7+['Sector13']+label_empty*7+['Sector14']+label_empty*7+['Sector15']+label_empty*7

x_layer_in_sector_tri = [
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C16 layer1',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C15 layer1',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C14 layer1',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C13 layer1',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C12 layer1',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C11 layer1',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C10 layer1',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C9 layer1',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C8 layer1',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C7 layer1',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C6 layer1',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C5 layer1',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C4 layer1',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C3 layer1',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C2 layer1',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', 'C1 layer1 ',
    '  ',	
    ' A1 layer1', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    'A2 layer1', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    'A3 layer1', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    'A4 layer1', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    'A5 layer1', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    'A6 layer1', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    'A7 layer1', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    'A8 layer1', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    'A9 layer1', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    'A10 layer1', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    'A11 layer1', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    'A12 layer1', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    'A13 layer1', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    'A14 layer1', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    'A15 layer1', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    'A16 layer1', ' ', ' ', ' ', ' ', ' ', ' ', ' '
]
