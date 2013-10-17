/*
 * sqlite3_demo.cpp
 *
 *  Created on: 2013年10月17日
 *      Author: icejoywoo
 */

#include <iostream>
#include <cstdlib>
using namespace std;
#include "sqlite3.h"

static int _callback_exec(void * notused,int argc, char ** argv, char ** aszColName)
{
	int i;
	for ( i=0; i<argc; i++ )
	{
		printf( "%s = %s\n", aszColName[i], argv[i] == 0 ? "NUL" : argv[i] );
	}
	return 0;
}
int main(int argc,char * argv[])
{
	const char * file ="/tmp/test.db";
	const char * sSQL = "select * from stu;";
	char * pErrMsg = 0;
	int ret = 0;
	sqlite3 * db = NULL;
	ret = sqlite3_open(file, &db);
	if ( ret != SQLITE_OK )
	{
		fprintf(stderr, "Could not open database: %s", sqlite3_errmsg(db));
		exit(1);
	}
	printf("Successfully connected to database\n");
	sqlite3_exec( db, sSQL, _callback_exec, 0, &pErrMsg );
	if ( ret != SQLITE_OK )
	{
		fprintf(stderr, "SQL error: %s\n", pErrMsg);
		sqlite3_free(pErrMsg);
	}

	sqlite3_close(db);
	db = NULL;

	return 0;
}



