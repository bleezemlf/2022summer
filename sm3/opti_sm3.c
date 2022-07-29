#include "sm3.h"
#include <immintrin.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <xmmintrin.h>
#define BUFFER_SIZE 1024
#define SM3_MSG_BLOCK_BYTESIZE 64
#define SM3_VECTOR_BLOCK_BYTESIZE 32

static uint32_t Tj[] = {
    0x79cc4519U, 0xf3988a32U, 0xe7311465U, 0xce6228cbU, 0x9cc45197U, 0x3988a32fU, 0x7311465eU, 0xe6228cbcU, 0xcc451979U, 0x988a32f3U, 0x311465e7U, 0x6228cbceU, 0xc451979cU, 0x88a32f39U, 0x11465e73U, 0x228cbce6U, 0x9d8a7a87U, 0x3b14f50fU, 0x7629ea1eU, 0xec53d43cU, 0xd8a7a879U, 0xb14f50f3U, 0x629ea1e7U, 0xc53d43ceU, 0x8a7a879dU, 0x14f50f3bU, 0x29ea1e76U, 0x53d43cecU, 0xa7a879d8U, 0x4f50f3b1U, 0x9ea1e762U, 0x3d43cec5U, 0x7a879d8aU, 0xf50f3b14U, 0xea1e7629U, 0xd43cec53U, 0xa879d8a7U, 0x50f3b14fU, 0xa1e7629eU, 0x43cec53dU, 0x879d8a7aU, 0x0f3b14f5U, 0x1e7629eaU, 0x3cec53d4U, 0x79d8a7a8U, 0xf3b14f50U, 0xe7629ea1U, 0xcec53d43U, 0x9d8a7a87U, 0x3b14f50fU, 0x7629ea1eU, 0xec53d43cU, 0xd8a7a879U, 0xb14f50f3U, 0x629ea1e7U, 0xc53d43ceU, 0x8a7a879dU, 0x14f50f3bU, 0x29ea1e76U, 0x53d43cecU, 0xa7a879d8U, 0x4f50f3b1U, 0x9ea1e762U, 0x3d43cec5U
};

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

static uint32_t IV[]
    = { 0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600, 0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e };

typedef struct sm3ctx {
    uint32_t v[8];
    uint8_t msg[8][64];
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

#define rol(data, len) (((data) << (len)) | ((data) >> (32 - (len))))
#define P0(X) (X ^ rol(X, 9) ^ rol(X, 17))
#define P1(X) (X ^ rol(X, 15) ^ rol(X, 23))
#define FF0(X, Y, Z) (X ^ Y ^ Z)
#define FF1(X, Y, Z) ((X & Y) | (X & Z) | (Y & Z))
#define GG0(X, Y, Z) (X ^ Y ^ Z)
#define GG1(X, Y, Z) ((X & Y) | (~X & Z))

#define loadw(x) _mm256_set_epi32(((uint32_t*)(msg[7]))[x], ((uint32_t*)(msg[6]))[x], ((uint32_t*)(msg[5]))[x], ((uint32_t*)(msg[4]))[x], ((uint32_t*)(msg[3]))[x], ((uint32_t*)(msg[2]))[x], ((uint32_t*)(msg[1]))[x], ((uint32_t*)(msg[0]))[x])
#define rol_256_epi32(x, i) _mm256_xor_si256(_mm256_slli_epi32((x), (i)), _mm256_srli_epi32((x), 32 - (i)))
#define P1_256_epi32(x) _mm256_xor_si256((x), (_mm256_xor_si256(rol_256_epi32((x), 15), rol_256_epi32((x), 23))))

#define R(A, B, C, D, E, F, G, H, x)                             \
    SS1 = rol((rol(A, 12) + E + Tj[j]), 7);                      \
    SS2 = SS1 ^ rol(A, 12);                                      \
    TT1 = FF##x(A, B, C) + D + SS2 + (((uint32_t*)(W1 + j)))[i]; \
    TT2 = GG##x(E, F, G) + H + SS1 + (((uint32_t*)(W + j)))[i];  \
    B = rol(B, 9);                                               \
    H = TT1;                                                     \
    F = rol(F, 19);                                              \
    D = P0(TT2);                                                 \
    j++

#define R8(A, B, C, D, E, F, G, H, x) \
    R(A, B, C, D, E, F, G, H, x);     \
    R(H, A, B, C, D, E, F, G, x);     \
    R(G, H, A, B, C, D, E, F, x);     \
    R(F, G, H, A, B, C, D, E, x);     \
    R(E, F, G, H, A, B, C, D, x);     \
    R(D, E, F, G, H, A, B, C, x);     \
    R(C, D, E, F, G, H, A, B, x);     \
    R(B, C, D, E, F, G, H, A, x)

static void CF(uint32_t v[32], const uint8_t msg[8][64], int round)
{
    uint32_t A, B, C, D, E, F, G, H;
    __m256i W[68], W1[64];
    __m256i temp;
    __m256i shuffle_v = _mm256_set_epi8(12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3, 12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3);
    for (int i = 0; i < 16; i++) {
        W[i] = loadw(i);
        W[i] = _mm256_shuffle_epi8(W[i], shuffle_v);
    }
    for (int i = 16; i < 68; i++) {
        temp = _mm256_xor_si256(W[i - 16], W[i - 9]);
        temp = _mm256_xor_si256(temp, rol_256_epi32(W[i - 3], 15));
        temp = P1_256_epi32(temp);
        temp = _mm256_xor_si256(temp, rol_256_epi32(W[i - 13], 7));
        W[i] = _mm256_xor_si256(temp, W[i - 6]);
    }
    for (int i = 0; i < 64; i++) {
        W1[i] = _mm256_xor_si256(W[i], W[i + 4]);
    }
    for (int i = 0; i < round; i++) {
        A = v[0];
        B = v[1];
        C = v[2];
        D = v[3];
        E = v[4];
        F = v[5];
        G = v[6];
        H = v[7];

        uint32_t SS1, SS2, TT1, TT2;
        int j = 0;

        R8(A, B, C, D, E, F, G, H, 0);
        R8(A, B, C, D, E, F, G, H, 0);
        R8(A, B, C, D, E, F, G, H, 1);
        R8(A, B, C, D, E, F, G, H, 1);
        R8(A, B, C, D, E, F, G, H, 1);
        R8(A, B, C, D, E, F, G, H, 1);
        R8(A, B, C, D, E, F, G, H, 1);
        R8(A, B, C, D, E, F, G, H, 1);

        /* for (j = 0; j < 16; j++) { */
        /*     SS1 = rol(rol(A, 12) + E + Tj[j], 7); */
        /*     SS2 = SS1 ^ rol(A, 12); */
        /*     TT1 = FF0(A, B, C) + D + SS2 + (((uint32_t*)(W1 + j)))[i]; */
        /*     TT2 = GG0(E, F, G) + H + SS1 + (((uint32_t*)(W + j)))[i]; */
        /*     D = C; */
        /*     C = rol(B, 9); */
        /*     B = A; */
        /*     A = TT1; */
        /*     H = G; */
        /*     G = rol(F, 19); */
        /*     F = E; */
        /*     E = P0(TT2); */
        /* } */
        /* for (j = 16; j < 64; j++) { */
        /*     SS1 = rol(rol(A, 12) + E + Tj[j], 7); */
        /*     SS2 = SS1 ^ rol(A, 12); */
        /*     TT1 = FF1(A, B, C) + D + SS2 + (((uint32_t*)(W1 + j)))[i]; */
        /*     TT2 = GG1(E, F, G) + H + SS1 + (((uint32_t*)(W + j)))[i]; */
        /*     D = C; */
        /*     C = rol(B, 9); */
        /*     B = A; */
        /*     A = TT1; */
        /*     H = G; */
        /*     G = rol(F, 19); */
        /*     F = E; */
        /*     E = P0(TT2); */
        /*     /1* debugPrintA_H; *1/ */
        /* } */
        v[0] ^= A;
        v[1] ^= B;
        v[2] ^= C;
        v[3] ^= D;
        v[4] ^= E;
        v[5] ^= F;
        v[6] ^= G;
        v[7] ^= H;
    }
}

static void sm3Init(Sm3Ctx* ctx)
{
    memcpy(ctx->v, IV, 32);
    ctx->num = 0;
    ctx->nblocks = 0;
    memset(ctx->msg, 0, 512);
}

static void sm3Update(Sm3Ctx* ctx, const uint8_t* msg, size_t msg_len)
{
    if (ctx->num) {
        size_t left = 512 - ctx->num;
        if (msg_len < left) {
            memcpy(ctx->msg + ctx->num, msg, msg_len);
            ctx->num += msg_len;
            return;
        } else {
            memcpy(ctx->msg + ctx->num, msg, left);
            CF(ctx->v, ctx->msg, 8);
            ctx->nblocks += 8;
            msg += left;
            msg_len -= left;
        }
    }
    while (msg_len >= SM3_MSG_BLOCK_BYTESIZE) {
        CF(ctx->v, msg, 8);
        ctx->nblocks += 8;
        msg += 512;
        msg_len -= 512;
    }
    ctx->num = msg_len;
    if (msg_len) {
        memcpy(ctx->msg, msg, msg_len);
    }
}

static void sm3Final(Sm3Ctx* ctx, uint8_t* v)
{
    size_t i;
    int round = ctx->num / 64;
    uint32_t* t_v = (uint32_t*)(v);
    uint64_t* count = (uint64_t*)((uint8_t*)(ctx->msg) + SM3_MSG_BLOCK_BYTESIZE - 8);
    *((uint8_t*)(ctx->msg) + (ctx->num)) = 0x80;
    if (ctx->num + 9 <= SM3_MSG_BLOCK_BYTESIZE) {
        round++;
        memset(ctx->msg + ctx->num + 1, 0, SM3_MSG_BLOCK_BYTESIZE - ctx->num - 9);
    } else {
        round += 2;
        memset(ctx->msg + ctx->num + 1, 0, SM3_MSG_BLOCK_BYTESIZE - ctx->num - 1);
        CF(ctx->v, ctx->msg, ctx->num / 8);
        memset(ctx->msg, 0, SM3_MSG_BLOCK_BYTESIZE - 8);
    }
    count[0] = (uint64_t)(ctx->nblocks) * 512 + (ctx->num << 3);
    count[0] = byteSwap_64(count[0]);
    CF(ctx->v, ctx->msg, round);
    for (i = 0; i < sizeof(ctx->v) / sizeof(ctx->v[0]); i++) {
        t_v[i] = byteSwap_32(ctx->v[i]);
    }
}

void optiSm3(uint8_t* dgst, const char* src)
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
void optiSm3FromFile(uint8_t* dgst, const char* file_name)
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
void testOptiSm3()
{
    uint8_t dgst1[SM3_VECTOR_BLOCK_BYTESIZE];
    const char src_str[] = "sm3";
    optiSm3(dgst1, src_str);
    for (int i = 0; i < 32; i++)
        printf("%02x", dgst1[i]);

    uint8_t dgst2[SM3_VECTOR_BLOCK_BYTESIZE]; //保存摘要的数组
    char file_name[] = "/home/ubuntu/ClionProjects/sm3/resource/file3";
    optiSm3FromFile(dgst2, file_name);
    for (int i = 0; i < 32; i++)
        printf("%02x", dgst2[i]);
}
