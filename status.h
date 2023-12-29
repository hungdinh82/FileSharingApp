/*
* define status 
* feature: 
*/

#ifndef __STATUS_H__
#define __STATUS_H__

typedef enum {
	USER_NOT_FOUND = 101,
	USER_IS_BLOCKED = 102,
	BLOCKED_USER = 103,
	PASSWORD_INVALID = 104,
	USER_IS_ONLINE = 105,
	ACCOUNT_IS_EXIST = 106,
	USERNAME_OR_PASSWORD_INVALID = 107,
	FILE_NOT_FOUND = 108,
	LOGIN_SUCCESS = 201,
	REGISTER_SUCCESS = 202,
	LOGOUT_SUCCESS = 203,
	COMMAND_INVALID = 301,
	SERVER_ERROR = 500
} StatusCode;



void messageCode(StatusCode code, char *msg);
#endif