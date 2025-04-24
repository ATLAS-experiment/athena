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
def getEventList(tree, msg, reverse_order = False):

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
            ei = getattr(tree, einame)
            eventList.append((ei.event_ID().run_number(), ei.event_ID().event_number()))
        elif 'AuxDyn' in einame:
            eventList.append(( getattr(tree, runName),  getattr(tree, einame)))
            
    tree.SetBranchStatus ('*', 1)

    # Write out the ordered index and event number pairs
    return eventList


def main(args):
    """Print event numbers of events in a file. Format: (run#, event#)"""

    import PyUtils.Logging as L
    msg = L.logging.getLogger('list-events')
    if args.verbose:
        msg.setLevel(L.logging.VERBOSE)
    else:
        msg.setLevel(L.logging.WARNING)

    if args.entries == '':
        args.entries = -1
    if args.tree_name == '':
        args.tree_name = "CollectionTree"
        
    msg.info('file:    [%s]', args.file)
    msg.info('tree:    [%s]', args.tree_name)
    msg.info('entries: %s',   args.entries)
    
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
            msg.debug(f"Event slice is parsed as [{smin},{smax}]")
            return smin, smax

    import PyUtils.RootUtils as ru
    ru.import_root()  # noqa: F841
    dumper = ru.RootFileDumper(args.file, args.tree_name)
    if args.entries in (-1,'','-1'):
        entries = dumper.tree.GetEntries()
    else:
        entries = args.entries
    smin, smax = get_event_range(entries)
    
    eventList = getEventList(dumper.tree, msg)[smin:smax]
    # print the output here to get the desired format (event per line)
    for ent in eventList:
        print(ent)
    # return nothing to avoid printing the output again
    return


# example of direct use
if __name__ == "__main__":
    import types, sys
    args=types.SimpleNamespace()
    if len(sys.argv) < 2:
        print("no filename given")
        sys.exit(1)
    args.file = sys.argv[1]
    args.entries = ''
    args.tree_name = ''
    args.verbose = False
    main(args)





