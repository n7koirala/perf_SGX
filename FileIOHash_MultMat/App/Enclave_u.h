#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_status_t etc. */


#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OCALL_PRINT_STRING_DEFINED__
#define OCALL_PRINT_STRING_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_string, (const char* str));
#endif
#ifndef OCALL_PRINT_BYTES_DEFINED__
#define OCALL_PRINT_BYTES_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_bytes, (const char* str, unsigned int n));
#endif
#ifndef OCALL_SETUP_CONNECTION_DEFINED__
#define OCALL_SETUP_CONNECTION_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_setup_connection, (void));
#endif
#ifndef OCALL_PRINT_HASH_DEFINED__
#define OCALL_PRINT_HASH_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_hash, (const char* str, unsigned int n));
#endif

sgx_status_t ecall_matmul_u(sgx_enclave_id_t eid, float a[65536], float b[65536], float c[65536], int NB);
sgx_status_t mult_enclave(sgx_enclave_id_t eid, int* retval, int a, int b);
sgx_status_t add_enclave(sgx_enclave_id_t eid, int* retval, int a, int b);
sgx_status_t add_in_enclave(sgx_enclave_id_t eid, int num1, int num2, int* sum, uint32_t len);
sgx_status_t sub_in_enclave(sgx_enclave_id_t eid, int num1, int num2, int* sub, uint32_t len);
sgx_status_t e_call_hello(sgx_enclave_id_t eid, const char* str);
sgx_status_t e_call_bytes(sgx_enclave_id_t eid, const char* str, unsigned int n);
sgx_status_t e_call_setup_connection(sgx_enclave_id_t eid);
sgx_status_t ecall_update_hash(sgx_enclave_id_t eid, char* buffer);
sgx_status_t ecall_setup_sha256(sgx_enclave_id_t eid);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
