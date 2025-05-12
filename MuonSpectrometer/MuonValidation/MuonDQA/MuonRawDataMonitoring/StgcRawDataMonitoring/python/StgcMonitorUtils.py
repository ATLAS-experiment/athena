#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

def padTriggerOccupancyLabels():
    columnLabels_AL, columnLabels_CL = ['']*71, ['']*71
    columnLabels_AS, columnLabels_CS = ['']*71, ['']*71
    rowLabels = ['']*56
    counterColumnLabels, counterRowLabels = 1, 1
    
    for columnLabelIt in range(0, 71):
        if (columnLabelIt/3 - 1) % 3 == 0:
            columnLabels_AL[columnLabelIt] = 'A' + str(2*counterColumnLabels - 1).zfill(2)
            columnLabels_CL[columnLabelIt] = 'C' + str(2*counterColumnLabels - 1).zfill(2)
            columnLabels_AS[columnLabelIt - 2] = 'A' + str(2*counterColumnLabels).zfill(2)
            columnLabels_CS[columnLabelIt - 2] = 'C' + str(2*counterColumnLabels).zfill(2)
            counterColumnLabels += 1
        
    for rowLabelIt in range(0, 56):
        if (rowLabelIt/5 - 1) % 4 == 0:
            rowLabels[rowLabelIt] = 'Q' + str(counterRowLabels)
            counterRowLabels += 1
                    
    return columnLabels_AL, columnLabels_CL, columnLabels_AS, columnLabels_CS, rowLabels

def wireOccupancyLabels():
    wireGroupNumberLabel = ['']*929
    counterGroupNumberLabel = 1
    
    for wireGroupNumberIt in range(0, 929):
        if (wireGroupNumberIt/29 + 3) % 4 == 0:
            wireGroupNumberLabel[wireGroupNumberIt] = 'S' + str(counterGroupNumberLabel).zfill(2)
            counterGroupNumberLabel += 1
        elif (wireGroupNumberIt - 76) % 116 == 0:
            wireGroupNumberLabel[wireGroupNumberIt] = 'S' + str(counterGroupNumberLabel).zfill(2)
            counterGroupNumberLabel += 1

    return wireGroupNumberLabel

def FEBLabels():
    labels = [ f"Q{int(feb/8)+1}/L{feb%8+1}" for feb in range(0, 24) ]
    return labels

columnLabels_AL = padTriggerOccupancyLabels()[0]
columnLabels_CL = padTriggerOccupancyLabels()[1]
columnLabels_AS = padTriggerOccupancyLabels()[2]
columnLabels_CS = padTriggerOccupancyLabels()[3]
rowLabels = padTriggerOccupancyLabels()[4]
wireGroupNumberLabel = wireOccupancyLabels()
FebLabels = FEBLabels()
