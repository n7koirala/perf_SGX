#ifndef ENCLAVE_T_H__
#define ENCLAVE_T_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include "sgx_edger8r.h" /* for sgx_ocall etc. */


#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

void ecall_matmul_u(float a[65536], float b[65536], float c[65536], int NB);
int mult_enclave(int a, int b);
int add_enclave(int a, int b);
void add_in_enclave(int num1, int num2, int* sum, uint32_t len);
void sub_in_enclave(int num1, int num2, int* sub, uint32_t len);
void e_call_hello(const char* str);
void e_call_bytes(const char* str, unsigned int n);
void e_call_setup_connection(void);
void ecall_update_hash(char* buffer);
void ecall_setup_sha256(void);

sgx_status_t SGX_CDECL ocall_print_string(const char* str);
sgx_status_t SGX_CDECL ocall_print_bytes(const char* str, unsigned int n);
sgx_status_t SGX_CDECL ocall_setup_connection(void);
sgx_status_t SGX_CDECL ocall_print_hash(const char* str, unsigned int n);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
