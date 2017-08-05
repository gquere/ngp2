#ifndef NGP_DISPLAY_H
#define NGP_DISPLAY_H

struct display;

/* API ************************************************************************/
void display_loop(struct display *this, const struct search *search, const struct entries *entries);

/* CONSTRUCTOR ****************************************************************/
struct display * display_new(void);
void display_delete(struct display *this);

#endif /* NGP_DISPLAY_H */
