#include "sm3.h"
#include <immintrin.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#define BUFFER_SIZE 1024
#define SM3_MSG_BLOCK_BYTESIZE 64
#define SM3_VECTOR_BLOCK_BYTESIZE 32

#define debugPrintA_H printf("%02d: %08x %08x %08x %08x %08x %08x %08x %08x \n", j, A, B, C, D, E, F, G, H);

#define byteSwap_64(x) (0xff00000000000000 & x << 56)                 \
    | (0x00ff000000000000 & x << 40) | (0x0000ff0000000000 & x << 24) \
    | (0x000000ff00000000 & x << 8) | (0x00000000ff000000 & x >> 8)   \
    | (0x0000000000ff0000 & x >> 24) | (0x000000000000ff00 & x >> 40) \
    | (0x00000000000000ff & x >> 56)

#define byteSwap_32(x) (0xff000000 & x << 24) \
    | (0x00ff0000 & x << 8)                   \
    | (0x0000ff00 & x >> 8)                   \
    | (0x000000ff & x >> 24)

static uint32_t IV[] = { 0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600, 0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e };

typedef struct sm3ctx {
    uint32_t v[SM3_VECTOR_BLOCK_BYTESIZE / sizeof(uint32_t)];
    uint8_t msg[SM3_MSG_BLOCK_BYTESIZE];
    size_t num;
    size_t nblocks;
} Sm3Ctx;

static inline void debugPrint256bit(uint32_t v[8])
{
    for (int i = 0; i < 8; i++) {
        printf("%08x ", v[i]);
    }
    printf("\n\n");
}
static inline void debugPrint(uint32_t* msg, size_t len)
{
    for (int i = 0; i < len; i++) {
        if (i % 8 == 0 && i != 0)
            printf("\n");
        printf("%08x ", msg[i]);
    }
    printf("\n\n");
}

static inline uint32_t rol(uint32_t data, uint32_t len)
{
    return data << len | data >> (sizeof(data) * 8 - len);
}

static inline uint32_t P0(uint32_t X)
{
    return X ^ rol(X, 9) ^ rol(X, 17);
}
static inline uint32_t P1(uint32_t X)
{
    return X ^ rol(X, 15) ^ rol(X, 23);
}
static inline uint32_t FF0(uint32_t X, uint32_t Y, uint32_t Z)
{
    return X ^ Y ^ Z;
}
static inline uint32_t FF1(uint32_t X, uint32_t Y, uint32_t Z)
{
    return (X & Y) | (X & Z) | (Y & Z);
}
static inline uint32_t GG0(uint32_t X, uint32_t Y, uint32_t Z)
{
    return X ^ Y ^ Z;
}
static inline uint32_t GG1(uint32_t X, uint32_t Y, uint32_t Z)
{
    return (X & Y) | (~X & Z);
}
static void CF(uint32_t v[SM3_VECTOR_BLOCK_BYTESIZE / sizeof(uint32_t)], const uint8_t msg[SM3_MSG_BLOCK_BYTESIZE])
{
    uint32_t A = v[0];
    uint32_t B = v[1];
    uint32_t C = v[2];
    uint32_t D = v[3];
    uint32_t E = v[4];
    uint32_t F = v[5];
    uint32_t G = v[6];
    uint32_t H = v[7];
    uint32_t W[68], W1[64];
    const uint32_t* pblock = (const uint32_t*)(msg);
    for (int j = 0; j < 16; j++) {
        W[j] = byteSwap_32(pblock[j]);
    }
    for (int j = 16; j < 68; j++) {
        W[j] = P1(W[j - 16] ^ W[j - 9] ^ rol(W[j - 3], 15)) ^ rol(W[j - 13], 7) ^ W[j - 6];
    }
    for (int j = 0; j < 64; j++) {
        W1[j] = W[j] ^ W[j + 4];
    }
    for (int j = 0; j < 16; j++) {
        uint32_t SS1 = rol(rol(A, 12) + E + rol(0x79cc4519, j), 7);
        uint32_t SS2 = SS1 ^ rol(A, 12);
        uint32_t TT1 = FF0(A, B, C) + D + SS2 + W1[j];
        uint32_t TT2 = GG0(E, F, G) + H + SS1 + W[j];
        D = C;
        C = rol(B, 9);
        B = A;
        A = TT1;
        H = G;
        G = rol(F, 19);
        F = E;
        E = P0(TT2);
    }
    for (int j = 16; j < 64; j++) {
        uint32_t SS1 = rol(rol(A, 12) + E + rol(0x7a879d8a, j), 7);
        uint32_t SS2 = SS1 ^ rol(A, 12);
        uint32_t TT1 = FF1(A, B, C) + D + SS2 + W1[j];
        uint32_t TT2 = GG1(E, F, G) + H + SS1 + W[j];
        D = C;
        C = rol(B, 9);
        B = A;
        A = TT1;
        H = G;
        G = rol(F, 19);
        F = E;
        E = P0(TT2);
        /* debugPrintA_H; */
    }
    v[0] ^= A;
    v[1] ^= B;
    v[2] ^= C;
    v[3] ^= D;
    v[4] ^= E;
    v[5] ^= F;
    v[6] ^= G;
    v[7] ^= H;
}

static void sm3Init(Sm3Ctx* ctx)
{
    memcpy(ctx->v, IV, 32);
    ctx->num = 0;
    ctx->nblocks = 0;
    memset(ctx->msg, 0, 64);
}

static void sm3Update(Sm3Ctx* ctx, const uint8_t* msg, size_t msg_len)
{
    /* for (int i = 0; i < 64; i++) { */
    /*     printf("%02x", ctx->msg[i]); */
    /* } */
    if (ctx->num) {
        size_t left = SM3_MSG_BLOCK_BYTESIZE - ctx->num;
        if (msg_len < left) {
            memcpy(ctx->msg + ctx->num, msg, msg_len);
            ctx->num += msg_len;
            return;
        } else {
            memcpy(ctx->msg + ctx->num, msg, left);
            CF(ctx->v, ctx->msg);
            ctx->nblocks++;
            msg += left;
            msg_len -= left;
        }
    }
    while (msg_len >= SM3_MSG_BLOCK_BYTESIZE) {
        CF(ctx->v, msg);
        ctx->nblocks++;
        msg += SM3_MSG_BLOCK_BYTESIZE;
        msg_len -= SM3_MSG_BLOCK_BYTESIZE;
    }
    ctx->num = msg_len;
    if (msg_len) {
        memcpy(ctx->msg, msg, msg_len);
    }
    /* for (int i = 0; i < 64; i++) { */
    /*     printf("%02x", ctx->msg[i]); */
    /* } */
    /* printf("\n"); */
}

static void sm3Final(Sm3Ctx* ctx, uint8_t* v)
{
    size_t i;
    uint32_t* t_v = (uint32_t*)(v);
    uint64_t* count = (uint64_t*)(ctx->msg + SM3_MSG_BLOCK_BYTESIZE - 8);
    ctx->msg[ctx->num] = 0x80;
    if (ctx->num + 9 <= SM3_MSG_BLOCK_BYTESIZE) {
        memset(ctx->msg + ctx->num + 1, 0, SM3_MSG_BLOCK_BYTESIZE - ctx->num - 9);
    } else {
        memset(ctx->msg + ctx->num + 1, 0, SM3_MSG_BLOCK_BYTESIZE - ctx->num - 1);
        CF(ctx->v, ctx->msg);
        memset(ctx->msg, 0, SM3_MSG_BLOCK_BYTESIZE - 8);
    }
    count[0] = (uint64_t)(ctx->nblocks) * 512 + (ctx->num << 3);
    count[0] = byteSwap_64(count[0]);
    CF(ctx->v, ctx->msg);
    for (i = 0; i < sizeof(ctx->v) / sizeof(ctx->v[0]); i++) {
        t_v[i] = byteSwap_32(ctx->v[i]);
    }
}

void simSm3(uint8_t* dgst, const char* src)
{
    Sm3Ctx ctx;
    sm3Init(&ctx);
    size_t str_length = strlen(src);
    clock_t start_time, end_time;
    start_time = clock();
    sm3Update(&ctx, src, str_length);
    sm3Final(&ctx, dgst);
    end_time = clock();
    printf("\nTotal time %f\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);
}
void simSm3FromFile(uint8_t* dgst, const char* file_name)
{
    Sm3Ctx ctx;
    sm3Init(&ctx);

    clock_t start, end;
    FILE* f1 = fopen(file_name, "r");
    if (f1 == NULL)
        exit(1);
    uint64_t file_length = 0;
    //计算文件大小
    while (fgetc(f1) != EOF) {
        file_length++;
    }
    rewind(f1);
    char buffer[BUFFER_SIZE]; //存储明文的buffer
    /* printf("\n%lu\n", file_length); */
    start = clock();
    int i;
    for (i = 0; i < file_length / BUFFER_SIZE; i++) {
        int t = fread(buffer, sizeof(char), BUFFER_SIZE, f1);
        sm3Update(&ctx, buffer, BUFFER_SIZE);
    }
    fread(buffer, sizeof(char), file_length - i * BUFFER_SIZE, f1);
    sm3Update(&ctx, buffer, file_length - i * BUFFER_SIZE); //逐次迭代计算文件中所有数值的摘要
    sm3Final(&ctx, dgst);
    end = clock();

    printf("\nTotal time %f\n", (double)(end - start) / CLOCKS_PER_SEC);
}
void testSimpleSm3()
{
    uint8_t dgst1[SM3_VECTOR_BLOCK_BYTESIZE];
    const char src_str[] = "sm3";
    simSm3(dgst1, src_str);
    for (int i = 0; i < 32; i++)
        printf("%02x", dgst1[i]);

    uint8_t dgst2[SM3_VECTOR_BLOCK_BYTESIZE]; //保存摘要的数组
    char file_name[] = "/home/ubuntu/ClionProjects/sm3/resource/file3";
    simSm3FromFile(dgst2, file_name);
    for (int i = 0; i < 32; i++)
        printf("%02x", dgst2[i]);
}
