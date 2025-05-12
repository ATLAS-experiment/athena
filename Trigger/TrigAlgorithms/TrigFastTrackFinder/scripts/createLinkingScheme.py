#!/usr/bin/env python
import numpy as np
import pandas as pd

def parseArgs():
    import argparse
    parser = argparse.ArgumentParser(description="create GBTS linking table")
    parser.add_argument("-o", "--output_file", default="binTables_ITK_RUN4_new.txt", help="output table for use by GBTS (default=%(default)s)")
    parser.add_argument("-p", "--min-link-probability", type=float, default=0.01, help="minimum linking probability (default=%(default)s, LRT=0.001)")
    parser.add_argument("-a", "--no-symmetrize", action='store_true', help="do not symmetrize by adding missing layers on the other side")
    parser.add_argument("-i", "--write-inverse-linking-scheme", action='store_true', help="write inverse linking scheme table")
    parser.add_argument("-w", "--write-scheme-file", default="inverse_linking_scheme_ITK_RUN4.txt", help="inverse linking scheme table file name (default=%(default)s)")
    parser.add_argument("input_csv", nargs="?", default="layer_connection_table.csv", help="input CSV layer connection table, output from Athena (default=%(default)s)")
    return parser.parse_args()

class Layer(object):
    def __init__(self, id):
        self.id = id
        self.nTargets = 0


class Pair(object):
    def __init__(self, src, dst, iter):
        self.src = src
        self.dst = dst
        self.iter = iter

args = parseArgs()

link_df = pd.read_csv(args.input_csv)

link_df = link_df.drop(link_df[link_df['probability'] < args.min_link_probability].index)

layer_from_idx = link_df['from'].values
layer_to_idx = link_df['to'].values

layer_idx = np.unique(np.concatenate((layer_from_idx, layer_to_idx)))

# fill layer dictionary

layer_dict = {}

for id in layer_idx:
    layer_dict[id] = Layer(id)

dst_idx = link_df['from']

for idx in dst_idx:
    l = layer_dict[idx]
    l.nTargets += 1

src_dict = {k: v for k, v in layer_dict.items() if v.nTargets == 0}
dst_dict = {k: v for k, v in layer_dict.items() if v.nTargets > 0}

# Iteration 1
iter = 1

pair_dict = {}
dst_layer_dict = {}
src_layer_dict = {}

print('Creating stages...')

while (True):
    print('Iteration ', iter, 'N src', len(src_dict), 'N dst', len(dst_dict))

    pair_dict[iter] = []
    src_layer_dict[iter] = []
    for key, lay in src_dict.items():
        src_layer_dict[iter].append(key)
    dst_layer_dict[iter] = []

    for key, lay in src_dict.items():
        dst_df = link_df[link_df['to'] == key]
        dst_vec = dst_df['from']
        for d in dst_vec:
            dst_dict[d].nTargets -= 1
            pair_dict[iter].append(Pair(key, d, iter))
            dst_layer_dict[iter].append(d)

    src_dict = {k: v for k, v in dst_dict.items() if v.nTargets == 0}
    dst_dict = {k: v for k, v in dst_dict.items() if v.nTargets > 0}
    dst_layer_dict[iter] = list(set(dst_layer_dict[iter]))
    iter += 1
    if len(src_dict) == 0 or len(dst_dict) == 0:
        break

print('Last iteration ', iter, 'N src', len(src_dict), 'N dst', len(dst_dict))

# symmetrization
if not args.no_symmetrize:
    missing_links = []

    for stage, coll in pair_dict.items():
        for p1 in coll:
            if p1.src < 20000:
                continue
            if p1.dst < 20000:
                continue

            src_vol = p1.src // 10000
            dst_vol = p1.dst // 10000

            skip = src_vol == 8 and dst_vol == 8
            if skip:
                continue
            src_rem1 = p1.src % 10000
            dst_rem1 = p1.dst % 10000

            isPaired = False

            for p2 in coll:
                if p1 == p2:
                    continue
                src_rem2 = p2.src % 10000
                dst_rem2 = p2.dst % 10000
                if src_rem1 == src_rem2 and dst_rem1 == dst_rem2:
                    isPaired = True
                    break
            if isPaired:
                continue
            # generate and store missing link

            new_src = -1

            if src_vol == 8:
                new_src = p1.src
            elif src_vol == 7:
                new_src = 90000 + src_rem1
            elif src_vol == 9:
                new_src = 70000 + src_rem1

            new_dst = -1

            if dst_vol == 8:
                new_dst = p1.dst
            elif dst_vol == 7:
                new_dst = 90000 + dst_rem1
            elif dst_vol == 9:
                new_dst = 70000 + dst_rem1

            missing_links.append([stage, new_src, new_dst])

    for ml in missing_links:
        pair_dict[ml[0]].append(Pair(ml[1], ml[2], ml[0]))

nConnsTotal = 0

for i, coll in pair_dict.items():
    nConnsTotal += len(coll)

if args.write_inverse_linking_scheme:
    link_file = open(args.write_scheme_file, 'w')
    link_file.write('%d\n' % (len(pair_dict)))
    for i, coll in pair_dict.items():
        link_file.write('%d %d\n' % (i, len(coll)))
        for p in coll:
            link_file.write('%d %d \n' % (p.src, p.dst))

    link_file.close()

print('writing output binTables...')

bin_table = open(args.output_file, 'w')

bin_table.write('%d 0.2\n' % (nConnsTotal))

conn_counter = 0

for stage, coll in pair_dict.items():
    for p in coll:
        bin_table.write('%d %d %d %d 1 1 100\n' % (conn_counter, stage, p.src, p.dst))
        bin_table.write('100\n')
        conn_counter += 1

bin_table.close()

print('all done!')
