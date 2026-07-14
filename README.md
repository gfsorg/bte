# BTE (Basic Text Editor)

Welcome! This is my text editor.

## Features

### Line Number
To make the line number you're on clear; BTE has a line number interface; Making it clearer to see what line you're on.

### Modes
BTE has 2 modes, normal mode and insert mode. By default you're in insert mode; to switch to normal mode, you simply press Esc. To get back to insert mode you press i.

### Commands
Now once you're in normal mode, you can enter commands using the slash key, the commands ar listed below:
/s (to save)
/sa (to save as. e.g.: /sa example.txt)
/q (to exit)
/qs (to save and then exit)
/qsa (to save as and then exit)
/edit (to enter insert mode)

## Installation
To install BTE, enter your terminal, and type these commands:

(if you haven't already, install git by using sudo apt install git)

git clone https://www.github.com/gfsorg/bte.git

after you've cloned the repo, run:

cd bte && chmod +x install.sh && ./install.sh

that's it! You can now run bte by running bte/bte filename
