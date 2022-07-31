#include "sm3.h"
#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
int main()
{
    char src_str[] = "sm3";
    char file_name[] = "/home/ubuntu/ClionProjects/sm3/resource/file3";
    printf("testOpensslSm3\n");
    testOpensslSm3(src_str, file_name);
    printf("\n\ntestSimSm3\n");
    testSimpleSm3(src_str, file_name);
    printf("\n\ntestOptiSm3\n");
    testOptiSm3(src_str, file_name);
    return 0;
}
