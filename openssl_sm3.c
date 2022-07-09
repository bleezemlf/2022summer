#include "sm3.h"
#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define BUFFER_SIZE 1024 //B
#define FILE1_SIZE 1024 //1KiB
#define FILE2_SIZE 1024 * 1024 //1MiB
#define FILE3_SIZE 1024 * 1024 * 128 //128MiB
void test_openssl_sm3()
{
    OpenSSL_add_all_algorithms();
    unsigned char sm3_value[EVP_MAX_MD_SIZE]; //保存输出的摘要值的数组
    /* printf("%d", EVP_MAX_MD_SIZE); //EVP_MAX_MD_SIZE = 64 */
    unsigned int sm3_len, i;
    EVP_MD_CTX* sm3ctx; //EVP消息摘要结构体
    sm3ctx = EVP_MD_CTX_new();
    char buf[BUFFER_SIZE];

    clock_t start, end;

    EVP_MD_CTX_init(sm3ctx); //初始化摘要结构体
    EVP_DigestInit_ex(sm3ctx, EVP_sm3(), NULL); //设置摘要算法和密码算法引擎，这里密码算法使用sm3，算法引擎使用OpenSSL默认引擎即软算法

    FILE* f1 = fopen("/home/ubuntu/ClionProjects/sm3/resource/file3", "r");
    start = clock();
    if (f1 == NULL)
        exit(1);
    for (int i = 0; i < FILE3_SIZE / BUFFER_SIZE; i++) {
        fread(buf, sizeof(char), BUFFER_SIZE, f1);
        EVP_DigestUpdate(sm3ctx, buf, BUFFER_SIZE); //逐次迭代计算文件中所有数值的摘要
    }
    end = clock();

    EVP_DigestFinal_ex(sm3ctx, sm3_value, &sm3_len); //摘要结束，输出摘要值
    EVP_MD_CTX_reset(sm3ctx); //释放内存

    printf("原始数据%s的摘要值为:\n", buf);
    for (i = 0; i < sm3_len; i++) {
        printf("0x%02x ", sm3_value[i]);
    }
    printf("\nTotal time%f", (double)(end - start) / CLOCKS_PER_SEC);
}
