#pragma once

void test(bool condition, const char* cond_str, const char* file, int line);

#define TEST(condition) test(condition, #condition, __FILE__, __LINE__);
