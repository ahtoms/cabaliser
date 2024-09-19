#ifndef PANDORA_CONNECT_H
#define PANDORA_CONNECT_H

#include <stdint.h>
#include <string.h>

#include "postgres_db.h"
#include "postgres_result_casts.h"

#define PANDORA_DB_STR "dbname = %s"
#define PANDORA_COUNT_N_QUBITS  "SELECT COUNT(*) FROM linked_circuit_qubit WHERE type = 'In'"

#define PANDORA_DECORATION "CALL decorate_circuit()" 

#define PANDORA_INITIAL  "SELECT type, qub_1 FROM linked_circuit_qubit WHERE type = 'In'"
#define PANDORA_GET_LAYER  "SELECT type, param, qub_1, qub_2, qub_3 FROM linked_circuit_qubit WHERE id = (SELECT decorated_circuit.id FROM decorated_circuit WHERE decorated_circuit.id = linked_circuit_qubit.id AND layer = $1)"


struct pandora_t {
    size_t db_name_len;
    char* db_name;
    size_t tag_name_len;
    char* tag_name;
    PGconn* conn; 
};
typedef struct pandora_t pandora_t; 


/*
 * pandora_create
 * constructor for Pandora connections
 * :: db_name : char* :: Name of the target database
 */
pandora_t* pandora_create(char* db_name);
void pandora_destroy(pandora_t* pan);

/*
 * pandora_connect
 * Creates a connection object to the pandora database
 * Thin wrapper around db_connect
 */
void pandora_connect(pandora_t* pan);
void pandora_disconnect(pandora_t* pan);

size_t pandora_get_n_qubits(pandora_t* pan);


#endif

