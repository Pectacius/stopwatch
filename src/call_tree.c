#include "call_tree.h"

#include <stdlib.h>

// =====================================================================================================================
// Private helper methods definitions
// =====================================================================================================================
static size_t find_max_function_id(const struct FunctionNode* arr, size_t len);

// =====================================================================================================================
// Public functions implementations
// =====================================================================================================================
struct FunctionCallNode* create_function_node(size_t function_id) {
  struct FunctionCallNode* node = malloc(sizeof(struct FunctionCallNode));

  if (node) {
    node->function_id = function_id;
    // Set rest of the fields to default values
    node->stack_depth = 0;
    node->num_elem = 0;
    node->callee = NULL;
    node->next = NULL;
  }
  return node;
}

// TODO: Change this to an iterative algorithm to prevent possibility of hitting stack limit
void destroy_tree(struct FunctionCallNode* call_tree) {
  if (call_tree != NULL) {
    destroy_tree(call_tree->callee);
    destroy_tree(call_tree->next);
    free(call_tree);
  }
}

struct FunctionCallNode* grow_tree_from_array(const struct FunctionNode* arr, size_t len) {
  struct FunctionCallNode* root; // Root of the tree
  // Cheap hash table by using array index as key. Although perhaps not the most memory efficient. No need for a complex
  // hash function.
  const size_t hash_table_size = find_max_function_id(arr, len) + 1;
  struct FunctionCallNode** node_list = calloc(hash_table_size, sizeof(struct FunctionCallNode*));
  // Fill in hash table
  for (size_t idx = 0; idx < len; idx++) {
    const size_t id = arr[idx].function_id;
    node_list[id] = create_function_node(id);

    //TODO: Change such that the root id is flexible
    // Assumes root is ID 1.
    if (id == 1) {
      root = node_list[id];
    }
  }

  for(size_t idx = 0; idx < len; idx++) {
    const size_t caller_id = arr[idx].caller_id;
    const size_t id = arr[idx].function_id;
    add_callee(node_list[caller_id], node_list[id]);
  }

  //Clean up hash table
  free(node_list);

  return root;
}

void add_same_lvl(struct FunctionCallNode* original_node, struct FunctionCallNode* new_node) {
  if (original_node->next == NULL) {
    original_node->next = new_node;
  } else {
    original_node->next[original_node->num_elem-1].next = new_node;
  }
  new_node->stack_depth = original_node->stack_depth;
  new_node->num_elem = original_node->num_elem;
  original_node->num_elem++;
}

void add_callee(struct FunctionCallNode* caller, struct FunctionCallNode* callee) {
  if (caller->callee == NULL) {
    caller->callee = callee;
    callee->stack_depth = caller->stack_depth + 1;
  } else {
    add_same_lvl(caller->callee, callee);
  }
}

// =====================================================================================================================
// Private helper function implementations
// =====================================================================================================================
static size_t find_max_function_id(const struct FunctionNode* arr, size_t len) {
  size_t max_id = 0;
  for (size_t idx = 0; idx < len; idx++) {
    if(arr[idx].function_id > max_id) {
      max_id = arr[idx].function_id;
    }
  }
  return max_id;
}