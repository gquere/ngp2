ngp2
====

ncurses-grep
------------

ngp2 lets you browse results of a grep-like search in a Vim-like style and open them in your favourite editor (vim) at the right line.


![yay](/demo.gif)


Features
--------

- search for a pattern/a regexp in a folder or a file
- by default, only source files are scanned, though a raw mode (-r) or special extensions may be specified (-t, -o)
- subsearches: search your search!


Usage
-----

- use arrows and page up/down or home/end to navigate the results
- hit enter to open a result in your favorite editor
- hit q to quit the current subsearch or otherwise quit the program
- use / for a subsearch to include new pattern
- use \ for a subsearch to exclude new pattern

Examples
--------

### Find all occurrences of "int" in sources files under /usr/src
```
ngp int /usr/src/
```

### Find case insensitive string "passw" in all files recursively under ./data/
```
ngp -ri passw ./data/
```

### Find all occurrences of "test(" only in python files, recursively under parent
```
ngp -o .py 'test(' ..
```

### Find numbers using a regexp in the current dir, ignoring directory ./test/
```
ngp -re '[0-9]\+' -x test
```

Requirements
------------

- libncurses-dev


Installation
------------

- make
- make install
