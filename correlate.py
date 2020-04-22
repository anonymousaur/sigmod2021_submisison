import numpy as np
import sys
import os
import corr_graph as cgraph
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import itertools

# Return the data (in string format) from the file
def extract(filename, cols, delim=','):
    data = np.loadtxt(filename, dtype=str, delimiter=delim, skiprows=1,
            usecols=cols)
    print("Finished loading data")
    return data

# Return the encoded data (ints) from the binary file
def extract_from_dump(filename, cols):
    data = np.load(filename)
    if cols is not None:
        return data[:,cols]
    return data


# Given raw string values in a column, map them to integers in the range [0, m]
def encode(col):
    uniq = np.unique(col)
    enc = {u:int(i) for (i, u) in enumerate(uniq)}
    mapped = np.zeros(len(col), dtype=int)
    for i, c in enumerate(col):
        mapped[i] = enc[c]
    return mapped

def dump_all(fname_in, fname_out):
    data = extract(fname_in, tuple(range(20)))
    encoded = np.zeros_like(data, dtype=int)
    for c in range(20):
        encoded[:,c] = encode(data[:,c])
    np.save(fname_out, encoded)

def plot(datax, datay, fname):
    plt.clf()
    plt.plot(datax, datay, '-.')
    plt.savefig(fname)

def load(fname, cols):
    fnames = fname.split(',')
    all_data = None
    for fn in fnames:
        data = None
        print("Loading", fn)
        if fname.endswith(".npy"):
            data = extract_from_dump(fn, cols)
        else:
            data = extract(fn, cols, delim='|')
        if all_data is None:
            all_data = np.copy(data)
        else:
            all_data = np.concatenate((data, all_data), axis=0)
    return all_data

def reencode_edges(edges, dims):
    edges0, edges1 = edges
    # Reassign IDs to nodes so that nearby nodes are correlated
    map1, map0 = {}, {}
    ix1, ix0 = 0, 0
    for n1, es in enumerate(edges1):
        if n1 not in map1:
            map1[n1] = ix1
            ix1 += 1
        for n0 in es.keys():
            if n0 not in map0:
                map0[n0] = ix0
                ix0 += 1
    for i in range(dims[0]):
        if i not in map0:
            map0[i] = ix0
            ix0 += 1
    for i in range(dims[1]):
        if i not in map1:
            map1[i] = ix1
            ix1 += 1
    overall = {}
    for n0, es in enumerate(edges0):
        d = {}
        for n1, w in es.items():
            d[map1[n1]] = w
        overall[map0[n0]] = d
    new_edges = []
    for i in range(len(overall)):
        new_edges.append(overall[i])
    return new_edges


def heatmap(fname, edge_map, dims):
    data = np.zeros(dims, dtype=int)
    for n0, es in enumerate(edge_map):
        for n1, w in es.items():
            data[n0][n1] += w
    data = np.log(data+1)
    plt.imshow(data, cmap='Reds', aspect='auto')
    plt.savefig(fname)

def corr(fname, pairs):
    encs = {}
    all_dims = set()
    for p in pairs:
        all_dims.add(p[0])
        all_dims.add(p[1])
    all_dims = list(all_dims)
    data = load(fname, all_dims)
    for i, d in enumerate(all_dims):
        encs[d]= encode(data[:,i])
    
    for p in pairs:
        enc1 = encs[p[0]]
        enc2 = encs[p[1]]
        senc1 = set(enc1)
        senc2 = set(enc2)
        dims = (len(senc1), len(senc2))
        print("Pair (%d, %d)" % (int(p[0]), int(p[1])))
        if len(senc2) > 10000:
            print('Col 1 has %d unique values: too many, skipping...' % \
                    len(senc2))
            continue
        print('Col 0 has %d unique values' % len(senc1))
        print('Col 1 has %d unique values' % len(senc2))
        cg = cgraph.CorrGraph(enc1, enc2)
        opt_edges0 = reencode_edges(cg.get_edges(), dims)
        heatmap("corr_heatmap_%d_%d.png" % (p[0], p[1]), opt_edges0, dims)
        data_x, data_y = cg.compute_tradeoff(strategy="pts")
        opt_edges0 = reencode_edges(cg.get_edges(), dims)
        heatmap("corr_heatmap_final_%d_%d.png" % (p[0], p[1]), opt_edges0, dims)

        print("Got %d data points" % len(data_x))
        minix = np.argmin(data_y)
        print("%d, %d: Minimum %f at %.02f%% outliers" % (p[0], p[1], data_y[minix], 100*data_x[minix]))
        np.savetxt("data/tradeoff_%d_%d_x.dat" % (p[0], p[1]), data_x)
        np.savetxt("data/tradeoff_%d_%d_y.dat" % (p[0], p[1]), data_y)
        plot(data_x, data_y, fname="plots/tradeoff_%d_%d.png" % (p[0], p[1]))
        print("===================")

if __name__ == "__main__":
    pairs = []
    if len(sys.argv) > 2:
        for pstr in sys.argv[2:]:
            cols = pstr.strip('()').split(',')
            col1 = int(cols[0])
            col2 = int(cols[1])
            pairs.append((col1, col2))
    else:
        eligible = [3,4,5,6,8]
        pairs = list(itertools.permutations(eligible, 2))
    print(pairs)
    corr(sys.argv[1], pairs)

