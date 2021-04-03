#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"

void init_char_buffer(struct CharBuffer* buffer, size_t initial_depth_buffer_size) {
    buffer->data = malloc(initial_depth_buffer_size);
    buffer->size = initial_depth_buffer_size;
    buffer->index = 0;
}

void release_char_buffer(struct CharBuffer* buffer) {
    free(buffer->data);
}

void push(struct CharBuffer* buffer, char value) {
    buffer->data[buffer->index] = value;
    buffer->index += 1;
    if(buffer->index >= buffer->size) {
        char* new_data = malloc(2*buffer->size);
        memmove(new_data, buffer->data, buffer->size);
        free(buffer->data);
        buffer->data = new_data;
        buffer->size *= 2;
    }
}

void push_string(struct CharBuffer* buffer, char* value, size_t len) {
    if(buffer->index + len >= buffer->size) {
        char* new_data = malloc(2*buffer->size);
        memmove(new_data, buffer->data, buffer->size);
        free(buffer->data);
        buffer->data = new_data;
        buffer->size *= 2;
    }
    memmove(buffer->data + buffer->index, value, len);
    buffer->index += len;
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
