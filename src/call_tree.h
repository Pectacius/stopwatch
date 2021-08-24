#ifndef LIBSTOPWATCH_SRC_CALL_TREE_H_
#define LIBSTOPWATCH_SRC_CALL_TREE_H_

#include <stdbool.h>
#include <stddef.h>

// n-ary tree structure
struct FunctionCallNode {
  size_t function_id;
  size_t stack_depth; // Stack depth of function relative to main function
  // Point to functions called by this function. Should only be single element.
  struct FunctionCallNode *last_callee;
  // Point to the previous node function called by the caller of this function.
  struct FunctionCallNode *prev_node;
};

struct FunctionNode {
  size_t function_id;
  size_t caller_id;
};

// An depth first non-owning iterator for the n-ary FunctionCallNode tree structure
struct FunctionCallTreeDFIter {
  struct FunctionCallNode **nodes_to_visit;
  size_t node_to_visit_size;
};

struct FunctionCallNode *create_function_call_node(size_t function_id);

// Note that this function will not update the stack depth appropriately as there is the case where the existing "tree"
// is appended to a new root which is a single node. This case will then invalidate the stack depth of the "existing"
// tree. It should be assumed that the existing stack depth values are no longer valid after using this function.
void function_call_node_add_callee(struct FunctionCallNode *caller, struct FunctionCallNode *callee);

void destroy_function_call_node(struct FunctionCallNode *call_tree);

// Transforms an array of FunctionNode to a FunctionCallNode n-ary tree. Precondition is that the array of FunctionNodes
// must have at least one element where the caller_id is 0. The first node will always be the node that corresponds to
// caller_id 0. In most use cases, this corresponds to the main function even though the input array did not contain a
// specific entry for the main function.
struct FunctionCallNode *function_call_node_grow_tree_from_array(const struct FunctionNode *arr, size_t len);

size_t function_call_node_get_num_nodes(const struct FunctionCallNode *tree);

// Note that this is non owning of the call tree
struct FunctionCallTreeDFIter *create_function_call_tree_DF_iter(struct FunctionCallNode *call_tree);

// Again, since this is non owning of the call tree, it will not free the call tree
void destroy_function_call_tree_DF_iter(struct FunctionCallTreeDFIter *iter);

bool function_call_tree_DF_iter_has_next(const struct FunctionCallTreeDFIter *iter);

const struct FunctionCallNode *function_call_tree_DF_iter_next(struct FunctionCallTreeDFIter *iter);

#endif //LIBSTOPWATCH_SRC_CALL_TREE_H_
