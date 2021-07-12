#include "str_table.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// =====================================================================================================================
// Private helpers definitions
// =====================================================================================================================

static unsigned int longest_fstring_len_col(const struct StringTable *table, unsigned int col_num);

static void set_table_border(char **cursor, unsigned int row_width);

static void set_table_entry(char **cursor, const unsigned int *col_widths, const struct StringTable *table, unsigned int row_num);

static void set_whitespace(char **cursor, unsigned int whitespace_count);

// =====================================================================================================================
// Public functions implementations
// =====================================================================================================================
struct StringTable *create_table(unsigned int width,
                                 unsigned int height,
                                 bool has_header,
                                 unsigned int indent_spacing) {
  struct StringTable *new_table = malloc(sizeof(struct StringTable));
  new_table->width = width;
  new_table->height = height;
  new_table->has_header = has_header;

  if (has_header) {
    new_table->height++; // Extra row for header
  }

  new_table->total_entries = new_table->width * new_table->height;

  new_table->contents = calloc(new_table->total_entries, sizeof(char *));

  new_table->indent_spacing = indent_spacing;
  new_table->indent_levels = calloc(new_table->total_entries, sizeof(unsigned int));

  return new_table;
}

int destroy_table(struct StringTable *table) {
  if (table == NULL) {
    return STR_TABLE_ERR;
  }

  if (table->contents != NULL) {
    for (long long idx = 0; idx < table->total_entries; idx++) {
      if (table->contents[idx] != NULL) {
        free(table->contents[idx]);
      }
    }
    free(table->contents);
  }

  if (table->indent_levels != NULL) {
    free(table->indent_levels);
  }

  free(table);
  return STR_TABLE_OK;
}

int add_entry_str(const struct StringTable *table, const char *value, unsigned int row_num, unsigned int col_num) {
  if (table == NULL) {
    return STR_TABLE_ERR;
  }

  if (table->contents == NULL) {
    return STR_TABLE_ERR;
  }

  if (col_num >= table->width) {
    return STR_TABLE_ERR;
  }
  if (row_num >= table->height) {
    return STR_TABLE_ERR;
  }

  const unsigned int effective_idx = row_num * table->width + col_num;

  if (table->contents[effective_idx] != NULL) {
    free(table->contents[effective_idx]);
  }
  table->contents[effective_idx] = malloc(sizeof(char) * (strlen(value) + 1)); // Extra value for the null terminator
  strcpy(table->contents[effective_idx], value);
  return STR_TABLE_OK;
}

int add_entry_lld(const struct StringTable *table, long long value, unsigned int row_num, unsigned int col_num) {
  unsigned int num_digits = (int) (log10((double) value)) + 1;

  char *num_str = malloc(sizeof(char) * (num_digits + 1)); // Extra value for the null terminator
  sprintf(num_str, "%lld", value);
  add_entry_str(table, num_str, row_num, col_num);
  free(num_str);
  return STR_TABLE_OK;
}

int set_indent_lvl(const struct StringTable *table, unsigned int indent_lvl, unsigned int row_num, unsigned int col_num) {
  if (table == NULL) {
    return STR_TABLE_ERR;
  }

  if (table->indent_levels == NULL) {
    return STR_TABLE_ERR;
  }

  if (col_num >= table->width) {
    return STR_TABLE_ERR;
  }

  if (row_num >= table->height) {
    return STR_TABLE_ERR;
  }

  const unsigned int effective_idx = row_num * table->width + col_num;

  table->indent_levels[effective_idx] = indent_lvl;
  return STR_TABLE_OK;
}

char *make_table_str(const struct StringTable *table) {
  if (table == NULL) {
    return NULL;
  }

  unsigned int num_char_row = 0;
  unsigned int num_rows = 0;

  // Array holding the space that each formatted entry takes
  unsigned int *col_widths = malloc(table->width * sizeof(unsigned int));

  // Compute number of characters in a row
  for (unsigned int col = 0; col < table->width; col++) {
    const unsigned int num_chars = longest_fstring_len_col(table, col);
    col_widths[col] = num_chars;
    num_char_row += (num_chars + 3); // Three extra for column line and left spacing and 1 for right spacing
  }
  num_char_row += 2; // Two more for column line and new line character

  // Compute number of rows
  num_rows += (table->height + 2); // One for border at the top and one for the border at the bottom
  if (table->has_header) {
    num_rows ++; // One additional one for the border between table header and body
  }

  // Allocate space to fit the total number of characters including null terminator
  char *table_str = malloc((num_char_row * num_rows + 1) * sizeof(char)); // Extra element for the null terminator
  char *cursor = table_str; // Cursor to manipulate the contents

  // Set the top border
  set_table_border(&cursor, num_char_row);
  for (unsigned int row = 0; row < table->height; row++) {
    set_table_entry(&cursor, col_widths, table, row);
    if (row == 0 && table->has_header) {
      set_table_border(&cursor, num_char_row);
    }
  }
  // Set top border
  set_table_border(&cursor, num_char_row);
  // Add null terminator
  *cursor = '\0';

  free(col_widths);
  return table_str;
}

// =====================================================================================================================
// Private helpers implementation
// =====================================================================================================================

// Preconditions:
//    - table is never null
//    - col_num is always less than the width of the table
static unsigned int longest_fstring_len_col(const struct StringTable *table, unsigned int col_num) {
  unsigned int longest_so_far = 0;

  for (unsigned int row = 0; row < table->height; row++) {
    const unsigned int effective_idx = row * table->width + col_num;
    size_t
        curr_len = strlen(table->contents[effective_idx]) + table->indent_levels[effective_idx] * table->indent_spacing;
    if (curr_len > longest_so_far) {
      longest_so_far = curr_len;
    }
  }
  return longest_so_far;
}

// The cursor will move forward as the characters are inserted to create the border
static void set_table_border(char **cursor, unsigned int row_width) {
  for (unsigned i = 0; i < row_width; i++) {
    if ((i == 0) || (i == (row_width - 2))) {
      **cursor = '|';
    } else if (i == row_width - 1) {
      **cursor = '\n';
    } else {
      **cursor = '-';
    }
    (*cursor)++;
  }
}

static void set_table_entry(char **cursor, const unsigned int *col_widths, const struct StringTable *table, unsigned int row_num) {
  for (unsigned int col = 0; col < table->width; col++) {
    // Fill in left border
    strncpy(*cursor, "| ", 2);
    (*cursor)+=2;

    const unsigned int effective_idx = row_num * table->width + col;
    const unsigned int left_indent_spaces = table->indent_spacing * table->indent_levels[effective_idx];
    const unsigned int str_len = strlen(table->contents[effective_idx]);
    const unsigned int right_spaces = col_widths[col] - left_indent_spaces - str_len + 1; // Extra one for right padding

    // Fill in left indent with whitespaces
    set_whitespace(cursor, left_indent_spaces);

    // Fill in the string without null terminator
    strncpy(*cursor, table->contents[effective_idx], str_len);
    (*cursor)+= str_len;

    // Fill in remaining available spaces with whitespaces
    set_whitespace(cursor, right_spaces);
  }
  // Fill in right border
  strncpy(*cursor, "|\n", 2);
  (*cursor)+=2;
}

static void set_whitespace(char **cursor, unsigned int whitespace_count) {
  for(unsigned int i = 0; i < whitespace_count; i++) {
    **cursor = ' ';
    (*cursor)++;
  }
}