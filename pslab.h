/*
 * Copyright 2018 Lenovo
 *
 * Licensed under the BSD-3 license. see LICENSE.Lenovo.txt for full text
 */
#ifndef PSLAB_H
#define PSLAB_H

#include <libpmem.h>



#define PSLAB_POOL_SIG "PMCH"
#define PSLAB_POOL_SIG_SIZE 4
#define PSLAB_POOL_VER_SIZE 12
#define PSLAB_ALIGN_MASK 0xfffffff8

#define PMEM_ALIGN 256

#pragma pack(1)

/* persistent slab pool */
typedef struct {
    char        signature[PSLAB_POOL_SIG_SIZE];
    uint32_t    length; /* 8 bytes aligned */
    char        version[PSLAB_POOL_VER_SIZE];
    uint8_t     reserved;
    uint8_t     checksum[2];
    atomic_uint_fast8_t     valid;  /* not checksumed */

    uint64_t    process_started;
    uint32_t    flush_time[2];

    uint32_t    slab_num;
    uint32_t    slab_page_size;
    uint32_t    slabclass_num;

    char alignment[127]; // lxdchange

    uint32_t    slabclass_sizes[];
} pslab_pool_t;

#define PSLAB_LINKED 1
#define PSLAB_CHUNKED 2
#define PSLAB_CHUNK 4

typedef struct {
    atomic_uint_fast8_t     id; /* slab class id */
    uint8_t     flags;       /* non-persistent */
    uint8_t     reserved[250]; /* make slab[] 8 bytes aligned */
    uint32_t    size;
    uint8_t     slab[]; // sizeof数组名, 计算的是整个数组的存储大小
} pslab_t;

#pragma pack()

#define PSLAB_FRAME_SIZE(pm) (sizeof (pslab_t) + (pm)->slab_page_size)
#define PSLAB_FIRST_FRAME(pm) ((pslab_t *)((char *)(pm) + (pm)->length))
#define PSLAB_NEXT_FRAME(pm, fp) \
    ((fp) ? (pslab_t *)((char *)(fp) + PSLAB_FRAME_SIZE(pm)) : \
    PSLAB_FIRST_FRAME(pm))
#define PSLAB_SLAB2FRAME(slab) \
    ((slab) ? (pslab_t *)((char *)(slab) - sizeof (pslab_t)) : NULL)

#define PSLAB_WALK_FROM(fp, s) \
    assert(pslab_start != NULL || ((char *) (s) - (char *) pslab_start) \
            % PSLAB_FRAME_SIZE(pslab_pool) == 0); \
    (fp) = (s) ? (s) : pslab_start; \
    for (int _i = (s) ? ((char *)(s) - (char *) pslab_start) \
            / PSLAB_FRAME_SIZE(pslab_pool) : 0; \
        (fp) >= pslab_start && (fp) < pslab_end; \
        _i++, (fp) = PSLAB_NEXT_FRAME(pslab_pool, (fp)))
#define PSLAB_WALK_ID() (_i)
#define PSLAB_WALK(fp) PSLAB_WALK_FROM((fp), NULL)



#define	PSLAB_POLICY_DRAM 0
#define	PSLAB_POLICY_PMEM 1
#define	PSLAB_POLICY_BALANCED 2

// // 改动 5
// extern pslab_t *dslab_start, *dslab_end;
// extern long long int pslab_dslab_offset;

#define pmem_member_persist(p, m) \
    pmem_persist(&(p)->m, sizeof ((p)->m))
#define pmem_member_flush(p, m) \
    pmem_flush(&(p)->m, sizeof ((p)->m))
#define pmem_flush_from(p, t, m) \
    pmem_flush(&(p)->m, sizeof (t) - offsetof(t, m));
#define pslab_item_data_persist(it) pmem_persist((it)->data, ITEM_dtotal(it)
#define pslab_item_data_flush(it) pmem_flush((it)->data, ITEM_dtotal(it))

// int pslab_create(char *pool_name, uint32_t pool_size, uint32_t slab_size,
//     uint32_t *slabclass_sizes, int slabclass_num);
int pslab_create(char *pool_name, uint64_t pool_size, uint32_t slab_size, uint32_t *slabclass_sizes, int slabclass_num);
int pslab_pre_recover(char *name, uint32_t *slab_sizes, int slab_max, int slab_page_size);
int pslab_do_recover(void);
time_t pslab_process_started(time_t process_started);
void pslab_update_flushtime(uint32_t time);
void pslab_use_slab(void *p, int id, unsigned int size);
void *pslab_get_free_slab(void *slab);
int pslab_contains(char *p);
uint64_t pslab_addr2off(void *addr);

extern bool pslab_force;

#endif

