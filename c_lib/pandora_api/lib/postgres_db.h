#ifndef POSTGRES_DATABASE_API_H
#define POSTGRES_DATABASE_API_H

#include <stdio.h>
#include <stdlib.h>

#include <libpq-fe.h>
// #include <postgresql/libpq-fe.h>

#define POSTGRES_BINARY_FORMAT (1) 
#define POSTGRES_STRING_FORMAT (0) 
#define DB_SET_SECURE_SEARCH "SELECT pg_catalog.set_config('search_path', '', false)"

/*
 * db_connect
 * Creates a connection object;
 * :: conninfo : char* :: String handling connection data
 */
PGconn* db_connect(const char* conninfo);

/*
 * db_conn_exit
 * Disconnects from the database and terminates
 * :: conn : PGconn* :: Connection object  
 */
void db_conn_exit(PGconn *conn);

/*
 * db_conn_teardown
 * Finishes a connection 
 * :: conn : PGconn* :: Connection object  
 */
void db_conn_teardown(PGconn *conn);

/*
 * db_connection_status
 * Tests the status of a connection
 * :: conn : PGconn* :: Active DB connection 
 */
void db_connection_status(PGconn* conn);

/*
 * db_set_secure_search
 * Best practice function
 * :: conn : PGconn* :: Connection object
 */
void db_set_secure_search(PGconn *conn);

/*
 * db_check_result_status
 * Checks that query results are correct
 * :: conn : PGconn* :: Connection object
 * :: result : PGresult* :: Query result
 * Terminates program if result object throws an error
 */
void db_check_result_status(PGconn* conn, PGresult* result);

#endif
