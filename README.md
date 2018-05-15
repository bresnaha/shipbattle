# shipbattle #

Authors: Dennis Chan, Cameron Chen, Cara Bresnahan

## Overview ##

[Image of Shipbattle]()

This is our networked reproduction of the classic board game battleship!
The server is capable of hosting a almost-real-time two-player epic,
animated shipbattle game.


## Running the Code ##
0. Navigate to the root directory of the repository `/shipbattle`

0. `make server/server` to compile server code.

1. `server/server <server port>` to run the executable and choose a port to set up the server.

2. `make` at the root to compile the captain (client) code. This executable requires input in the form
```sh
captain <server address> <server port>
```
where `server address` and `server port`. The server port for captain
should match the command line argument for server. An example of a successful server takes an input like: `./captain localhost 4444`, where the server is running on the same comptuer as the client(s).


## Running the Game ##

1. Once the game has started, enter your username! The username can be as
long as you want, but only eight characters will be displayed and used to
identify you! E.g. `charlie`, or `curtcharlie`

2. You will be prompted to place the five ships you're alloted, and their
sizes are {2, 3, 3, 4, 5}. The ships cannot overlap or have any parts of a
ship go out-of-bound of the 10x10 sea grid. Their placement are sequential
in the ascending order of their sizes: the first ship you enter has the
size two, three after, another three after... etc. The actual placement
takes the following form: `X Y O`, where `X` indicates the location along
the x axis, `Y` the y axis, `O` is an character input either `h` or `v`
indicating the orientation of horizontal or vertical, a <= Y <= j and 0 <= `Y`
< 10.

3. After both you and your opponent has set up your boards, you will be
prompted to drop bombs in the form of `X Y`. The same constraints apply to
`X` and `Y` as step 2.

4. Keep playing until you have sunk your opponent's ships! 


## Notes ##

- The terminal window needs to be *sufficiently* large for the user interface to properly render. 


## Acknowledgement ##

Thanks for Charlie Curtsinger for debugging help. 
