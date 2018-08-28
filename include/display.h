#ifndef NGP_DISPLAY_H
#define NGP_DISPLAY_H

#include "search.h"

struct display;

/* API ************************************************************************/
void display_loop(struct display *this, const struct search *search);

/* CONSTRUCTOR ****************************************************************/
struct display * display_new(void);
void display_delete(struct display *this);

#endif /* NGP_DISPLAY_H */
