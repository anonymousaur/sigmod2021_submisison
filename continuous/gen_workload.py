import numpy as np
import argparse

# Generates queries whose distribution matches that of the points.


class QueryGen(object):
    def __init__(self, dataset, dtype="int"):
        self.data = dataset
        self.dtype = dtype
        self.dim = self.data.shape[1]
    
    def get_range(self, pt, dims, widths):
        ranges = {}
        for d, w in zip(dims, widths):
            ranges[d] = tuple(np.array([pt[d]-w/2, pt[d]+w/2]).astype(self.dtype))
        return ranges

    def query_str(self, ranges):
        s = "==========\n"
        for d in range(self.dim):
            if d in ranges:
                s += "ranges %s %s\n" % (str(ranges[d][0]), str(ranges[d][1]))
            else:
                s += "none\n"
        return s

    def gen_proportional(self, dims, widths, n, outfile):
        print("Generating proportional queries...")
        ixs = np.random.randint(len(self.data), size=n)
        pts = self.data[ixs,:]
        f = open(outfile, 'w')
        for i in range(n):
            r = self.get_range(pts[i], dims, widths)
            f.write(self.query_str(r))
        f.close()

    def gen_uniform(self, dims, widths, n, outfile):
        print("Generating uniform queries...")
        assert len(dims) == 1
        mapped_dim = dims[0]
        w = widths[0]
        vals = self.data[:, mapped_dim]
        buckets = np.unique(np.floor((vals - vals.min())/(w/2)))
        # First, select a bucket uniformly at random. Then select a starting point within that
        # bucket uniformly at random.
        b = np.random.choice(buckets, size=n, replace=True)
        p = np.random.uniform(0, w/2, size=n)
        pts = np.zeros((n, self.dim))
        pts[:, mapped_dim] = b*w/2 + vals.min() + p
        f = open(outfile, 'w')
        for i in range(n):
            r = self.get_range(pts[i], dims, widths)
            f.write(self.query_str(r))
        f.close()

def gen_from_spec(args, distribution, nq, output):
    dataset = np.fromfile(args["datafile"], dtype=int).reshape(-1, args["ncols"])
    cols = [args["map_dims"][0]] + [td[0] for td in args["target_dims"]]
    dataset = dataset[:,cols]
    qg = QueryGen(dataset)
    filter_dims = [0]
    filter_widths = [args["map_dims"][1]] if distribution != "point" else [2]
    if distribution == "uniform":
        qg.gen_uniform(filter_dims, filter_widths, nq, output)
    else:
        qg.gen_proportional(filter_dims, filter_widths, nq, output)

if __name__ == "__main__":
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
    parser.add_argument("--distribution",
            type=str,
            choices=["uniform", "proportional"],
            required=True,
            help="Distribution of queries. If proportional, will generate queries proportional to " + \
                "the distribution of points along the mapped dimension. Otherwise, queries will be " + \
                "uniform along the mapped dimension wherever there are points.")
    parser.add_argument("--output",
            type=str,
            required=True,
            help="File to write queries to")
    
    args = parser.parse_args()
    dataset = np.fromfile(args.dataset, dtype=args.dtype).reshape(-1, args.dim)
    qg = QueryGen(dataset)
    assert (len(args.filter_dims) > 0)
    assert (len(args.filter_dims) == len(args.filter_widths))
    
    if args.distribution == "proportional":
        qg.gen_proportional(args.filter_dims, args.filter_widths, args.nqueries, args.output)
    elif args.distribution == "uniform":
        qg.gen_uniform(args.filter_dims, args.filter_widths, args.nqueries, args.output)





