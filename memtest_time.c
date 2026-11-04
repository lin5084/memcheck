#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

        // write
        for(i=0; i<count; i++){
            p[i] = val;
        }
        // readback check
        for(i=0; i<count; i++)
        {
            if(p[i] != val)
            {
                printf("  [ERROR] addr=%p, expect=0x%08X, read=0x%08X\n",
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
    printf("==================================================\n");

    int total_err = 0;
    clock_t total_start = clock();  // 全部测试起始时间
    for(int round = 1; round <= loops; round++)
    {
        clock_t round_start = clock();
        int err = mem_test(buf, bytes);
        clock_t round_end = clock();

        double round_time_s = (double)(round_end - round_start) / CLOCKS_PER_SEC;

        total_err += err;

        int remain = loops - round;
        if(err == 0){
            printf("[Progress] Round %d/%d done. No error. Remain: %d rounds. Total errors so far: %d, This round time: %.3f s\n",
                   round, loops, remain, total_err, round_time_s);
        }else{
            printf("[Progress] Round %d/%d done. Found %d errors this round! Remain: %d rounds. Total errors: %d, This round time: %.3f s\n",
                   round, loops, err, remain, total_err, round_time_s);
        }
        // 每轮 flush 一下，确保 adb shell 实时看到输出
        fflush(stdout);
    }
    clock_t total_end = clock();
    double total_time_s = (double)(total_end - total_start) / CLOCKS_PER_SEC;
    double avg_time_s = total_time_s / loops;

    printf("==================================================\n");
    if(total_err ==0){
        printf("==== ALL PASS ====\nNo memory error in all %d rounds.\n", loops);
    }else{
        printf("==== FAIL ====\nTotal error count: %d\n", total_err);
    }
    printf("Total elapsed time: %.3f s\n", total_time_s);
    printf("Average time per round: %.3f s\n", avg_time_s);

    free(buf);
    return 0;
}
