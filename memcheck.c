#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int patterns[] = {
    0x00000000,
    0xFFFFFFFF,
    0x55555555,
    0xAAAAAAAA,
    0x12345678,
    0x87654321
};
#define PATTERN_CNT (sizeof(patterns)/sizeof(patterns[0]))

// 一轮内存测试，返回本轮错误数量
int mem_test(void *mem_base, size_t size_bytes)
{
    size_t i;
    int err_cnt = 0;
    unsigned int *p = (unsigned int *)mem_base;
    size_t count = size_bytes / sizeof(unsigned int);

    for(int pat=0; pat<PATTERN_CNT; pat++)
    {
        unsigned int val = patterns[pat];
        printf("  Test pattern: 0x%08X\n", val);

        // write
        for(i=0; i<count; i++){
            p[i] = val;
        }
        // readback check
        for(i=0; i<count; i++)
        {
            if(p[i] != val)
            {
                printf("ERROR! addr=%p, expect=0x%08X, read=0x%08X\n",
                       &p[i], val, p[i]);
                err_cnt++;
            }
        }
    }
    return err_cnt;
}

int main(int argc, char** argv)
{
    if(argc !=3){
        printf("Usage: %s size_KB loop_count\n", argv[0]);
        printf("Example: %s 8192 10\n",argv[0]);
        printf("size_KB: test memory size(KB)\n");
        printf("loop_count: total test rounds\n");
        printf("WARNING: Do NOT use too large size, may cause OOM/crash!\n");
        return 1;
    }
    size_t size_kb = atoi(argv[1]);
    int loops = atoi(argv[2]);
    size_t bytes = size_kb * 1024;

    void *buf = malloc(bytes);
    if(!buf){
        printf("malloc failed!\n");
        return -1;
    }
    printf("Start DDR memory test, size: %zu KB, total loops: %d\n", size_kb, loops);

    int total_err = 0;
    for(int round = 1; round <= loops; round++)
    {
        printf("\n==== Round %d / %d ====\n", round, loops);
        int err = mem_test(buf, bytes);
        total_err += err;
        if(err >0)
        {
            printf("Round %d found %d errors!\n", round, err);
        }
    }

    if(total_err ==0){
        printf("\n==== ALL PASS ====\nNo memory error in all rounds.\n");
    }else{
        printf("\n==== FAIL ====\nTotal error count: %d\n", total_err);
    }

    free(buf);
    return 0;
}
