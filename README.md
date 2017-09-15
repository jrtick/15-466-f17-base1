# *Carcassonne Lite*

*Carcassonne Lite* is *Jordan Tick*'s implementation of [*Carcassonne Lite*](http://graphics.cs.cmu.edu/courses/15-466-f17/game1-designs/jrtick) for game1 in 15-466-f17.

*Include a Screenshot Here*

## Build Notes
Gameplay defaults to being between two players. If you run ./main X, you will play with X players.

## Asset Pipeline

I had drew the Carcassonne tiles based off the tileset provided in the design document. 
I then downloaded a free font known as Maxwell, which I took a screenshot of from (https://www.behance.net/gallery/33291771/FREE-Maxwell-Font-Family).
I concatenated these two images into one image, which I then use offsets from in my code to certain tiles/characters.

## Architecture

*Provide a brief introduction to how you implemented the design. Talk about the basic structure of your code.*

## Reflection

*Reflect on the assignment. What was difficult? What worked well? If you were doing it again, what would you change?*

*Reflect on the design document. What was clear and what was ambiguous? How did you resolve the ambiguities?*


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
