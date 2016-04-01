#define NUMPLAYERS 2		// number of players
enum {PLAYER_1, PLAYER_2};	// player information indices

enum SC_SIGTYPE {		// server-to-client signal types
	SC_STD			// standard message containing vars
};

enum CS_SIGTYPE {		// client-to-server signal types
	CS_STD			// standard message containing vars
};

#define SC_STDSIZE 2		// server to client packet size
enum SC {P1DIR, P2DIR};		// server to client buffer indices

#define CS_STDSIZE 1		// client to server packet size
enum CS {PDIR};			// server to client buffer indices

