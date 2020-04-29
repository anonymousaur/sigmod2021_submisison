fname = "dmv_initial.rewriter"

valset = []
for i, line in enumerate(open(fname)):
    if i == 0 or i % 2 == 1:
        continue
    vals = [int(x) for x in line.split()]
    valset.extend(vals)

print("Unique values: %d" % len(set(valset)))
