# *Carcassonne Lite*

*Carcassonne Lite* is *Jordan Tick*'s implementation of [*Carcassonne Lite*](http://graphics.cs.cmu.edu/courses/15-466-f17/game1-designs/jrtick) for game1 in 15-466-f17.


![alt text](https://raw.githubusercontent.com/jrtick/15-466-f17-base1/master/screenshots/screenshot.png)

## Build Notes
Gameplay defaults to being between two players. If you run "./main X" instead of just "./main", you will play with X players.

To rotate a tile, you can either right click or press 'r'.

## Asset Pipeline

I drew the Carcassonne tiles based off the tileset provided in the design document. 
I then downloaded a free font known as Maxwell, which I took a screenshot of from (https://www.behance.net/gallery/33291771/FREE-Maxwell-Font-Family).
I concatenated these two images into one image, from which I defined offsets into so that I could reference certain tiles/characters in my code.

## Architecture
I like recursive data structures! I modelled the board as a graph, where every tile knows about its left/right/up/down neighbor. These pointers are NULL if no such neighbor exists.
The benefit of this data structure is that I wouldn't have to keep expanding some 2D grid representation of the board every time a new tile was added. Also, operations to find out if a castle was completed could nicely be written recursively as a sort of 'flood fill operation'.

The difficulty, however, was keeping a 'processed' flag on each tile and making sure these flags didn't get corrupted. If a function wanted to use the processed flag, it had to eventually flip ALL processed flags on the board to maintin validity.

For other architecture notes:
 - In my texture atlas, I stored everything as a 2D grid so things could be easily indexed.
 - turn cycles to next player when a tile is placed, meaning it happens when the user clicks a valid spot on the board causing the current tile to be placed
 - Interaction with the board stops being possible once game over occurs (all 71 tiles placed)

## Reflection

The texture atlas part of the project worked very well! There were no hiccups once I understood texture coordinates went from (0,0) in the bottom-left and (1,1) in the top-right.

The hardest part of this project was making the recursive board data structure fully functional. But it was also the most fun! I feel like the solutions are much more elegant.

If I had more time on this project, I would've picked a more complete font, centered the letters better on my texture atlas, and created a more polished GUI, potentially highlighting on the board where the current tile could potentially be placed. I like my data structures the way they currently are, though, so I wouldn't want to change much of anything already implemented.

I actually wrote this design document, so I'd say the instructions were pretty clear to me!


# About Base1

This game is based on Base1, starter code for game1 in the 15-466-f17 course. It was developed by Jim McCann, and is released into the public domain.

## Requirements

 - modern C++ compiler
 - glm
 - libSDL2
 - libpng

On Linux or OSX these requirements should be available from your package manager without too much hassle.

## Building

This code has been set up to be built with [FT jam](https://www.freetype.org/jam/).

### Getting Jam

For more information on Jam, see the [Jam Documentation](https://www.perforce.com/documentation/jam-documentation) page at Perforce, which includes both reference documentation and a getting started guide.

On unixish OSs, Jam is available from your package manager:
```
	brew install ftjam #on OSX
	apt get ftjam #on Debian-ish Linux
```

On Windows, you can get a binary [from sourceforge](https://sourceforge.net/projects/freetype/files/ftjam/2.5.2/ftjam-2.5.2-win32.zip/download),
and put it somewhere in your `%PATH%`.
(Possibly: also set the `JAM_TOOLSET` variable to `VISUALC`.)

### Bulding
Open a terminal (on windows, a Visual Studio Command Prompt), change to this directory, and type:
```
	jam
```

### Building (local libs)

Depending on your OSX, clone 
[kit-libs-linux](https://github.com/ixchow/kit-libs-linux),
[kit-libs-osx](https://github.com/ixchow/kit-libs-osx),
or [kit-libs-win](https://github.com/ixchow/kit-libs-win)
as a subdirectory of the current directory.

The Jamfile sets up library and header search paths such that local libraries will be preferred over system libraries.
