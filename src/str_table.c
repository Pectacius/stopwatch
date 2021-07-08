#include "str_table.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

struct StringTable* create_table(unsigned int width, unsigned int height, unsigned int has_header) {
  struct StringTable* new_table = malloc(sizeof(struct StringTable));
  new_table->width = width;
  new_table->height = height;
  new_table->has_header = has_header;
  new_table->contents = calloc(width*height, sizeof(char*)); // Should be array of 0s

  return new_table;
}

int destroy_table(struct StringTable* table) {
  if (table == NULL) {
    return STR_TABLE_ERR;
  }
  if(table->contents != NULL) {
    for(long long idx = 0; idx < table->width*table->height; idx++) {
      if(table->contents[idx] != NULL) {
        free(table->contents[idx]);
      }
    }
    free(table->contents);
  }
  free(table);
  return STR_TABLE_OK;
}

int add_entry(struct StringTable* table, const char* value, unsigned int row_num, unsigned int col_num) {
  if(table == NULL) {
    return STR_TABLE_ERR;
  }
  if (col_num >= table->width) {
    return STR_TABLE_ERR;
  }
  if(row_num >= table->height) {
    return STR_TABLE_ERR;
  }

  unsigned int effective_idx = row_num*table->width + col_num;

  if(table->contents[effective_idx] != NULL) {
    free(table->contents[effective_idx]);
  }
  table->contents[effective_idx] = malloc(sizeof(char) * (strlen(value) + 1)); // Extra value for the null terminator
  strcpy(table->contents[effective_idx], value);
  return STR_TABLE_OK;
}

int add_lld_entry(struct StringTable* table, long long value, unsigned int row_num, unsigned int col_num) {
  if(table == NULL) {
    return STR_TABLE_ERR;
  }
  if (col_num >= table->width) {
    return STR_TABLE_ERR;
  }
  if(row_num >= table->height) {
    return STR_TABLE_ERR;
  }

  unsigned int effective_idx = row_num*table->width + col_num;

  unsigned int num_digits = (int)(log10((double)value)) + 1;

  if(table->contents[effective_idx] != NULL) {
    free(table->contents[effective_idx]);
  }
  table->contents[effective_idx] = malloc(sizeof(char) * (num_digits + 1)); // Extra value for the null terminator
  sprintf(table->contents[effective_idx], "%lld", value);
  return STR_TABLE_OK;
}


char* get_entry(struct StringTable* table, unsigned int row_num, unsigned int col_num) {
  if(table == NULL) {
    return NULL;
  }
  if (col_num >= table->width) {
    return NULL;
  }
  if(row_num >= table->height) {
    return NULL;
  }

  unsigned int effective_idx = row_num*table->width + col_num;

  if(table->contents[effective_idx] == NULL) {
    return NULL;
  }

  char* result = malloc(sizeof(char)*(strlen(table->contents[effective_idx]) + 1));
  strcpy(result, table->contents[effective_idx]);
  return result;
}

unsigned int longest_str_len_col(struct StringTable* table, unsigned int col_num) {
  if (col_num > table->width) {
    return 0;
  }
  unsigned int longest_so_far = 0;
  for(unsigned int idx = 0; idx < table->height; idx++) {
    unsigned int effective_idx = idx*table->width + col_num;
    //printf("table width: %d\n", table->width);
    //printf("table height: %d\n", table->height);
    //printf("eff idx: %d\n", effective_idx);
    size_t curr_len = strlen(table->contents[effective_idx]);
    if(curr_len > longest_so_far) {
      longest_so_far = curr_len;
    }
  }
  return longest_so_far;
}

