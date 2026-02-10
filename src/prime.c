#include <math.h>

#include "prime.h"

// Method to check if number is prime
    // Prime -> return 1
    // Not Prime -> return 0
    // Undefined -) return -1
int is_prime(const int x) {
    // First prime numbers are 2 and 3
    if (x < 2) { return -1; }
    if (x < 4) { return 1; }
    // Prime numbers > 3 should not be divisible by 2
    if ((x % 2) == 0) { return 0; }
    // Check ODD divisors from 3 to sqrt(x)
        // Ex. If x = 100, the largest divisor to check is 10
        // Ex. If x = 49, the largest divisor to check is 7
    for (int i = 3; i * i <= x; i += 2) {
        // If x is divisible by any value in this range, it is NOT a prime number
        if ((x % i) == 0) {
            return 0;
        }
    }
    // After the loop check, number is prime
    return 1;
}

// Method to compute next prime number based on input
int next_prime(int x) {
    while (is_prime(x) != 1) {
        x++;
    }
    return x;
}
