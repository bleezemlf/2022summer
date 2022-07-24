#include "sm3.h"
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
void testSIMD()
{
    __m256i a = _mm256_set_epi64x(1, 2, 3, 4);
    __m256i b = _mm256_set_epi64x(5, 6, 7, 8);
    __m256i sum = _mm256_add_epi64(a, b);
    int64_t ra, rb, rc, rd;
    ra = sum[0];
    rb = sum[1];
    rc = sum[2];
    rd = sum[3];
    printf("%ld,%ld,%ld,%ld", ra, rb, rc, rd);
}
