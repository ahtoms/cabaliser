#include "instructions.h"

/*
 * instruction_queue_create
 * Constructor for an instruction queue
 * :: n_qubits : const size_t :: Number of qubits supported by the queue
 * Queue is allocated on the heap and requires the us of the destructor function instruction_queue_destroy
 */
instruction_queue_t* instruction_queue_create(const size_t n_qubits)
{
    void* instructions = NULL;
    const size_t instruction_table_size = n_qubits * MAX_INSTRUCTION_SEQUENCE_LEN * CLIFFORD_OPCODE_WIDTH; 
    posix_memalign(&instructions, CACHE_SIZE, instruction_table_size); 

    memset(instructions, 0x00, instruction_table_size); 

    instruction_queue_t* que = NULL; 
    posix_memalign((void**)&que, CACHE_SIZE,  sizeof(instruction_queue_t));
    que->table = instructions;
    que->n_qubits = n_qubits;
    
    return que;
}


/*
 * instruction_queue_destroy
 * Destructor for an instruction queue 
 * :: que : instruction_queue_t* :: Instruction queue to be deallocated
 * Frees queue related memory from the heap
 */
void instruction_queue_destroy(instruction_queue_t* que)
{
    free(que->table);
    free(que);
}
