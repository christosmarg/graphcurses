# graphcurses

A simple ncurses graph generator.

![preview](https://user-images.githubusercontent.com/54286563/102025554-66e14380-3da1-11eb-81e0-ea7cfc8c1161.png)

## Dependencies

* `ncurses` 
* `libmatheval`

## Usage

```shell
$ cd path/to/graphcurses
$ make && make run
$ make clean # optional
```

You can install `graphcurses` by running `sudo make install clean`.
The binary will be installed at `/usr/local/bin`

## To Do

* Add slope, curvature etc.
* Add curses coordinates
* Fix axes plotting bug
* Add point numbering on axes
