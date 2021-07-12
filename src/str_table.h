#ifndef LIBSTOPWATCH_SRC_STR_TABLE_H_
#define LIBSTOPWATCH_SRC_STR_TABLE_H_

#include <stdbool.h> // Seriously doubt anyone is still stuck pre C99

#define STR_TABLE_ERR -1
#define STR_TABLE_OK 0

// Structure for indicating table cell position. Much superior to adding row and column to function arguments as it is
// very easy to mix / confuse them
struct StringTableCellPos {
  unsigned int row_num; // Row number zero indexed
  unsigned int col_num; // Column number zero indexed
};

struct StringTable {
  char **contents;              // Contents of each cell in table
  unsigned int width;           // Number of columns
  unsigned int height;          // Number of rows excluding header if exists
  long long total_entries;      // Total number of cells. Equals the length of `contents` and `indent_levels`
  bool has_header;
  unsigned int indent_spacing;  // Number of spaces to use for indentation
  unsigned int *indent_levels;  // Indentation levels of each cell in the entire table including header and body
};

struct StringTable *create_table(unsigned int width, unsigned int height, bool has_header, unsigned int indent_spacing);

int destroy_table(struct StringTable *table);

// Is non-owning of the variable `value`. The caller may allocate the variable `value` but calling this function will
// not transfer the ownership implying that the caller will still need to handle the lifetime of the variable `value`.
int add_entry_str(const struct StringTable *table, const char *value, struct StringTableCellPos pos);

int add_entry_lld(const struct StringTable *table, long long value, struct StringTableCellPos pos);

int set_indent_lvl(const struct StringTable *table, unsigned int indent_lvl, struct StringTableCellPos pos);

char *make_table_str(const struct StringTable *table);

#endif //LIBSTOPWATCH_SRC_STR_TABLE_H_
