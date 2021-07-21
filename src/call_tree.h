#ifndef LIBSTOPWATCH_SRC_CALL_TREE_H_
#define LIBSTOPWATCH_SRC_CALL_TREE_H_

#include <stddef.h>
struct FunctionCallNode {
  size_t function_id;
  size_t stack_depth;
  size_t num_elem;
  // Point to functions called by this function. Should only be single element.
  struct FunctionCallNode* callee;
  // Point to the next function called by the caller of this function. Is array of length num_callees.
  struct FunctionCallNode* next;
};

struct FunctionNode {
  size_t function_id;
  size_t caller_id;
};

struct FunctionCallNode* create_function_node(size_t function_id);

void add_same_lvl(struct FunctionCallNode* original_node, struct FunctionCallNode* new_node);

void add_callee(struct FunctionCallNode* caller, struct FunctionCallNode* callee);

void destroy_tree(struct FunctionCallNode* call_tree);

struct FunctionCallNode* grow_tree_from_array(const struct FunctionNode* arr, size_t len);

#endif //LIBSTOPWATCH_SRC_CALL_TREE_H_
