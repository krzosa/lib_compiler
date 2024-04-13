/*
The bigint code was written by Christoffer Lerno, he is the programmer
behind C3. He allowed me to use this code without any restrictions. Great guy!
You can check out C3 compiler: https://github.com/c3lang/c3c
He also writes very helpful blogs about compilers: https://c3.handmade.network/blog
*/

#ifndef malloc_arena
    #define malloc_arena(size) LC_PushSize(L->arena, size)
#endif
#define ALLOC_DIGITS(_digits) ((_digits) ? (uint64_t *)malloc_arena(sizeof(uint64_t) * (_digits)) : NULL)

LC_FUNCTION uint32_t LC_u32_min(uint32_t a, uint32_t b) {
    return a < b ? a : b;
}

LC_FUNCTION size_t LC_size_max(size_t a, size_t b) {
    return a > b ? a : b;
}

LC_FUNCTION unsigned LC_unsigned_max(unsigned a, unsigned b) {
    return a > b ? a : b;
}

LC_FUNCTION uint64_t *LC_Bigint_ptr(LC_BigInt *big_int) {
    return big_int->digit_count == 1 ? &big_int->digit : big_int->digits;
}

LC_FUNCTION LC_BigInt LC_Bigint_u64(uint64_t val) {
    LC_BigInt result = {0};
    LC_Bigint_init_unsigned(&result, val);
    return result;
}

LC_FUNCTION void LC_normalize(LC_BigInt *big_int) {
    uint64_t *digits        = LC_Bigint_ptr(big_int);
    unsigned  last_non_zero = UINT32_MAX;
    for (unsigned i = 0; i < big_int->digit_count; i++) {
        if (digits[i] != 0) {
            last_non_zero = i;
        }
    }
    if (last_non_zero == UINT32_MAX) {
        big_int->is_negative = false;
        big_int->digit_count = 0;
        return;
    }
    big_int->digit_count = last_non_zero + 1;
    if (!last_non_zero) {
        big_int->digit = digits[0];
    }
}

LC_FUNCTION char LC_digit_to_char(uint8_t digit, bool upper) {
    if (digit <= 9) {
        return (char)(digit + '0');
    }
    if (digit <= 35) {
        return (char)(digit + (upper ? 'A' : 'a') - 10);
    }
    LC_ASSERT(NULL, !"Can't reach");
    return 0;
}

LC_FUNCTION bool LC_bit_at_index(LC_BigInt *big_int, size_t index) {
    size_t digit_index = index / 64;
    if (digit_index >= big_int->digit_count) {
        return false;
    }
    size_t    digit_bit_index = index % 64;
    uint64_t *digits          = LC_Bigint_ptr(big_int);
    uint64_t  digit           = digits[digit_index];
    return ((digit >> digit_bit_index) & 0x1U) == 0x1U;
}

LC_FUNCTION size_t LC_Bigint_bits_needed(LC_BigInt *big_int) {
    size_t full_bits          = big_int->digit_count * 64;
    size_t leading_zero_count = LC_Bigint_clz(big_int, full_bits);
    size_t bits_needed        = full_bits - leading_zero_count;
    return bits_needed + big_int->is_negative;
}

LC_FUNCTION void LC_Bigint_init_unsigned(LC_BigInt *big_int, uint64_t value) {
    if (value == 0) {
        big_int->digit_count = 0;
        big_int->is_negative = false;
        return;
    }
    big_int->digit_count = 1;
    big_int->digit       = value;
    big_int->is_negative = false;
}

LC_FUNCTION void LC_Bigint_init_signed(LC_BigInt *dest, int64_t value) {
    if (value >= 0) {
        LC_Bigint_init_unsigned(dest, (uint64_t)value);
        return;
    }
    dest->is_negative = true;
    dest->digit_count = 1;
    dest->digit       = ((uint64_t)(-(value + 1))) + 1;
}

LC_FUNCTION void LC_Bigint_init_bigint(LC_BigInt *dest, LC_BigInt *src) {
    if (src->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }
    if (src->digit_count == 1) {
        dest->digit_count = 1;
        dest->digit       = src->digit;
        dest->is_negative = src->is_negative;
        return;
    }
    dest->is_negative = src->is_negative;
    dest->digit_count = src->digit_count;
    dest->digits      = ALLOC_DIGITS(dest->digit_count);
    LC_MemoryCopy(dest->digits, src->digits, sizeof(uint64_t) * dest->digit_count);
}

LC_FUNCTION void LC_Bigint_negate(LC_BigInt *dest, LC_BigInt *source) {
    LC_Bigint_init_bigint(dest, source);
    dest->is_negative = !dest->is_negative;
    LC_normalize(dest);
}

LC_FUNCTION void LC_to_twos_complement(LC_BigInt *dest, LC_BigInt *source, size_t bit_count) {
    if (bit_count == 0 || source->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }
    if (source->is_negative) {
        LC_BigInt negated = {0};
        LC_Bigint_negate(&negated, source);

        LC_BigInt inverted = {0};
        LC_Bigint_not(&inverted, &negated, bit_count, false);

        LC_BigInt one = {0};
        LC_Bigint_init_unsigned(&one, 1);

        LC_Bigint_add(dest, &inverted, &one);
        return;
    }

    dest->is_negative       = false;
    uint64_t *source_digits = LC_Bigint_ptr(source);
    if (source->digit_count == 1) {
        dest->digit = source_digits[0];
        if (bit_count < 64) {
            dest->digit &= (1ULL << bit_count) - 1;
        }
        dest->digit_count = 1;
        LC_normalize(dest);
        return;
    }
    unsigned digits_to_copy = (unsigned int)(bit_count / 64);
    unsigned leftover_bits  = (unsigned int)(bit_count % 64);
    dest->digit_count       = digits_to_copy + ((leftover_bits == 0) ? 0 : 1);
    if (dest->digit_count == 1 && leftover_bits == 0) {
        dest->digit = source_digits[0];
        if (dest->digit == 0) dest->digit_count = 0;
        return;
    }
    dest->digits = (uint64_t *)malloc_arena(dest->digit_count * sizeof(uint64_t));
    for (size_t i = 0; i < digits_to_copy; i += 1) {
        uint64_t digit  = (i < source->digit_count) ? source_digits[i] : 0;
        dest->digits[i] = digit;
    }
    if (leftover_bits != 0) {
        uint64_t digit               = (digits_to_copy < source->digit_count) ? source_digits[digits_to_copy] : 0;
        dest->digits[digits_to_copy] = digit & ((1ULL << leftover_bits) - 1);
    }
    LC_normalize(dest);
}

LC_FUNCTION size_t LC_Bigint_clz(LC_BigInt *big_int, size_t bit_count) {
    if (big_int->is_negative || bit_count == 0) {
        return 0;
    }
    if (big_int->digit_count == 0) {
        return bit_count;
    }
    size_t count = 0;
    for (size_t i = bit_count - 1;;) {
        if (LC_bit_at_index(big_int, i)) {
            return count;
        }
        count++;
        if (i == 0) break;
        i--;
    }
    return count;
}

LC_FUNCTION bool LC_Bigint_eql(LC_BigInt a, LC_BigInt b) {
    return LC_Bigint_cmp(&a, &b) == LC_CmpRes_EQ;
}

LC_FUNCTION void LC_from_twos_complement(LC_BigInt *dest, LC_BigInt *src, size_t bit_count, bool is_signed) {
    LC_ASSERT(NULL, !src->is_negative);

    if (bit_count == 0 || src->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }

    if (is_signed && LC_bit_at_index(src, bit_count - 1)) {
        LC_BigInt negative_one = {0};
        LC_Bigint_init_signed(&negative_one, -1);

        LC_BigInt minus_one = {0};
        LC_Bigint_add(&minus_one, src, &negative_one);

        LC_BigInt inverted = {0};
        LC_Bigint_not(&inverted, &minus_one, bit_count, false);

        LC_Bigint_negate(dest, &inverted);
        return;
    }

    LC_Bigint_init_bigint(dest, src);
}

void LC_Bigint_init_data(LC_BigInt *dest, uint64_t *digits, unsigned int digit_count, bool is_negative) {
    if (digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }

    if (digit_count == 1) {
        dest->digit_count = 1;
        dest->digit       = digits[0];
        dest->is_negative = is_negative;
        LC_normalize(dest);
        return;
    }

    dest->digit_count = digit_count;
    dest->is_negative = is_negative;
    dest->digits      = ALLOC_DIGITS(digit_count);
    LC_MemoryCopy(dest->digits, digits, sizeof(uint64_t) * digit_count);

    LC_normalize(dest);
}

LC_FUNCTION bool LC_Bigint_fits_in_bits(LC_BigInt *big_int, size_t bit_count, bool is_signed) {
    LC_ASSERT(NULL, big_int->digit_count != 1 || big_int->digit != 0);
    if (bit_count == 0) {
        return LC_Bigint_cmp_zero(big_int) == LC_CmpRes_EQ;
    }
    if (big_int->digit_count == 0) {
        return true;
    }

    if (!is_signed) {
        size_t full_bits          = big_int->digit_count * 64;
        size_t leading_zero_count = LC_Bigint_clz(big_int, full_bits);
        return bit_count >= full_bits - leading_zero_count;
    }

    LC_BigInt one = {0};
    LC_Bigint_init_unsigned(&one, 1);

    LC_BigInt shl_amt = {0};
    LC_Bigint_init_unsigned(&shl_amt, bit_count - 1);

    LC_BigInt max_value_plus_one = {0};
    LC_Bigint_shl(&max_value_plus_one, &one, &shl_amt);

    LC_BigInt max_value = {0};
    LC_Bigint_sub(&max_value, &max_value_plus_one, &one);

    LC_BigInt min_value = {0};
    LC_Bigint_negate(&min_value, &max_value_plus_one);

    LC_CmpRes min_cmp = LC_Bigint_cmp(big_int, &min_value);
    LC_CmpRes max_cmp = LC_Bigint_cmp(big_int, &max_value);

    return (min_cmp == LC_CmpRes_GT || min_cmp == LC_CmpRes_EQ) && (max_cmp == LC_CmpRes_LT || max_cmp == LC_CmpRes_EQ);
}

LC_FUNCTION uint64_t LC_Bigint_as_unsigned(LC_BigInt *bigint) {
    LC_ASSERT(NULL, !bigint->is_negative);
    if (bigint->digit_count == 0) {
        return 0;
    }
    if (bigint->digit_count != 1) {
        LC_ASSERT(NULL, !"Bigint exceeds u64");
    }
    return bigint->digit;
}

#if defined(_MSC_VER)

LC_FUNCTION bool LC_add_u64_overflow(uint64_t op1, uint64_t op2, uint64_t *result) {
    *result = op1 + op2;
    return *result < op1 || *result < op2;
}

LC_FUNCTION bool LC_sub_u64_overflow(uint64_t op1, uint64_t op2, uint64_t *result) {
    *result = op1 - op2;
    return *result > op1;
}

LC_FUNCTION bool LC_mul_u64_overflow(uint64_t op1, uint64_t op2, uint64_t *result) {
    *result = op1 * op2;

    if (op1 == 0 || op2 == 0) return false;
    if (op1 > UINT64_MAX / op2) return true;
    if (op2 > UINT64_MAX / op1) return true;
    return false;
}

#else

LC_FUNCTION bool LC_add_u64_overflow(uint64_t op1, uint64_t op2, uint64_t *result) {
    return __builtin_uaddll_overflow((unsigned long long)op1, (unsigned long long)op2,
                                     (unsigned long long *)result);
}

LC_FUNCTION bool LC_sub_u64_overflow(uint64_t op1, uint64_t op2, uint64_t *result) {
    return __builtin_usubll_overflow((unsigned long long)op1, (unsigned long long)op2,
                                     (unsigned long long *)result);
}

LC_FUNCTION bool LC_mul_u64_overflow(uint64_t op1, uint64_t op2, uint64_t *result) {
    return __builtin_umulll_overflow((unsigned long long)op1, (unsigned long long)op2,
                                     (unsigned long long *)result);
}

#endif

LC_FUNCTION void LC_Bigint_add(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    if (op1->digit_count == 0) {
        LC_Bigint_init_bigint(dest, op2);
        return;
    }
    if (op2->digit_count == 0) {
        LC_Bigint_init_bigint(dest, op1);
        return;
    }
    if (op1->is_negative == op2->is_negative) {
        dest->is_negative = op1->is_negative;

        uint64_t *op1_digits = LC_Bigint_ptr(op1);
        uint64_t *op2_digits = LC_Bigint_ptr(op2);
        uint64_t  overflow   = LC_add_u64_overflow(op1_digits[0], op2_digits[0], &dest->digit);
        if (overflow == 0 && op1->digit_count == 1 && op2->digit_count == 1) {
            dest->digit_count = 1;
            LC_normalize(dest);
            return;
        }
        unsigned i           = 1;
        uint64_t first_digit = dest->digit;
        dest->digits         = ALLOC_DIGITS(LC_unsigned_max(op1->digit_count, op2->digit_count) + 1);
        dest->digits[0]      = first_digit;

        for (;;) {
            bool     found_digit = false;
            uint64_t x           = (uint64_t)overflow;
            overflow             = 0;

            if (i < op1->digit_count) {
                found_digit    = true;
                uint64_t digit = op1_digits[i];
                overflow += LC_add_u64_overflow(x, digit, &x);
            }

            if (i < op2->digit_count) {
                found_digit    = true;
                uint64_t digit = op2_digits[i];
                overflow += LC_add_u64_overflow(x, digit, &x);
            }

            dest->digits[i] = x;
            i += 1;

            if (!found_digit) {
                dest->digit_count = i;
                LC_normalize(dest);
                return;
            }
        }
    }
    LC_BigInt *op_pos;
    LC_BigInt *op_neg;
    if (op1->is_negative) {
        op_neg = op1;
        op_pos = op2;
    } else {
        op_pos = op1;
        op_neg = op2;
    }

    LC_BigInt op_neg_abs = {0};
    LC_Bigint_negate(&op_neg_abs, op_neg);
    LC_BigInt *bigger_op;
    LC_BigInt *smaller_op;
    switch (LC_Bigint_cmp(op_pos, &op_neg_abs)) {
    case LC_CmpRes_EQ:
        LC_Bigint_init_unsigned(dest, 0);
        return;
    case LC_CmpRes_LT:
        bigger_op         = &op_neg_abs;
        smaller_op        = op_pos;
        dest->is_negative = true;
        break;
    case LC_CmpRes_GT:
        bigger_op         = op_pos;
        smaller_op        = &op_neg_abs;
        dest->is_negative = false;
        break;
    default:
        LC_ASSERT(NULL, !"UNREACHABLE");
    }
    uint64_t *bigger_op_digits  = LC_Bigint_ptr(bigger_op);
    uint64_t *smaller_op_digits = LC_Bigint_ptr(smaller_op);
    uint64_t  overflow          = (uint64_t)LC_sub_u64_overflow(bigger_op_digits[0], smaller_op_digits[0], &dest->digit);
    if (overflow == 0 && bigger_op->digit_count == 1 && smaller_op->digit_count == 1) {
        dest->digit_count = 1;
        LC_normalize(dest);
        return;
    }
    uint64_t first_digit = dest->digit;
    dest->digits         = ALLOC_DIGITS(bigger_op->digit_count);
    dest->digits[0]      = first_digit;
    unsigned i           = 1;

    for (;;) {
        bool     found_digit   = false;
        uint64_t x             = bigger_op_digits[i];
        uint64_t prev_overflow = overflow;
        overflow               = 0;

        if (i < smaller_op->digit_count) {
            found_digit    = true;
            uint64_t digit = smaller_op_digits[i];
            overflow += LC_sub_u64_overflow(x, digit, &x);
        }
        if (LC_sub_u64_overflow(x, prev_overflow, &x)) {
            found_digit = true;
            overflow += 1;
        }
        dest->digits[i] = x;
        i += 1;

        if (!found_digit || i >= bigger_op->digit_count) {
            break;
        }
    }
    LC_ASSERT(NULL, overflow == 0);
    dest->digit_count = i;
    LC_normalize(dest);
}

LC_FUNCTION void LC_Bigint_add_wrap(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2, size_t bit_count, bool is_signed) {
    LC_BigInt unwrapped = {0};
    LC_Bigint_add(&unwrapped, op1, op2);
    LC_Bigint_truncate(dest, &unwrapped, bit_count, is_signed);
}

LC_FUNCTION void LC_Bigint_sub(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    LC_BigInt op2_negated = {0};
    LC_Bigint_negate(&op2_negated, op2);
    LC_Bigint_add(dest, op1, &op2_negated);
    return;
}

LC_FUNCTION void LC_Bigint_sub_wrap(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2, size_t bit_count, bool is_signed) {
    LC_BigInt op2_negated = {0};
    LC_Bigint_negate(&op2_negated, op2);
    LC_Bigint_add_wrap(dest, op1, &op2_negated, bit_count, is_signed);
    return;
}

LC_FUNCTION void mul_overflow(uint64_t op1, uint64_t op2, uint64_t *lo, uint64_t *hi) {
    uint64_t u1 = (op1 & 0xffffffff);
    uint64_t v1 = (op2 & 0xffffffff);
    uint64_t t  = (u1 * v1);
    uint64_t w3 = (t & 0xffffffff);
    uint64_t k  = (t >> 32);

    op1 >>= 32;
    t           = (op1 * v1) + k;
    k           = (t & 0xffffffff);
    uint64_t w1 = (t >> 32);

    op2 >>= 32;
    t = (u1 * op2) + k;
    k = (t >> 32);

    *hi = (op1 * op2) + w1 + k;
    *lo = (t << 32) + w3;
}

LC_FUNCTION void LC_mul_scalar(LC_BigInt *dest, LC_BigInt *op, uint64_t scalar) {
    LC_Bigint_init_unsigned(dest, 0);

    LC_BigInt bi_64;
    LC_Bigint_init_unsigned(&bi_64, 64);

    uint64_t *op_digits = LC_Bigint_ptr(op);
    size_t    i         = op->digit_count - 1;

    while (1) {
        LC_BigInt shifted;
        LC_Bigint_shl(&shifted, dest, &bi_64);

        uint64_t result_scalar;
        uint64_t carry_scalar;
        mul_overflow(scalar, op_digits[i], &result_scalar, &carry_scalar);

        LC_BigInt result;
        LC_Bigint_init_unsigned(&result, result_scalar);

        LC_BigInt carry;
        LC_Bigint_init_unsigned(&carry, carry_scalar);

        LC_BigInt carry_shifted;
        LC_Bigint_shl(&carry_shifted, &carry, &bi_64);

        LC_BigInt tmp;
        LC_Bigint_add(&tmp, &shifted, &carry_shifted);

        LC_Bigint_add(dest, &tmp, &result);

        if (i == 0) {
            break;
        }
        i -= 1;
    }
}

LC_FUNCTION void LC_Bigint_mul(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    if (op1->digit_count == 0 || op2->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }
    uint64_t *op1_digits = LC_Bigint_ptr(op1);
    uint64_t *op2_digits = LC_Bigint_ptr(op2);

    uint64_t carry;
    mul_overflow(op1_digits[0], op2_digits[0], &dest->digit, &carry);
    if (carry == 0 && op1->digit_count == 1 && op2->digit_count == 1) {
        dest->is_negative = (op1->is_negative != op2->is_negative);
        dest->digit_count = 1;
        LC_normalize(dest);
        return;
    }

    LC_Bigint_init_unsigned(dest, 0);

    LC_BigInt bi_64;
    LC_Bigint_init_unsigned(&bi_64, 64);

    size_t i = op2->digit_count - 1;
    for (;;) {
        LC_BigInt shifted;
        LC_Bigint_shl(&shifted, dest, &bi_64);

        LC_BigInt scalar_result;
        LC_mul_scalar(&scalar_result, op1, op2_digits[i]);

        LC_Bigint_add(dest, &scalar_result, &shifted);

        if (i == 0) {
            break;
        }
        i -= 1;
    }

    dest->is_negative = (op1->is_negative != op2->is_negative);
    LC_normalize(dest);
}

LC_FUNCTION void LC_Bigint_mul_wrap(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2, size_t bit_count, bool is_signed) {
    LC_BigInt unwrapped = {0};
    LC_Bigint_mul(&unwrapped, op1, op2);
    LC_Bigint_truncate(dest, &unwrapped, bit_count, is_signed);
}

LC_FUNCTION unsigned LC_count_leading_zeros(uint32_t val) {
    if (val == 0) return 32;

#if _MSC_VER
    unsigned long Index;
    _BitScanReverse(&Index, val);
    return Index ^ 31;
#else
    return __builtin_clz(val);
#endif
}

// Make a 64-bit integer from a high / low pair of 32-bit integers.
LC_FUNCTION uint64_t LC_make_64(uint32_t hi, uint32_t lo) {
    return (((uint64_t)hi) << 32) | ((uint64_t)lo);
}

// Return the high 32 bits of a 64 bit value.
LC_FUNCTION uint32_t LC_hi_32(uint64_t value) {
    return (uint32_t)(value >> 32);
}

// Return the low 32 bits of a 64 bit value.
LC_FUNCTION uint32_t LC_lo_32(uint64_t val) {
    return (uint32_t)val;
}

// Implementation of Knuth's Algorithm D (Division of nonnegative integers)
// from "Art of Computer Programming, Volume 2", section 4.3.1, p. 272. The
// variables here have the same names as in the algorithm. Comments explain
// the algorithm and any deviation from it.
LC_FUNCTION void LC_knuth_div(uint32_t *u, uint32_t *v, uint32_t *q, uint32_t *r, unsigned m, unsigned n) {
    LC_ASSERT(NULL, u && "Must provide dividend");
    LC_ASSERT(NULL, v && "Must provide divisor");
    LC_ASSERT(NULL, q && "Must provide quotient");
    LC_ASSERT(NULL, u != v && u != q && v != q && "Must use different memory");
    LC_ASSERT(NULL, n > 1 && "n must be > 1");

    // b denotes the base of the number system. In our case b is 2^32.
    uint64_t b = ((uint64_t)1) << 32;

    // D1. [Normalize.] Set d = b / (v[n-1] + 1) and multiply all the digits of
    // u and v by d. Note that we have taken Knuth's advice here to use a power
    // of 2 value for d such that d * v[n-1] >= b/2 (b is the base). A power of
    // 2 allows us to shift instead of multiply and it is easy to determine the
    // shift amount from the leading zeros.  We are basically normalizing the u
    // and v so that its high bits are shifted to the top of v's range without
    // overflow. Note that this can require an extra word in u so that u must
    // be of length m+n+1.
    unsigned shift   = LC_count_leading_zeros(v[n - 1]);
    uint32_t v_carry = 0;
    uint32_t u_carry = 0;
    if (shift) {
        for (unsigned i = 0; i < m + n; ++i) {
            uint32_t u_tmp = u[i] >> (32 - shift);
            u[i]           = (u[i] << shift) | u_carry;
            u_carry        = u_tmp;
        }
        for (unsigned i = 0; i < n; ++i) {
            uint32_t v_tmp = v[i] >> (32 - shift);
            v[i]           = (v[i] << shift) | v_carry;
            v_carry        = v_tmp;
        }
    }
    u[m + n] = u_carry;

    // D2. [Initialize j.]  Set j to m. This is the loop counter over the places.
    int j = (int)m;
    do {
        // D3. [Calculate q'.].
        //     Set qp = (u[j+n]*b + u[j+n-1]) / v[n-1]. (qp=qprime=q')
        //     Set rp = (u[j+n]*b + u[j+n-1]) % v[n-1]. (rp=rprime=r')
        // Now test if qp == b or qp*v[n-2] > b*rp + u[j+n-2]; if so, decrease
        // qp by 1, increase rp by v[n-1], and repeat this test if rp < b. The test
        // on v[n-2] determines at high speed most of the cases in which the trial
        // value qp is one too large, and it eliminates all cases where qp is two
        // too large.
        uint64_t dividend = LC_make_64(u[j + n], u[j + n - 1]);
        uint64_t qp       = dividend / v[n - 1];
        uint64_t rp       = dividend % v[n - 1];
        if (qp == b || qp * v[n - 2] > b * rp + u[j + n - 2]) {
            qp--;
            rp += v[n - 1];
            if (rp < b && (qp == b || qp * v[n - 2] > b * rp + u[j + n - 2])) {
                qp--;
            }
        }

        // D4. [Multiply and subtract.] Replace (u[j+n]u[j+n-1]...u[j]) with
        // (u[j+n]u[j+n-1]..u[j]) - qp * (v[n-1]...v[1]v[0]). This computation
        // consists of a simple multiplication by a one-place number, combined with
        // a subtraction.
        // The digits (u[j+n]...u[j]) should be kept positive; if the result of
        // this step is actually negative, (u[j+n]...u[j]) should be left as the
        // true value plus b**(n+1), namely as the b's complement of
        // the true value, and a "borrow" to the left should be remembered.
        int64_t borrow = 0;
        for (unsigned i = 0; i < n; ++i) {
            uint64_t p      = ((uint64_t)qp) * ((uint64_t)(v[i]));
            int64_t  subres = ((int64_t)(u[j + i])) - borrow - LC_lo_32(p);
            u[j + i]        = LC_lo_32((uint64_t)subres);
            borrow          = LC_hi_32(p) - LC_hi_32((uint64_t)subres);
        }
        bool is_neg = u[j + n] < borrow;
        u[j + n] -= LC_lo_32((uint64_t)borrow);

        // D5. [Test remainder.] Set q[j] = qp. If the result of step D4 was
        // negative, go to step D6; otherwise go on to step D7.
        q[j] = LC_lo_32(qp);
        if (is_neg) {
            // D6. [Add back]. The probability that this step is necessary is very
            // small, on the order of only 2/b. Make sure that test data accounts for
            // this possibility. Decrease q[j] by 1
            q[j]--;
            // and add (0v[n-1]...v[1]v[0]) to (u[j+n]u[j+n-1]...u[j+1]u[j]).
            // A carry will occur to the left of u[j+n], and it should be ignored
            // since it cancels with the borrow that occurred in D4.
            bool carry = false;
            for (unsigned i = 0; i < n; i++) {
                uint32_t limit = LC_u32_min(u[j + i], v[i]);
                u[j + i] += v[i] + carry;
                carry = u[j + i] < limit || (carry && u[j + i] == limit);
            }
            u[j + n] += carry;
        }

        // D7. [Loop on j.]  Decrease j by one. Now if j >= 0, go back to D3.
    } while (--j >= 0);

    // D8. [Unnormalize]. Now q[...] is the desired quotient, and the desired
    // remainder may be obtained by dividing u[...] by d. If r is non-null we
    // compute the remainder (urem uses this).
    if (r) {
        // The value d is expressed by the "shift" value above since we avoided
        // multiplication by d by using a shift left. So, all we have to do is
        // shift right here.
        if (shift) {
            uint32_t carry = 0;
            for (int i = (int)n - 1; i >= 0; i--) {
                r[i]  = (u[i] >> shift) | carry;
                carry = u[i] << (32 - shift);
            }
        } else {
            for (int i = (int)n - 1; i >= 0; i--) {
                r[i] = u[i];
            }
        }
    }
}

LC_FUNCTION void LC_Bigint_unsigned_division(LC_BigInt *op1, LC_BigInt *op2, LC_BigInt *Quotient, LC_BigInt *Remainder) {
    LC_CmpRes cmp = LC_Bigint_cmp(op1, op2);
    if (cmp == LC_CmpRes_LT) {
        if (!Quotient) {
            LC_Bigint_init_unsigned(Quotient, 0);
        }
        if (!Remainder) {
            LC_Bigint_init_bigint(Remainder, op1);
        }
        return;
    }
    if (cmp == LC_CmpRes_EQ) {
        if (!Quotient) {
            LC_Bigint_init_unsigned(Quotient, 1);
        }
        if (!Remainder) {
            LC_Bigint_init_unsigned(Remainder, 0);
        }
        return;
    }

    uint64_t *lhs      = LC_Bigint_ptr(op1);
    uint64_t *rhs      = LC_Bigint_ptr(op2);
    unsigned  lhsWords = op1->digit_count;
    unsigned  rhsWords = op2->digit_count;

    // First, compose the values into an array of 32-bit words instead of
    // 64-bit words. This is a necessity of both the "short division" algorithm
    // and the Knuth "classical algorithm" which requires there to be native
    // operations for +, -, and * on an m bit value with an m*2 bit result. We
    // can't use 64-bit operands here because we don't have native results of
    // 128-bits. Furthermore, casting the 64-bit values to 32-bit values won't
    // work on large-endian machines.
    unsigned n = rhsWords * 2;
    unsigned m = (lhsWords * 2) - n;

    // Allocate space for the temporary values we need either on the stack, if
    // it will fit, or on the heap if it won't.
    uint32_t  space[128];
    uint32_t *U = NULL;
    uint32_t *V = NULL;
    uint32_t *Q = NULL;
    uint32_t *R = NULL;
    if ((Remainder ? 4 : 3) * n + 2 * m + 1 <= 128) {
        U = &space[0];
        V = &space[m + n + 1];
        Q = &space[(m + n + 1) + n];
        if (Remainder) {
            R = &space[(m + n + 1) + n + (m + n)];
        }
    } else {
        U = (uint32_t *)malloc_arena(sizeof(uint32_t) * (m + n + 1));
        V = (uint32_t *)malloc_arena(sizeof(uint32_t) * n);
        Q = (uint32_t *)malloc_arena(sizeof(uint32_t) * (m + n));
        if (Remainder) {
            R = (uint32_t *)malloc_arena(sizeof(uint32_t) * n);
        }
    }

    // Initialize the dividend
    LC_MemoryZero(U, (m + n + 1) * sizeof(uint32_t));
    for (unsigned i = 0; i < lhsWords; ++i) {
        uint64_t tmp = lhs[i];
        U[i * 2]     = LC_lo_32(tmp);
        U[i * 2 + 1] = LC_hi_32(tmp);
    }
    U[m + n] = 0; // this extra word is for "spill" in the Knuth algorithm.

    // Initialize the divisor
    LC_MemoryZero(V, (n) * sizeof(uint32_t));
    for (unsigned i = 0; i < rhsWords; ++i) {
        uint64_t tmp = rhs[i];
        V[i * 2]     = LC_lo_32(tmp);
        V[i * 2 + 1] = LC_hi_32(tmp);
    }

    // initialize the quotient and remainder
    LC_MemoryZero(Q, (m + n) * sizeof(uint32_t));
    if (Remainder) LC_MemoryZero(R, n * sizeof(uint32_t));

    // Now, adjust m and n for the Knuth division. n is the number of words in
    // the divisor. m is the number of words by which the dividend exceeds the
    // divisor (i.e. m+n is the length of the dividend). These sizes must not
    // contain any zero words or the Knuth algorithm fails.
    for (unsigned i = n; i > 0 && V[i - 1] == 0; i--) {
        n--;
        m++;
    }
    for (unsigned i = m + n; i > 0 && U[i - 1] == 0; i--) {
        m--;
    }

    // If we're left with only a single word for the divisor, Knuth doesn't work
    // so we implement the short division algorithm here. This is much simpler
    // and faster because we are certain that we can divide a 64-bit quantity
    // by a 32-bit quantity at hardware speed and short division is simply a
    // series of such operations. This is just like doing short division but we
    // are using base 2^32 instead of base 10.
    LC_ASSERT(NULL, n != 0 && "Divide by zero?");
    if (n == 1) {
        uint32_t divisor = V[0];
        uint32_t rem     = 0;
        for (int i = (int)m; i >= 0; i--) {
            uint64_t partial_dividend = LC_make_64(rem, U[i]);
            if (partial_dividend == 0) {
                Q[i] = 0;
                rem  = 0;
            } else if (partial_dividend < divisor) {
                Q[i] = 0;
                rem  = LC_lo_32(partial_dividend);
            } else if (partial_dividend == divisor) {
                Q[i] = 1;
                rem  = 0;
            } else {
                Q[i] = LC_lo_32(partial_dividend / divisor);
                rem  = LC_lo_32(partial_dividend - (Q[i] * divisor));
            }
        }
        if (R) {
            R[0] = rem;
        }
    } else {
        // Now we're ready to invoke the Knuth classical divide algorithm. In this
        // case n > 1.
        LC_knuth_div(U, V, Q, R, m, n);
    }

    // If the caller wants the quotient
    if (Quotient) {
        Quotient->is_negative = false;
        Quotient->digit_count = lhsWords;
        if (lhsWords == 1) {
            Quotient->digit = LC_make_64(Q[1], Q[0]);
        } else {
            Quotient->digits = ALLOC_DIGITS(lhsWords);
            for (size_t i = 0; i < lhsWords; i += 1) {
                Quotient->digits[i] = LC_make_64(Q[i * 2 + 1], Q[i * 2]);
            }
        }
    }

    // If the caller wants the remainder
    if (Remainder) {
        Remainder->is_negative = false;
        Remainder->digit_count = rhsWords;
        if (rhsWords == 1) {
            Remainder->digit = LC_make_64(R[1], R[0]);
        } else {
            Remainder->digits = ALLOC_DIGITS(rhsWords);
            for (size_t i = 0; i < rhsWords; i += 1) {
                Remainder->digits[i] = LC_make_64(R[i * 2 + 1], R[i * 2]);
            }
        }
    }
}

LC_FUNCTION void LC_Bigint_div_trunc(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    LC_ASSERT(NULL, op2->digit_count != 0); // division by zero
    if (op1->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }
    uint64_t *op1_digits = LC_Bigint_ptr(op1);
    uint64_t *op2_digits = LC_Bigint_ptr(op2);
    if (op1->digit_count == 1 && op2->digit_count == 1) {
        dest->digit       = op1_digits[0] / op2_digits[0];
        dest->digit_count = 1;
        dest->is_negative = op1->is_negative != op2->is_negative;
        LC_normalize(dest);
        return;
    }
    if (op2->digit_count == 1 && op2_digits[0] == 1) {
        // X / 1 == X
        LC_Bigint_init_bigint(dest, op1);
        dest->is_negative = op1->is_negative != op2->is_negative;
        LC_normalize(dest);
        return;
    }

    LC_BigInt *op1_positive;
    LC_BigInt  op1_positive_data;
    if (op1->is_negative) {
        LC_Bigint_negate(&op1_positive_data, op1);
        op1_positive = &op1_positive_data;
    } else {
        op1_positive = op1;
    }

    LC_BigInt *op2_positive;
    LC_BigInt  op2_positive_data;
    if (op2->is_negative) {
        LC_Bigint_negate(&op2_positive_data, op2);
        op2_positive = &op2_positive_data;
    } else {
        op2_positive = op2;
    }

    LC_Bigint_unsigned_division(op1_positive, op2_positive, dest, NULL);
    dest->is_negative = op1->is_negative != op2->is_negative;
    LC_normalize(dest);
}

LC_FUNCTION void LC_Bigint_div_floor(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    if (op1->is_negative != op2->is_negative) {
        LC_Bigint_div_trunc(dest, op1, op2);
        LC_BigInt mult_again = {0};
        LC_Bigint_mul(&mult_again, dest, op2);
        mult_again.is_negative = op1->is_negative;
        if (LC_Bigint_cmp(&mult_again, op1) != LC_CmpRes_EQ) {
            LC_BigInt tmp = {0};
            LC_Bigint_init_bigint(&tmp, dest);
            LC_BigInt neg_one = {0};
            LC_Bigint_init_signed(&neg_one, -1);
            LC_Bigint_add(dest, &tmp, &neg_one);
        }
        LC_normalize(dest);
    } else {
        LC_Bigint_div_trunc(dest, op1, op2);
    }
}

LC_FUNCTION void LC_Bigint_rem(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    LC_ASSERT(NULL, op2->digit_count != 0); // division by zero
    if (op1->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }
    uint64_t *op1_digits = LC_Bigint_ptr(op1);
    uint64_t *op2_digits = LC_Bigint_ptr(op2);

    if (op1->digit_count == 1 && op2->digit_count == 1) {
        dest->digit       = op1_digits[0] % op2_digits[0];
        dest->digit_count = 1;
        dest->is_negative = op1->is_negative;
        LC_normalize(dest);
        return;
    }
    if (op2->digit_count == 2 && op2_digits[0] == 0 && op2_digits[1] == 1) {
        // special case this divisor
        LC_Bigint_init_unsigned(dest, op1_digits[0]);
        dest->is_negative = op1->is_negative;
        LC_normalize(dest);
        return;
    }

    if (op2->digit_count == 1 && op2_digits[0] == 1) {
        // X % 1 == 0
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }

    LC_BigInt *op1_positive;
    LC_BigInt  op1_positive_data;
    if (op1->is_negative) {
        LC_Bigint_negate(&op1_positive_data, op1);
        op1_positive = &op1_positive_data;
    } else {
        op1_positive = op1;
    }

    LC_BigInt *op2_positive;
    LC_BigInt  op2_positive_data;
    if (op2->is_negative) {
        LC_Bigint_negate(&op2_positive_data, op2);
        op2_positive = &op2_positive_data;
    } else {
        op2_positive = op2;
    }

    LC_Bigint_unsigned_division(op1_positive, op2_positive, NULL, dest);
    dest->is_negative = op1->is_negative;
    LC_normalize(dest);
}

LC_FUNCTION void LC_Bigint_mod(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    if (op1->is_negative) {
        LC_BigInt first_rem;
        LC_Bigint_rem(&first_rem, op1, op2);
        first_rem.is_negative = !op2->is_negative;
        LC_BigInt op2_minus_rem;
        LC_Bigint_add(&op2_minus_rem, op2, &first_rem);
        LC_Bigint_rem(dest, &op2_minus_rem, op2);
        dest->is_negative = false;
    } else {
        LC_Bigint_rem(dest, op1, op2);
        dest->is_negative = false;
    }
}

LC_FUNCTION void LC_Bigint_or(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    if (op1->digit_count == 0) {
        LC_Bigint_init_bigint(dest, op2);
        return;
    }
    if (op2->digit_count == 0) {
        LC_Bigint_init_bigint(dest, op1);
        return;
    }
    if (op1->is_negative || op2->is_negative) {
        size_t big_bit_count = LC_size_max(LC_Bigint_bits_needed(op1), LC_Bigint_bits_needed(op2));

        LC_BigInt twos_comp_op1 = {0};
        LC_to_twos_complement(&twos_comp_op1, op1, big_bit_count);

        LC_BigInt twos_comp_op2 = {0};
        LC_to_twos_complement(&twos_comp_op2, op2, big_bit_count);

        LC_BigInt twos_comp_dest = {0};
        LC_Bigint_or(&twos_comp_dest, &twos_comp_op1, &twos_comp_op2);

        LC_from_twos_complement(dest, &twos_comp_dest, big_bit_count, true);
    } else {
        dest->is_negative    = false;
        uint64_t *op1_digits = LC_Bigint_ptr(op1);
        uint64_t *op2_digits = LC_Bigint_ptr(op2);
        if (op1->digit_count == 1 && op2->digit_count == 1) {
            dest->digit_count = 1;
            dest->digit       = op1_digits[0] | op2_digits[0];
            LC_normalize(dest);
            return;
        }
        dest->digit_count = LC_unsigned_max(op1->digit_count, op2->digit_count);
        dest->digits      = ALLOC_DIGITS(dest->digit_count);
        for (size_t i = 0; i < dest->digit_count; i += 1) {
            uint64_t digit = 0;
            if (i < op1->digit_count) {
                digit |= op1_digits[i];
            }
            if (i < op2->digit_count) {
                digit |= op2_digits[i];
            }
            dest->digits[i] = digit;
        }
        LC_normalize(dest);
    }
}

LC_FUNCTION void LC_Bigint_and(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    if (op1->digit_count == 0 || op2->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }
    if (op1->is_negative || op2->is_negative) {
        size_t big_bit_count = LC_size_max(LC_Bigint_bits_needed(op1), LC_Bigint_bits_needed(op2));

        LC_BigInt twos_comp_op1 = {0};
        LC_to_twos_complement(&twos_comp_op1, op1, big_bit_count);

        LC_BigInt twos_comp_op2 = {0};
        LC_to_twos_complement(&twos_comp_op2, op2, big_bit_count);

        LC_BigInt twos_comp_dest = {0};
        LC_Bigint_and(&twos_comp_dest, &twos_comp_op1, &twos_comp_op2);

        LC_from_twos_complement(dest, &twos_comp_dest, big_bit_count, true);
    } else {
        dest->is_negative    = false;
        uint64_t *op1_digits = LC_Bigint_ptr(op1);
        uint64_t *op2_digits = LC_Bigint_ptr(op2);
        if (op1->digit_count == 1 && op2->digit_count == 1) {
            dest->digit_count = 1;
            dest->digit       = op1_digits[0] & op2_digits[0];
            LC_normalize(dest);
            return;
        }

        dest->digit_count = LC_unsigned_max(op1->digit_count, op2->digit_count);
        dest->digits      = ALLOC_DIGITS(dest->digit_count);

        size_t i = 0;
        for (; i < op1->digit_count && i < op2->digit_count; i += 1) {
            dest->digits[i] = op1_digits[i] & op2_digits[i];
        }
        for (; i < dest->digit_count; i += 1) {
            dest->digits[i] = 0;
        }
        LC_normalize(dest);
    }
}

LC_FUNCTION void LC_Bigint_xor(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    if (op1->digit_count == 0) {
        LC_Bigint_init_bigint(dest, op2);
        return;
    }
    if (op2->digit_count == 0) {
        LC_Bigint_init_bigint(dest, op1);
        return;
    }
    if (op1->is_negative || op2->is_negative) {
        size_t big_bit_count = LC_size_max(LC_Bigint_bits_needed(op1), LC_Bigint_bits_needed(op2));

        LC_BigInt twos_comp_op1 = {0};
        LC_to_twos_complement(&twos_comp_op1, op1, big_bit_count);

        LC_BigInt twos_comp_op2 = {0};
        LC_to_twos_complement(&twos_comp_op2, op2, big_bit_count);

        LC_BigInt twos_comp_dest = {0};
        LC_Bigint_xor(&twos_comp_dest, &twos_comp_op1, &twos_comp_op2);

        LC_from_twos_complement(dest, &twos_comp_dest, big_bit_count, true);
    } else {
        dest->is_negative    = false;
        uint64_t *op1_digits = LC_Bigint_ptr(op1);
        uint64_t *op2_digits = LC_Bigint_ptr(op2);

        LC_ASSERT(NULL, op1->digit_count > 0 && op2->digit_count > 0);
        if (op1->digit_count == 1 && op2->digit_count == 1) {
            dest->digit_count = 1;
            dest->digit       = op1_digits[0] ^ op2_digits[0];
            LC_normalize(dest);
            return;
        }
        dest->digit_count = LC_unsigned_max(op1->digit_count, op2->digit_count);
        dest->digits      = ALLOC_DIGITS(dest->digit_count);
        size_t i          = 0;
        for (; i < op1->digit_count && i < op2->digit_count; i += 1) {
            dest->digits[i] = op1_digits[i] ^ op2_digits[i];
        }
        for (; i < dest->digit_count; i += 1) {
            if (i < op1->digit_count) {
                dest->digits[i] = op1_digits[i];
            } else if (i < op2->digit_count) {
                dest->digits[i] = op2_digits[i];
            } else {
                LC_ASSERT(NULL, !"Unreachable");
            }
        }
        LC_normalize(dest);
    }
}

LC_FUNCTION void LC_Bigint_shl(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    LC_ASSERT(NULL, !op2->is_negative);

    if (op2->digit_count == 0) {
        return;
    }
    if (op2->digit_count != 1) {
        LC_ASSERT(NULL, !"Unsupported: shift left by amount greater than 64 bit integer");
    }
    LC_Bigint_shl_int(dest, op1, LC_Bigint_as_unsigned(op2));
}

LC_FUNCTION void LC_Bigint_shl_int(LC_BigInt *dest, LC_BigInt *op1, uint64_t shift) {
    if (shift == 0) {
        LC_Bigint_init_bigint(dest, op1);
        return;
    }

    if (op1->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }

    uint64_t *op1_digits = LC_Bigint_ptr(op1);

    if (op1->digit_count == 1 && shift < 64) {
        dest->digit = op1_digits[0] << shift;
        if (dest->digit > op1_digits[0]) {
            dest->digit_count = 1;
            dest->is_negative = op1->is_negative;
            return;
        }
    }

    uint64_t digit_shift_count    = shift / 64;
    uint64_t leftover_shift_count = shift % 64;

    dest->digits      = ALLOC_DIGITS(op1->digit_count + digit_shift_count + 1);
    dest->digit_count = (unsigned)digit_shift_count;
    uint64_t carry    = 0;
    for (size_t i = 0; i < op1->digit_count; i += 1) {
        uint64_t digit                  = op1_digits[i];
        dest->digits[dest->digit_count] = carry | (digit << leftover_shift_count);
        dest->digit_count++;
        if (leftover_shift_count > 0) {
            carry = digit >> (64 - leftover_shift_count);
        } else {
            carry = 0;
        }
    }
    dest->digits[dest->digit_count] = carry;
    dest->digit_count += 1;
    dest->is_negative = op1->is_negative;
    LC_normalize(dest);
}

LC_FUNCTION void LC_Bigint_shl_trunc(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2, size_t bit_count, bool is_signed) {
    LC_BigInt unwrapped = {0};
    LC_Bigint_shl(&unwrapped, op1, op2);
    LC_Bigint_truncate(dest, &unwrapped, bit_count, is_signed);
}

LC_FUNCTION void LC_Bigint_shr(LC_BigInt *dest, LC_BigInt *op1, LC_BigInt *op2) {
    LC_ASSERT(NULL, !op2->is_negative);

    if (op1->digit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }

    if (op2->digit_count == 0) {
        LC_Bigint_init_bigint(dest, op1);
        return;
    }

    if (op2->digit_count != 1) {
        LC_ASSERT(NULL, !"Unsupported: shift right by amount greater than 64 bit integer");
    }

    uint64_t *op1_digits = LC_Bigint_ptr(op1);
    uint64_t  shift_amt  = LC_Bigint_as_unsigned(op2);

    if (op1->digit_count == 1) {
        dest->digit       = shift_amt < 64 ? op1_digits[0] >> shift_amt : 0;
        dest->digit_count = 1;
        dest->is_negative = op1->is_negative;
        LC_normalize(dest);
        return;
    }

    uint64_t digit_shift_count    = shift_amt / 64;
    uint64_t leftover_shift_count = shift_amt % 64;

    if (digit_shift_count >= op1->digit_count) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }

    dest->digit_count = (unsigned)(op1->digit_count - digit_shift_count);
    uint64_t *digits;
    if (dest->digit_count == 1) {
        digits = &dest->digit;
    } else {
        digits       = ALLOC_DIGITS(dest->digit_count);
        dest->digits = digits;
    }

    uint64_t carry = 0;
    for (size_t op_digit_index = op1->digit_count - 1;;) {
        uint64_t digit            = op1_digits[op_digit_index];
        size_t   dest_digit_index = op_digit_index - digit_shift_count;
        digits[dest_digit_index]  = carry | (digit >> leftover_shift_count);
        carry                     = digit << (64 - leftover_shift_count);

        if (dest_digit_index == 0) break;
        op_digit_index -= 1;
    }
    dest->is_negative = op1->is_negative;
    LC_normalize(dest);
}

LC_FUNCTION void LC_Bigint_not(LC_BigInt *dest, LC_BigInt *op, size_t bit_count, bool is_signed) {
    if (bit_count == 0) {
        LC_Bigint_init_unsigned(dest, 0);
        return;
    }

    if (is_signed) {
        LC_BigInt twos_comp = {0};
        LC_to_twos_complement(&twos_comp, op, bit_count);

        LC_BigInt inverted = {0};
        LC_Bigint_not(&inverted, &twos_comp, bit_count, false);

        LC_from_twos_complement(dest, &inverted, bit_count, true);
        return;
    }

    LC_ASSERT(NULL, !op->is_negative);

    dest->is_negative   = false;
    uint64_t *op_digits = LC_Bigint_ptr(op);
    if (bit_count <= 64) {
        dest->digit_count = 1;
        if (op->digit_count == 0) {
            if (bit_count == 64) {
                dest->digit = UINT64_MAX;
            } else {
                dest->digit = (1ULL << bit_count) - 1;
            }
        } else if (op->digit_count == 1) {
            dest->digit = ~op_digits[0];
            if (bit_count != 64) {
                uint64_t
                    mask = (1ULL << bit_count) - 1;
                dest->digit &= mask;
            }
        }
        LC_normalize(dest);
        return;
    }
    dest->digit_count = (unsigned int)((bit_count + 63) / 64);
    LC_ASSERT(NULL, dest->digit_count >= op->digit_count);
    dest->digits = ALLOC_DIGITS(dest->digit_count);
    size_t i     = 0;
    for (; i < op->digit_count; i += 1) {
        dest->digits[i] = ~op_digits[i];
    }
    for (; i < dest->digit_count; i += 1) {
        dest->digits[i] = 0xffffffffffffffffULL;
    }
    size_t digit_index     = dest->digit_count - 1;
    size_t digit_bit_index = bit_count % 64;
    if (digit_bit_index != 0) {
        uint64_t
            mask = (1ULL << digit_bit_index) - 1;
        dest->digits[digit_index] &= mask;
    }
    LC_normalize(dest);
}

LC_FUNCTION void LC_Bigint_truncate(LC_BigInt *dst, LC_BigInt *op, size_t bit_count, bool is_signed) {
    LC_BigInt twos_comp;
    LC_to_twos_complement(&twos_comp, op, bit_count);
    LC_from_twos_complement(dst, &twos_comp, bit_count, is_signed);
}

LC_FUNCTION LC_CmpRes LC_Bigint_cmp(LC_BigInt *op1, LC_BigInt *op2) {
    if (op1->is_negative && !op2->is_negative) return LC_CmpRes_LT;
    if (!op1->is_negative && op2->is_negative) return LC_CmpRes_GT;
    if (op1->digit_count > op2->digit_count) return op1->is_negative ? LC_CmpRes_LT : LC_CmpRes_GT;
    if (op2->digit_count > op1->digit_count) return op1->is_negative ? LC_CmpRes_GT : LC_CmpRes_LT;
    if (op1->digit_count == 0) return LC_CmpRes_EQ;

    uint64_t *op1_digits = LC_Bigint_ptr(op1);
    uint64_t *op2_digits = LC_Bigint_ptr(op2);
    for (unsigned i = op1->digit_count - 1;; i--) {
        uint64_t op1_digit = op1_digits[i];
        uint64_t op2_digit = op2_digits[i];

        if (op1_digit > op2_digit) {
            return op1->is_negative ? LC_CmpRes_LT : LC_CmpRes_GT;
        }
        if (op1_digit < op2_digit) {
            return op1->is_negative ? LC_CmpRes_GT : LC_CmpRes_LT;
        }
        if (i == 0) {
            return LC_CmpRes_EQ;
        }
    }
}

LC_FUNCTION char *LC_Bigint_str(LC_BigInt *bigint, uint64_t base) {
    LC_StringList out = {0};
    if (bigint->digit_count == 0) {
        return "0";
    }
    if (bigint->is_negative) {
        LC_Addf(L->arena, &out, "-");
    }
    if (bigint->digit_count == 1 && base == 10) {
        LC_Addf(L->arena, &out, "%llu", (unsigned long long)bigint->digit);
    } else {
        size_t len   = bigint->digit_count * 64;
        char  *start = (char *)malloc_arena(len);
        char  *buf   = start;

        LC_BigInt  digit_bi = {0};
        LC_BigInt  a1       = {0};
        LC_BigInt  a2       = {0};
        LC_BigInt  base_bi  = {0};
        LC_BigInt *a        = &a1;
        LC_BigInt *other_a  = &a2;

        LC_Bigint_init_bigint(a, bigint);
        LC_Bigint_init_unsigned(&base_bi, base);

        for (;;) {
            LC_Bigint_rem(&digit_bi, a, &base_bi);
            uint8_t digit = (uint8_t)LC_Bigint_as_unsigned(&digit_bi);
            *(buf++)      = LC_digit_to_char(digit, false);
            LC_Bigint_div_trunc(other_a, a, &base_bi);
            {
                LC_BigInt *tmp = a;
                a              = other_a;
                other_a        = tmp;
            }
            if (LC_Bigint_cmp_zero(a) == LC_CmpRes_EQ) {
                break;
            }
        }

        // reverse

        for (char *ptr = buf - 1; ptr >= start; ptr--) {
            LC_Addf(L->arena, &out, "%c", *ptr);
        }
    }
    LC_String s = LC_MergeString(L->arena, out);
    return s.str;
}

LC_FUNCTION int64_t LC_Bigint_as_signed(LC_BigInt *bigint) {
    if (bigint->digit_count == 0) return 0;
    if (bigint->digit_count != 1) {
        LC_ASSERT(NULL, !"LC_BigInt larger than i64");
    }

    if (bigint->is_negative) {
        // TODO this code path is untested
        if (bigint->digit <= 9223372036854775808ULL) {
            return (-((int64_t)(bigint->digit - 1))) - 1;
        }
        LC_ASSERT(NULL, !"LC_BigInt does not fit in i64");
    }
    return (int64_t)bigint->digit;
}

LC_FUNCTION LC_CmpRes LC_Bigint_cmp_zero(LC_BigInt *op) {
    if (op->digit_count == 0) {
        return LC_CmpRes_EQ;
    }
    return op->is_negative ? LC_CmpRes_LT : LC_CmpRes_GT;
}

LC_FUNCTION double LC_Bigint_as_float(LC_BigInt *bigint) {
    if (LC_Bigint_fits_in_bits(bigint, 64, bigint->is_negative)) {
        return bigint->is_negative ? (double)LC_Bigint_as_signed(bigint) : (double)LC_Bigint_as_unsigned(bigint);
    }
    LC_BigInt div;
    uint64_t  mult = 0x100000000000ULL;
    double    mul  = 1;
    LC_Bigint_init_unsigned(&div, mult);
    LC_BigInt current;
    LC_Bigint_init_bigint(&current, bigint);
    double f = 0;
    do {
        LC_BigInt temp;
        LC_Bigint_mod(&temp, &current, &div);
        f += LC_Bigint_as_signed(&temp) * mul;
        mul *= mult;
        LC_Bigint_div_trunc(&temp, &current, &div);
        current = temp;
    } while (current.digit_count > 0);
    return f;
}