/** @file
 * Zawiera implementację interfejsu memory_util.h
 *
 * @author Mateusz Nowakowski <mn418323@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 07.04.2020
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "memory_util.h"

bool init_arr_2D(void ***data_ptr, size_t type, uint64_t width, uint64_t height) {
    void **data = NULL;
    data = malloc(sizeof(int *) * width);
    if (data == NULL)
        return false;
    bool is_ok = true;
    for (uint64_t i = 0; i < width; i++) {
        data[i] = NULL;
        if (!init_array_1D(&data[i], type, height))
            is_ok = false;
    }
    if (!is_ok) {
        free_array(data, width);
        return false;
    }
    *data_ptr = data;
    return true;
}

bool init_array_1D(void **data_ptr, size_t type, uint64_t len) {
    void *data = NULL;
    data = calloc(len, type);
    if (data == NULL)
        return false;
    *data_ptr = data;
    return true;
}

void free_array(void **arr, uint64_t width) {
    if (arr == NULL)
        return;
    for (uint64_t i = 0; i < width; i++) {
        if (arr[i] != NULL)
            free(arr[i]);
    }
    free(arr);
}