## SAPGUI MultiLogon

A little tool to return an indispensable feature to SAPGUI for Windows (versions >710):

*The ctrl+click on radio buttons*

![Example usage](img/example.jpg?raw=true)

If you're an SAP consultant, you know what that's for ;)

This tool is not a patch and doesn't modify any files on your computer (specially not any file from SAP owned software!). It works by patching SAPGUI's running code in memory.

### Compilation

I use the Pelles C IDE and compiler, but you can easily compile it using any other Windows C11 compiler.

### How to use

Just run the application. It will find your local SAPGUI installation and start it.

It also works if SAPGUI is already started.

### License

This code is distributed under the MIT License, meaning you can freely and unrestrictedly use it, change it, share it, distribute it and package it with your own programs as long as you keep the copyright notice, license and disclaimer.

Copyright(c) 2015 Guilherme Maeda

http://abap.ninja