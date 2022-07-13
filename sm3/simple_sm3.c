#include "sm3.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SM3_WORD_SIZE 32
#define SM3_SIZE 256
#define SM3_MSG_BLOCK_BYTESIZE 64
#define SM3_VECTOR_BLOCK_BYTESIZE 32

uint32_t IV[] = { 0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600, 0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e };

typedef struct sm3ctx {
    uint32_t v[SM3_VECTOR_BLOCK_BYTESIZE / sizeof(uint32_t)];
    uint8_t msg[SM3_MSG_BLOCK_BYTESIZE];
    size_t num;
} Sm3Ctx;

inline uint32_t rol(uint32_t data, uint32_t len)
{
    return data << len | data >> (sizeof(data) * 8 - len);
}

inline uint32_t P0(uint32_t X)
{
    return X ^ rol(X, 9) ^ rol(X, 17);
}
inline uint32_t P1(uint32_t X)
{
    return X ^ rol(X, 15) ^ rol(X, 23);
}
inline uint32_t FF0(uint32_t X, uint32_t Y, uint32_t Z)
{
    return X ^ Y ^ Z;
}
inline uint32_t FF1(uint32_t X, uint32_t Y, uint32_t Z)
{
    return (X & Y) | (X & Z) | (Y & Z);
}
inline uint32_t GG0(uint32_t X, uint32_t Y, uint32_t Z)
{
    return X ^ Y ^ Z;
}
inline uint32_t GG1(uint32_t X, uint32_t Y, uint32_t Z)
{
    return (X & Y) | (~X & Z);
}
void CF(uint32_t v[SM3_VECTOR_BLOCK_BYTESIZE / sizeof(uint32_t)], const uint8_t msg[SM3_MSG_BLOCK_BYTESIZE])
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
    memcpy(W, msg, SM3_MSG_BLOCK_BYTESIZE);
    for (int j = 16; j < 68; j++) {
        W[j] = P0(W[j - 16] ^ W[j - 9] ^ rol(W[j - 3], 15)) ^ rol(W[j - 13], 7) ^ W[j - 6];
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
    v[0] ^= A;
    v[1] ^= B;
    v[2] ^= C;
    v[3] ^= D;
    v[4] ^= E;
    v[5] ^= F;
    v[6] ^= G;
    v[7] ^= H;
}

void sm3Init(Sm3Ctx* ctx)
{
    memcpy(ctx->v, IV, 32);
    ctx->num = 0;
}

void Sm3Update(Sm3Ctx* ctx, const uint8_t* msg, size_t msg_len)
{
    if (ctx->num) {
        size_t left = SM3_MSG_BLOCK_BYTESIZE - ctx->num;
        if (msg_len < left) {
            memcpy(ctx->msg + ctx->num, msg, msg_len);
            ctx->num += msg_len;
            return;
        } else {
            memcpy(ctx->msg + ctx->num, msg, left);
            CF(ctx->v, ctx->msg);
            msg += left;
            msg_len -= left;
        }
    }
    while (msg_len >= SM3_MSG_BLOCK_BYTESIZE) {
        CF(ctx->v, msg);
        msg += SM3_MSG_BLOCK_BYTESIZE;
        msg_len -= SM3_MSG_BLOCK_BYTESIZE;
    }
    ctx->num = msg_len;
    if (msg_len) {
        memcpy(ctx->msg, msg, msg_len);
    }
}
void Sm3Final(Sm3Ctx* ctx, uint8_t* v)
{
    size_t i;
    uint32_t* t_v = (uint32_t*)(v);
    uint64_t* count = (uint64_t*)(ctx->msg + SM3_MSG_BLOCK_BYTESIZE - 8);
    ctx->msg[ctx->num] = 0x80;
    if (ctx->num + 9 <= SM3_MSG_BLOCK_BYTESIZE) {
        memset(ctx->msg + ctx->num + 1, 0, SM3_MSG_BLOCK_BYTESIZE - ctx->num - 9);
    }
}

void testSimpleSm3()
{
    Sm3Ctx ctx;
    sm3Init(&ctx);
}
