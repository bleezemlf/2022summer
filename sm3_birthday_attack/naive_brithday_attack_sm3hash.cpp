#include <stdio.h>
#include <string.h>
#include "sm3hash.h"
#include <stdlib.h>
#include <time.h>

#define coll_num 3

char* genRandomString(int length)
{
	int flag, i;
	char* string;
	if ((string = (char*)malloc(length)) == NULL)
	{
		printf("Malloc failed!\n");
		return NULL;
	}

	for (i = 0; i < length - 1; i++)
	{
		flag = rand() % 3;
		switch (flag)
		{
		case 0:
			string[i] = 'A' + rand() % 26;
			break;
		case 1:
			string[i] = 'a' + rand() % 26;
			break;
		case 2:
			string[i] = '0' + rand() % 10;
			break;
		default:
			string[i] = 'x';
			break;
		}
	}
	string[length - 1] = '\0';
	return string;
}


int main(void)
{
	char* buff1;
	char* buff2;
	srand((unsigned)time(NULL));

	while (true) {
		buff1 = genRandomString(64);
		buff2 = genRandomString(64);
		unsigned int buff1_len = strlen((char*)buff1);
		unsigned int buff2_len = strlen((char*)buff2);
		unsigned char hash_value1[64];
		unsigned char hash_value2[64];
		unsigned int i, hash_len;

		sm3_hash(buff1, buff1_len, hash_value1, &hash_len);
		sm3_hash(buff2, buff2_len, hash_value2, &hash_len);
		int counter = 0;

		clock_t start_time, end_time;
		start_time = clock();

		for (int i = 0;i < coll_num;i++) {
			if (hash_value1[i] == hash_value2[i]) {
				counter++;
			}
		}
		if (counter == coll_num) {
			printf("find a partial collision \n\n");
			printf("first data:%s\n", buff1);
			printf("last data:%s\n", buff2);
			printf("first sm3\n");
			for (i = 0; i < hash_len; i++)
			{
				printf("0x%x  ", hash_value1[i]);
			}
			printf("\n\n");
			printf("last sm3\n");
			for (i = 0; i < hash_len; i++)
			{
				printf("0x%x  ", hash_value2[i]);
			}
			printf("\n\n");

			end_time = clock();
			printf("\nTotal time %f\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);

			break;
		}
		free(buff1);
		free(buff2);

	}
	return 0;
}