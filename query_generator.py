
VAL_RANGE = (1, 26)

qfile = "osm_queries.dat"
q = open(qfile, 'w')
for i in range(VAL_RANGE[0], VAL_RANGE[1]):
    q.write("===\n \n%d\n" % i)
q.close()
