#ifndef LIBSTOPWATCH_SRC_CALL_TREE_H_
#define LIBSTOPWATCH_SRC_CALL_TREE_H_

#include <stdbool.h>
#include <stddef.h>
struct FunctionCallNode {
  size_t function_id;
  size_t stack_depth; // Stack depth of function relative to main function
  // Point to functions called by this function. Should only be single element.
  struct FunctionCallNode* callee;
  // Point to the next function called by the caller of this function. Is array of length num_callees.
  struct FunctionCallNode* next;
};

struct FunctionNode {
  size_t function_id;
  size_t caller_id;
};

struct FunctionCallTreeDFIter {
  struct FunctionCallNode** nodes_to_visit;
  size_t node_to_visit_size;
  struct FunctionCallNode* next_node;
};

struct FunctionCallNode* create_function_call_node(size_t function_id);

void function_call_node_add_same_lvl(struct FunctionCallNode* original_node, struct FunctionCallNode* new_node);

void function_call_node_add_callee(struct FunctionCallNode* caller, struct FunctionCallNode* callee);

void destroy_function_call_node(struct FunctionCallNode* call_tree);

struct FunctionCallNode* function_call_node_grow_tree_from_array(const struct FunctionNode* arr, size_t len);

size_t function_call_node_get_num_nodes(const struct FunctionCallNode* tree);

// Note that this is non owning of the call tree
struct FunctionCallTreeDFIter* create_function_call_tree_DF_iter(struct FunctionCallNode* call_tree);

// Again, since this is non owning of the call tree, it will not free the call tree
void destroy_function_call_tree_DF_iter(struct FunctionCallTreeDFIter* iter);

bool function_call_tree_DF_iter_has_next(const struct FunctionCallTreeDFIter* iter);

const struct FunctionCallNode* function_call_tree_DF_iter_next(struct FunctionCallTreeDFIter* iter);

#endif //LIBSTOPWATCH_SRC_CALL_TREE_H_
