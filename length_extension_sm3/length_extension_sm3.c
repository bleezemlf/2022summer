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
static uint32_t IVLE[] = { 0x44F0061E, 0x69FA6FDF, 0xC290C494, 0x654A05DC ,0x0C053DA7 ,0xE5C52B84 ,0xEF93A9D6 ,0x7D3FFF88 };


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
    memset(ctx->msg, 0, 64);//填充0
}

static void sm3InitLE(Sm3Ctx* ctx) {
    memcpy(ctx->v, IVLE, 32);
    ctx->num = 0;
    ctx->nblocks = 0;
    memset(ctx->msg,0,64);
}

static void sm3Update(Sm3Ctx* ctx, const uint8_t* msg, size_t msg_len)
{
    printf("IV:\n");
    for (int i = 0; i < 8; i++) { 
         printf("%02x", ctx->v[i]);
     } 
    printf("\n\n");
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
    //for (int i = 0; i < 8; i++) { 
    //     printf("%02x", ctx->v[i]); 
    // } 
    // printf("\n"); 
}//将明文转化成ctx，尚未hash

static void sm3Final(Sm3Ctx* ctx, uint8_t* v)
{
    size_t i;
    uint32_t* t_v = (uint32_t*)(v);
    uint64_t* count = (uint64_t*)(ctx->msg + SM3_MSG_BLOCK_BYTESIZE - 8);
    ctx->msg[ctx->num] = 0x80;//msg加tag 0x80
    if (ctx->num + 9 <= SM3_MSG_BLOCK_BYTESIZE) {
        memset(ctx->msg + ctx->num + 1, 0, SM3_MSG_BLOCK_BYTESIZE - ctx->num - 9);
    } else {
        memset(ctx->msg + ctx->num + 1, 0, SM3_MSG_BLOCK_BYTESIZE - ctx->num - 1);
        CF(ctx->v, ctx->msg);
        memset(ctx->msg, 0, SM3_MSG_BLOCK_BYTESIZE - 8);
    }

    count[0] = (uint64_t)(ctx->nblocks) * 512 + (ctx->num << 3);
    count[0] = byteSwap_64(count[0]);//加末尾

 /*   printf("msg:\n");
    for (int i = 0; i < 64; i++) {
        printf("%02x", ctx->msg[i]);
    }
    printf("\n\n");*/

    CF(ctx->v, ctx->msg);//此时v中存储hash


    for (i = 0; i < sizeof(ctx->v) / sizeof(ctx->v[0]); i++) {
        t_v[i] = byteSwap_32(ctx->v[i]);
    }
    
}

static void sm3FinalLE(Sm3Ctx* ctx, uint8_t* v)
{
    size_t i;
    uint32_t* t_v = (uint32_t*)(v);
    uint64_t* count = (uint64_t*)(ctx->msg + SM3_MSG_BLOCK_BYTESIZE - 8);
    ctx->msg[ctx->num] = 0x80;//msg加tag 0x80
    if (ctx->num + 9 <= SM3_MSG_BLOCK_BYTESIZE) {
        memset(ctx->msg + ctx->num + 1, 0, SM3_MSG_BLOCK_BYTESIZE - ctx->num - 9);
    }
    else {
        memset(ctx->msg + ctx->num + 1, 0, SM3_MSG_BLOCK_BYTESIZE - ctx->num - 1);
        CF(ctx->v, ctx->msg);
        memset(ctx->msg, 0, SM3_MSG_BLOCK_BYTESIZE - 8);
    }

    count[0] = (uint64_t)(ctx->nblocks) * 512 + (ctx->num << 3);
    count[0] = byteSwap_64(count[0]);//加末尾

    //根据forge_data的长度重新填充extension的长度
    ctx->msg[62] = 0x02;
    ctx->msg[63] = 0x60;

    //printf("LE:\n");
    //for (int i = 0; i < 64; i++) {
    //    printf("%02x", ctx->msg[i]);
    //}
    //printf("\n\n");


    CF(ctx->v, ctx->msg);//此时v中存储hash


    for (i = 0; i < sizeof(ctx->v) / sizeof(ctx->v[0]); i++) {
        t_v[i] = byteSwap_32(ctx->v[i]);
    }

}



void simSm3(uint8_t* dgst, const char* src, size_t str_length)
{
    Sm3Ctx ctx;
    sm3Init(&ctx);
    //size_t str_length = sizeof(src);
    clock_t start_time, end_time;
    start_time = clock();
    sm3Update(&ctx, src, str_length);
    sm3Final(&ctx, dgst);
    end_time = clock();
    //printf("\nTotal time %f\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);
}

void simSm3LE(uint8_t* dgst, const char* src, size_t str_length)
{
    Sm3Ctx ctx;
    sm3InitLE(&ctx);
    //size_t str_length = sizeof(src);
    clock_t start_time, end_time;
    start_time = clock();
    sm3Update(&ctx, src, str_length);
    sm3FinalLE(&ctx, dgst);
    end_time = clock();
    //printf("\nTotal time %f\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);
}

int main() {
    const char data[] = { 0x68,0x65,0x6c,0x6c,0x6f,0x20,0x77,0x6f,0x72,0x6c,0x64,0x00 };//hello world
    const char forge_data[] = { 0x68,0x65,0x6c,0x6c,0x6f,0x20,0x77,0x6f,0x72,0x6c,0x64,0x80,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x58,
        0x67,0x6f,0x6f,0x64,0x20,0x6d,0x6f,0x72,0x6e,0x69,0x6e,0x67,0x00 };
    //sizeof(forge_data)=76=64+12 = 原有数据长度的填充至的512的倍数+扩展数据长度
    //用修改过的sm3对扩展数据extension进行加密，修改其初始向量于长度260

    const char extension[] = "good morning";//0x67 0x6f 0x6f 0x64 0x20 0x6d 0x6f 0x72 0x6e 0x69 0x6e 0x67 0x00
    uint8_t hash[64];
    uint8_t hashLE[64];//若攻击成功，则hashLE是forge_data的摘要

    //0x94 0xeb 0x75 0xc6 0xf4 0xca 0x64 0x7e 0xea 0x6f 0x24 0xcb 0x9b 0x01 0xa2 0x9f 0xda 0x45
    //0x24 0xa0 0x1e 0xc6 0x9b 0x91 0x9c 0xea 0xdf 0x65 0x7d 0xf0 0xe4 0x93

    simSm3(hash, data,sizeof(data)-1);//hash设为hashLE的初始向量IVLE

    printf("data:\n");

    for (int i = 0;i < sizeof(data);i++) {
        printf("0x%02x ", data[i]);

    }
    printf("\n");

    printf("data_hash:\n");

    for (int i = 0;i < 32;i++) {
        printf("0x%02x ", hash[i]);

    }
    printf("\n\n");

    simSm3LE(hashLE, extension, sizeof(extension) - 1);

    printf("extension:\n");

    for (int i = 0;i < sizeof(extension);i++) {
        printf("0x%02x ", extension[i]);

    }
    printf("\n");

    printf("extension_hash*\n");

    for (int i = 0;i < 32;i++) {
        printf("0x%02x ", hashLE[i]);

    }
    printf("\n\n");

    simSm3(hash, forge_data, sizeof(forge_data) - 1);

    printf("forge_date:\n");

    for (int i = 0;i < sizeof(forge_data);i++) {
        printf("0x%02x ", forge_data[i]);

    }
    printf("\n");

    printf("forge_data_hash\n");

    for (int i = 0;i < 32;i++) {
        printf("0x%02x ", hash[i]);

    }
    printf("\n\n");



    return 0;

}

