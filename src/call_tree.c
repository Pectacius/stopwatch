#include "call_tree.h"

#include <stdlib.h>

// =====================================================================================================================
// Private helper methods definitions
// =====================================================================================================================
static size_t find_max_function_id(const struct FunctionNode *arr, size_t len);

// Depth first traverses the tree and updates the stack depth accordingly. Note that this cannot be done when creating
// the tree. The root of the tree will always have a stack depth of 0 hence depth should always be 0 when tree is the
// "root" of the entire tree.
static void update_stack_depth(struct FunctionCallNode* tree, size_t depth);

// =====================================================================================================================
// Public functions implementations
// =====================================================================================================================
struct FunctionCallNode *create_function_call_node(size_t function_id) {
  struct FunctionCallNode *node = malloc(sizeof(struct FunctionCallNode));

  if (node) {
    node->function_id = function_id;
    // Set rest of the fields to default values
    node->stack_depth = 0;
    node->last_callee = NULL;
    node->prev_node = NULL;
  }
  return node;
}

// TODO: Change this to an iterative algorithm to prevent possibility of hitting stack limit
void destroy_function_call_node(struct FunctionCallNode *call_tree) {
  if (call_tree != NULL) {
    struct FunctionCallNode *children_nodes = call_tree->last_callee;
    struct FunctionCallNode *delete_node = NULL;

    while (children_nodes) {
      delete_node = children_nodes;
      children_nodes = children_nodes->prev_node;
      destroy_function_call_node(delete_node);
      delete_node = NULL;
    }
    free(call_tree);
    call_tree = NULL;
  }
}

struct FunctionCallNode *function_call_node_grow_tree_from_array(const struct FunctionNode *arr, size_t len) {
  // Root of the tree. Index 0 is reserved for identifying main
  struct FunctionCallNode *root = create_function_call_node(0);
  // Cheap hash table by using array index as key. Although perhaps not the most memory efficient. No need for a complex
  // hash function.
  const size_t hash_table_size = find_max_function_id(arr, len) + 1;
  struct FunctionCallNode **node_list = calloc(hash_table_size, sizeof(struct FunctionCallNode *));
  // Fill in hash table
  node_list[0] = root;
  for (size_t idx = 0; idx < len; idx++) {
    const size_t id = arr[idx].function_id;
    node_list[id] = create_function_call_node(id);
  }

  for (size_t idx = 0; idx < len; idx++) {
    const size_t caller_id = arr[idx].caller_id;
    const size_t id = arr[idx].function_id;
    function_call_node_add_callee(node_list[caller_id], node_list[id]);
  }

  update_stack_depth(root, 0);

  //Clean up hash table
  free(node_list);
  node_list = NULL;

  return root;
}

void function_call_node_add_callee(struct FunctionCallNode *caller, struct FunctionCallNode *callee) {
  if (caller->last_callee) {
    callee->prev_node = caller->last_callee;
  }
  caller->last_callee = callee;
  callee->stack_depth = caller->stack_depth + 1;
}

size_t function_call_node_get_num_nodes(const struct FunctionCallNode *tree) {
  if (tree == NULL) {
    return 0;
  } else {
    size_t num_nodes = 1;
    struct FunctionCallNode *children = tree->last_callee;
    while (children) {
      num_nodes += function_call_node_get_num_nodes(children);
      children = children->prev_node;
    }
    return num_nodes;
  }
}

struct FunctionCallTreeDFIter *create_function_call_tree_DF_iter(struct FunctionCallNode *call_tree) {
  size_t max_depth = function_call_node_get_num_nodes(call_tree);

  struct FunctionCallTreeDFIter *iter = malloc(sizeof(struct FunctionCallTreeDFIter));
  iter->nodes_to_visit = malloc(sizeof(struct FunctionCallNode *) * max_depth);
  iter->node_to_visit_size = 1;
  iter->nodes_to_visit[iter->node_to_visit_size - 1] = call_tree;
  return iter;
}

void destroy_function_call_tree_DF_iter(struct FunctionCallTreeDFIter *iter) {
  if (iter) {
    if (iter->nodes_to_visit) {
      free(iter->nodes_to_visit);
      iter->nodes_to_visit = NULL;
    }
    free(iter);
  }
}

bool function_call_tree_DF_iter_has_next(const struct FunctionCallTreeDFIter *iter) {
  return iter->node_to_visit_size > 0;
}

const struct FunctionCallNode *function_call_tree_DF_iter_next(struct FunctionCallTreeDFIter *iter) {
  struct FunctionCallNode *curr_node = iter->nodes_to_visit[iter->node_to_visit_size - 1];
  iter->node_to_visit_size--;
  struct FunctionCallNode *curr_node_children = curr_node->last_callee;

  const struct FunctionCallNode *next = curr_node;

  while (curr_node_children) {
    iter->nodes_to_visit[iter->node_to_visit_size] = curr_node_children;
    curr_node_children = curr_node_children->prev_node;
    iter->node_to_visit_size++;
  }

  return next;
}

// =====================================================================================================================
// Private helper function implementations
// =====================================================================================================================
static size_t find_max_function_id(const struct FunctionNode *arr, size_t len) {
  size_t max_id = 0;
  for (size_t idx = 0; idx < len; idx++) {
    if (arr[idx].function_id > max_id) {
      max_id = arr[idx].function_id;
    }
  }
  return max_id;
}

static void update_stack_depth(struct FunctionCallNode* tree, size_t depth) {
  if (tree != NULL) {
    tree->stack_depth = depth;
    struct FunctionCallNode *children = tree->last_callee;
    while (children) {
      update_stack_depth(children, depth + 1);
      children = children->prev_node;
    }
  }
}
