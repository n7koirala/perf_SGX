#include "Enclave_u.h"
#include <errno.h>

typedef struct ms_ecall_matmul_u_t {
	float* ms_a;
	float* ms_b;
	float* ms_c;
	int ms_NB;
} ms_ecall_matmul_u_t;

typedef struct ms_mult_enclave_t {
	int ms_retval;
	int ms_a;
	int ms_b;
} ms_mult_enclave_t;

typedef struct ms_add_enclave_t {
	int ms_retval;
	int ms_a;
	int ms_b;
} ms_add_enclave_t;

typedef struct ms_add_in_enclave_t {
	int ms_num1;
	int ms_num2;
	int* ms_sum;
	uint32_t ms_len;
} ms_add_in_enclave_t;

typedef struct ms_sub_in_enclave_t {
	int ms_num1;
	int ms_num2;
	int* ms_sub;
	uint32_t ms_len;
} ms_sub_in_enclave_t;

typedef struct ms_e_call_hello_t {
	const char* ms_str;
	size_t ms_str_len;
} ms_e_call_hello_t;

typedef struct ms_e_call_bytes_t {
	const char* ms_str;
	unsigned int ms_n;
} ms_e_call_bytes_t;

typedef struct ms_ecall_update_hash_t {
	char* ms_buffer;
} ms_ecall_update_hash_t;

typedef struct ms_ocall_print_string_t {
	const char* ms_str;
} ms_ocall_print_string_t;

typedef struct ms_ocall_print_bytes_t {
	const char* ms_str;
	unsigned int ms_n;
} ms_ocall_print_bytes_t;

typedef struct ms_ocall_print_hash_t {
	const char* ms_str;
	unsigned int ms_n;
} ms_ocall_print_hash_t;

static sgx_status_t SGX_CDECL Enclave_ocall_print_string(void* pms)
{
	ms_ocall_print_string_t* ms = SGX_CAST(ms_ocall_print_string_t*, pms);
	ocall_print_string(ms->ms_str);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_print_bytes(void* pms)
{
	ms_ocall_print_bytes_t* ms = SGX_CAST(ms_ocall_print_bytes_t*, pms);
	ocall_print_bytes(ms->ms_str, ms->ms_n);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_setup_connection(void* pms)
{
	if (pms != NULL) return SGX_ERROR_INVALID_PARAMETER;
	ocall_setup_connection();
	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_print_hash(void* pms)
{
	ms_ocall_print_hash_t* ms = SGX_CAST(ms_ocall_print_hash_t*, pms);
	ocall_print_hash(ms->ms_str, ms->ms_n);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[4];
} ocall_table_Enclave = {
	4,
	{
		(void*)Enclave_ocall_print_string,
		(void*)Enclave_ocall_print_bytes,
		(void*)Enclave_ocall_setup_connection,
		(void*)Enclave_ocall_print_hash,
	}
};
sgx_status_t ecall_matmul_u(sgx_enclave_id_t eid, float a[65536], float b[65536], float c[65536], int NB)
{
	sgx_status_t status;
	ms_ecall_matmul_u_t ms;
	ms.ms_a = (float*)a;
	ms.ms_b = (float*)b;
	ms.ms_c = (float*)c;
	ms.ms_NB = NB;
	status = sgx_ecall(eid, 0, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t mult_enclave(sgx_enclave_id_t eid, int* retval, int a, int b)
{
	sgx_status_t status;
	ms_mult_enclave_t ms;
	ms.ms_a = a;
	ms.ms_b = b;
	status = sgx_ecall(eid, 1, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t add_enclave(sgx_enclave_id_t eid, int* retval, int a, int b)
{
	sgx_status_t status;
	ms_add_enclave_t ms;
	ms.ms_a = a;
	ms.ms_b = b;
	status = sgx_ecall(eid, 2, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t add_in_enclave(sgx_enclave_id_t eid, int num1, int num2, int* sum, uint32_t len)
{
	sgx_status_t status;
	ms_add_in_enclave_t ms;
	ms.ms_num1 = num1;
	ms.ms_num2 = num2;
	ms.ms_sum = sum;
	ms.ms_len = len;
	status = sgx_ecall(eid, 3, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t sub_in_enclave(sgx_enclave_id_t eid, int num1, int num2, int* sub, uint32_t len)
{
	sgx_status_t status;
	ms_sub_in_enclave_t ms;
	ms.ms_num1 = num1;
	ms.ms_num2 = num2;
	ms.ms_sub = sub;
	ms.ms_len = len;
	status = sgx_ecall(eid, 4, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t e_call_hello(sgx_enclave_id_t eid, const char* str)
{
	sgx_status_t status;
	ms_e_call_hello_t ms;
	ms.ms_str = str;
	ms.ms_str_len = str ? strlen(str) + 1 : 0;
	status = sgx_ecall(eid, 5, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t e_call_bytes(sgx_enclave_id_t eid, const char* str, unsigned int n)
{
	sgx_status_t status;
	ms_e_call_bytes_t ms;
	ms.ms_str = str;
	ms.ms_n = n;
	status = sgx_ecall(eid, 6, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t e_call_setup_connection(sgx_enclave_id_t eid)
{
	sgx_status_t status;
	status = sgx_ecall(eid, 7, &ocall_table_Enclave, NULL);
	return status;
}

sgx_status_t ecall_update_hash(sgx_enclave_id_t eid, char* buffer)
{
	sgx_status_t status;
	ms_ecall_update_hash_t ms;
	ms.ms_buffer = buffer;
	status = sgx_ecall(eid, 8, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t ecall_setup_sha256(sgx_enclave_id_t eid)
{
	sgx_status_t status;
	status = sgx_ecall(eid, 9, &ocall_table_Enclave, NULL);
	return status;
}

