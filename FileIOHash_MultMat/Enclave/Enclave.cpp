/*
 * Copyright (C) 2011-2018 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "sgx_tcrypto.h"
#include "stdlib.h"

#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include "Enclave.h"

#ifndef NOENCLAVE
# include "Enclave_t.h"  /* print_string */
#endif

#define SGX_SHA256_HASH_SIZE 32
/*
 * printf:
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
#ifndef NOENCLAVE
void printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
}
#endif

sgx_sha_state_handle_t ctx = NULL;
sgx_status_t sgx_ret = SGX_SUCCESS;

void ecall_setup_sha256()
{
  sgx_ret = sgx_sha256_init(&ctx);
}

void ecall_matmul_u(float a[256*256],
                    float b[256*256],
                    float c[256*256], int NB)
{
  int i, j, k, I;
  double tmp;
  for (i = 0; i < NB; i++)
  {
    I=i*NB;
    for (j = 0; j < NB; j++)
    {
      tmp=c[I+j];
      for (k = 0; k < NB; k++)
      {
        tmp+=a[I+k]*b[k*NB+j];
      }
      c[I+j]=tmp;
    }
  }
}

int add_enclave(int a, int b){
        return a + b;
    }

    int mult_enclave(int a, int b){
            return a * b;
        }

void add_in_enclave(int num1, int num2, int *sum, uint32_t len)
{
    printf("\nEntered add_in_enclave ecall\n");
    *sum = num1 + num2;
    printf("Computed %i + %i = %i\n", num1, num2, *sum);

    return;
}

void sub_in_enclave(int num1, int num2, int *sub, uint32_t len)
{
    printf("Entered sub_in_enclave ecall\n");
    *sub = num1 - num2;
    printf("Computed %i - %i = %i\n", num1, num2, *sub);

    return;
}

void e_call_hello(const char* str)
{
  ocall_print_string(str);
}

 void e_call_setup_connection(){
   ocall_setup_connection();
 }

void e_call_bytes(const char* str, unsigned int n)
{
  ocall_print_bytes(str, n);
}
/* Updates sha256 has calculation based on the input message
* Parameters:
*   Return: sgx_status_t  - SGX_SUCCESS or failure as defined in sgx_error.
*   Input:  sgx_sha_state_handle_t sha_handle - Handle to the SHA256 state
*           uint8_t *p_src - Pointer to the input stream to be hashed
*           uint32_t src_len - Length of the input stream to be hashed  */

void ecall_update_hash(char *buffer)
{
  sgx_ret = sgx_sha256_update((const uint8_t *)(&buffer),
                                        sizeof(buffer),
                                        ctx);
  if(ctx)
  {
    sgx_status_t ret = sgx_sha256_close(ctx);
    sgx_ret = (sgx_ret != SGX_SUCCESS)? sgx_ret : ret;
  }

  char* buffer_p = new char[SGX_SHA256_HASH_SIZE];

  sgx_ret = sgx_sha256_get_hash(ctx, (sgx_sha256_hash_t*)buffer_p);

  //ocall_print_bytes((char*)buffer_p, sizeof(buffer_p));
  ocall_print_hash((char*)buffer_p, sizeof(buffer_p));
  //delete[] buffer_p;

}
