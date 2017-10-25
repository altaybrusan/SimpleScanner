/*!\brief	ArgusAPI.h
 * NAME: ArgusAPI.h
 * DESCRIPTION: TeledyneDalsa Argus Camera API header file
 * DESIGN NOTE: 
 *
 * Copyright(r) TeledyneDalsa 2011. All rights reserved.
 */
#if !defined ARGUS_API_H
#define ARGUS_API_H

#ifdef __cplusplus
	#define DLLDIR_C_API(aType) extern "C" aType __cdecl
#else
	#define DLLDIR_C_API(aType) aType __cdecl
#endif


/* Dll interface function prototype	*/
DLLDIR_C_API(int) _camera_connect( char *pTargetAddr, int TargetPort, int *WSAStatus,
									char *pResp, char *pLogin, char *pPassword, int timeout );
DLLDIR_C_API(int) _camera_cmd( char *pCmd, char *pResp, int timeout );
DLLDIR_C_API(int) _camera_disconnect( int *WSAStatus, char *pResp, int timeout );

DLLDIR_C_API (int) _camera_get_image( int argc, char *const *argv, char *pResp );

#endif
