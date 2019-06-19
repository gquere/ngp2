#ifndef NGP_TREE_H
#define NGP_TREE_H

struct leaf
{
    struct leaf * leaves[256];  // ascii alphabet
    int terminate;              // set if a word finishes on the leaf
};

struct tree
{
    struct leaf *root;
};


/* API ************************************************************************/
uint8_t is_string_in_tree(const struct tree *this, const char *string);
uint8_t is_string_in_tree_size(const struct tree *this, const char *string, const size_t string_len);
void tree_add_string(struct tree *this, const char *string);

/* CONTRUCTOR *****************************************************************/
struct tree * tree_new(void);
void tree_delete(struct tree *this);

#endif /* NGP_TREE_H */
