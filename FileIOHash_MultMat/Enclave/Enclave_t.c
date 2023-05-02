#include "Enclave_t.h"

#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */
#include "sgx_lfence.h" /* for sgx_lfence */

#include <errno.h>
#include <mbusafecrt.h> /* for memcpy_s etc */
#include <stdlib.h> /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz) do {	\
	if (!(ptr) || ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_ENCLAVE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_within_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define ADD_ASSIGN_OVERFLOW(a, b) (	\
	((a) += (b)) < (b)	\
)


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

static sgx_status_t SGX_CDECL sgx_ecall_matmul_u(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_matmul_u_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_matmul_u_t* ms = SGX_CAST(ms_ecall_matmul_u_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	float* _tmp_a = ms->ms_a;
	size_t _len_a = 65536 * sizeof(float);
	float* _in_a = NULL;
	float* _tmp_b = ms->ms_b;
	size_t _len_b = 65536 * sizeof(float);
	float* _in_b = NULL;
	float* _tmp_c = ms->ms_c;

	CHECK_UNIQUE_POINTER(_tmp_a, _len_a);
	CHECK_UNIQUE_POINTER(_tmp_b, _len_b);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_a != NULL && _len_a != 0) {
		if ( _len_a % sizeof(*_tmp_a) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_a = (float*)malloc(_len_a);
		if (_in_a == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_a, _len_a, _tmp_a, _len_a)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	if (_tmp_b != NULL && _len_b != 0) {
		if ( _len_b % sizeof(*_tmp_b) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_b = (float*)malloc(_len_b);
		if (_in_b == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_b, _len_b, _tmp_b, _len_b)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}

	ecall_matmul_u(_in_a, _in_b, _tmp_c, ms->ms_NB);

err:
	if (_in_a) free(_in_a);
	if (_in_b) free(_in_b);
	return status;
}

static sgx_status_t SGX_CDECL sgx_mult_enclave(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_mult_enclave_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_mult_enclave_t* ms = SGX_CAST(ms_mult_enclave_t*, pms);
	sgx_status_t status = SGX_SUCCESS;



	ms->ms_retval = mult_enclave(ms->ms_a, ms->ms_b);


	return status;
}

static sgx_status_t SGX_CDECL sgx_add_enclave(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_add_enclave_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_add_enclave_t* ms = SGX_CAST(ms_add_enclave_t*, pms);
	sgx_status_t status = SGX_SUCCESS;



	ms->ms_retval = add_enclave(ms->ms_a, ms->ms_b);


	return status;
}

static sgx_status_t SGX_CDECL sgx_add_in_enclave(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_add_in_enclave_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_add_in_enclave_t* ms = SGX_CAST(ms_add_in_enclave_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	int* _tmp_sum = ms->ms_sum;
	uint32_t _tmp_len = ms->ms_len;
	size_t _len_sum = _tmp_len;
	int* _in_sum = NULL;

	CHECK_UNIQUE_POINTER(_tmp_sum, _len_sum);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_sum != NULL && _len_sum != 0) {
		if ( _len_sum % sizeof(*_tmp_sum) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		if ((_in_sum = (int*)malloc(_len_sum)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_sum, 0, _len_sum);
	}

	add_in_enclave(ms->ms_num1, ms->ms_num2, _in_sum, _tmp_len);
	if (_in_sum) {
		if (memcpy_s(_tmp_sum, _len_sum, _in_sum, _len_sum)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}

err:
	if (_in_sum) free(_in_sum);
	return status;
}

static sgx_status_t SGX_CDECL sgx_sub_in_enclave(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_sub_in_enclave_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_sub_in_enclave_t* ms = SGX_CAST(ms_sub_in_enclave_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	int* _tmp_sub = ms->ms_sub;
	uint32_t _tmp_len = ms->ms_len;
	size_t _len_sub = _tmp_len;
	int* _in_sub = NULL;

	CHECK_UNIQUE_POINTER(_tmp_sub, _len_sub);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_sub != NULL && _len_sub != 0) {
		if ( _len_sub % sizeof(*_tmp_sub) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		if ((_in_sub = (int*)malloc(_len_sub)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_sub, 0, _len_sub);
	}

	sub_in_enclave(ms->ms_num1, ms->ms_num2, _in_sub, _tmp_len);
	if (_in_sub) {
		if (memcpy_s(_tmp_sub, _len_sub, _in_sub, _len_sub)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}

err:
	if (_in_sub) free(_in_sub);
	return status;
}

static sgx_status_t SGX_CDECL sgx_e_call_hello(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_e_call_hello_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_e_call_hello_t* ms = SGX_CAST(ms_e_call_hello_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	const char* _tmp_str = ms->ms_str;
	size_t _len_str = ms->ms_str_len ;
	char* _in_str = NULL;

	CHECK_UNIQUE_POINTER(_tmp_str, _len_str);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_str != NULL && _len_str != 0) {
		_in_str = (char*)malloc(_len_str);
		if (_in_str == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_str, _len_str, _tmp_str, _len_str)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

		_in_str[_len_str - 1] = '\0';
		if (_len_str != strlen(_in_str) + 1)
		{
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}

	e_call_hello((const char*)_in_str);

err:
	if (_in_str) free(_in_str);
	return status;
}

static sgx_status_t SGX_CDECL sgx_e_call_bytes(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_e_call_bytes_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_e_call_bytes_t* ms = SGX_CAST(ms_e_call_bytes_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	const char* _tmp_str = ms->ms_str;
	unsigned int _tmp_n = ms->ms_n;
	size_t _len_str = _tmp_n;
	char* _in_str = NULL;

	CHECK_UNIQUE_POINTER(_tmp_str, _len_str);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_str != NULL && _len_str != 0) {
		if ( _len_str % sizeof(*_tmp_str) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_str = (char*)malloc(_len_str);
		if (_in_str == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_str, _len_str, _tmp_str, _len_str)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}

	e_call_bytes((const char*)_in_str, _tmp_n);

err:
	if (_in_str) free(_in_str);
	return status;
}

static sgx_status_t SGX_CDECL sgx_e_call_setup_connection(void* pms)
{
	sgx_status_t status = SGX_SUCCESS;
	if (pms != NULL) return SGX_ERROR_INVALID_PARAMETER;
	e_call_setup_connection();
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_update_hash(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_update_hash_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_update_hash_t* ms = SGX_CAST(ms_ecall_update_hash_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	char* _tmp_buffer = ms->ms_buffer;
	size_t _len_buffer = sizeof(char);
	char* _in_buffer = NULL;

	CHECK_UNIQUE_POINTER(_tmp_buffer, _len_buffer);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_buffer != NULL && _len_buffer != 0) {
		if ( _len_buffer % sizeof(*_tmp_buffer) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_buffer = (char*)malloc(_len_buffer);
		if (_in_buffer == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_buffer, _len_buffer, _tmp_buffer, _len_buffer)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}

	ecall_update_hash(_in_buffer);

err:
	if (_in_buffer) free(_in_buffer);
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_setup_sha256(void* pms)
{
	sgx_status_t status = SGX_SUCCESS;
	if (pms != NULL) return SGX_ERROR_INVALID_PARAMETER;
	ecall_setup_sha256();
	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* ecall_addr; uint8_t is_priv; uint8_t is_switchless;} ecall_table[10];
} g_ecall_table = {
	10,
	{
		{(void*)(uintptr_t)sgx_ecall_matmul_u, 0, 0},
		{(void*)(uintptr_t)sgx_mult_enclave, 0, 0},
		{(void*)(uintptr_t)sgx_add_enclave, 0, 0},
		{(void*)(uintptr_t)sgx_add_in_enclave, 0, 0},
		{(void*)(uintptr_t)sgx_sub_in_enclave, 0, 0},
		{(void*)(uintptr_t)sgx_e_call_hello, 0, 0},
		{(void*)(uintptr_t)sgx_e_call_bytes, 0, 0},
		{(void*)(uintptr_t)sgx_e_call_setup_connection, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_update_hash, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_setup_sha256, 0, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[4][10];
} g_dyn_entry_table = {
	4,
	{
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
	}
};


sgx_status_t SGX_CDECL ocall_print_string(const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_print_string_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_string_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(str, _len_str);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_string_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_string_t));
	ocalloc_size -= sizeof(ms_ocall_print_string_t);

	if (str != NULL) {
		ms->ms_str = (const char*)__tmp;
		if (_len_str % sizeof(*str) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, str, _len_str)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_str);
		ocalloc_size -= _len_str;
	} else {
		ms->ms_str = NULL;
	}
	
	status = sgx_ocall(0, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_print_bytes(const char* str, unsigned int n)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = n;

	ms_ocall_print_bytes_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_bytes_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(str, _len_str);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_bytes_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_bytes_t));
	ocalloc_size -= sizeof(ms_ocall_print_bytes_t);

	if (str != NULL) {
		ms->ms_str = (const char*)__tmp;
		if (_len_str % sizeof(*str) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, str, _len_str)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_str);
		ocalloc_size -= _len_str;
	} else {
		ms->ms_str = NULL;
	}
	
	ms->ms_n = n;
	status = sgx_ocall(1, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_setup_connection(void)
{
	sgx_status_t status = SGX_SUCCESS;
	status = sgx_ocall(2, NULL);

	return status;
}
sgx_status_t SGX_CDECL ocall_print_hash(const char* str, unsigned int n)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = n;

	ms_ocall_print_hash_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_hash_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(str, _len_str);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_hash_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_hash_t));
	ocalloc_size -= sizeof(ms_ocall_print_hash_t);

	if (str != NULL) {
		ms->ms_str = (const char*)__tmp;
		if (_len_str % sizeof(*str) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, str, _len_str)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_str);
		ocalloc_size -= _len_str;
	} else {
		ms->ms_str = NULL;
	}
	
	ms->ms_n = n;
	status = sgx_ocall(3, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

