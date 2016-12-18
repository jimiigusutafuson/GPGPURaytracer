#pragma once

#define DEFAULTSCRNWIDTH (800)
#define DEFAULTSCRNHEIGHT (800)

#define	SAFE_RELEASE(x)	if( x ) { (x)->Release();	(x) = NULL; }
#define SAFE_DELETE(x)	if( x ) { delete(x);		(x) = NULL; }