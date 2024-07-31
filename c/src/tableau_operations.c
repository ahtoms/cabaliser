#define INSTRUCTIONS_TABLE
#include "tableau_operations.h"


/*
 * tableau_remove_zero_X_columns
 * Uses hadamards to clear any zeroed X columns
 * :: tab : tableau_t* :: The tableau to act on
 * Acts in place over the tableau
 */
void tableau_remove_zero_X_columns(tableau_t* tab, clifford_queue_t* c_que)
{
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->n_qubits; i++)
    {
       if (CTZ_SENTINEL == tableau_ctz(tab->slices_x[i], tab->n_qubits)) 
       {
         __inline_tableau_hadamard(tab, i);
         __inline_clifford_queue_local_clifford_right(c_que, i, _H_);   
       } 
    } 
    return;
}


/*
 * tableau_Z_block_diagonal
 * Ensures that the Z block of the tableau is diagonal
 * :: tab : tableau_t* :: Tableau to act on
 *
 */
void tableau_Z_zero_diagonal(tableau_t* tab, clifford_queue_t* c_que)
{
    #pragma GCC unroll 8
    for (size_t i = 0; i < tab->n_qubits; i++)
    { 
        uint8_t z = __inline_slice_get_bit(tab->slices_z[i], i);  
        instruction_t operator = (z && (_I_)) | (!z && (_S_)); 
        __inline_clifford_queue_local_clifford_right(c_que, i, operator);   
    }
    return;
}


/*
 * tableau_X_upper_right_triangular
 * Makes the X block upper right triangular 
 * :: tab : tableau_t* :: Tableau to operate on    
 */
void tableau_X_diagonal(tableau_t* tab, clifford_queue_t* c_que)
{
    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        // X(i, i) != 1 
        if (1 != __inline_slice_get_bit(tab->slices_x[i], i))
        {  
             
        }

        // TODO VECTORISE
        for (size_t j = i + 1; j < tab->n_qubits; j++) 
        {
            if (__inline_slice_get_bit(tab->slices_x[j], i))
            {
                tableau_rowsum(tab, i, j);
            }
        }

        for (size_t j = i - 1; j > 0; j--) 
        {
            if (__inline_slice_get_bit(tab->slices_x[j], i))
            {
                tableau_rowsum(tab, i, j);
            }
        }

    }

    return;
}


void tableau_H(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= slice_x[i] & slice_z[i];      
    }  

    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}


void tableau_S(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= slice_x[i] & slice_z[i];      
        slice_z[i] ^= slice_x[i]; 
    }  
}

void tableau_Z(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
/*
 * Doubled S gate
 * S: (r ^= x.z; z ^= x)
 * r1 = r0 ^ (x.z0); z1 = z0 ^ x 
 * r2 = r0 ^ (x.z0) ^ (x.(z0 ^ x)); z2 = z0 ^ x ^ x 
 * r2 = r0 ^ x.(z0 ^ z0 ^ x); z2 = z0 
 * r2 = r0 ^ x; z2 = z0 
 */

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= slice_x[i];      
    }  
}


void tableau_R(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * Triple S gate
     * S : (r ^= x.z; z ^= x)
     * r1 = r0 ^ (x.z0); z1 = z0 ^ x 
     *
     * r2 = r0 ^ (x.z0) ^ (x.(z0 ^ x)); z2 = z0 ^ x ^ x 
     * r2 = r0 ^ x.(z0 ^ z0 ^ x); z2 = z0 
     * r2 = r0 ^ x; z2 = z0 
     *
     * r3 = r0 ^ x ^ x.z2; z3 = z2 ^ x 
     * r3 = r0 ^ x ^ x.z0; z3 = z0 ^ x 
     *
     * R : (r ^= x.~z; z ^= x)
     */

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= slice_x[i] & ~slice_z[i];
        slice_z[i] ^= slice_x[i]; 
    }  
}

void tableau_X(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
/*
 * HZH Gate
 * Flip X and Z, perform a Z, then flip X and Z again
 * H : (r ^= x.z; x <-> z) 
 * Z : (r ^= x)
 * 
 * H
 * r_1 = r_0 ^ x0.z0; x1 = z0; z1 = x0  
 * 
 * Z
 * r_2 = r_1 ^ x1; x2 = x1; z2 = z1  
 * r_2 = r_0 ^ x0.z0 ^ z0; x2 = z0; z2 = x0  
 *
 * H
 * r_3 = r_0 ^ x0.z0 ^ z0 ^ z2.x2; x3 = z2; z3 = x2  
 * r_3 = r_0 ^ x0.z0 ^ z0 ^ z0.x0; x3 = x0; z3 = z0  
 * r_3 = r_0 ^ z0;
 *
 */
    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= slice_z[i];      
    }
}

void tableau_Y(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * Y = XZ
     * Z : (r ^= x)
     * X : (r ^= z)
     *
     * r_1 = r_0 ^ x 
     *
     * r_2 = r_0 ^ x ^ z 
     *
     * Y : r ^= x ^ z
     */
    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= slice_z[i] ^ slice_x[i];      
    }  
}

void tableau_HX(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * X : (r ^= z)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_0 ^ z ^ x.z 
     * r_2 = r_0 ^ (~x & z) 
     * Swap x and z
     */
    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= ~slice_x[i] & slice_z[i];      
    }  
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}

void tableau_SX(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
    /*
     * X : (r ^= z)
     * S : (r ^= x.z; z ^= x)
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_0 ^ z ^ x.z 
     * r_2 = r_0 ^ (~x & z) 
     * z_2 = z_0 ^ x
     */

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= ~slice_x[i] & slice_z[i];      
        slice_z[i] ^= slice_x[i]; 
    }  
}



void tableau_RX(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
    /*
     * X : (r ^= z)
     * R : (r ^= x.~z; z ^= x)
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_0 ^ z ^ x.~z 
     * r_2 = r_0 ^ (z | x) 
     * z_2 = z_0 ^ x
     */

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= slice_x[i] | slice_z[i];      
        slice_z[i] ^= slice_x[i]; 
    }  
}

void tableau_HZ(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
    /*
     * Z : (r ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ x 
     *
     * r_2 = r_0 ^ x ^ x.z 
     * r_2 = r_0 ^ (x & ~z) 
     * z_2 = x_0
     * x_2 = z_0
     */

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= ~slice_z[i] & slice_x[i];      
    }  
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}



void tableau_HY(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
    /*
     * Y : r ^= x ^ z
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ x ^ z 
     *
     * r_2 = r_0 ^ x ^ z ^ x.z 
     * r_2 = r_0 ^ (x | z) 
     * z_2 = x_0
     * x_2 = z_0
     */

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= slice_z[i] ^ slice_x[i];      
    }  
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}


void tableau_SH(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    /*
     * H : (r ^= x.z; x <-> z) 
     * S : (r ^= x.z; z ^= x)
     *
     * r_1 = r_0 ^ x.z 
     * z_1 = x_0
     * x_1 = z_0
     *
     * r_2 = r_1 ^ x_1.z_1
     * r_2 = r_0 ^ x_0.z_0 ^ z_0.x_0
     * r_2 = r_0
     * z_2 ^= x_2 -> x_0 ^= z_0
     * 
     */

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_x[i] ^= slice_z[i];
    }  
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}


void tableau_RH(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * H : (r ^= x.z; x <-> z) 
     * R : (r ^= x.~z; z ^= x)
     *
     * r_1 = r_0 ^ x.z 
     * z_1 = x_0
     * x_1 = z_0
     *
     * r_2 = r_1 ^ x_1.~z_1
     * r_2 = r_0 ^ x_0.z_0 ^ z_0.~x_0
     * r_2 = r_0 ^ z_0
     * z_2 ^= x_2 -> x_0 ^= z_0
     *
     */

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= slice_z[i];
        slice_x[i] ^= slice_z[i];
    }  
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}


void tableau_HS(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    /*
     * S : (r ^= x.z; z ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ x.z 
     * z_1 = z_0 ^ x
     *
     * r_2 = r_1 ^ x.z_1
     * r_2 = r_0 ^ x.z_0 ^ x.(z_0 ^ x)
     * z_2 = x_1
     * x_2 = z_1
     *
     */

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_x[i] ^= slice_z[i];
    }  
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}

void tableau_HR(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    /*
     * R : (r ^= x.~z; z ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ x.~z 
     * z_1 = z_0 ^ x
     *
     * r_2 = r_1 ^ x.~z_1
     * r_2 = r_0 
     *
     * z_2 = x_1
     * x_2 = z_1 = z_0 ^ x_0
     *
     */

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_z[i] ^= slice_x[i];
    }  
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}

void tableau_HSX(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * X : (r ^= z)
     * S : (r ^= x.z; z ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_1 ^ x.z_0
     * r_2 = r_0 ^ z_0 ^ x.z_0 
     * r_2 = r_0 ^ ~x.z_0
     * z_2 = z_0 ^ x 
     *
     * r_3 = r_2 ^ x_0.z_2 
     * r_3 = r_0 ^ ~x_0.z_0 ^ x_0.(z_0 ^ x_0)
     * r_3 = r_0 ^ x_0 ^ z_0
     * x_3 = z_2 = x_0 ^ z_0 
     * z_3 = x_2 = x_0
     */

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= slice_x[i] ^ slice_z[i];
        slice_z[i] ^= slice_x[i];
    }  
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}

void tableau_HRX(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * X : (r ^= z)
     * R : (r ^= x.~z; z ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_1 ^ x.~z_0
     * r_2 = r_0 ^ z_0 ^ x.~z_0 
     * r_2 = r_0 ^ (x | z_0)
     * z_2 = z_0 ^ x 
     *
     * r_3 = r_2 ^ x_0.z_2 
     * r_3 = r_0 ^ ~x_0.z_0 ^ x_0.(z_0 ^ x_0)
     * r_3 = r_0 ^ z
     * x_3 = z_2 = x_0 ^ z_0 
     * z_3 = x_2 = x_0
     */

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= slice_z[i];
        slice_z[i] ^= slice_x[i];
    }  
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}

void tableau_SHY(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= slice_z[i] ^ slice_x[i];
        slice_x[i] ^= slice_z[i];
    }  
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}

void tableau_RHY(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= slice_x[i];
        slice_x[i] ^= slice_z[i];
    }  
    void* ptr = tab->slices_x[targ];
    tab->slices_x[targ] = tab->slices_z[targ];
    tab->slices_z[targ] = ptr;
}

void tableau_HSH(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= ~slice_x[i] & slice_z[i];
        slice_x[i] ^= slice_z[i];
    }  
}

void tableau_HRH(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= slice_x[i] & slice_z[i];
        slice_x[i] ^= slice_z[i];
    }  
}


void tableau_RHS(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= slice_x[i] | slice_z[i];
        slice_x[i] ^= slice_z[i];
    }  
}


void tableau_SHR(tableau_t* tab, const size_t targ)
{
    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    // TODO Dispatch
    #pragma GCC unroll 8 
    for (size_t i = 0; i < tab->slice_len; i++)
    {
        slice_r[i] ^= slice_x[i] & ~slice_z[i];
        slice_x[i] ^= slice_z[i];
    }  
}


