# bridge_c
Need the following:
ethtool -K eth0 tx off rx off gso off tso off gro off lro off

## Profiling support (gprof)

Compile with: `make -j4 PROFILE=1`

When ready, hit Ctrl+C to exit, then: `gprof bridge | less`
