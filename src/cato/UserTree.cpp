#include "UserTree.h"

#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>

#include "debug.h"

using namespace llvm;

UserTree::UserTree(Value *root)
{
    _root = new UserTreeNode;
    _root->value = root;
    _root->parent = nullptr;
    generate_tree(_root);
}

UserTree::~UserTree() { delete_tree(_root); }

void UserTree::generate_tree(UserTreeNode *curr_node)
{
    // If the current instruction has users we extend the tree
    if (curr_node->value->getNumUses() > 0)
    {
        // For each user create a new child node for the current node and
        // recursively call this function again to keep generating the tree
        for (auto *user : curr_node->value->users())
        {
            auto new_node = new UserTreeNode;
            new_node->value = user;
            new_node->parent = curr_node;
            curr_node->neighbours.push_back(new_node);
            generate_tree(new_node);
        }
    }
    // If the current instruction does not have any users it is either a leaf
    // or the following users are not in this function and we have to follow
    // a return statement or a function call
    else
    {
        if (auto *return_inst = dyn_cast<ReturnInst>(curr_node->value))
        {
            Debug(errs() << "Path ends in return instruction\n";);
            Debug(errs() << "Following the instruction chain into the callee function\n";);
            auto func = return_inst->getFunction();
            for (auto *user : func->users())
            {
                auto new_node = new UserTreeNode;
                new_node->value = user;
                new_node->parent = curr_node;
                curr_node->neighbours.push_back(new_node);
                generate_tree(new_node);
            }
        }
        else if (auto *call_inst = dyn_cast<CallInst>(curr_node->value))
        {
            Debug(errs() << "Path ends in call instruction\n";);

            auto *last_inst = curr_node->parent->value;
            Value *matching_argument = nullptr;

            for (unsigned int i = 0; i < call_inst->arg_size(); i++)
            {
                if (last_inst == call_inst->getArgOperand(i))
                {
                    Function *called_function = call_inst->getCalledFunction();
                    if (called_function->arg_begin() + i < called_function->arg_end())
                    {
                        matching_argument = called_function->arg_begin() + i;
                    }
                }
            }

            if (matching_argument != nullptr && matching_argument->hasNUsesOrMore(1))
            {
                Debug(
                    errs()
                        << "    Following the instruction chain into the called function\n";);

                for (auto *user : matching_argument->users())
                {
                    auto new_node = new UserTreeNode;
                    new_node->value = user;
                    new_node->parent = curr_node;
                    curr_node->neighbours.push_back(new_node);
                    generate_tree(new_node);
                }
            }
            else
            {
                Debug(errs() << "    The instruction chain can't be followed into the called "
                                "function\n";);
                _leafs.push_back(curr_node);
            }
        }
        else
        {
            _leafs.push_back(curr_node);
        }
    }
}

void UserTree::delete_tree(UserTreeNode *curr_node)
{
    if (curr_node->neighbours.size() > 0)
    {
        for (auto *node : curr_node->neighbours)
        {
            delete_tree(node);
        }
        delete curr_node;
    }
    else
    {
        delete curr_node;
    }
}

std::vector<std::vector<llvm::Value *>> UserTree::get_all_paths()
{
    std::vector<std::vector<llvm::Value *>> paths;
    paths.reserve(_leafs.size());

    for (auto *leaf : _leafs)
    {
        std::vector<Value *> tmp_path;
        UserTreeNode *tmp_node = leaf;

        while (tmp_node != nullptr)
        {
            tmp_path.push_back(tmp_node->value);
            tmp_node = tmp_node->parent;
        }
        paths.push_back(std::vector<Value *>(tmp_path.rbegin(), tmp_path.rend()));
    }

    return paths;
}
