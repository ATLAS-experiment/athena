#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# write out a Digraph in  dot format

def dot(G, fn, nodesToHighlight=[]):
    """dot() is a function to output a graph which suppords adj() that writes
    out a .dot file for graph visualisation"""
    
    lines = [
        "digraph G{\n",
        "layout=twopi ranksep=3 ratio=auto\n"]

    for v in range(G.V):

        for w in G.adj(v): 
            line = '%d -> %d;' % (v, w)
            lines.append(line)

    for v in nodesToHighlight:
        assert v < G.V
        lines.append('%d  [color=red, style=filled];' % v)

    lines.append('}\n')

    text = '\n'.join(lines)

    with open(fn, 'w') as fh:
        fh.write(text)
        
