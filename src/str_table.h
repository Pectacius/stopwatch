#ifndef LIBSTOPWATCH_SRC_STR_TABLE_H_
#define LIBSTOPWATCH_SRC_STR_TABLE_H_

#include <stdbool.h> // Seriously doubt anyone is still stuck pre C99
#include <stddef.h>

#define STR_TABLE_ERR -1
#define STR_TABLE_OK 0

// Structure for indicating table cell position. Much superior to adding row and column to function arguments as it is
// very easy to mix / confuse them
struct StringTableCellPos {
  size_t row_num; // Row number zero indexed
  size_t col_num; // Column number zero indexed
};

struct StringTable {
  char **contents;         // Contents of each cell in table
  size_t width;           // Number of columns
  size_t height;          // Number of rows excluding header if exists
  size_t total_entries;   // Total number of cells. Equals the length of `contents` and `indent_levels`
  bool has_header;
  size_t indent_spacing;  // Number of spaces to use for indentation
  size_t *indent_levels;  // Indentation levels of each cell in the entire table including header and body
};

struct StringTable *create_table(size_t width, size_t height, bool has_header, size_t indent_spacing);

int destroy_table(struct StringTable *table);

// Is non-owning of the variable `value`. The caller may allocate the variable `value` but calling this function will
// not transfer the ownership implying that the caller will still need to handle the lifetime of the variable `value`.
int add_entry_str(const struct StringTable *table, const char *value, struct StringTableCellPos pos);

int add_entry_lld(const struct StringTable *table, long long value, struct StringTableCellPos pos);

int set_indent_lvl(const struct StringTable *table, size_t indent_lvl, struct StringTableCellPos pos);

char *make_table_str(const struct StringTable *table);

#endif //LIBSTOPWATCH_SRC_STR_TABLE_H_
