ngp2
====

ncurses-grep
------------

ngp2 is a rewrite from the discontinued [ncurses-grep](https://github.com/gquere/ngp), aiming to be more stable process-wise and reliable search-wise.

It lets you browse results of a grep-like search in a Vim-like style and open them in your favourite editor (vim) at the right line.

Features
--------

- search for a pattern/a regexp in a folder or a file
- by default, only source files are scanned, though a raw mode or special extensions may be specified
- follow/no-follow symlinks, exclude directories, ignore case

Usage
-----

- use arrows and page up/down to navigate the results
- hit enter to open a result
- hit q to quit the current search


Requirements
------------

- libncurses-dev


Installation
------------

- make
- make install
