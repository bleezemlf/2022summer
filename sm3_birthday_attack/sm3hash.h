#ifndef HEADER_C_FILE_SM3_HASH_H
#define HEADER_C_FILE_SM3_HASH_H

#ifdef  __cplusplus
extern "C" {
#endif

	int sm3_hash(char* message, size_t len, unsigned char* hash, unsigned int* hash_len);

#ifdef  __cplusplus
}
#endif

#endif  /* end of HEADER_C_FILE_SM3_HASH_H */
