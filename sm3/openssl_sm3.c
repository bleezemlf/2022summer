#include <openssl/evp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define BUFFER_SIZE 1024 //B
int opensslSm3(uint8_t* dgst, const char* src)
{

    int flag = 0;
    uint32_t sm3_len;
    EVP_MD_CTX* sm3ctx = EVP_MD_CTX_new(); //新建结构体

    clock_t start, end;

    EVP_MD_CTX_init(sm3ctx); //初始化摘要结构体
    EVP_DigestInit_ex(sm3ctx, EVP_sm3(), NULL); //设置摘要算法和密码算法引擎

    uint64_t str_length = strlen(src);
    start = clock();
    EVP_DigestUpdate(sm3ctx, src, str_length);
    EVP_DigestFinal_ex(sm3ctx, dgst, &sm3_len); //摘要结束，输出摘要值
    end = clock();
    printf("\nstring is \"%s\" length %d Total time %f\n", src, str_length, (double)(end - start) / CLOCKS_PER_SEC);
    EVP_MD_CTX_reset(sm3ctx); //释放内存
    return 0;
}
//从文件中读取数据，并进行hash
int opensslSm3FromFile(uint8_t* dgst, const char* file_name)
{
    int flag = 0;
    uint32_t sm3_len;
    EVP_MD_CTX* sm3ctx = EVP_MD_CTX_new(); //新建结构体
    char buffer[BUFFER_SIZE]; //存储明文的buffer

    clock_t start, end;

    EVP_MD_CTX_init(sm3ctx); //初始化摘要结构体
    EVP_DigestInit_ex(sm3ctx, EVP_sm3(), NULL); //设置摘要算法和密码算法引擎

    FILE* f1 = fopen(file_name, "r");
    if (f1 == NULL)
        exit(1);
    uint64_t file_length = 0;
    //计算文件大小
    while (fgetc(f1) != EOF) {
        file_length++;
    }
    rewind(f1);
    start = clock();
    int i;
    for (i = 0; i < file_length / BUFFER_SIZE; i++) {
        int t = fread(buffer, sizeof(char), BUFFER_SIZE, f1);
        EVP_DigestUpdate(sm3ctx, buffer, BUFFER_SIZE); //逐次迭代计算文件中所有数值的摘要
    }
    fread(buffer, sizeof(char), file_length - i * BUFFER_SIZE, f1);
    EVP_DigestUpdate(sm3ctx, buffer, file_length - i * BUFFER_SIZE); //逐次迭代计算文件中所有数值的摘要
    end = clock();

    printf("\nfile name is \"%s\" file_length %d Total time %f\n", file_name, file_length, (double)(end - start) / CLOCKS_PER_SEC);
    EVP_DigestFinal_ex(sm3ctx, dgst, &sm3_len); //摘要结束，输出摘要值
    EVP_MD_CTX_reset(sm3ctx); //释放内存
    return 0;
}
/* int testOpensslSm3(const void* msg, size_t len, const void* dgst) */
int testOpensslSm3(char* src_str, char* file_name)
{
    uint8_t dgst2[EVP_MAX_MD_SIZE]; //保存摘要的数组
    opensslSm3(dgst2, src_str);
    for (int i = 0; i < 32; i++) {
        printf("%02x", dgst2[i]);
    }

    uint8_t dgst1[EVP_MAX_MD_SIZE]; //保存摘要的数组
    opensslSm3FromFile(dgst1, file_name);
    for (int i = 0; i < 32; i++) {
        printf("%02x", dgst1[i]);
    }
    return 0;
}
