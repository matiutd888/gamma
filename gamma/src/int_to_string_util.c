/** @file
 * Zawiera implementację interfejsu int_to_string_util.h
 *
 * @author Mateusz Nowakowski <mn418323@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 12.05.2020
 */
#include "int_to_string_util.h"
#include <stdint.h>

uint64_t write_int(char *s, uint64_t l, uint32_t n) {
    uint32_t count = digit_count(n);
    uint32_t it = 0;
    while (it < count) {
        s[count + l - it - 1] = (n % 10) + '0';
        n /= 10;
        it++;
    }
    return l + count;
}

uint32_t digit_count(uint64_t n) {
    if (n == 0)
        return 1;

    uint32_t count = 0;
    while (n != 0) {
        n /= 10;
        count++;
    }
    return count;
}