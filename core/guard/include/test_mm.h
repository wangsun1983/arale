#ifndef __TEST_MM_H__
#define __TEST_MM_H__
#include "klibc.h"

#define TEST_MEMORY_SMALL 8
#define TEST_MEMORY_MEDIUM 1024*4
#define TEST_MEMORY_LARGE 1024*1024

#define TEST_LOOPS 1

int start_test_mm();

int test_vmalloc_1();
int test_vmalloc_2();
int test_vmalloc_3();

int test_kmalloc_1();
int test_kmalloc_2();
int test_kmalloc_3();
int test_kmalloc_4();

int test_pmalloc_1();
int test_pmalloc_2();
int test_pmalloc_3();

#endif
