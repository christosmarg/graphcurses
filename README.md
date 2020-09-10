# graphcurses

A simple ncurses graph generator.

![Screenshot_20200308_212613](https://user-images.githubusercontent.com/54286563/76169715-b3994d80-6183-11ea-97e0-4003239ecfe3.png)

## Dependencies

* `ncurses` 
* `matheval`

## Usage

```shell
$ cd path/to/graphcurses
$ make
$ make run
```
In order to install do
```shell
$ cd path/to/graphcurses
$ make
$ sudo make install
$ make clean # optional
```
The binary will be installed at `/usr/local/bin/`

## To Do

* Improve key handling
* Add slope, curvature etc.
* ~~Add an options window~~
* Add coordinates using cursor pointing
* ~~Add derivative calculator~~
* Fix axes plotting bug
* Add point numbering on axes
