# shipbattle #

## Overview ##


## Running the code ##
0. Navigate to the root directory of the repository `/shipbattle`

0. `make server/server` to compile server code.

1. `server/server <server port>` to run the executable and choose a port to set up the server.

2. `make` at the root to compile the captain (client) code. This executable requires input in the form
```sh
captain <server address> <server port>
```
where `server address` and `server port`. The server port is currently static on 4444. An example of a successful server takes an input like: `./captain localhost 4444`, where the server is running on the same comptuer as the client(s).


## Notes ##

- The server port will default to 4444 until further notice.

- The terminal window needs to be *sufficiently* large for the user interface to properly render. 
