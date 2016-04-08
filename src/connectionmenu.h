#ifndef CONNECTIONFAIL_MENU
#define CONNECTIONFAIL_MENU

enum ConnectionStatus { CONNECTION_FAIL, CONNECTION_SUCCESS };

/* Show menu indiciating connection error to user. */
void showconnectionstatus(enum ConnectionStatus status);

#endif // CONNECTIONFAIL_MENU
