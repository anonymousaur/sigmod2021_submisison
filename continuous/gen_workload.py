import numpy as np
import argparse

# Generates queries whose distribution matches that of the points.

parser = argparse.ArgumentParser("QueryGen")
parser.add_argument("--dataset",
        type=str,
        required=True,
        help="Pointer to the dataset to generate relevant queries")
parser.add_argument("--dim",
        type=int,
        required=True,
        help="Dimension of the dataset")
parser.add_argument("--dtype",
        type=str,
        required=True,
        help="dtype of the dataset")
parser.add_argument("--nqueries",
        type=int,
        required=True,
        help="Number of queries to generate")
parser.add_argument("--filter-dims",
        type=int,
        nargs="+",
        required=True,
        help="Dimensions to generate queries for")
parser.add_argument("--filter-widths",
        type=float,
        nargs="+",
        required=True,
        help="Widths along each dimension (len(--type) == len(--width))." + \
                "If negative, generates widths uniformly")
parser.add_argument("--output",
        type=str,
        required=True,
        help="File to write queries to")

args = parser.parse_args()

class QueryGen(object):
    def __init__(self, dataset):
        self.data = dataset
        self.dim = self.data.shape[1]
    
    def get_range(self, pt, dims, widths):
        ranges = {}
        for d, w in zip(dims, widths):
            ranges[d] = (pt[d]-w/2, pt[d]+w/2)
        return ranges

    def query_str(self, ranges):
        s = "==========\n"
        for d in range(self.dim):
            if d in ranges:
                s += "ranges %f %f\n" % (ranges[d][0], ranges[d][1])
            else:
                s += "none\n"
        return s

    def gen(self, dims, widths, n, outfile):
        ixs = np.random.randint(len(self.data), size=n)
        pts = self.data[ixs,:]
        f = open(outfile, 'w')
        for i in range(n):
            r = self.get_range(pts[i], dims, widths)
            f.write(self.query_str(r))
        f.close()

dataset = np.fromfile(args.dataset, dtype=args.dtype).reshape(-1, args.dim)
qg = QueryGen(dataset)
assert (len(args.filter_dims) > 0)
assert (len(args.filter_dims) == len(args.filter_widths))

qg.gen(args.filter_dims, args.filter_widths, args.nqueries, args.output)






