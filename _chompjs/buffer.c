/*
 * Copyright 2020-2021 Mariusz Obajtek. All rights reserved.
 * License: https://github.com/Nykakin/chompjs/blob/master/LICENSE
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "buffer.h"

void init_char_buffer(struct CharBuffer* buffer, size_t initial_depth_buffer_size) {
    buffer->data = malloc(initial_depth_buffer_size);
    buffer->memory_buffer_length = initial_depth_buffer_size;
    buffer->index = 0;
}

void release_char_buffer(struct CharBuffer* buffer) {
    free(buffer->data);
}

void check_capacity(struct CharBuffer* buffer, size_t to_save) {
    if(buffer->index + to_save >= buffer->memory_buffer_length) {
        buffer->data = realloc(buffer->data, 2*buffer->memory_buffer_length);
        buffer->memory_buffer_length *= 2;
    }
}

void push(struct CharBuffer* buffer, char value) {
    check_capacity(buffer, 1);
    buffer->data[buffer->index] = value;
    buffer->index += 1;
}

void push_string(struct CharBuffer* buffer, const char* value, size_t len) {
    check_capacity(buffer, len);
    memcpy(buffer->data + buffer->index, value, len);
    buffer->index += len;
}

void push_number(struct CharBuffer* buffer, long value) {
    int size_in_chars = (int)((ceil(log10(value))));
    check_capacity(buffer, size_in_chars);
    buffer->index += sprintf(buffer->data + buffer->index, "%ld", value);
}

void pop(struct CharBuffer* buffer) {
    buffer->index -= 1;
}

char top(struct CharBuffer* buffer) {
    return buffer->data[buffer->index-1];
}

bool empty(struct CharBuffer* buffer) {
    return buffer->index <= 0;
}

void clear(struct CharBuffer* buffer) {
    buffer->index = 0;
}

size_t size(struct CharBuffer* buffer) {
    return buffer->index;
}
