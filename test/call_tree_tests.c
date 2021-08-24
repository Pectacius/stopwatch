#include "call_tree.h"

#include <assert.h>

void test_call_tree_single_node_from_root() {
  struct FunctionNode test_node = {1, 0};

  struct FunctionCallNode *tree = function_call_node_grow_tree_from_array(&test_node, 1);
  struct FunctionCallTreeDFIter *iter = create_function_call_tree_DF_iter(tree);

  // Root Node
  assert(function_call_tree_DF_iter_has_next(iter) == true);
  const struct FunctionCallNode *next = function_call_tree_DF_iter_next(iter);
  assert(next->function_id == 0);
  assert(next->stack_depth == 0);

  // Single node attached to root
  assert(function_call_tree_DF_iter_has_next(iter) == true);
  next = function_call_tree_DF_iter_next(iter);
  assert(next->function_id == 1);
  assert(next->stack_depth == 1);

  destroy_function_call_tree_DF_iter(iter);
  destroy_function_call_node(tree);
}

void test_call_tree_multi_nodes_from_root() {
#define multi_nodes_from_root_size 3
  struct FunctionNode test_node[multi_nodes_from_root_size] = {{1, 0}, {3, 0}, {9, 0}};

  struct FunctionCallNode
      *tree = function_call_node_grow_tree_from_array(test_node, multi_nodes_from_root_size);
  struct FunctionCallTreeDFIter *iter = create_function_call_tree_DF_iter(tree);

  // Root Node
  assert(function_call_tree_DF_iter_has_next(iter) == true);
  const struct FunctionCallNode *next = function_call_tree_DF_iter_next(iter);
  assert(next->function_id == 0);
  assert(next->stack_depth == 0);

  // In this special case, the expected node traversal order after the root is identical to the input order
  for (size_t idx = 0; idx < multi_nodes_from_root_size; idx++) {
    assert(function_call_tree_DF_iter_has_next(iter) == true);
    next = function_call_tree_DF_iter_next(iter);
    assert(next->function_id == test_node[idx].function_id);
    assert(next->stack_depth == 1);
  }

  destroy_function_call_tree_DF_iter(iter);
  destroy_function_call_node(tree);
}

void test_call_tree_linked_list_tree() {
#define linked_list_tree_size 5
  struct FunctionNode test_node[linked_list_tree_size] = {{1, 5}, {5, 0}, {4, 10}, {9, 4}, {10, 1}};

  struct FunctionCallNode
      *tree = function_call_node_grow_tree_from_array(test_node, linked_list_tree_size);
  struct FunctionCallTreeDFIter *iter = create_function_call_tree_DF_iter(tree);

  // Root Node
  assert(function_call_tree_DF_iter_has_next(iter) == true);
  const struct FunctionCallNode *next = function_call_tree_DF_iter_next(iter);
  assert(next->function_id == 0);
  assert(next->stack_depth == 0);

  // The pointer fields do not matter for this test case, hence they are just kept at NULL
  struct FunctionCallNode expected_order[linked_list_tree_size] =
      {{5, 1, NULL, NULL}, {1, 2, NULL, NULL}, {10, 3, NULL, NULL}, {4, 4, NULL, NULL}, {9, 5, NULL, NULL}};
  // In this special case, the expected node traversal order after the root is identical to the input order
  for (size_t idx = 0; idx < linked_list_tree_size; idx++) {
    assert(function_call_tree_DF_iter_has_next(iter) == true);
    next = function_call_tree_DF_iter_next(iter);
    assert(next->function_id == expected_order[idx].function_id);
    assert(next->stack_depth == expected_order[idx].stack_depth);
  }

  destroy_function_call_tree_DF_iter(iter);
  destroy_function_call_node(tree);
}

void test_call_tree_multi_level_different_n() {
#define multi_level_different_n_size 6
  struct FunctionNode test_node[multi_level_different_n_size] = {{4, 0}, {9, 10}, {10, 0}, {12, 10}, {20, 10}, {21, 4}};

  struct FunctionCallNode
      *tree = function_call_node_grow_tree_from_array(test_node, multi_level_different_n_size);
  struct FunctionCallTreeDFIter *iter = create_function_call_tree_DF_iter(tree);

  // Root Node
  assert(function_call_tree_DF_iter_has_next(iter) == true);
  const struct FunctionCallNode *next = function_call_tree_DF_iter_next(iter);
  assert(next->function_id == 0);
  assert(next->stack_depth == 0);

  // The pointer fields do not matter for this test case, hence they are just kept at NULL
  struct FunctionCallNode expected_order[multi_level_different_n_size] =
      {{4, 1, NULL, NULL}, {21, 2, NULL, NULL}, {10, 1, NULL, NULL}, {9, 2, NULL, NULL}, {12, 2, NULL, NULL},
       {20, 2, NULL, NULL}};
  // In this special case, the expected node traversal order after the root is identical to the input order
  for (size_t idx = 0; idx < multi_level_different_n_size; idx++) {
    assert(function_call_tree_DF_iter_has_next(iter) == true);
    next = function_call_tree_DF_iter_next(iter);
    assert(next->function_id == expected_order[idx].function_id);
    assert(next->stack_depth == expected_order[idx].stack_depth);
  }

  destroy_function_call_tree_DF_iter(iter);
  destroy_function_call_node(tree);
}

void test_call_tree_multi_level_balanced() {
#define multi_level_balanced_size 16
  struct FunctionNode test_node[multi_level_balanced_size] =
      {{1, 7}, {2, 16}, {3, 12}, {4, 1}, {5, 10}, {6, 5}, {7, 0}, {8, 1}, {9, 3}, {10, 0}, {11, 5}, {12, 0}, {13, 3},
       {14, 2}, {15, 2}, {16, 0}};

  struct FunctionCallNode
      *tree = function_call_node_grow_tree_from_array(test_node, multi_level_balanced_size);
  struct FunctionCallTreeDFIter *iter = create_function_call_tree_DF_iter(tree);

  // Root Node
  assert(function_call_tree_DF_iter_has_next(iter) == true);
  const struct FunctionCallNode *next = function_call_tree_DF_iter_next(iter);
  assert(next->function_id == 0);
  assert(next->stack_depth == 0);

  // The pointer fields do not matter for this test case, hence they are just kept at NULL
  struct FunctionCallNode expected_order[multi_level_balanced_size] =
      {{7, 1, NULL, NULL}, {1, 2, NULL, NULL}, {4, 3, NULL, NULL}, {8, 3, NULL, NULL}, {10, 1, NULL, NULL},
       {5, 2, NULL, NULL}, {6, 3, NULL, NULL}, {11, 3, NULL, NULL}, {12, 1, NULL, NULL}, {3, 2, NULL, NULL},
       {9, 3, NULL, NULL}, {13, 3, NULL, NULL}, {16, 1, NULL, NULL}, {2, 2, NULL, NULL}, {14, 3, NULL, NULL},
       {15, 3, NULL, NULL}};
  // In this special case, the expected node traversal order after the root is identical to the input order
  for (size_t idx = 0; idx < multi_level_balanced_size; idx++) {
    assert(function_call_tree_DF_iter_has_next(iter) == true);
    next = function_call_tree_DF_iter_next(iter);
    assert(next->function_id == expected_order[idx].function_id);
    assert(next->stack_depth == expected_order[idx].stack_depth);
  }

  destroy_function_call_tree_DF_iter(iter);
  destroy_function_call_node(tree);
}

void test_call_tree_multi_level_unbalanced() {
#define multi_level_unbalanced_size 10
  struct FunctionNode test_node[multi_level_unbalanced_size] =
      {{1, 4}, {2, 3}, {3, 0}, {4, 0}, {5, 3}, {6, 2}, {7, 3}, {8, 7}, {9, 3}, {10, 5}};

  struct FunctionCallNode
      *tree = function_call_node_grow_tree_from_array(test_node, multi_level_unbalanced_size);
  struct FunctionCallTreeDFIter *iter = create_function_call_tree_DF_iter(tree);

  // Root Node
  assert(function_call_tree_DF_iter_has_next(iter) == true);
  const struct FunctionCallNode *next = function_call_tree_DF_iter_next(iter);
  assert(next->function_id == 0);
  assert(next->stack_depth == 0);

  // The pointer fields do not matter for this test case, hence they are just kept at NULL
  struct FunctionCallNode expected_order[multi_level_unbalanced_size] =
      {{3, 1, NULL, NULL}, {2, 2, NULL, NULL}, {6, 3, NULL, NULL}, {5, 2, NULL, NULL}, {10, 3, NULL, NULL},
       {7, 2, NULL, NULL}, {8, 3, NULL, NULL}, {9, 2, NULL, NULL}, {4, 1, NULL, NULL}, {1, 2, NULL, NULL}};
  // In this special case, the expected node traversal order after the root is identical to the input order
  for (size_t idx = 0; idx < multi_level_unbalanced_size; idx++) {
    assert(function_call_tree_DF_iter_has_next(iter) == true);
    next = function_call_tree_DF_iter_next(iter);
    assert(next->function_id == expected_order[idx].function_id);
    assert(next->stack_depth == expected_order[idx].stack_depth);
  }

  destroy_function_call_tree_DF_iter(iter);
  destroy_function_call_node(tree);

}

void test_call_tree_large_input() {
#define large_input_size 56
  struct FunctionNode test_node[large_input_size] =
      {{1, 0}, {2, 1}, {5, 0}, {6, 0}, {7, 0}, {10, 1}, {15, 74}, {16, 74}, {17, 74}, {18, 15}, {19, 15}, {20, 10},
       {21, 10}, {22, 10}, {23, 10}, {24, 10}, {25, 10}, {30, 21}, {31, 21}, {33, 10}, {34, 33}, {36, 33}, {37, 33},
       {38, 33}, {39, 33}, {40, 1}, {41, 40}, {45, 40}, {46, 40}, {47, 40}, {48, 40}, {52, 36}, {53, 24}, {54, 24},
       {55, 24}, {56, 24}, {60, 1}, {65, 60}, {67, 60}, {70, 1}, {71, 2}, {73, 38}, {74, 38}, {80, 1}, {81, 80},
       {82, 80}, {83, 37}, {84, 37}, {91, 48}, {92, 48}, {93, 73}, {94, 73}, {96, 73}, {98, 73}, {99, 1}, {460, 46}};

  struct FunctionCallNode
      *tree = function_call_node_grow_tree_from_array(test_node, large_input_size);
  struct FunctionCallTreeDFIter *iter = create_function_call_tree_DF_iter(tree);

  // Root Node
  assert(function_call_tree_DF_iter_has_next(iter) == true);
  const struct FunctionCallNode *next = function_call_tree_DF_iter_next(iter);
  assert(next->function_id == 0);
  assert(next->stack_depth == 0);

  // The pointer fields do not matter for this test case, hence they are just kept at NULL
  struct FunctionCallNode expected_order[large_input_size] =
      {{1, 1}, {2, 2}, {71, 3}, {10, 2}, {20, 3}, {21, 3}, {30, 4}, {31, 4}, {22, 3}, {23, 3}, {24, 3}, {53, 4},
       {54, 4}, {55, 4}, {56, 4}, {25, 3}, {33, 3}, {34, 4}, {36, 4}, {52, 5}, {37, 4}, {83, 5}, {84, 5}, {38, 4},
       {73, 5}, {93, 6}, {94, 6}, {96, 6}, {98, 6}, {74, 5}, {15, 6}, {18, 7}, {19, 7}, {16, 6}, {17, 6}, {39, 4},
       {40, 2}, {41, 3}, {45, 3}, {46, 3}, {460, 4}, {47, 3}, {48, 3}, {91, 4}, {92, 4}, {60, 2}, {65, 3}, {67, 3},
       {70, 2}, {80, 2}, {81, 3}, {82, 3}, {99, 2}, {5, 1}, {6, 1}, {7, 1}};
  // In this special case, the expected node traversal order after the root is identical to the input order
  for (size_t idx = 0; idx < large_input_size; idx++) {
    assert(function_call_tree_DF_iter_has_next(iter) == true);
    next = function_call_tree_DF_iter_next(iter);
    assert(next->function_id == expected_order[idx].function_id);
    assert(next->stack_depth == expected_order[idx].stack_depth);
  }

  destroy_function_call_tree_DF_iter(iter);
  destroy_function_call_node(tree);

}

int main() {
  test_call_tree_single_node_from_root();

  test_call_tree_multi_nodes_from_root();

  test_call_tree_linked_list_tree();

  test_call_tree_multi_level_different_n();

  test_call_tree_multi_level_balanced();

  test_call_tree_multi_level_unbalanced();

  test_call_tree_large_input();
}
