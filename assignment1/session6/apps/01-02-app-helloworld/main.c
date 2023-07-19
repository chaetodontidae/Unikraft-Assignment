#include <uk/test.h>

int factorial(int n) {
  int result = 1;
  for (int i = 1; i <= n; i++) {
    result *= i;
  }

  return result;
}

int is_prime(int n){
    if (n < 2) {
        return 0;
    }
    for (int i = 2; i <= n/i; i++) {
         if (n % i == 0) {
             return 0;
         }
    }
    return 1;
}

UK_TESTCASE(factorial_testsuite, factorial_test_positive)
{
       UK_TEST_EXPECT_SNUM_EQ(factorial(2), 2);
}

UK_TESTCASE(prime_testsuite, prime_test_positive)
{
       UK_TEST_EXPECT_SNUM_EQ(is_prime(1), 0);
       UK_TEST_EXPECT_SNUM_EQ(is_prime(2), 1);
       UK_TEST_EXPECT_SNUM_EQ(is_prime(3), 1);
       UK_TEST_EXPECT_SNUM_EQ(is_prime(4), 0);
       UK_TEST_EXPECT_SNUM_EQ(is_prime(5), 1);
       UK_TEST_EXPECT_SNUM_EQ(is_prime(6), 0);
       UK_TEST_EXPECT_SNUM_EQ(is_prime(7), 1);
       UK_TEST_EXPECT_SNUM_EQ(is_prime(8), 0);
       UK_TEST_EXPECT_SNUM_EQ(is_prime(9), 0);
}

uk_testsuite_register(factorial_testsuite, NULL);

uk_testsuite_register(prime_testsuite, NULL);

int main(int argc, char **argv) {
    return 0;
}
