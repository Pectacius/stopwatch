#ifndef LIBSTOPWATCH_SRC_STR_TABLE_H_
#define LIBSTOPWATCH_SRC_STR_TABLE_H_

#define STR_TABLE_ERR -1
#define STR_TABLE_OK 0

#define STR_TABLE_LABEL_TRUE 1
#define STR_TABLE_LABEL_FALSE 0

struct StringTable {
  char** contents;
  unsigned int width;
  unsigned int height;
  unsigned int has_header;
};

struct StringTable* create_table(unsigned int width, unsigned int height, unsigned int has_header);

int destroy_table(struct StringTable* table);

int add_entry(struct StringTable* table, const char* value, unsigned int row_num, unsigned int col_num);

int add_lld_entry(struct StringTable* table, long long value, unsigned int row_num, unsigned int col_num);

// Creates a copy of the string at the specified index. The caller is now the owner of the string returned
char* get_entry(struct StringTable* table, unsigned int row_num, unsigned int col_num);

unsigned int longest_str_len_col(struct StringTable* table, unsigned int col_num);

#endif //LIBSTOPWATCH_SRC_STR_TABLE_H_
