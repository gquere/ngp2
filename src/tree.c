#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "tree.h"


/* UTILS **********************************************************************/
static struct leaf * leaf_new(void)
{
    return calloc(1, sizeof(struct leaf));
}

static struct leaf * add_to_leaf(struct leaf *this, const char letter)
{
    int index = letter - 97;

    if (index < 0 || index >= 26) {
        return NULL;
    }

    if (this->leaves[index]) {
        return this->leaves[index];
    }

    this->leaves[index] = leaf_new();

    return this->leaves[index];
}

/* recursively delete leaves */
static void leaves_delete(struct leaf *this)
{
    if (!this) {
        return;
    }

    int i = 0;
    for (i = 0; i < 26; i++) {
        leaves_delete(this->leaves[i]);
    }

    free(this);
}


/* API ************************************************************************/
uint8_t is_string_in_tree(const struct tree *this, const char *string)
{
    size_t i = 0;
    struct leaf *leaf = this->root;

    for (i = 0; i < strlen(string); i++) {
        int index = string[i] - 97;

        if (index < 0 || index >= 26) {
            return 0;
        }

        leaf = leaf->leaves[index];
        if (!leaf) {
            return 0;
        }
    }

    return leaf->terminate;
}

void tree_add_string(struct tree *this, const char *string)
{
    size_t i = 0;
    struct leaf *leaf = this->root;

    for (i = 0; i < strlen(string); i++) {
        leaf = add_to_leaf(leaf, string[i]);
        if (leaf == NULL) {
           return;
        }
    }

    leaf->terminate = 1;
}


/* CONTRUCTOR *****************************************************************/
struct tree * tree_new(void)
{
    struct tree *this = calloc(1, sizeof(struct tree));
    this->root = leaf_new();

    return this;
}

void tree_delete(struct tree *this)
{
    leaves_delete(this->root);
    free(this);
}
