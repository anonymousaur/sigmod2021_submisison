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

def load(fname, cols, delim=","):
    fnames = fname.split(',')
    all_data = None
    for fn in fnames:
        data = None
        print("Loading", fn)
        if fname.endswith(".npy"):
            data = extract_from_dump(fn, cols)
        else:
            data = extract(fn, cols, delim=delim)
        if all_data is None:
            all_data = np.copy(data)
        else:
            all_data = np.concatenate((data, all_data), axis=0)
    return all_data

def write_rewriter(fname, edges, mapped_dim, target_dim):
    # Write the edge graph as an input file for the rewriter.
    f = open(fname, 'w')
    f.write("%d %d\n" % (mapped_dim, target_dim))
    for em, et_list in edges.items():
        f.write("%d %d\n" % (em, len(et_list)))
        f.write("%s\n" % (' '.join(str(x) for x in et_list)))
    f.close()

def generate_outlier_list(fname, data, edges, mapped_dim, target_dim): 
    max_mapped_val = np.max(data[:, mapped_dim]) + 1
    print("Max mapped value: %d" % max_mapped_val)
    edge_codes = []
    # Since we only care about pairs, we do mapped_dim + target_dim * (1+max(mapped_dim))
    for em, et_list in edges.items():
        for et in et_list:
            edge_codes.append(em + et*max_mapped_val)
    coded = data[:, mapped_dim] + max_mapped_val * data[:, target_dim]
    outlier_ixs = np.where(np.logical_not(np.isin(coded, edge_codes)))
    outlier_ixs[0].astype(int).tofile(fname)

def corr(fname, pair, other_dims=None):
    encs = {}
    all_dims = set(other_dims)
    all_dims.add(pair[0])
    all_dims.add(pair[1])
    all_dims = list(all_dims)
    data = load(fname, all_dims, delim="|")
    encoded_data = np.zeros((len(data), len(all_dims)), dtype=int)
    dimension_map = {}
    for i, d in enumerate(all_dims):
        encoded_data[:,i]= encode(data[:,i])
        dimension_map[d] = i
    file1 = fname.split(',')[0]
    fbase = os.path.splitext(os.path.basename(file1))[0]
    encoded_data.astype(np.int32).tofile(fbase + ".enc")
    print("Encoded dataset written to:", fbase + ".enc")

    mapped_dim = dimension_map[pair[0]]
    target_dim = dimension_map[pair[1]]
    # Switched, bc the first argument to corr graph is the target dim
    enc1 = encoded_data[:, target_dim]
    enc2 = encoded_data[:, mapped_dim]
    senc1 = set(enc1)
    senc2 = set(enc2)
    dims = (len(senc1), len(senc2))
    print("Pair (%d, %d)" % (int(pair[0]), int(pair[1])))
    if len(senc2) > 10000:
        print('Col 1 has %d unique values: too many, skipping...' % \
                len(senc2))
        return
    print('Target dimension has %d unique values' % len(senc1))
    print('Mapped dimension has %d unique values' % len(senc2))
    cg = cgraph.CorrGraph(enc1, enc2)
    # The edges returned here map each value in the mapped dimension to a set of values in the
    # target dimension: dict: mapped_dim_value -> list of target_dim_values
    initial_edges = cg.clone_edge_graph()
    fname = fbase + "_initial.rewriter"
    print("Initial overhead = %f" % cg.scan_overhead(strategy="pts"))
    write_rewriter(fname, initial_edges, mapped_dim, target_dim)
    print("Wrote initial rewriter to", fname)
    final_edges, final_overhead, num_outliers = cg.minimize_overhead(strategy="pts")
    print("Final overhead = %f, Num outliers = %d" % (final_overhead, num_outliers))
    fname = fbase + "_final.rewriter"
    write_rewriter(fname, final_edges, mapped_dim, target_dim)
    print("Wrote final rewriter to", fname)
    fname = fbase + ".outliers"
    generate_outlier_list(fname, encoded_data, final_edges, mapped_dim, target_dim) 
    print("Wrote outlier list to", fname)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Requires <filename> <mapped dim, target dim>")
        sys.exit(1)
    cols = sys.argv[2].strip('()').split(',')
    col1 = int(cols[0])
    col2 = int(cols[1])
    pair = (col1, col2)
    other_dims = []
    if len(sys.argv) > 3:
        other_dims = [int(x) for x in sys.argv[3].split(',')]
    corr(sys.argv[1], pair, other_dims=other_dims)

