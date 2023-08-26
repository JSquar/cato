#ifndef CATO_USER_TREE_H
#define CATO_USER_TREE_H

#include <llvm/IR/User.h>
#include <llvm/IR/Value.h>

#include <memory>
#include <utility>
#include <vector>

/**
 * Tree node
 *
 * Consists of:
 *  value: The instruction that the node represents
 *  parent: parent node in the tree
 *  neighbours: child nodes of this node in the tree
 **/
struct UserTreeNode
{
    llvm::Value *value;
    UserTreeNode *parent;
    std::vector<UserTreeNode *> neighbours;
};

/**
 * A UserTree is a directed acyclic graph (DAG) of IR instructions (in form of User* objects)
 *
 * The root of a UserTree is an IR instruction.
 * The tree gets build by inspecting all Users of the current node and adding them
 * as child nodes. This process is repeated recursively until it reaches nodes that don't
 * have users.
 * Effectively the UserTree shows all chains of instructions in the code that directly
 * or indirectly use the root node instruction.
 * This can for example be used to find all memory accesses to a pointer, by following
 * the instruction chains from the pointer declaration until you find a load, store or
 * free instruction
 **/
class UserTree
{
  private:
    UserTreeNode *_root;

    std::vector<UserTreeNode *> _leafs;

    /**
     * Create the tree recursively
     **/
    void generate_tree(UserTreeNode *curr_node);

    /**
     * Delete the tree recursively and free all memory
     **/
    void delete_tree(UserTreeNode *curr_node);

    bool has_cycle_with_user(UserTreeNode *curr_node, llvm::User* user);

  public:
    /**
     * Constructor automatically creates the full UserTree for the passed root instruction
     **/
    UserTree(llvm::Value *root);

    ~UserTree();

    /**
     * Returns all paths from the root node to all leaf nodes
     **/
    std::vector<std::vector<llvm::Value *>> get_all_paths();
};

#endif
