# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# @file PyUtils.scripts.list-events
# @purpose list event numbers in a given file
# @author Marcin Nowak
# @date April 2025

__doc__ = "Print out event numbers of the events in a file. Format: (run#, event#)"
__author__ = "Marcin Nowak"

### imports -------------------------------------------------------------------
import PyUtils.acmdlib as acmdlib

### options  -------------------------------------------------------------------
@acmdlib.command(name='list-events')
@acmdlib.argument('-f', '--file',
                  help='Athena file to scan')
@acmdlib.argument('-t', '--tree-name',
                  default='CollectionTree',
                  help='name of the TTree to scan')
@acmdlib.argument('--entries',
                  default='',
                  help='a list of entries (indices, not event numbers) or an expression (like 0:3) leading to such a list, to inspect')
@acmdlib.argument('-v', '--verbose',
                  action='store_true',
                  default=False,
                  help="""Enable verbose printout""")

### functions -----------------------------------------------------------------
def getEventsFromTree(tree, msg):

    eiNames = ['EventInfoAuxDyn.eventNumber',
               'EventInfoAux.',
               'Bkg_EventInfoAux.',
               'xAOD::EventAuxInfo_v3_EventInfoAux.',
               'xAOD::EventAuxInfo_v2_EventInfoAux.',
               'xAOD::EventAuxInfo_v1_EventInfoAux.',
               'xAOD::EventAuxInfo_v3_Bkg_EventInfoAux.',
               'xAOD::EventAuxInfo_v2_Bkg_EventInfoAux.',
               'xAOD::EventAuxInfo_v1_Bkg_EventInfoAux.',
               'McEventInfo',
               'ByteStreamEventInfo',
               'EventInfo_p4_McEventInfo',
               'EventInfo_p4_ByteStreamEventInfo']
    runName = 'EventInfoAuxDyn.runNumber'

    tree.GetEntry(0)
    einame = None
    for n in eiNames:
        if hasattr(tree, n):
            einame = n
            break
    if einame is None:
        msg.error('Cannot find event info, aborting.')
        return []
    msg.info("Using branch: %s", einame)
    
    tree.SetBranchStatus ('*', 0)
    tree.SetBranchStatus (einame, 1)
    if 'AuxDyn' in einame:
        tree.SetBranchStatus (runName, 1)

    eventList = []
    for idx in range(tree.GetEntriesFast()):
        tree.GetEntry(idx)
        if einame.endswith('Aux.'):
            ei = getattr(tree, einame)
            eventList.append((ei.runNumber, ei.eventNumber))
        elif einame.endswith('Info'):
            eid = getattr(tree, einame).m_event_ID
            eventList.append((eid.m_run_number, eid.m_event_number))
        elif 'AuxDyn' in einame:
            eventList.append(( getattr(tree, runName),  getattr(tree, einame)))
            
    tree.SetBranchStatus ('*', 1)

    # Write out the ordered index and event number pairs
    return eventList


def getEventList(file, tree_name="CollectionTree", entries='', verbose=False):
    """Get list of event+run numbers for given entries in a file/tree"""

    import PyUtils.Logging as L
    msg = L.logging.getLogger('list-events')
    if verbose:
        msg.setLevel(L.logging.VERBOSE)
    else:
        msg.setLevel(L.logging.WARNING)

    msg.info('file:    [%s]', file)
    msg.info('tree:    [%s]', tree_name)
    msg.info('entries: %s',   entries)
    
    def get_event_range(entry):
            smin, smax = 0, None
            # Parse user input
            if isinstance(entry, str):
                # We support three main cases in this format: 5:10 (5th to 10th),
                # 5: (5th to the end), and :5 (from the start to 5th)
                if ':' in entry:
                    vals = entry.split(':')
                    smin = int(vals[0]) if len(vals) > 0 and vals[0].isdigit() else 0
                    smax = int(vals[1]) if len(vals) > 1 and vals[1].isdigit() else None
                # This is the case where the user inputs the total number of events
                elif entry.isdigit():
                    smin = 0
                    smax = int(entry) if int(entry) > 0 else None
            # Handle the case where the input is a number (i.e. default)
            elif isinstance(entry, int):
                smin = 0
                smax = entry if entry > 0 else None
            # If we come across an unhandled case, bail out
            else:
                msg.warning(f"Unknown entries argument {entry}, will list all events...")
            return smin, smax

    import PyUtils.RootUtils as ru
    ru.import_root()  # noqa: F841
    try:
        dumper = ru.RootFileDumper(file, tree_name)
    except AttributeError as e:
        msg.error( *e.args )
        return []

    smin, smax = 0, None
    if entries in (-1,'','-1'):
        smax = dumper.tree.GetEntries()
    else:
        smin, smax = get_event_range(entries)
    msg.debug(f"Getting Event numbers for entries [{smin},{smax}]")
    
    return getEventsFromTree(dumper.tree, msg)[smin:smax]


def main(args):
    """Print event numbers of events in a file. Format: (run#, event#)"""

    eventList = getEventList(args.file, args.tree_name, args.entries, args.verbose)
    # print the output here to get the desired format (event per line)
    for ent in eventList:
        print(ent)
    # return nothing to avoid printing the output again
    return


# example of direct use
if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        print("no filename given")
        sys.exit(1)
    eventList = getEventList(sys.argv[1])
    for ent in eventList:
        print(ent)





