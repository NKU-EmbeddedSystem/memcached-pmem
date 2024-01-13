Copyright and License
-----------

Copyright 2018 Lenovo

Licensed under the BSD-3 license. see LICENSE.Lenovo.txt for full text

Description
-----------

This project provides a sample of persistent memory support in memcached. It enables memcached to use persistent memory for data caching. For more information on memcached and persistent memory, visit http://www.memcached.org and http://pmem.io

Installing
-----------

* To install memcached with persistent memory support:

  1. Clone the project

  2. 'cd' to the directory where the project is cloned and invoke the 'configure' scripts:

.. code-block:: console

       ./configure --enable-pslab

  3. Invoke the 'make' program:

.. code-block:: console

       make

  4. If 'make' succeeds, you can install the program:

.. code-block:: console

       make install

Requirements
-----------

Besides the libraries depended on by memcached, persistent memory enhancement needs the Persistent Memory Development Kit (PMDK) to build. PMDK is available from https://github.com/pmem/pmdk


Usage
-----------

Start memcached with persistent memory
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Assuming that the persistent memory device is '/dev/pmem0', format it with ext4 file system type:

.. code-block:: console

   mkfs -t ext4 /dev/pmem0

Then mount it in dax mode:

.. code-block:: console

   mount -o dax /dev/pmem0 /mnt/pmfs

Finnaly start memcached:

.. code-block:: console

   memcached -d -m 500 -u root -l 192.168.50.10 -p 12000 -c 256 -P /tmp/memcached.pid -vvv

   memcached -u root -o pslab_file=/mnt/pmfs/pool,pslab_force

   memcached -u root -P /tmp/memcached.pid -o pslab_file=/mnt/aep/pool,pslab_force

To use persistent memory caching all the data:

-m: memory limit
.. code-block:: console

   -m 0 代表不分配 DRAM 资源
   如果不存在 -m 0 而是, -m 0 和 pslab共存的话, 默认 slab 分配的 policy 是“先 DRAM 后 PMEM”
   可以通过 pslab_policy 控制 slab 分配

   memcached -u root -m 0 -o pslab_file=/mnt/pmfs/pool,pslab_force

   memcached -u root -P /tmp/memcached.pid -m 0 -o pslab_file=/mnt/aep/pool,pslab_force

   memcached -u root -P /tmp/memcached.pid -m 0 -o pslab_file=/mnt/aep/pool,pslab_size=512,pslab_force

Restart memcached with data recovery
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
With persistent memory support enabled, memcached can recover the data stored in persistent memory back from abrupt termination caused by system panic or application crash.

.. code-block:: console

   memcached -u root -o pslab_file=/mnt/pmfs/pool,pslab_force,pslab_recover

   memcached -u root -o pslab_file=/mnt/aep/pool,pslab_force,pslab_recover

lenevo-pmem-memcached

DRAM test:

   ./memcached -u root -P /tmp/memcached.pid -m 4096 -t 16

   memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_dram_result -x 3 -t 16 --ratio=9:1 --test-time=120 --data-size-range=3000-4000 --data-size-pattern=R --key-prefix=memtier- --key-minimum=1 --key-maximum=100000 --key-pattern=R:R

PMEM test:

   ./memcached -u root -P /tmp/memcached.pid -m 0 -t 16 -o pslab_file=/mnt/aep/pool,pslab_size=4096,pslab_force

   memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_pmem_result -x 3 -t 16 --ratio=9:1 --test-time=120 --data-size-range=3000-4000 --data-size-pattern=R --key-prefix=memtier- --key-minimum=1 --key-maximum=100000 --key-pattern=R:R

Single Thread:
   ./memcached -u root -P /tmp/memcached.pid -t 1 -o pslab_file=/mnt/aep/pool,pslab_size=4096,pslab_policy=pmem,pslab_force
   memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_pmem_1t_result -x 3 -t 16 --ratio=9:1 --test-time=120 --data-size-range=3000-4000 --data-size-pattern=R --key-prefix=memtier- --key-minimum=1 --key-maximum=100000 --key-pattern=R:R
   memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_pmem_1t_result -t 1 --ratio=9:1 --test-time=120 --data-size-range=1-4000 --data-size-pattern=R --key-prefix=memtier- --key-minimum=1 --key-maximum=10000 --key-pattern=R:R
   memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_pmem_1t_result -x 1 -t 1 --ratio=9:1 --test-time=120 --data-size-range=1-400000 --data-size-pattern=R --key-prefix=memtier- --key-minimum=1 --key-maximum=100000 --key-pattern=R:R
   memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_pmem_1t_result -x 1 -t 1 --ratio=9:1 --test-time=120 --data-size-range=1-40000 --data-size-pattern=R --key-prefix=memtier- --key-minimum=1 --key-maximum=100000 --key-pattern=R:R
   memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_pmem_1_result -x 3 -t 1 --ratio=9:1 --test-time=120 --data-size=200 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999
   
   memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_pmem_1_result -x 3 -t 1 --ratio=9:1 --test-time=120 --data-size=262000 --key-prefix=memtier- --key-minimum=32 --key-maximum=32
   
   ./memcached -u root -P /tmp/memcached.pid -t 1 -m 4096
   memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_dram_1t_result -x 3 -t 16 --ratio=9:1 --test-time=120 --data-size-range=3000-4000 --data-size-pattern=R --key-prefix=memtier- --key-minimum=32 --key-maximum=32 --key-pattern=R:R



   memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_256b_1_result -x 3 -t 1 --ratio=9:1 --test-time=120 --data-size=200 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999


   ./memcached -u root -o pslab_file=/mnt/aep/pool,pslab_force,pslab_recover
memcached-1.6.18

   ./configure --enable-pslab / --enable-extstore
   ./memcached -u root -d -P /tmp/memcached.pid -m 0 -t 16 -o pslab_file=/mnt/aep/pool,pslab_size=4096,pslab_force
   ./memcached -u root -o pslab_file=/mnt/aep/pool,pslab_force,pslab_recover

   ./memcached -u root -m 0 -t 16 -o pslab_file=/mnt/aep/pool,pslab_size=4096,pslab_force
   ./memcached -u root -o pslab_file=/mnt/aep/pool,pslab_force,pslab_recover


Memcachedb:
   ./configure --enable-threads
   make
   make install

   memcachedb -N -p 11211 -u root -P /tmp/memcachedb.pid -t 16 -H /home/lxdd/memcachedb/memcachedb_output   

Memcached Restartable mode
   ./memcached -u root -P /tmp/memcached.pid -m 4096 -t 16 -e /mnt/aep/pool

   kill -s SIGUSR1 pid

Memcached Extstore mode
   ./configure --enable-extstore
   ./memcached -u root -P /tmp/memcached.pid -m 4096 -t 16 -o ext_path=/home/lxdd/memcached-1.6.18/:4G

   memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_newversion_dram_result -x 3 -t 16 --ratio=9:1 --test-time=120 --data-size-range=3000-4000 --data-size-pattern=R --key-prefix=memtier- --key-minimum=1 --key-maximum=100000 --key-pattern=R:R


压测: 
numactl -C 32-64 ./memcached -u root -P /tmp/memcached.pid -t 1 -o pslab_file=/mnt/aep/pool,pslab_size=32768,pslab_policy=pmem,pslab_force,slab_sizes=64-96-128-192-256-384-512-1024-2048-4096-8192
包含benchmark_cycle版本的memcached运行:
numactl -C 32-64 ./memcached -u root -P /tmp/memcached.pid -t 16 -o pslab_file=/mnt/aep/pool,pslab_size=32768,pslab_policy=pmem,pslab_force,slab_sizes=64-96-128-192-256-384-512-1024-2048-4096-8192,benchmark_cycles=10
注意 memtier 驱动的客户端连接数, 和 key 值访问模式, 多个客户端连接时, 实际产生的不同 key 值数量排序为: P:P > R:R > S:S
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_1t_16t_default_datasize_18_result -t 16 --ratio=999:1 --test-time=120 --data-size=18 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P



.. numactl -C 2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94 ./memcached -u root -P /tmp/memcached.pid -t 1 -o pslab_file=/mnt/aep/pool,pslab_size=65536,pslab_policy=pmem,pslab_force,slab_sizes=64-96-112-128-144-160-176-192-208-224-240-256-384-512-1024-2048-4096-8192
numactl -C 1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,77,79,81,83,85,87,89,91,93,95 ./memcached -u root -P /tmp/memcached.pid -t 1 -o pslab_file=/mnt/aep/pool,pslab_size=65536,pslab_policy=pmem,pslab_force,slab_sizes=64-96-112-128-144-160-176-192-208-224-240-256-384-512-1024-2048-4096-8192
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_1t_16t_default_datasize_18_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=18 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_1t_16t_default_datasize_50_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=50 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_1t_16t_default_datasize_114_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=114 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_1t_16t_default_datasize_178_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=178 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_1t_16t_default_datasize_306_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=306 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_1t_16t_default_datasize_434_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=434 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_1t_16t_default_datasize_946_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=946 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_1t_16t_default_datasize_1970_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=1970 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_1t_16t_default_datasize_4018_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=4018 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P



numactl -C 1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,77,79,81,83,85,87,89,91,93,95 ./memcached -u root -P /tmp/memcached.pid -t 2 -o pslab_file=/mnt/aep/pool,pslab_size=65536,pslab_policy=pmem,pslab_force,slab_sizes=64-96-112-128-144-160-176-192-208-224-240-256-384-512-1024-2048-4096-8192
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_2t_16t_default_datasize_18_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=18 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_2t_16t_default_datasize_50_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=50 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_2t_16t_default_datasize_114_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=114 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_2t_16t_default_datasize_178_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=178 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_2t_16t_default_datasize_306_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=40 --data-size=306 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_2t_16t_default_datasize_434_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=434 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_2t_16t_default_datasize_946_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=946 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_2t_16t_default_datasize_1970_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=40 --data-size=1970 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_2t_16t_default_datasize_4018_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=40 --data-size=4018 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P


numactl -C 1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,77,79,81,83,85,87,89,91,93,95 ./memcached -u root -P /tmp/memcached.pid -t 4 -o pslab_file=/mnt/aep/pool,pslab_size=65536,pslab_policy=pmem,pslab_force,slab_sizes=64-96-112-128-144-160-176-192-208-224-240-256-384-512-1024-2048-4096-8192
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_4t_16t_default_datasize_18_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=18 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_4t_16t_default_datasize_50_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=50 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_4t_16t_default_datasize_114_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=114 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_4t_16t_default_datasize_178_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=178 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_4t_16t_default_datasize_306_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=306 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_4t_16t_default_datasize_434_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=434 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_4t_16t_default_datasize_946_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=946 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_4t_16t_default_datasize_1970_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=1970 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_4t_16t_default_datasize_4018_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=4018 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P

numactl -C 1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,77,79,81,83,85,87,89,91,93,95 ./memcached -u root -P /tmp/memcached.pid -t 6 -o pslab_file=/mnt/aep/pool,pslab_size=65536,pslab_policy=pmem,pslab_force,slab_sizes=64-96-112-128-144-160-176-192-208-224-240-256-384-512-1024-2048-4096-8192
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_6t_16t_default_datasize_18_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=18 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_6t_16t_default_datasize_50_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=50 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_6t_16t_default_datasize_114_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=114 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_6t_16t_default_datasize_178_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=178 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_6t_16t_default_datasize_306_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=306 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_6t_16t_default_datasize_434_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=434 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_6t_16t_default_datasize_946_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=946 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_6t_16t_default_datasize_1970_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=1970 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_6t_16t_default_datasize_4018_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=4018 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P

numactl -C 1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,77,79,81,83,85,87,89,91,93,95 ./memcached -u root -P /tmp/memcached.pid -t 8 -o pslab_file=/mnt/aep/pool,pslab_size=65536,pslab_policy=pmem,pslab_force,slab_sizes=64-96-112-128-144-160-176-192-208-224-240-256-384-512-1024-2048-4096-8192
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_18_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=18 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_50_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=50 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_114_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=114 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P

memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_130_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=130 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_146_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=146 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_162_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=162 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P


memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_178_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=178 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_306_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=306 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_434_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=434 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_946_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=946 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_1970_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=1970 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_4018_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=4018 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
./memcached -u root -P /tmp/memcached.pid -t 8 -o pslab_file=/mnt/aep/pool,pslab_size=65536,pslab_policy=pmem,pslab_force,slab_sizes=64-96-112-128-144-160-176-192-208-224-240-256-384-512-1024-2048-4096-8192
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_50_dram_pmem_pair_result -t 16 -x 3 --ratio=999:1 --test-time=120 --data-size=50 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_114_dram_pmem_pair_result -t 16 -x 3 --ratio=999:1 --test-time=120 --data-size=114 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P

memtier_benchmark -s localhost -p 11211 -P memcache_text --pipeline=8 --out-file=memtier_8t_16t_default_datasize_178_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=178 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --pipeline=8 --out-file=memtier_8t_16t_default_datasize_946_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=946 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P


memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=data_size_range -t 16 --ratio=1:1 --test-time=120 --data-size-range=1-256 --data-size-pattern=S --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P

## YCSB 测试 ## 键值对1KB, 需要较多PMEM
numactl -C 1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,77,79,81,83,85,87,89,91,93,95 ./memcached -u root -P /tmp/memcached.pid -t 8 -o pslab_file=/mnt/aep/pool,pslab_size=131072,pslab_policy=pmem,pslab_force,slab_sizes=64-96-112-128-144-160-176-192-208-224-240-256-384-512-1024-2048-4096-8192
./bin/ycsb.sh load memcached -s -P workloads/workload_wh -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh run memcached -s -P workloads/workload_wh -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh load memcached -s -P workloads/workload_rw -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh run memcached -s -P workloads/workload_rw -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh load memcached -s -P workloads/workload_ro -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh run memcached -s -P workloads/workload_ro -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100

./bin/ycsb.sh load memcached -s -P workloads/workload_uh -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -p memcached.protocol=text -threads 100
./bin/ycsb.sh run memcached -s -P workloads/workload_uh -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -p memcached.protocol=text -threads 100
./bin/ycsb.sh load memcached -s -P workloads/workload_ru -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -p memcached.protocol=text -threads 100
./bin/ycsb.sh run memcached -s -P workloads/workload_ru -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -p memcached.protocol=text -threads 100

./bin/ycsb.sh load memcached -s -P workloads/workloada -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh run memcached -s -P workloads/workloada -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh load memcached -s -P workloads/workloadb -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh run memcached -s -P workloads/workloadb -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh load memcached -s -P workloads/workloadc -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh run memcached -s -P workloads/workloadc -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh load memcached -s -P workloads/workloadd -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh run memcached -s -P workloads/workloadd -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh load memcached -s -P workloads/workloade -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh run memcached -s -P workloads/workloade -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh load memcached -s -P workloads/workloadf -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh run memcached -s -P workloads/workloadf -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh load memcached -s -P workloads/workload_wh -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100
./bin/ycsb.sh run memcached -s -P workloads/workload_wh -p memcached.hosts=127.0.0.1 -p memcached.port=11211 -threads 100

memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_146_dram_pmem_pair_result -t 16 -c 16 --ratio=999:1 --test-time=120 --data-size=130 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=3333333333 --key-pattern=P:P &
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_146_dram_pmem_pair_result -t 16 -c 16 --ratio=999:1 --test-time=120 --data-size=146 --key-prefix=memtier- --key-minimum=3333333334 --key-maximum=6666666666 --key-pattern=P:P &
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_162_dram_pmem_pair_result -t 16 -c 16 --ratio=999:1 --test-time=120 --data-size=162 --key-prefix=memtier- --key-minimum=6666666667 --key-maximum=9999999999 --key-pattern=P:P &



memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_146_dram_pmem_pair_result -t 16 -c 16 --ratio=999:1 --test-time=120 --data-size=162 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=3333333333 --key-pattern=P:P &
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_146_dram_pmem_pair_result -t 16 -c 16 --ratio=999:1 --test-time=120 --data-size=162 --key-prefix=memtier- --key-minimum=3333333334 --key-maximum=6666666666 --key-pattern=P:P &
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_162_dram_pmem_pair_result -t 16 -c 16 --ratio=999:1 --test-time=120 --data-size=162 --key-prefix=memtier- --key-minimum=6666666667 --key-maximum=9999999999 --key-pattern=P:P &


numactl -C 32-64 ./memcached -u root -P /tmp/memcached.pid -t 16 -o pslab_file=/mnt/aep/pool,pslab_size=65536,pslab_policy=pmem,pslab_force,slab_sizes=64-96-112-128-144-160-176-192-208-224-240-256-384-512-1024-2048-4096-8192
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_tmp_result -t 16 --ratio=999:1 --test-time=120 --data-size=18 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_tmp_result -t 16 --ratio=999:1 --test-time=120 --data-size=50 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_tmp_result -t 16 --ratio=999:1 --test-time=120 --data-size=114 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_tmp_result -t 16 --ratio=999:1 --test-time=120 --data-size=130 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_tmp_result -t 16 --ratio=999:1 --test-time=120 --data-size=146 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_tmp_result -t 16 --ratio=999:1 --test-time=120 --data-size=162 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_tmp_result -t 16 --ratio=999:1 --test-time=120 --data-size=178 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P

memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_178_dram_pmem_pair_result -t 1 -c 1 --ratio=5:5 --test-time=20 --data-size=178 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P --expiry-range=1-10
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_8t_16t_default_datasize_178_dram_pmem_pair_result -t 1 -c 1 --ratio=5:5 --test-time=20 --data-size=178 --key-prefix=memtier- --key-minimum=100000 --key-maximum=999999 --key-pattern=P:P --expiry-range=1-3



numactl -C 1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,77,79,81,83,85,87,89,91,93,95 ./memcached -u root -P /tmp/memcached.pid -t 10 -o pslab_file=/mnt/aep/pool,pslab_size=65536,pslab_policy=pmem,pslab_force,slab_sizes=64-96-112-128-144-160-176-192-208-224-240-256-384-512-1024-2048-4096-8192
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_10t_16t_default_datasize_18_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=18 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_10t_16t_default_datasize_50_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=50 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_10t_16t_default_datasize_114_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=114 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_10t_16t_default_datasize_178_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=178 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_10t_16t_default_datasize_306_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=306 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_10t_16t_default_datasize_434_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=434 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_10t_16t_default_datasize_946_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=946 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_10t_16t_default_datasize_1970_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=1970 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_10t_16t_default_datasize_4018_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=4018 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P

numactl -C 1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,77,79,81,83,85,87,89,91,93,95 ./memcached -u root -P /tmp/memcached.pid -t 12 -o pslab_file=/mnt/aep/pool,pslab_size=65536,pslab_policy=pmem,pslab_force,slab_sizes=64-96-112-128-144-160-176-192-208-224-240-256-384-512-1024-2048-4096-8192
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_12t_16t_default_datasize_18_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=18 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_12t_16t_default_datasize_50_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=50 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_12t_16t_default_datasize_114_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=114 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_12t_16t_default_datasize_178_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=178 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_12t_16t_default_datasize_306_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=306 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_12t_16t_default_datasize_434_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=434 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_12t_16t_default_datasize_946_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=946 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_12t_16t_default_datasize_1970_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=1970 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_12t_16t_default_datasize_4018_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=4018 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P

numactl -C 1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,77,79,81,83,85,87,89,91,93,95 ./memcached -u root -P /tmp/memcached.pid -t 14 -o pslab_file=/mnt/aep/pool,pslab_size=65536,pslab_policy=pmem,pslab_force,slab_sizes=64-96-112-128-144-160-176-192-208-224-240-256-384-512-1024-2048-4096-8192
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_14t_16t_default_datasize_18_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=18 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_14t_16t_default_datasize_50_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=50 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_14t_16t_default_datasize_114_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=114 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_14t_16t_default_datasize_178_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=178 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_14t_16t_default_datasize_306_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=306 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_14t_16t_default_datasize_434_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=434 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_14t_16t_default_datasize_946_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=946 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_14t_16t_default_datasize_1970_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=1970 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_14t_16t_default_datasize_4018_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=4018 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P


numactl -C 1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,73,75,77,79,81,83,85,87,89,91,93,95 ./memcached -u root -P /tmp/memcached.pid -t 16 -o pslab_file=/mnt/aep/pool,pslab_size=65536,pslab_policy=pmem,pslab_force,slab_sizes=64-96-112-128-144-160-176-192-208-224-240-256-384-512-1024-2048-4096-8192
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_16t_16t_default_datasize_18_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=18 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_16t_16t_default_datasize_50_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=50 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_16t_16t_default_datasize_114_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=114 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_16t_16t_default_datasize_178_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=178 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_16t_16t_default_datasize_306_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=306 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_16t_16t_default_datasize_434_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=434 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_16t_16t_default_datasize_946_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=946 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_16t_16t_default_datasize_1970_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=1970 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P
memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_16t_16t_default_datasize_4018_dram_pmem_pair_result -t 16 --ratio=999:1 --test-time=120 --data-size=4018 --key-prefix=memtier- --key-minimum=1000000000 --key-maximum=9999999999 --key-pattern=P:P

