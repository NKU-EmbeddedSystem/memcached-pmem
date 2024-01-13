# Hybrid-Memcached
## Compile
mkdir build && cd build
../configure
make -j

## Run
./memcached -u root -P /tmp/memcached.pid -t 1 -o pslab_file=/mnt/aep/pool,pslab_size=32768,pslab_policy=pmem,pslab_force,slab_sizes=64-96-128-192-256-384-512-1024-2048-4096-8192
