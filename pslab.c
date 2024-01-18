/*
 * Copyright 2018 Lenovo
 *
 * Licensed under the BSD-3 license. see LICENSE.Lenovo.txt for full text
 */
#include "memcached.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdatomic.h>


/*
#define PSLAB_POOL_SIG "PMCH"
#define PSLAB_POOL_SIG_SIZE 4
#define PSLAB_POOL_VER_SIZE 12
#define PSLAB_ALIGN_MASK 0xfffffff8

#pragma pack(1)

typedef struct {
    char        signature[PSLAB_POOL_SIG_SIZE];
    uint32_t    length; 
    char        version[PSLAB_POOL_VER_SIZE];
    uint8_t     reserved;
    uint8_t     checksum[2];
    atomic_uint_fast8_t     valid;  

    uint64_t    process_started;
    uint32_t    flush_time[2];

    uint32_t    slab_num;
    uint32_t    slab_page_size;
    uint32_t    slabclass_num;
    uint32_t    slabclass_sizes[];
} pslab_pool_t;

#define PSLAB_LINKED 1
#define PSLAB_CHUNKED 2
#define PSLAB_CHUNK 4

typedef struct {
    atomic_uint_fast8_t     id; // slab class id
    uint8_t     flags;       // non-persistent
    uint8_t     reserved[6]; // make slab[] 8 bytes aligned
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
*/




static pslab_pool_t *pslab_pool;
static pslab_t *pslab_start, *pslab_end;


// lxdchange 6
static pslab_pool_t *dslab_pool;
static pslab_t *dslab_start, *dslab_end;


uint64_t pslab_addr2off(void *addr) {
    return ((char *) addr >= (char *) pslab_start) ?
        (char *) addr - (char *) pslab_start : 0;
}

#define ADDR_ALIGNED(addr) (((addr) + 256 - 1) & (~255))


#define pslab_off2addr(off) ((off) ? (void *) ((char *)pslab_start + (off)) : NULL)

#define pslab_addr2slab(addr) ((char *) (addr) >= (char *) pslab_start ? \
    (pslab_t *) ((char *)(addr) - ((char *)(addr) - (char *) pslab_start) % \
    PSLAB_FRAME_SIZE(pslab_pool)) : NULL)

int pslab_contains(char *p) { // lxdchange 3
    if (p >= (char *) pslab_start && p < (char *) pslab_end)
        return 1;
    if (p >= (char *) dslab_start && p < (char *) dslab_end)
        return 1;
    return 0;
}

void pslab_use_slab(void *p, int id, unsigned int size) {
    ////printf("begin, pslab_use_slab\n");
    pslab_t *fp = PSLAB_SLAB2FRAME(p);
    fp->size = size;
    ////printf("pmem_member_persist, pslab_t size\n");
    pmem_member_persist(fp, size); // lxdchange
    ////printf("end pmem_member_persist, pslab_t size\n");
    atomic_store(&fp->id, id);
    ////printf("pmem_member_persist, pslab_t id\n");
    pmem_member_persist(fp, id); // lxdchange
    ////printf("end pmem_member_persist, pslab_t id\n");
    ////printf("end, pslab_use_slab\n");
}

void *pslab_get_free_slab(void *slab) {
    // printf("begin pslab_get_free_slab\n");
    static pslab_t *cur = NULL;
    pslab_t *fp = PSLAB_SLAB2FRAME(slab); /* dram slab 到 pmem pslab 的转换, 即包装一下 */

    if (fp == NULL)
        cur = fp;
    else if (fp != cur)
        return NULL;
    PSLAB_WALK_FROM(fp, PSLAB_NEXT_FRAME(pslab_pool, cur)) {
        if (atomic_load(&fp->id) == 0 || (fp->flags & (PSLAB_LINKED | PSLAB_CHUNK)) == 0) {
            cur = fp;
            return fp->slab; /* pmem pslab 到 dram slab 的转换, 即读取一下 */
        }
    }
    cur = NULL;
    // printf("end pslab_get_free_slab\n");
    return NULL;
}

static uint8_t pslab_chksum0;

static uint8_t pslab_do_checksum(void *buf, uint32_t len) {
    uint8_t sum = 0;
    uint8_t *end = (uint8_t *)buf + len;
    uint8_t *cur = buf;

    while (cur < end)
        sum = (uint8_t) (sum + *(cur++));
    return sum;
}

#define pslab_do_checksum_member(p, m) \
    pslab_do_checksum(&(p)->m, sizeof ((p)->m))

static void pslab_checksum_init() {
    assert(pslab_pool != NULL);
    pslab_chksum0 = 0;
    pslab_chksum0 += pslab_do_checksum(pslab_pool,
        offsetof(pslab_pool_t, checksum));
    pslab_chksum0 += pslab_do_checksum_member(pslab_pool, process_started);
    pslab_chksum0 += pslab_do_checksum(&pslab_pool->slab_num,
        pslab_pool->length - offsetof(pslab_pool_t, slab_num));
}

static uint8_t pslab_checksum_check(int i) {
    uint8_t sum = pslab_chksum0;
    sum += pslab_do_checksum_member(pslab_pool, checksum[i]);
    sum += pslab_do_checksum_member(pslab_pool, flush_time[i]);
    return sum;
}

static void pslab_checksum_update(int sum, int i) {
    pslab_pool->checksum[i] = (uint8_t) (~(pslab_chksum0 + sum) + 1);
}

void pslab_update_flushtime(uint32_t time) {
    printf("pslab_update_flushtime function call\n");
    int i = (atomic_load(&pslab_pool->valid) - 1) ^ 1;

    pslab_pool->flush_time[i] = time;
    pslab_checksum_update(pslab_do_checksum(&time, sizeof (time)), i);
    pmem_member_flush(pslab_pool, flush_time);
    pmem_member_persist(pslab_pool, checksum); // lxdchange

    atomic_store(&pslab_pool->valid, i + 1);
    pmem_member_persist(pslab_pool, valid);
}

time_t pslab_process_started(time_t process_started) {
    static time_t process_started_new;

    if (process_started) {
        process_started_new = process_started;
        return pslab_pool->process_started;
    } else {
        return process_started_new;
    }
}

int pslab_do_recover() {
    // printf("pslab_do_recover function call\n");
    // ITEM_LINKED, ITEM_CAS, ITEM_PSLAB (ITEM_CHUNK)
    pslab_t *fp;
    uint8_t *ptr;
    int i, size, perslab;

    settings.oldest_live = pslab_pool->flush_time[atomic_load(&pslab_pool->valid) - 1];

    /* current_time will be resetted by clock_handler afterwards. Set
     * it temporarily, so that functions depending on it can be reused
     * during recovery */
    current_time = process_started - pslab_pool->process_started;

    PSLAB_WALK(fp) {
        fp->flags = 0;
    }

    /* check for linked and chunked slabs and mark all chunks */
    PSLAB_WALK(fp) { // 以 Frame 为单位进行遍历,  Frame 是对 original memcached 中 slab_page 的进一步封装, Frame = pslab_t + slab_page
        if (atomic_load(&fp->id) == 0)
            continue;
        size = fp->size;
        perslab = pslab_pool->slab_page_size / size;
        for (i = 0, ptr = fp->slab; i < perslab; i++, ptr += size) {
            item *it = (item *) ptr;

            if (atomic_load(&it->it_flags) & ITEM_LINKED) {
                // printf("an item, with ITEM_LINKED");
                if (item_is_flushed(it) ||
                        (it->exptime != 0 && it->exptime <= current_time)) {
                    atomic_store(&it->it_flags, ITEM_PSLAB); // 准备后续重新挂载到 PSLAB chain 中
                    ////printf("pmem_member_persist it->it_flags in pslab_do_recover\n");
                    pmem_member_persist(it, it_flags);
                    ////printf("end pmem_member_persist\n");
                } else {
                    fp->flags |= PSLAB_LINKED;
                    if (atomic_load(&it->it_flags) & ITEM_CHUNKED)
                        fp->flags |= PSLAB_CHUNKED;
                }
            } else if (atomic_load(&it->it_flags) & ITEM_CHUNK) {
                // printf("here2\n");
                ((item_chunk *)it)->head = NULL; /* non-persistent */
            }
        }
    }

    /* relink alive chunks */
    PSLAB_WALK(fp) {
        // printf("relink alive chunks\n");
        if (atomic_load(&fp->id) == 0 || (fp->flags & PSLAB_CHUNKED) == 0)
            continue;

        size = fp->size;
        perslab = pslab_pool->slab_page_size / size;
        for (i = 0, ptr = fp->slab; i < perslab; i++, ptr += size) {
            item *it = (item *) ptr;

            if ((atomic_load(&it->it_flags) & ITEM_LINKED) && (atomic_load(&it->it_flags) & ITEM_CHUNKED)) {
                // printf("here3\n");
                item_chunk *nch;
                item_chunk *ch = (item_chunk *) ITEM_data(it);
                ch->head = it;
                while ((nch = pslab_off2addr(ch->next_poff)) != NULL) {
                    pslab_t *nfp = pslab_addr2slab(nch);
                    nfp->flags |= PSLAB_CHUNK;

                    nch->head = it;
                    ch->next = nch;
                    nch->prev = ch;
                    ch = nch;
                }
            }
        }
    }

    /* relink linked slabs and free free ones */
    PSLAB_WALK(fp) {
        int id;

        if (atomic_load(&fp->id) == 0 || (fp->flags & (PSLAB_LINKED | PSLAB_CHUNK)) == 0)
            continue;

        if (do_slabs_renewslab(fp->id, (char *)fp->slab) == 0)
            return -1;

        id = atomic_load(&fp->id);
        size = fp->size;
        perslab = pslab_pool->slab_page_size / size;
        for (i = 0, ptr = fp->slab; i < perslab; i++, ptr += size) {
            item *it = (item *) ptr;
            if (atomic_load(&it->it_flags) & ITEM_LINKED) {
                // printf("relink the item\n");
                // printf("here5\n");
                do_slab_realloc(it, id);
                do_item_relink(it, hash(ITEM_key(it), it->nkey)); // PMEM 中非空闲的item, 直接重新挂载
            } else if ((atomic_load(&it->it_flags) & ITEM_CHUNK) == 0 ||
                    ((item_chunk *)it)->head == NULL) {
                // printf("here6\n");
                assert((atomic_load(&it->it_flags) & ITEM_CHUNKED) == 0);
                do_slabs_free(ptr, 0, id); // PMEM 中原来的空闲部分, 还是转换成 dram 挂载到 slabclass id中
            }
        }
    }

    return 0;
}

int pslab_pre_recover(char *name, uint32_t *slab_sizes, int slab_max,
        int slab_page_size) {
       ////printf("begin pslab_pre_recover function call\n");
    size_t mapped_len;
    int is_pmem;
    int i;
    /* lxd modification */
       ////printf("pmem_map_file in pslab_pre_recover\n");
    if ((pslab_pool = pmem_map_file(name, 0, PMEM_FILE_EXCL,
            0, &mapped_len, &is_pmem)) == NULL) {
        fprintf(stderr, "pmem_map_file failed\n");
        return -1;
    }

    int *tmp_pslab_pool = (int *)pslab_pool;
    int addition  = *tmp_pslab_pool;
    pslab_pool = (pslab_pool_t *)((char*)pslab_pool + addition);
    printf("in pre_recover, the pslab_pool address is: %llu\n", (unsigned long long int)pslab_pool);


    if (!is_pmem && (pslab_force == false)) {
        fprintf(stderr, "%s is not persistent memory\n", name);
        return -1;
    }
    if (strncmp(pslab_pool->signature, PSLAB_POOL_SIG, PSLAB_POOL_SIG_SIZE) != 0) {
        fprintf(stderr, "pslab pool unknown signature\n");
        return -1;
    }
    pslab_checksum_init();
    if (pslab_checksum_check(atomic_load(&pslab_pool->valid) - 1)) {
        fprintf(stderr, "pslab pool bad checksum\n");
        return -1;
    }
    if (strncmp(pslab_pool->version, VERSION, PSLAB_POOL_VER_SIZE) != 0) {
        fprintf(stderr, "pslab pool version mismatch\n");
        return -1;
    }
    if (pslab_pool->slab_page_size != slab_page_size) {
        fprintf(stderr, "pslab pool slab size mismatch\n");
        return -1;
    }

    assert(slab_max > pslab_pool->slabclass_num);
    for (i = 0; i < pslab_pool->slabclass_num; i++)
        slab_sizes[i] = pslab_pool->slabclass_sizes[i];
    slab_sizes[i] = 0;

    pslab_start = PSLAB_FIRST_FRAME(pslab_pool);
    pslab_end = (pslab_t *) ((char *) pslab_start + pslab_pool->slab_num
        * PSLAB_FRAME_SIZE(pslab_pool));

    ////printf("end pslab_pre_recover function call\n");

    return 0;
}

bool pslab_force;

int pslab_create(char *pool_name, uint64_t pool_size, uint32_t slab_page_size, // settings.slab_page_size, default to 1MB page size
        uint32_t *slabclass_sizes, int slabclass_num) { // lxdchange 4
    
    size_t simu_mapped_len;
    int simu_is_pmem;
    simu_pslab_pool_file_path = "/mnt/aep/simu_pool";
    simu_pslab_pool_size = 32LL * 1024 * 1024 * 1024;
    simu_aligns = 256;
    assert(simu_pslab_pool_size > (1024 * 1024));
    simu_num_chunks = (simu_pslab_pool_size - (1024 * 1024)) / simu_aligns;




    if ((simu_pslab_pool = (char*)pmem_map_file(simu_pslab_pool_file_path, simu_pslab_pool_size,
            PMEM_FILE_CREATE, 0666, &simu_mapped_len, &simu_is_pmem)) == NULL) {
        fprintf(stderr, "simu_pmem_map_file failed\n");
        return -1;
    }
    pmem_memset_nodrain(simu_pslab_pool, 0, simu_pslab_pool_size);
    printf("Init simu_pslab_pool %llu\n", (unsigned long long int)simu_pslab_pool);
    simu_pslab_pool = (char*)ADDR_ALIGNED((unsigned long long int)simu_pslab_pool);
    printf("Alig simu_pslab_pool %llu\n", (unsigned long long int)simu_pslab_pool);



    ////printf("begin pslab_create function\n");

    ////printf("The pool_size in pslab_create is: %lu\n", pool_size);
    ////printf("the pool name is %s, the pool_size is %lu, the slab_page_size is %u, the slabclass_num is %d\n", pool_name, pool_size, slab_page_size, slabclass_num);
    size_t mapped_len;
    int is_pmem;
    uint32_t length;
    pslab_t *fp;
    int i;

    int alignment = 1; // 0 for not alignment, 1 for alignment;
    if(alignment == 0){
        printf("not alignment\n");
    }
    else{
        printf("alignment\n");
    }
    

    if ((pslab_pool = pmem_map_file(pool_name, pool_size,
            PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmem)) == NULL) {
        fprintf(stderr, "pmem_map_file failed\n");
        return -1;
    }
    dslab_pool = (pslab_pool_t *)((char*)malloc(pool_size));
    memset(dslab_pool, 0, pool_size);
    // pool_start = (char*)pslab_pool;
    printf("Init pslab_pool %llu\n", (unsigned long long int)pslab_pool);
    printf("Init dslab_pool %llu\n", (unsigned long long int)dslab_pool);

    unsigned long long int pslab_pool_addr = (unsigned long long int)pslab_pool;
    int addition = PMEM_ALIGN - (pslab_pool_addr % PMEM_ALIGN);
    if(alignment == 0){ addition = addition + 1; }
    int *tmp_pslab_pool = (int *)pslab_pool;
    *tmp_pslab_pool = addition;
    pslab_pool = (pslab_pool_t *)((char*)pslab_pool + addition);
    pool_size = pool_size - addition;


    

    unsigned long long int dslab_pool_addr = (unsigned long long int)dslab_pool;
    int addition2 = PMEM_ALIGN - (dslab_pool_addr % PMEM_ALIGN);
    if(alignment == 0){ addition2 = addition2 + 1; }
    dslab_pool = (pslab_pool_t *)((char*)dslab_pool + addition2);

    pslab_dslab_offset = (char*)(pslab_pool) - (char*)(dslab_pool);


    printf("pslab_pool_at: %llu\n", (unsigned long long int)pslab_pool);
    printf("dslab_pool_at: %llu\n", (unsigned long long int)dslab_pool);
    printf("pslab_dslab_off: %llu\n", (unsigned long long int)pslab_dslab_offset);



    if (!is_pmem && (pslab_force == false)) {
        fprintf(stderr, "%s is not persistent memory\n", pool_name);
        return -1;
    }

    length = (sizeof (pslab_pool_t) + sizeof (pslab_pool->slabclass_sizes[0])
        * slabclass_num + 7) & PSLAB_ALIGN_MASK;
    pmem_memset_nodrain(pslab_pool, 0, length);
    printf("length is: %llu\n", (unsigned long long int)length);
    (void) memcpy(pslab_pool->signature, PSLAB_POOL_SIG, PSLAB_POOL_SIG_SIZE);
    pslab_pool->length = length;
    snprintf(pslab_pool->version, PSLAB_POOL_VER_SIZE, VERSION);
    pslab_pool->slab_page_size = slab_page_size;
    pslab_pool->slab_num = (pool_size - pslab_pool->length)
        / PSLAB_FRAME_SIZE(pslab_pool);
    
    printf("pslab_fram_size is: %llu\n", (unsigned long long int)PSLAB_FRAME_SIZE(pslab_pool));
    printf("pslab_t size is: %llu\n", (unsigned long long int)(sizeof(pslab_t)));
    printf("pslab_pool slab_num is: %llu\n", (unsigned long long int)pslab_pool->slab_num);

    pslab_start = PSLAB_FIRST_FRAME(pslab_pool);
    pslab_end = (pslab_t *) ((char *) pslab_start + pslab_pool->slab_num
        * PSLAB_FRAME_SIZE(pslab_pool));
    dslab_start = (pslab_t *)((char*)pslab_start - pslab_dslab_offset);
    dslab_end   = (pslab_t *)((char*)pslab_end   - pslab_dslab_offset);
    printf("pslab_start: %llu\n", (unsigned long long int)pslab_start);
    printf("pslab_end: %llu\n", (unsigned long long int)pslab_end);
    printf("dslab_start: %llu\n", (unsigned long long int)dslab_start);
    printf("dslab_end: %llu\n", (unsigned long long int)dslab_end);
    

    PSLAB_WALK(fp) {
        pmem_memset_nodrain(fp, 0, sizeof (pslab_t));
    }
    ////printf("end pmem_memset_nodrain\n");

    pslab_pool->slabclass_num = slabclass_num;
    for (i = 0; i < slabclass_num; i++)
    {
        pslab_pool->slabclass_sizes[i] = slabclass_sizes[i]; // slabclass_sizes 来源于 dump, dump 来源于 DRAM slab settings
    }
        

    assert(process_started != 0);
    pslab_pool->process_started = (uint64_t) process_started;

    pslab_checksum_init();
    pslab_checksum_update(0, 0);

    ////printf("pmem_persist pslab_pool in pslab_create\n");
    pmem_persist(pslab_pool, pslab_pool->length);
    ////printf("end pmem_persist\n");

    atomic_store(&pslab_pool->valid, 1);
    ////printf("pmem_member_persist pslab_pool->valid in pslab_create\n");
    pmem_member_persist(pslab_pool, valid);
    ////printf("end pmem_member_persist\n");

    //// end pslab_create function\n");

    return 0;
}

