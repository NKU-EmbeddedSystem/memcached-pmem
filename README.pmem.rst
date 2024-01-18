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
   memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_pmem_1t_result -x 1 -t 1 --ratio=9:1 --test-time=240 --data-size-range=1-400000 --data-size-pattern=R --key-prefix=memtier- --key-minimum=1 --key-maximum=100000 --key-pattern=R:R
   memtier_benchmark -s localhost -p 11211 -P memcache_text --out-file=memtier_pmem_1t_result -x 1 -t 1 --ratio=9:1 --test-time=240 --data-size-range=1-40000 --data-size-pattern=R --key-prefix=memtier- --key-minimum=1 --key-maximum=100000 --key-pattern=R:R
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