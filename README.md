ngp2
====

ncurses-grep
------------

ngp2 is a rewrite from the discontinued [ncurses-grep](https://github.com/gquere/ngp), aiming to be more stable process-wise and reliable search-wise.

It lets you browse results of a grep-like search in a Vim-like style and open them in your favourite editor (vim) at the right line.


![yay](/capture.png)


Features
--------

- search for a pattern/a regexp in a folder or a file
- by default, only source files are scanned, though a raw mode (-r) or special extensions may be specified (-t, -o)
- by default, doesn't follow symlinks to files, override with -f
- subsearch: search your search! (use '/' after the first results)


Usage
-----

- use arrows and page up/down to navigate the results, or the vim keys
- hit enter to open a result in your favorite editor
- hit q to quit the current search or the program

Example:
```
ngp int /usr/src/
```


Requirements
------------

- libncurses-dev


Installation
------------

- make
- make install
