#include <tfhe/tfhe.h>
#include <tfhe/tfhe_io.h>
#include "filenames_p.h"
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
#include <cstdio>
#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>
#include <omp.h>
#include <vector>
#include <istream>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <map>
#include <sstream>
using namespace std;

#define MAX 1000
#define PORT 8080
#define SA struct sockaddr

int tmp_socket;
bool list_of_results = false;

vector<string> split(string s, string delimiter)
{
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	string token;
	vector<string> res;

	while ((pos_end = s.find(delimiter, pos_start)) != string::npos)
	{
		token = s.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		res.push_back(token);
	}

	res.push_back(s.substr(pos_start));
	return res;
}

// Logic Functions
// 0
void e_and(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	int i;
#pragma omp parallel for private(i) shared(result)
	for (i = 0; i < nb_bits; i++)
	{
		bootsAND(&result[i], &a[i], &b[i], bk);
	}
}

// 1
void e_nand(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	int i;
#pragma omp parallel for private(i) shared(result)
	for (i = 0; i < nb_bits; i++)
	{
		bootsNAND(&result[i], &a[i], &b[i], bk);
	}
}

// 2
void e_or(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	int i;
#pragma omp parallel for private(i) shared(result)
	for (i = 0; i < nb_bits; i++)
	{
		bootsOR(&result[i], &a[i], &b[i], bk);
	}
}

// 3
void e_nor(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	int i;
#pragma omp parallel for private(i) shared(result)
	for (i = 0; i < nb_bits; i++)
	{
		bootsNOR(&result[i], &a[i], &b[i], bk);
	}
}

// 4
void e_xor(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	int i;
#pragma omp parallel for private(i) shared(result)
	for (i = 0; i < nb_bits; i++)
	{
		bootsXOR(&result[i], &a[i], &b[i], bk);
	}
}

// 5
void e_xnor(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	int i;
#pragma omp parallel for private(i) shared(result)
	for (i = 0; i < nb_bits; i++)
	{
		bootsXNOR(&result[i], &a[i], &b[i], bk);
	}
}

// 9
void e_not(LweSample *result, const LweSample *a, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	int i;
#pragma omp parallel for private(i) shared(result)
	for (i = 0; i < nb_bits; i++)
	{
		bootsNOT(&result[i], &a[i], bk);
	}
}

// 10
void e_shl(LweSample *result, const LweSample *a, const int shift_amt, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	LweSample *temp = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	for (int i = 0; i < nb_bits; i++)
	{
		bootsCOPY(&result[i], &a[i], bk);
	}

	for (int i = 0; i < shift_amt; i++)
	{
		for (int j = 1; j < nb_bits; j++)
		{
			bootsCOPY(&temp[j], &result[j - 1], bk);
		}

		for (int k = 1; k < nb_bits; k++)
		{
			bootsCOPY(&result[k], &temp[k], bk);
		}

		bootsCONSTANT(&result[0], 0, bk); // set lowest bit to 0
	}

	delete_gate_bootstrapping_ciphertext_array(nb_bits, temp);
}

// 11
void e_shr(LweSample *result, const LweSample *a, const int shift_amt, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{

	LweSample *temp = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	for (int i = 0; i < nb_bits; i++)
	{
		bootsCOPY(&result[i], &a[i], bk);
	}

	for (int i = 0; i < shift_amt; i++)
	{
		for (int j = 0; j < nb_bits - 1; j++)
		{
			bootsCOPY(&temp[j], &result[j + 1], bk);
		}

		for (int k = 0; k < nb_bits; k++)
		{
			bootsCOPY(&result[k], &temp[k], bk);
		}

		bootsCONSTANT(&result[nb_bits - 1], 0, bk); // set highest bit to 0
	}

	delete_gate_bootstrapping_ciphertext_array(nb_bits, temp);
}

// 12
void e_mux(LweSample *result, const LweSample *sel, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	int i;
#pragma omp parallel for private(i) shared(result)
	for (i = 0; i < nb_bits; i++)
	{
		bootsMUX(&result[i], &sel[0], &a[i], &b[i], bk);
	}
}

// Arithmetic Functions

// 13
void comparator(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const int select, const TFheGateBootstrappingCloudKeySet *bk)
{

	LweSample *not_a = new_gate_bootstrapping_ciphertext_array(1, bk->params);
	LweSample *not_b = new_gate_bootstrapping_ciphertext_array(1, bk->params);
	LweSample *temp = new_gate_bootstrapping_ciphertext_array(10, bk->params);
	LweSample *greater_than = new_gate_bootstrapping_ciphertext_array(1, bk->params);
	LweSample *equal = new_gate_bootstrapping_ciphertext_array(1, bk->params);
	LweSample *less_than = new_gate_bootstrapping_ciphertext_array(1, bk->params);

	// initialize cascading inputs
	bootsCONSTANT(&greater_than[0], 0, bk);
	bootsCONSTANT(&less_than[0], 0, bk);
	bootsCONSTANT(&equal[0], 1, bk);

	// run 1 bit comparators
	for (int i = (nb_bits - 1); i > -1; i--)
	{
		// for (int i = 0; i < nb_bits; i++) {
		//  invert inputs
		bootsNOT(&not_a[0], &a[i], bk);
		bootsNOT(&not_b[0], &b[i], bk);

		// compute greater than path
		bootsNOT(&temp[0], &greater_than[0], bk);
		bootsNAND(&temp[1], &a[i], &not_b[0], bk);
		bootsNAND(&temp[2], &temp[1], &equal[0], bk);
		bootsNOT(&temp[3], &temp[2], bk);
		bootsNAND(&greater_than[0], &temp[0], &temp[3], bk);
		bootsNOT(&temp[8], &less_than[0], bk);
		bootsAND(&greater_than[0], &temp[8], &greater_than[0], bk);

		// compute less than path
		bootsNOT(&temp[4], &less_than[0], bk);
		bootsNAND(&temp[5], &not_a[0], &b[i], bk);
		bootsNAND(&temp[6], &temp[5], &equal[0], bk);
		bootsNOT(&temp[7], &temp[6], bk);
		bootsNAND(&less_than[0], &temp[7], &temp[4], bk);
		bootsNOT(&temp[9], &greater_than[0], bk);
		bootsAND(&less_than[0], &temp[9], &less_than[0], bk);

		// compute equality path
		bootsNOR(&equal[0], &greater_than[0], &less_than[0], bk);
	}

	// select desired output
	if (select == 0)
	{ // ecmpeq
		bootsCOPY(&result[0], &equal[0], bk);
	}
	else if (select == 1)
	{ // ecmpl
		bootsCOPY(&result[0], &less_than[0], bk);
	}
	else if (select == 2)
	{ // ecmpg
		bootsCOPY(&result[0], &greater_than[0], bk);
	}
	else if (select == 3)
	{ // ecmpgeq
		bootsOR(&result[0], &equal[0], &greater_than[0], bk);
	}
	else if (select == 4)
	{ // ecmpleq
		bootsOR(&result[0], &equal[0], &less_than[0], bk);
	}
	else if (select == 5)
	{ // ecmpneq
		bootsCOPY(&result[0], &equal[0], bk);
		bootsNOT(&result[0], &result[0], bk);
	}

	// align to word size
	for (int i = 1; i < nb_bits; i++)
	{
		bootsCONSTANT(&result[i], 0, bk);
	}

	delete_gate_bootstrapping_ciphertext_array(1, not_a);
	delete_gate_bootstrapping_ciphertext_array(1, not_b);
	delete_gate_bootstrapping_ciphertext_array(8, temp);
	delete_gate_bootstrapping_ciphertext_array(1, greater_than);
	delete_gate_bootstrapping_ciphertext_array(1, equal);
	delete_gate_bootstrapping_ciphertext_array(1, less_than);
	}
	// 6
	/*
	void subtracter(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
	{

		LweSample *borrow = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
		LweSample *temp = new_gate_bootstrapping_ciphertext_array(3, bk->params);

		// run half subtractor
		bootsXOR(&result[0], &a[0], &b[0], bk);
		bootsNOT(&temp[0], &a[0], bk);
		bootsAND(&borrow[0], &temp[0], &b[0], bk);

		// run full subtractors
		for (int i = 1; i < nb_bits; i++)
		{

			// Calculate difference
			bootsXOR(&temp[0], &a[i], &b[i], bk);
			bootsXOR(&result[i], &temp[0], &borrow[i - 1], bk);

			// Calculate borrow
			bootsNOT(&temp[1], &a[i], bk);
			bootsAND(&temp[2], &temp[1], &b[i], bk);
			bootsNOT(&temp[0], &temp[0], bk);
			bootsAND(&temp[1], &borrow[i - 1], &temp[0], bk);
			bootsOR(&borrow[i], &temp[2], &temp[1], bk);
		}

		delete_gate_bootstrapping_ciphertext_array(nb_bits, borrow);
		delete_gate_bootstrapping_ciphertext_array(3, temp);
	}
*/
	void subtracter(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
	{

		LweSample *borrow = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
		LweSample *temp_0 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
		LweSample *temp_1 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
		LweSample *temp_2 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
		// run half subtractor
		bootsXOR(&result[0], &a[0], &b[0], bk);
		bootsNOT(&temp_0[0], &a[0], bk);
		bootsAND(&borrow[0], &temp_0[0], &b[0], bk);
		
#pragma omp parallel for
		for(int i = 1; i<nb_bits; i++){
			bootsXOR(&temp_0[i], &a[i], &b[i], bk);
			bootsNOT(&temp_1[i], &a[i], bk);

		}

		// run full subtractors
		for (int i = 1; i < nb_bits; i++)
		{

			// Calculate difference
			bootsXOR(&result[i], &temp_0[i], &borrow[i - 1], bk);
			
			if (i != nb_bits-1){
			// Calculate borrow
			bootsAND(&temp_2[i], &temp_1[i], &b[i], bk);
			bootsNOT(&temp_0[i], &temp_0[i], bk);
			bootsAND(&temp_1[i], &borrow[i - 1], &temp_0[i], bk);
			bootsOR(&borrow[i], &temp_2[i], &temp_1[i], bk);
			}
		}

		delete_gate_bootstrapping_ciphertext_array(nb_bits, borrow);
		delete_gate_bootstrapping_ciphertext_array(nb_bits, temp_0);
		delete_gate_bootstrapping_ciphertext_array(nb_bits, temp_1);
		delete_gate_bootstrapping_ciphertext_array(nb_bits, temp_2);

	}

	void adder_naive(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
	{

		LweSample *carry = new_gate_bootstrapping_ciphertext_array(nb_bits + 1, bk->params);
		LweSample *temp = new_gate_bootstrapping_ciphertext_array(2, bk->params);

		// initialize first carry to 0
		bootsCONSTANT(&carry[0], 0, bk);

		// run full adders

		for (int i = 0; i < nb_bits; i++)
		{
			// Compute sum
			bootsXOR(&temp[0], &a[i], &b[i], bk);
			bootsXOR(&result[i], &carry[i], &temp[0], bk);
			// Compute carry
			bootsAND(&temp[1], &a[i], &b[i], bk);
			bootsAND(&temp[0], &carry[i], &temp[0], bk);
			bootsOR(&carry[i + 1], &temp[0], &temp[1], bk);
		}
		delete_gate_bootstrapping_ciphertext_array(nb_bits + 1, carry);
		delete_gate_bootstrapping_ciphertext_array(2, temp);
	}

	// 7 - Modified by O
	void adder(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
	{

		LweSample *carry = new_gate_bootstrapping_ciphertext_array(nb_bits + 1, bk->params);
		LweSample *temp = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

		// initialize first carry to 0
		bootsCONSTANT(&carry[0], 0, bk);
		int i;
#pragma omp parallel for private(i) shared(temp)
		for (i = 0; i < nb_bits; i++)
		{
			bootsXOR(&temp[i], &a[i], &b[i], bk);
		}

		// run full adders
		for (int i = 0; i < nb_bits; i++)
		{
			// Compute sum
			bootsXOR(&result[i], &carry[i], &temp[i], bk);

			// Compute carry
			bootsMUX(&carry[i + 1], &temp[i], &carry[i], &a[i], bk);
		}

		delete_gate_bootstrapping_ciphertext_array(nb_bits + 1, carry);
		delete_gate_bootstrapping_ciphertext_array(1, temp);
	}

	// 8
	void add_supplement(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
	{

		if (nb_bits == 0)
		{
			return;
		}
		LweSample *carry = new_gate_bootstrapping_ciphertext_array(nb_bits + 1, bk->params);
		LweSample *temp = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
		bootsCONSTANT(&carry[0], 0, bk);


#pragma omp parallel for
		for (int i=0; i < nb_bits; i++){
			bootsXOR(&temp[i], &a[i], &b[i], bk);

		}
		for (int i = 0; i < nb_bits-1; i++)
		{
			// Compute carry
			bootsMUX(&carry[i + 1], &temp[i], &carry[i], &a[i], bk);

		}


		int i;
#pragma omp parallel for
		for (i = 0; i < nb_bits; i++)
		{
			// Compute sum
			bootsXOR(&result[i], &carry[i], &temp[i], bk);
		}


		delete_gate_bootstrapping_ciphertext_array(nb_bits + 1, carry);
		delete_gate_bootstrapping_ciphertext_array(1, temp);
	}

	void multiplier(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
	{

		LweSample *tmp_array = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
		LweSample *sum = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

		for (int i = 0; i < nb_bits; ++i)
		{
			// initialize temp values to 0
			bootsCONSTANT(&sum[i], 0, bk);
			bootsCONSTANT(&result[i], 0, bk);
			bootsCONSTANT(&tmp_array[i], 0, bk);

		}

		int i;

		for (i = 0; i < nb_bits; ++i)
		{
			int j;
#pragma omp parallel for private(j) shared(tmp_array)   
			for (j = 0; j < nb_bits - i; ++j)
			{
				bootsAND(&tmp_array[j], &a[i], &b[j], bk);
			}
			add_supplement(sum + i, tmp_array, sum + i, nb_bits - i, bk);
		}

		for (int j = 0; j < nb_bits; j++)
		{
			bootsCOPY(&result[j], &sum[j], bk);
		}

		delete_gate_bootstrapping_ciphertext_array(nb_bits, tmp_array);
		delete_gate_bootstrapping_ciphertext_array(nb_bits, sum);
	}

	/*
	   void multiplier(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
	   {

	   LweSample *tmp_array = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	   LweSample *sum = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	   for (int i = 0; i < nb_bits; ++i)
	   {
	// initialize temp values to 0
	bootsCONSTANT(&sum[i], 0, bk);
	bootsCONSTANT(&result[i], 0, bk);
	}

	for (int i = 0; i < nb_bits; ++i)
	{
	for (int k = 0; k < nb_bits; ++k)
	{
	bootsCONSTANT(&tmp_array[k], 0, bk);
	}
	int j;
#pragma omp parallel for private(j) shared(tmp_array)
for (int j = 0; j < nb_bits - i; ++j)
{
bootsAND(&tmp_array[j], &a[i], &b[j], bk);
}
add_supplement(sum + i, tmp_array, sum + i, nb_bits - i, bk);
}

for (int j = 0; j < nb_bits; j++)
{
bootsCOPY(&result[j], &sum[j], bk);
}

delete_gate_bootstrapping_ciphertext_array(nb_bits, tmp_array);
delete_gate_bootstrapping_ciphertext_array(nb_bits, sum);
}
*/
/*NEW INSTRUCTIONS FOR PULPFHE STARTS HERE*/

// 100
void p_adder_32(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	LweSample *p0 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *p1 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *p2 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *p3 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *p4 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *p5 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	LweSample *g0 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *g1 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *g2 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *g3 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *g4 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *g5 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	LweSample *temp = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *carry_out = new_gate_bootstrapping_ciphertext_array(1, bk->params);

	// Stage 1
	// level 1: 32 parallel gates
	int i;
#pragma omp parallel for private(i) shared(p0, g0)
	for (i = 0; i < nb_bits; i++)
	{
		bootsXOR(&p0[i], &a[i], &b[i], bk);
		bootsAND(&g0[i], &a[i], &b[i], bk);
	}

	// Stage 2
	// level 2: 30 parallel gates
	// level 3: 15 parallel gates (OR gate depends on AND)
#pragma omp parallel for private(i) shared(p1, g1)
	for (i = 0; i < nb_bits; i++)
	{
		if (i == 0)
		{
			bootsCOPY(&g1[i], &g0[i], bk);
		}
		else
		{
			bootsAND(&temp[i], &p0[i], &g0[i - 1], bk);
			bootsOR(&g1[i], &temp[i], &g0[i], bk);
			bootsAND(&p1[i], &p0[i], &p0[i - 1], bk);
		}
	}

	// Stage 3
	// level 4: 27 parallel gates
	// level 5: 14 parallel gates
#pragma omp parallel for private(i) shared(g2, p2)
	for (i = 1; i < nb_bits; i++)
	{
		if (i == 1)
		{
			bootsCOPY(&g2[i], &g1[i], bk);
		}
		else if (i == 2)
		{
			bootsAND(&temp[i], &g1[i - 2], &p1[i], bk);
			bootsOR(&g2[i], &temp[i], &g1[i], bk);
		}
		else
		{
			bootsAND(&temp[i], &p1[i], &g1[i - 2], bk);
			bootsOR(&g2[i], &temp[i], &g1[i], bk);
			bootsAND(&p2[i], &p1[i], &p1[i - 2], bk);
		}
	}

	// Stage 4
	// level 6: 21 parallel gates
	// level 7: 12 parallel gates
#pragma omp parallel for private(i) shared(g3, p3)
	for (i = 3; i < nb_bits; i++)
	{
		if (i == 3)
		{
			bootsCOPY(&g3[i], &g2[i], bk);
		}
		else if (i == 4)
		{
			bootsAND(&temp[i], &g1[i - 4], &p2[i], bk);
			bootsOR(&g3[i], &temp[i], &g2[i], bk);
		}
		else if (i < 7)
		{
			bootsAND(&temp[i], &g2[i - 4], &p2[i], bk);
			bootsOR(&g3[i], &temp[i], &g2[i], bk);
		}
		else
		{
			bootsAND(&temp[i], &p2[i], &g2[i - 4], bk);
			bootsOR(&g3[i], &temp[i], &g2[i], bk);
			bootsAND(&p3[i], &p2[i], &p2[i - 4], bk);
		}
	}

	// Stage 5
	// level 8: 9 parallel gates
	// level 9: 8 parallel gates
#pragma omp parallel for private(i) shared(g4, p4)
	for (i = 7; i < nb_bits; i++)
	{
		if (i == 7)
		{
			bootsCOPY(&g4[i], &g3[i], bk);
		}
		else if (i == 8)
		{
			bootsAND(&temp[i], &g1[i - 8], &p3[i], bk);
			bootsOR(&g4[i], &temp[i], &g3[i], bk);
		}
		else if (i < 11)
		{
			bootsAND(&temp[i], &g2[i - 8], &p3[i], bk);
			bootsOR(&g4[i], &temp[i], &g3[i], bk);
		}
		else if (i < 15)
		{
			bootsAND(&temp[i], &g3[i - 8], &p3[i], bk);
			bootsOR(&g4[i], &temp[i], &g3[i], bk);
		}
		else
		{
			bootsAND(&temp[i], &p3[i], &g3[i - 8], bk);
			bootsOR(&g4[i], &temp[i], &g3[i], bk);
			bootsAND(&p4[i], &p3[i], &p3[i - 8], bk);
		}
	}

#pragma omp parallel for private(i) shared(g5, p5)
	for (int i = 15; i < nb_bits; i++)
	{
		if (i == 15)
		{
			bootsCOPY(&g5[i], &g4[i], bk);
		}
		if (i == 16)
		{
			bootsAND(&temp[i], &g1[i - 8], &p4[i], bk);
			bootsOR(&g5[i], &temp[i], &g4[i], bk);
		}
		if (i < 19)
		{
			bootsAND(&temp[i], &g2[i - 8], &p4[i], bk);
			bootsOR(&g5[i], &temp[i], &g4[i], bk);
		}
		if (i < 22)
		{
			bootsAND(&temp[i], &g3[i - 8], &p4[i], bk);
			bootsOR(&g5[i], &temp[i], &g4[i], bk);
		}
		if (i < 26)
		{
			bootsAND(&temp[i], &g4[i - 8], &p4[i], bk);
			bootsOR(&g5[i], &temp[i], &g4[i], bk);
		}
		else
		{
			// Adjust the index offsets based on the stage number
			bootsAND(&temp[i], &p4[i], &g4[i - 16], bk);
			bootsOR(&g5[i], &temp[i], &g4[i], bk);
			bootsAND(&p5[i], &p4[i], &p4[i - 16], bk);
		}
	}

	// Generate Sum and Carry Out
	// level 10: 15 parallel gates
#pragma omp parallel for private(i) shared(result)
	for (i = 0; i < nb_bits; i++)
	{
		if (i == 0)
		{
			bootsCOPY(&result[i], &p0[i], bk);
		}
		else if (i == 1)
		{
			bootsXOR(&result[i], &g1[i - 1], &p0[i], bk);
		}
		else if (i < 4)
		{
			bootsXOR(&result[i], &g2[i - 1], &p0[i], bk);
		}
		else if (i < 8)
		{
			bootsXOR(&result[i], &g3[i - 1], &p0[i], bk);
		}
		else if (i < 16)
		{
			bootsXOR(&result[i], &g4[i - 1], &p0[i], bk);
		}
		else
		{
			bootsXOR(&result[i], &g5[i - 1], &p0[i], bk);
		}
	}
	bootsCOPY(&carry_out[0], &g5[nb_bits - 1], bk);
}

// 101
void p_adder_16(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	LweSample *p0 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *p1 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *p2 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *p3 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *p4 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	LweSample *g0 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *g1 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *g2 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *g3 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *g4 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	LweSample *temp = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *carry_out = new_gate_bootstrapping_ciphertext_array(1, bk->params);

	// Stage 1
	// level 1: 32 parallel gates
	int i;
#pragma omp parallel for private(i) shared(p0, g0)
	for (i = 0; i < 16; i++)
	{
		bootsXOR(&p0[i], &a[i], &b[i], bk);
		bootsAND(&g0[i], &a[i], &b[i], bk);
	}

	// Stage 2
	// level 2: 30 parallel gates
	// level 3: 15 parallel gates (OR gate depends on AND)
#pragma omp parallel for private(i) shared(p1, g1)
	for (i = 0; i < nb_bits; i++)
	{
		if (i == 0)
		{
			bootsCOPY(&g1[i], &g0[i], bk);
		}
		else
		{
			bootsAND(&temp[i], &p0[i], &g0[i - 1], bk);
			bootsOR(&g1[i], &temp[i], &g0[i], bk);
			bootsAND(&p1[i], &p0[i], &p0[i - 1], bk);
		}
	}

	// Stage 3
	// level 4: 27 parallel gates
	// level 5: 14 parallel gates
#pragma omp parallel for private(i) shared(g2, p2)
	for (i = 1; i < nb_bits; i++)
	{
		if (i == 1)
		{
			bootsCOPY(&g2[i], &g1[i], bk);
		}
		else if (i == 2)
		{
			bootsAND(&temp[i], &g1[i - 2], &p1[i], bk);
			bootsOR(&g2[i], &temp[i], &g1[i], bk);
		}
		else
		{
			bootsAND(&temp[i], &p1[i], &g1[i - 2], bk);
			bootsOR(&g2[i], &temp[i], &g1[i], bk);
			bootsAND(&p2[i], &p1[i], &p1[i - 2], bk);
		}
	}

	// Stage 4
	// level 6: 21 parallel gates
	// level 7: 12 parallel gates
#pragma omp parallel for private(i) shared(g3, p3)
	for (i = 3; i < nb_bits; i++)
	{
		if (i == 3)
		{
			bootsCOPY(&g3[i], &g2[i], bk);
		}
		else if (i == 4)
		{
			bootsAND(&temp[i], &g1[i - 4], &p2[i], bk);
			bootsOR(&g3[i], &temp[i], &g2[i], bk);
		}
		else if (i < 7)
		{
			bootsAND(&temp[i], &g2[i - 4], &p2[i], bk);
			bootsOR(&g3[i], &temp[i], &g2[i], bk);
		}
		else
		{
			bootsAND(&temp[i], &p2[i], &g2[i - 4], bk);
			bootsOR(&g3[i], &temp[i], &g2[i], bk);
			bootsAND(&p3[i], &p2[i], &p2[i - 4], bk);
		}
	}

	// Stage 5
	// level 8: 9 parallel gates
	// level 9: 8 parallel gates
#pragma omp parallel for private(i) shared(g4, p4)
	for (i = 7; i < nb_bits; i++)
	{
		if (i == 7)
		{
			bootsCOPY(&g4[i], &g3[i], bk);
		}
		else if (i == 8)
		{
			bootsAND(&temp[i], &g1[i - 8], &p3[i], bk);
			bootsOR(&g4[i], &temp[i], &g3[i], bk);
		}
		else if (i < 11)
		{
			bootsAND(&temp[i], &g2[i - 8], &p3[i], bk);
			bootsOR(&g4[i], &temp[i], &g3[i], bk);
		}
		else if (i < 15)
		{
			bootsAND(&temp[i], &g3[i - 8], &p3[i], bk);
			bootsOR(&g4[i], &temp[i], &g3[i], bk);
		}
		else
		{
			bootsAND(&temp[i], &p3[i], &g3[i - 8], bk);
			bootsOR(&g4[i], &temp[i], &g3[i], bk);
			bootsAND(&p4[i], &p3[i], &p3[i - 8], bk);
		}
	}

	// Generate Sum and Carry Out
	// level 10: 15 parallel gates
#pragma omp parallel for private(i) shared(result)
	for (i = 0; i < nb_bits; i++)
	{
		if (i == 0)
		{
			bootsCOPY(&result[i], &p0[i], bk);
		}
		else if (i == 1)
		{
			bootsXOR(&result[i], &g1[i - 1], &p0[i], bk);
		}
		else if (i < 4)
		{
			bootsXOR(&result[i], &g2[i - 1], &p0[i], bk);
		}
		else if (i < 8)
		{
			bootsXOR(&result[i], &g3[i - 1], &p0[i], bk);
		}
		else
		{
			bootsXOR(&result[i], &g4[i - 1], &p0[i], bk);
		}
	}
	bootsCOPY(&carry_out[0], &g4[15], bk);
}

// 102
void p_adder_8(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	LweSample *p0 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *p1 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *p2 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *p3 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *p4 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	LweSample *g0 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *g1 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *g2 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *g3 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *g4 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	LweSample *temp = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *carry_out = new_gate_bootstrapping_ciphertext_array(1, bk->params);
	// Stage 1
	// level 1: 32 parallel gates
	int i;
#pragma omp parallel for private(i) shared(p0, g0)
	for (i = 0; i < nb_bits; i++)
	{
		bootsXOR(&p0[i], &a[i], &b[i], bk);
		bootsAND(&g0[i], &a[i], &b[i], bk);
	}

	// Stage 2
	// level 2: 30 parallel gates
	// level 3: 15 parallel gates (OR gate depends on AND)
#pragma omp parallel for private(i) shared(p1, g1)
	for (i = 0; i < nb_bits; i++)
	{
		if (i == 0)
		{
			bootsCOPY(&g1[i], &g0[i], bk);
		}
		else
		{
			bootsAND(&temp[i], &p0[i], &g0[i - 1], bk);
			bootsOR(&g1[i], &temp[i], &g0[i], bk);
			bootsAND(&p1[i], &p0[i], &p0[i - 1], bk);
		}
	}

	// Stage 3
	// level 4: 27 parallel gates
	// level 5: 14 parallel gates
#pragma omp parallel for private(i) shared(g2, p2)
	for (i = 1; i < nb_bits; i++)
	{
		if (i == 1)
		{
			bootsCOPY(&g2[i], &g1[i], bk);
		}
		else if (i == 2)
		{
			bootsAND(&temp[i], &g1[i - 2], &p1[i], bk);
			bootsOR(&g2[i], &temp[i], &g1[i], bk);
		}
		else
		{
			bootsAND(&temp[i], &p1[i], &g1[i - 2], bk);
			bootsOR(&g2[i], &temp[i], &g1[i], bk);
			bootsAND(&p2[i], &p1[i], &p1[i - 2], bk);
		}
	}

	// Stage 4
	// level 6: 21 parallel gates
	// level 7: 12 parallel gates
#pragma omp parallel for private(i) shared(g3, p3)
	for (i = 3; i < nb_bits; i++)
	{
		if (i == 3)
		{
			bootsCOPY(&g3[i], &g2[i], bk);
		}
		else if (i == 4)
		{
			bootsAND(&temp[i], &g1[i - 4], &p2[i], bk);
			bootsOR(&g3[i], &temp[i], &g2[i], bk);
		}
		else if (i < 7)
		{
			bootsAND(&temp[i], &g2[i - 4], &p2[i], bk);
			bootsOR(&g3[i], &temp[i], &g2[i], bk);
		}
		else
		{
			bootsAND(&temp[i], &p2[i], &g2[i - 4], bk);
			bootsOR(&g3[i], &temp[i], &g2[i], bk);
			bootsAND(&p3[i], &p2[i], &p2[i - 4], bk);
		}
	}

	// Generate Sum and Carry Out
	// level 10: 15 parallel gates
#pragma omp parallel for private(i) shared(result)
	for (i = 0; i < nb_bits; i++)
	{
		if (i == 0)
		{
			bootsCOPY(&result[i], &p0[i], bk);
		}
		else if (i == 1)
		{
			bootsXOR(&result[i], &g1[i - 1], &p0[i], bk);
		}
		else if (i < 4)
		{
			bootsXOR(&result[i], &g2[i - 1], &p0[i], bk);
		}
		else if (i < 8)
		{
			bootsXOR(&result[i], &g3[i - 1], &p0[i], bk);
		}
		//    else
		//    {
		//      bootsXOR(&result[i], &g4[i - 1], &p0[i], bk);
		//    }
	}
	bootsCOPY(&carry_out[0], &g4[nb_bits - 1], bk);
}

// 103
void p_adder(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	if (nb_bits == 8)
	{

		p_adder_8(result, a, b, nb_bits, bk);
	}
	else if (nb_bits == 16)
	{
		p_adder_16(result, a, b, nb_bits, bk);
	}
	else if (nb_bits == 32)
	{
		p_adder_32(result, a, b, nb_bits, bk);
	}
	else
	{
		// this the normal adder function.
		adder(result, a, b, nb_bits, bk);
	}
}

// Helper function
void e_shl_p(LweSample *result, const LweSample *a, const LweSample *LSB, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	LweSample *temp = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	for (int i = 1; i < nb_bits; i++)
	{
		bootsCOPY(&result[i], &a[i - 1], bk);
	}

	// for (int j = 1; j < nb_bits; j++)
	// {
	//     bootsCOPY(&temp[j], &result[j - 1], bk);
	// }

	// for (int k = 1; k < nb_bits; k++)
	// {
	//     bootsCOPY(&result[k], &temp[k], bk);
	// }
	// const LweSample* tmp_LSB = &LSB;
	bootsCOPY(&result[0], &LSB[0], bk); // set lowest bit to the new LSB

	delete_gate_bootstrapping_ciphertext_array(nb_bits, temp);
}

// 105
void rot_r(LweSample *result, const LweSample *a, const int amnt, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	LweSample *temp = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	for (int i = 0; i < nb_bits; i++)
	{
		bootsCOPY(&result[i], &a[i], bk);
	}

	for (size_t i = 0; i < amnt; i++)
	{
		LweSample *lsb = new_gate_bootstrapping_ciphertext(bk->params);
		bootsCOPY(lsb, &result[0], bk);

		for (int j = 0; j < nb_bits - 1; j++)
		{
			bootsCOPY(&temp[j], &result[j + 1], bk);
		}

		bootsCOPY(&temp[nb_bits - 1], lsb, bk);

		for (int j = 0; j < nb_bits; j++)
		{
			bootsCOPY(&result[j], &temp[j], bk);
		}
	}

	delete_gate_bootstrapping_ciphertext_array(nb_bits, temp);
}

// 106
void rot_l(LweSample *result, const LweSample *a, const int amnt, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	LweSample *temp = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	for (int i = 0; i < nb_bits; i++)
	{
		bootsCOPY(&result[i], &a[i], bk);
	}

	for (size_t i = 0; i < amnt; i++)
	{
		LweSample *msb = new_gate_bootstrapping_ciphertext(bk->params);
		bootsCOPY(msb, &result[nb_bits - 1], bk);

		for (int j = 1; j < nb_bits; j++)
		{
			bootsCOPY(&temp[j], &result[j - 1], bk);
		}

		bootsCOPY(&temp[0], msb, bk);

		for (int j = 0; j < nb_bits; j++)
		{
			bootsCOPY(&result[j], &temp[j], bk);
		}
	}

	delete_gate_bootstrapping_ciphertext_array(nb_bits, temp);
}

// 107
void blake3(vector<LweSample *> &result, vector<LweSample *> msg, const vector<LweSample *> v, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	list_of_results = true;

	LweSample *a = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *b = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *c = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *d = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *m0 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *m1 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

#pragma omp parallel sections
	{
#pragma omp section
		{
			for (size_t i = 0; i < nb_bits; i++)
			{
				bootsCOPY(&a[i], &v[0][i], bk);
			}
		}

#pragma omp section
		{
			for (size_t i = 0; i < nb_bits; i++)
			{
				bootsCOPY(&b[i], &v[1][i], bk);
			}
		}

#pragma omp section
		{
			for (size_t i = 0; i < nb_bits; i++)
			{
				bootsCOPY(&c[i], &v[2][i], bk);
			}
		}

#pragma omp section
		{
			for (size_t i = 0; i < nb_bits; i++)
			{
				bootsCOPY(&d[i], &v[3][i], bk);
			}
		}
#pragma omp section
		{
			for (size_t i = 0; i < nb_bits; i++)
			{
				bootsCOPY(&m0[i], &msg[0][i], bk);
			}
		}

#pragma omp section
		{
			for (size_t i = 0; i < nb_bits; i++)
			{
				bootsCOPY(&m1[i], &msg[1][i], bk);
			}
		}
	}

	// Step 1: a = a + b + m0
	// Step 1.1: a = a + b

	LweSample *tmp_a = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *tmp_d = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *tmp_c = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *tmp_b = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	p_adder(tmp_a, a, b, nb_bits, bk);

	// Step 1.2: a = a + m0
	p_adder(a, tmp_a, m0, nb_bits, bk);

	// Step 2: d = (d XOR a ) >>> 16
	// Step 2.1: d = d XOR a
	size_t i;
#pragma omp parallel for private(i) shared(tmp_d)
	for (i = 0; i < nb_bits; i++)
	{
		bootsXOR(&tmp_d[i], &d[i], &a[i], bk);
	}

#pragma omp parallel sections
	{
		// Step 2.2: d = d >>> 16
		/** SKIP THIS ROTATION IF OUR INPUT SIZE IS 16, ROTATING BY 16 YIELDS THE SAME NUMBER*/
#pragma omp section
		{
			rot_r(d, tmp_d, 16, nb_bits, bk);
		}
#pragma omp section
		{
			// Step 3: c = c + d
			for (size_t i = 0; i < nb_bits; i++)
			{
				bootsCOPY(&tmp_c[i], &c[i], bk);
			}
		}
	}

	p_adder(c, tmp_c, d, nb_bits, bk);

	// Step 4: b = (b XOR c) >>> 12
	// Step 4.1: b = b XOR c
#pragma omp parallel for private(i) shared(tmp_b)
	for (i = 0; i < nb_bits; i++)
	{
		bootsXOR(&tmp_b[i], &b[i], &c[i], bk);
	}

	// Step 4.2: b = b >>> 12
	rot_r(b, tmp_b, 12, nb_bits, bk);

	// Step 5: a = a + b + m1
	// Step 5.1: a = a + b

	p_adder(tmp_a, a, b, nb_bits, bk);

	// Step 5.2: a = a + m1
	p_adder(a, tmp_a, m1, nb_bits, bk);

	// Step 6: d = (d XOR a ) >>> 8
	// Step 6.1: d = d XOR a
#pragma omp parallel for private(i) shared(tmp_d)
	for (i = 0; i < nb_bits; i++)
	{
		bootsXOR(&tmp_d[i], &d[i], &a[i], bk);
	}

#pragma omp parallel sections
	{

#pragma omp section
		{
			// Step 6.2: d = d >>> 8
			rot_r(d, tmp_d, 8, nb_bits, bk);
		}
#pragma omp section
		{
			// Step 7: c = c + d
			for (size_t i = 0; i < nb_bits; i++)
			{
				bootsCOPY(&tmp_c[i], &c[i], bk);
			}
		}
	}

	p_adder(c, tmp_c, d, nb_bits, bk);

	// Step 8: b = (b XOR c) >>> 12
	// Step 8.1: b = b XOR c
#pragma omp parallel for private(i) shared(tmp_b)
	for (i = 0; i < nb_bits; i++)
	{
		bootsXOR(&tmp_b[i], &b[i], &c[i], bk);
	}

	// Step 8.2: b = b >>> 7
	rot_r(b, tmp_b, 7, nb_bits, bk);

	result.push_back(a);
	result.push_back(b);
	result.push_back(c);
	result.push_back(d);
}

// helper function
void compare_bit(LweSample *result, const LweSample *a, const LweSample *b, const LweSample *lsb_carry, LweSample *tmp, const TFheGateBootstrappingCloudKeySet *bk)
{
	bootsXNOR(tmp, a, b, bk);
	bootsMUX(result, tmp, lsb_carry, a, bk);
}

// 108
void max_tmp(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	LweSample *tmps = new_gate_bootstrapping_ciphertext_array(2, bk->params);

	// initialize the carry to 0
	bootsCONSTANT(&tmps[0], 0, bk);
	bootsCONSTANT(&tmps[1], 0, bk);

	// run the elementary comparator gate n times
	for (int i = 0; i < nb_bits; i++)
	{
		compare_bit(&tmps[0], &a[i], &b[i], &tmps[0], &tmps[1], bk);
	}
	// tmps[0] is the result of the comparaison: 0 if a is larger, 1 if b is larger
	// select the max and copy it to the result
	int i;
#pragma omp parallel for private(i) shared(result)
	for (i = 0; i < nb_bits; i++)
	{
		bootsMUX(&result[i], &tmps[0], &a[i], &b[i], bk);
	}

	delete_gate_bootstrapping_ciphertext_array(2, tmps);
}

void max(LweSample *result, vector<LweSample *> a, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	LweSample *tmps = new_gate_bootstrapping_ciphertext_array(2, bk->params);
	LweSample *max = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *current = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	for (size_t i = 0; i < nb_bits; i++)
	{
		bootsCOPY(&max[i], &a[0][i], bk);
	}

	size_t size = a.size();
	for (size_t indx = 0; indx < size; indx++)
	{

		for (size_t i = 0; i < nb_bits; i++)
		{
			bootsCOPY(&current[i], &a[indx][i], bk);
		}

		// initialize the carry to 0
		bootsCONSTANT(&tmps[0], 0, bk);
		bootsCONSTANT(&tmps[1], 0, bk);

		// run the elementary comparator gate n times
		for (int i = 0; i < nb_bits; i++)
		{
			compare_bit(&tmps[0], &max[i], &current[i], &tmps[0], &tmps[1], bk);
		}
		// tmps[0] is the result of the comparaison: 0 if a is larger, 1 if b is larger
		// select the max and copy it to the result
		int c;
#pragma omp parallel for private(c) shared(result)
		for (c = 0; c < nb_bits; c++)
		{
			bootsMUX(&result[c], &tmps[0], &max[c], &current[c], bk);
		}

		for (size_t i = 0; i < nb_bits; i++)
		{
			bootsCOPY(&max[i], &result[i], bk);
		}
	}
	delete_gate_bootstrapping_ciphertext_array(nb_bits, max);
	delete_gate_bootstrapping_ciphertext_array(nb_bits, current);
	delete_gate_bootstrapping_ciphertext_array(2, tmps);
}

// 109
void min_tmp(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	LweSample *tmps = new_gate_bootstrapping_ciphertext_array(2, bk->params);

	// initialize the carry to 0
	bootsCONSTANT(&tmps[0], 0, bk);
	// run the elementary comparator gate n times
	for (int i = 0; i < nb_bits; i++)
	{
		compare_bit(&tmps[0], &a[i], &b[i], &tmps[0], &tmps[1], bk);
	}
	// tmps[0] is the result of the comparaison: 0 if a is larger, 1 if b is larger
	// select the min and copy it to the result
	int i;
#pragma omp parallel for private(i) shared(result)
	for (int i = 0; i < nb_bits; i++)
	{
		bootsMUX(&result[i], &tmps[0], &b[i], &a[i], bk);
	}

	delete_gate_bootstrapping_ciphertext_array(2, tmps);
}

void min(LweSample *result, vector<LweSample *> a, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	LweSample *tmps = new_gate_bootstrapping_ciphertext_array(2, bk->params);
	LweSample *min = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *current = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	for (size_t i = 0; i < nb_bits; i++)
	{
		bootsCOPY(&min[i], &a[0][i], bk);
	}

	size_t size = a.size();
	for (size_t indx = 1; indx < size; indx++)
	{

		for (size_t i = 1; i < nb_bits; i++)
		{
			bootsCOPY(&current[i], &a[indx][i], bk);
		}

		// initialize the carry to 0
		bootsCONSTANT(&tmps[0], 0, bk);
		bootsCONSTANT(&tmps[1], 0, bk);

		// run the elementary comparator gate n times
		for (int i = 0; i < nb_bits; i++)
		{
			compare_bit(&tmps[0], &min[i], &current[i], &tmps[0], &tmps[1], bk);
		}
		// tmps[0] is the result of the comparaison: 0 if a is larger, 1 if b is larger
		// select the max and copy it to the result
		int c;
#pragma omp parallel for private(c) shared(result)
		for (c = 0; c < nb_bits; c++)
		{
			bootsMUX(&result[c], &tmps[0], &current[c], &min[c], bk);
		}

		for (size_t i = 0; i < nb_bits; i++)
		{
			bootsCOPY(&min[i], &result[i], bk);
		}
	}
	delete_gate_bootstrapping_ciphertext_array(nb_bits, min);
	delete_gate_bootstrapping_ciphertext_array(nb_bits, current);
	delete_gate_bootstrapping_ciphertext_array(2, tmps);
}

// 110 ...
void relu(LweSample *result, const LweSample *a, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	LweSample *p2 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	bootsNOT(&p2[nb_bits - 1], &a[nb_bits - 1], bk);
	for (size_t i = 0; i < nb_bits - 1; i++)
	{
		bootsCONSTANT(&p2[i], 0, bk);
	}

	LweSample *p3 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	for (size_t i = 0; i < nb_bits; i++)
	{
		bootsCOPY(&p3[i], &p2[nb_bits - i - 1], bk);
	}

	multiplier(result, p3, a, nb_bits, bk);

	delete_gate_bootstrapping_ciphertext_array(nb_bits, p2);
	delete_gate_bootstrapping_ciphertext_array(nb_bits, p3);
}

void relu_tmp(LweSample *result, const LweSample *a, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	cout << "this is relu..." << endl;

	LweSample *tmps = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	bootsCOPY(&tmps[nb_bits - 1], &a[nb_bits - 1], bk);
	for (size_t i = 0; i < nb_bits - 1; i++)
	{
		bootsCONSTANT(&tmps[i], 0, bk);
	}

	LweSample *zero = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	for (size_t i = 0; i < nb_bits; i++)
	{
		bootsCONSTANT(&zero[i], 0, bk);
	}

	LweSample *one = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	for (size_t i = 1; i < nb_bits; i++)
	{
		bootsCONSTANT(&one[i], 0, bk);
	}
	bootsCONSTANT(&one[0], 1, bk);

	LweSample *p1 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	multiplier(p1, zero, tmps, nb_bits, bk);

	LweSample *p2 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	bootsNOT(&p2[nb_bits - 1], &tmps[nb_bits - 1], bk);
	for (size_t i = 0; i < nb_bits - 1; i++)
	{
		bootsCOPY(&p2[i], &tmps[i], bk);
	}

	LweSample *p3 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	for (size_t i = 0; i < nb_bits; i++)
	{
		bootsCOPY(&p3[i], &p2[nb_bits - i - 1], bk);
	}

	LweSample *p4 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	multiplier(p4, p3, a, nb_bits, bk);

	adder(result, p1, p4, nb_bits, bk);

	delete_gate_bootstrapping_ciphertext_array(2, tmps);
}

// 111
void div(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	//  Q is the dividend, copy the bits from a to Q
	LweSample *Q = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	// A is the remainder, initialize to 0
	LweSample *A = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	// M is the divisor, copy the bits from b to M
	LweSample *M = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	for (int i = 0; i < nb_bits; i++)
		bootsCOPY(&Q[i], &a[i], bk);

	for (int i = 0; i < nb_bits; i++)
	{
		bootsCONSTANT(&A[i], 0, bk);
	}
	for (int i = 0; i < nb_bits; i++)
	{
		bootsCOPY(&M[i], &b[i], bk);
	}

	// The most siginificant bit
	LweSample *MSB = new_gate_bootstrapping_ciphertext(bk->params);
	LweSample *zero = new_gate_bootstrapping_ciphertext(bk->params);
	bootsCONSTANT(zero, 0, bk);
	LweSample *Q_tmp = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *A_tmp = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *A_msb = new_gate_bootstrapping_ciphertext(bk->params);
	LweSample *not_A_msb = new_gate_bootstrapping_ciphertext(bk->params);
	LweSample *A_m = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *A_sub = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	// Loop over the each bit of Q and M
	for (int i = 0; i < nb_bits; i++)
	{
		// Left shift Q, and replace the LSB with 0
		e_shl_p(Q_tmp, Q, zero, nb_bits, bk);

		// Store the MSB
		bootsCOPY(MSB, &Q[nb_bits - 1], bk);

		// update the Q after shifing
		for (int i = 0; i < nb_bits; i++)
		{
			bootsCOPY(&Q[i], &Q_tmp[i], bk);
		}

		// Left shift A, and replace the LSB with the MSB from the previous steps.
		e_shl_p(A_tmp, A, MSB, nb_bits, bk);

		// Compute A = A - M
		subtracter(A, A_tmp, M, nb_bits, bk);

		// Get the MSB of A
		bootsCOPY(A_msb, &A[nb_bits - 1], bk);
		// Compute Not(MSB of A)
		bootsNOT(not_A_msb, A_msb, bk);

		// Replace the LSB of Q with Not(MSB of A)
		bootsCOPY(&Q[0], not_A_msb, bk);

		// Compute A = A + M
		p_adder(A_m, A, M, nb_bits, bk);

		// If (A[0] == 1), A = A + M. Else, A is not changed.
		// This can be parallelized
#pragma omp parallel for private(i) shared(A)
		for (int i = 0; i < nb_bits; i++)
		{
			bootsMUX(&A[i], A_msb, &A_m[i], &A[i], bk);
		}
	}
	for (size_t i = 0; i < nb_bits; i++)
	{
		bootsCOPY(&result[i], &Q[i], bk);
	}
}

// 112
void mod(LweSample *result, const LweSample *a, const LweSample *b, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	//  Q is the dividend, copy the bits from a to Q
	LweSample *Q = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	// A is the remainder, initialize to 0
	LweSample *A = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	// M is the divisor, copy the bits from b to M
	LweSample *M = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	for (int i = 0; i < nb_bits; i++)
		bootsCOPY(&Q[i], &a[i], bk);

	for (int i = 0; i < nb_bits; i++)
	{
		bootsCONSTANT(&A[i], 0, bk);
	}
	for (int i = 0; i < nb_bits; i++)
	{
		bootsCOPY(&M[i], &b[i], bk);
	}

	// The most siginificant bit
	LweSample *MSB = new_gate_bootstrapping_ciphertext(bk->params);
	LweSample *zero = new_gate_bootstrapping_ciphertext(bk->params);
	bootsCONSTANT(zero, 0, bk);
	LweSample *Q_tmp = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *A_tmp = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *A_msb = new_gate_bootstrapping_ciphertext(bk->params);
	LweSample *not_A_msb = new_gate_bootstrapping_ciphertext(bk->params);
	LweSample *A_m = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *A_sub = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	// Loop over the each bit of Q and M
	for (int i = 0; i < nb_bits; i++)
	{
		// Left shift Q, and replace the LSB with 0
		e_shl_p(Q_tmp, Q, zero, nb_bits, bk);

		// Store the MSB
		bootsCOPY(MSB, &Q[nb_bits - 1], bk);

		// update the Q after shifing
		for (int i = 0; i < nb_bits; i++)
		{
			bootsCOPY(&Q[i], &Q_tmp[i], bk);
		}

		// Left shift A, and replace the LSB with the MSB from the previous steps.
		e_shl_p(A_tmp, A, MSB, nb_bits, bk);

		// Compute A = A - M
		subtracter(A, A_tmp, M, nb_bits, bk);

		// Get the MSB of A
		bootsCOPY(A_msb, &A[nb_bits - 1], bk);
		// Compute Not(MSB of A)
		bootsNOT(not_A_msb, A_msb, bk);

		// Replace the LSB of Q with Not(MSB of A)
		bootsCOPY(&Q[0], not_A_msb, bk);

		// Compute A = A + M
		p_adder(A_m, A, M, nb_bits, bk);

		// If (A[0] == 1), A = A + M. Else, A is not changed.
		// This can be parallelized
#pragma omp parallel for private(i) shared(A)
		for (int i = 0; i < nb_bits; i++)
		{
			bootsMUX(&A[i], A_msb, &A_m[i], &A[i], bk);
		}
	}
	for (size_t i = 0; i < nb_bits; i++)
	{
		bootsCOPY(&result[i], &A[i], bk);
	}
}

// 113
void mean(LweSample *result, vector<LweSample *> a, LweSample *c_count, int count, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	LweSample *sum = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *tmp_sum = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	LweSample *n1 = a[0];
	for (size_t i = 0; i < nb_bits; i++)
	{
		bootsCOPY(&sum[i], &n1[i], bk);
	}

	// sum the values
	for (size_t i = 1; i < count; i++)
	{
		LweSample *n2 = a[i];
		p_adder(tmp_sum, sum, n2, nb_bits, bk);

		for (size_t j = 0; j < nb_bits; j++)
		{
			bootsCOPY(&sum[j], &tmp_sum[j], bk);
		}
	}

	div(result, sum, c_count, nb_bits, bk);
}

// 114
void e_sqrt(LweSample *result, const LweSample *a, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	LweSample *x = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *y = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *x_y = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *x_2 = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *two = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	int n_iters;

	if (nb_bits == 8)
		n_iters = 5;
	else if (nb_bits == 16)
		n_iters = 10;
	else if (nb_bits == 32)
		n_iters = 20;
	else
		cerr << "Unsupported bitsize " << nb_bits << " in sqrt instruction" << endl;

	for (int i = 0; i < nb_bits; i++)
	{
		bootsCOPY(&x[i], &a[i], bk);
	}

	bootsCONSTANT(&y[0], 1, bk);

	for (int i = 1; i < nb_bits; i++)
	{
		bootsCONSTANT(&y[i], 0, bk);
	}

	for (int i = 0; i < nb_bits; i++)
	{
		if (i == 1)
			bootsCONSTANT(&two[i], 1, bk);
		else
			bootsCONSTANT(&two[i], 0, bk);
	}

	for (int i = 0; i < n_iters; i++)
	{

		p_adder(x_y, x, y, nb_bits, bk);

		div(x, x_y, two, nb_bits, bk);

		// skip the last division
		if (i != n_iters - 1)
		{
			div(y, a, x, nb_bits, bk);
		}
	}

	for (int i = 0; i < nb_bits; i++)
	{
		bootsCOPY(&result[i], &x[i], bk);
	}
}

// 115
void variance(LweSample *result, vector<LweSample *> a, LweSample *c_count, int count, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	cout << "this is variance ... " << endl;
	LweSample *m = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	// compute the mean
	mean(m, a, c_count, count, nb_bits, bk);
	// sub and square
	vector<LweSample *> subs_sqrs;
	vector<LweSample *> subs;
	vector<LweSample *> sqrs;
	for (size_t i = 0; i < count; i++)
	{
		LweSample *sub = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
		LweSample *sqr = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
		subs.push_back(sub);
		sqrs.push_back(sqr);
	}
	size_t i;
#pragma omp parallel for private(i) shared(subs, a)
	for (i = 0; i < count; i++)
	{
		subtracter(subs[i], a[i], m, nb_bits, bk);
	}

	for (i = 0; i < count; i++)
	{
		multiplier(sqrs[i], subs[i], subs[i], nb_bits, bk);

		subs_sqrs.push_back(sqrs[i]);
	}

	/// sum of squares
	LweSample *sum = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	LweSample *tmp_sum = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	LweSample *n1 = subs_sqrs[0];
	// #pragma omp parallel for
	for (size_t i = 0; i < nb_bits; i++)
	{
		bootsCOPY(&sum[i], &n1[i], bk);
	}

	// sum the values
	// #pragma omp parallel for
	for (size_t i = 1; i < count; i++)
	{
		LweSample *n2 = subs_sqrs[i];
		p_adder(tmp_sum, sum, n2, nb_bits, bk);

		// #pragma omp paralle for
		for (size_t j = 0; j < nb_bits; j++)
		{
			bootsCOPY(&sum[j], &tmp_sum[j], bk);
		}
	}

	// divide by N-1
	//  LweSample *n = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	//  LweSample *one = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);
	//  bootsCONSTANT(one, 1, bk);

	// subtracter(n, c_count, one, nb_bits ,bk);
	div(result, sum, c_count, nb_bits, bk);
}

// 116
void stdev(LweSample *result, vector<LweSample *> a, LweSample *c_count, int count, const int nb_bits, const TFheGateBootstrappingCloudKeySet *bk)
{
	LweSample *var = new_gate_bootstrapping_ciphertext_array(nb_bits, bk->params);

	variance(var, a, c_count, count, nb_bits, bk);

	e_sqrt(result, var, nb_bits, bk);
}

/*NEW PULPFHE INSTRUCTIONS END HERE*/

void op_select(char *instruction, TFheGateBootstrappingCloudKeySet *bk, const TFheGateBootstrappingParameterSet *params)
{
	char *token;
	token = strtok(instruction, " ");
	int wordSize = atoi(token);
	token = strtok(NULL, " ");
	int operation = atoi(token);

	LweSample *ciphertext1 = new_gate_bootstrapping_ciphertext_array(wordSize, params);
	LweSample *ciphertext2 = new_gate_bootstrapping_ciphertext_array(wordSize, params);
	LweSample *ciphertext3 = new_gate_bootstrapping_ciphertext_array(wordSize, params);

	LweSample *result = new_gate_bootstrapping_ciphertext_array(wordSize, params);
	vector<LweSample *> result_lst;

	FILE *ctxt_one_data;
	FILE *ctxt_two_data;
	FILE *ctxt_three_data;

	map<string, vector<LweSample *>> lsts;

	if (operation < 9)
	{ // 2 input ciphertexts
		token = strtok(NULL, " ");
		ctxt_one_data = fopen(token, "rb");
		token = strtok(NULL, " ");
		ctxt_two_data = fopen(token, "rb");

		for (int i = 0; i < wordSize; i++)
		{
			import_gate_bootstrapping_ciphertext_fromFile(ctxt_one_data, &ciphertext1[i], params);
			import_gate_bootstrapping_ciphertext_fromFile(ctxt_two_data, &ciphertext2[i], params);
		}

		fclose(ctxt_one_data);
		fclose(ctxt_two_data);

		if (operation == 0)
		{ // AND
			e_and(result, ciphertext1, ciphertext2, wordSize, bk);
		}
		else if (operation == 1)
		{ // NAND
			e_nand(result, ciphertext1, ciphertext2, wordSize, bk);
		}
		else if (operation == 2)
		{ // OR
			e_or(result, ciphertext1, ciphertext2, wordSize, bk);
		}
		else if (operation == 3)
		{ // NOR
			e_nor(result, ciphertext1, ciphertext2, wordSize, bk);
		}
		else if (operation == 4)
		{ // XOR
			e_xor(result, ciphertext1, ciphertext2, wordSize, bk);
		}
		else if (operation == 5)
		{ // XNOR
			e_xnor(result, ciphertext1, ciphertext2, wordSize, bk);
		}
		else if (operation == 6)
		{ // subtract
			subtracter(result, ciphertext1, ciphertext2, wordSize, bk);
		}
		else if (operation == 7)
		{ // add
			p_adder(result, ciphertext1, ciphertext2, wordSize, bk);
		}
		else if (operation == 8)
		{ // multiply
			multiplier(result, ciphertext1, ciphertext2, wordSize, bk);
		}
		else
		{
			cout << "Unknown Assembly Instruction ... Code " << operation << endl;
			exit(1);
		}
	}

	else if (operation < 12)
	{ // 1 input ciphertext
		token = strtok(NULL, " ");
		ctxt_one_data = fopen(token, "rb");

		for (int i = 0; i < wordSize; i++)
		{
			import_gate_bootstrapping_ciphertext_fromFile(ctxt_one_data, &ciphertext1[i], params);
		}

		fclose(ctxt_one_data);

		if (operation == 9)
		{ // NOT
			e_not(result, ciphertext1, wordSize, bk);
		}
		else if (operation == 10)
		{ // shift left
			token = strtok(NULL, " ");
			int shift_amount = atoi(token);
			e_shl(result, ciphertext1, shift_amount, wordSize, bk);
		}
		else if (operation == 11)
		{ // shift right
			token = strtok(NULL, " ");
			int shift_amount = atoi(token);
			e_shr(result, ciphertext1, shift_amount, wordSize, bk);
		}
		else
		{
			cout << "Unknown Assembly Instruction ... Code " << operation << endl;
			exit(1);
		}
	}

	else if (operation == 12)
	{ // MUX
		token = strtok(NULL, " ");
		ctxt_one_data = fopen(token, "rb");
		token = strtok(NULL, " ");
		ctxt_two_data = fopen(token, "rb");
		token = strtok(NULL, " ");
		ctxt_three_data = fopen(token, "rb");

		for (int i = 0; i < wordSize; i++)
		{
			import_gate_bootstrapping_ciphertext_fromFile(ctxt_one_data, &ciphertext1[i], params);
			import_gate_bootstrapping_ciphertext_fromFile(ctxt_two_data, &ciphertext2[i], params);
			import_gate_bootstrapping_ciphertext_fromFile(ctxt_three_data, &ciphertext3[i], params);
		}

		fclose(ctxt_one_data);
		fclose(ctxt_two_data);
		fclose(ctxt_three_data);

		e_mux(result, ciphertext1, ciphertext2, ciphertext3, wordSize, bk);
	}

	else if (operation == 13)
	{ // comp
		token = strtok(NULL, " ");
		ctxt_one_data = fopen(token, "rb");
		token = strtok(NULL, " ");
		ctxt_two_data = fopen(token, "rb");
		token = strtok(NULL, " ");
		int select = atoi(token);

		for (int i = 0; i < wordSize; i++)
		{
			import_gate_bootstrapping_ciphertext_fromFile(ctxt_one_data, &ciphertext1[i], params);
			import_gate_bootstrapping_ciphertext_fromFile(ctxt_two_data, &ciphertext2[i], params);
		}

		fclose(ctxt_one_data);
		fclose(ctxt_two_data);

		comparator(result, ciphertext1, ciphertext2, wordSize, select, bk);
	}

	else if (operation == 14)
	{ // econst
		token = strtok(NULL, " ");
		int64_t ptxt_val = atoi(token);
		for (int i = 0; i < wordSize; i++)
		{
			bootsCONSTANT(&result[i], (ptxt_val >> i) & 1, bk);
		}
	}

	else if (operation > 100)
	{
		if (operation == 101)
		{
			// elist
			token = strtok(NULL, " ");
			vector<LweSample *> v;
			lsts[token] = v;
			cout << "RECEIVED: elist " << token << endl;
		}

		else if (operation == 102)
		{
			// secread_l
			char *lst = strtok(NULL, " ");
			char *size = strtok(NULL, " ");
			cout << "RECEIVED: secread_l " << lst << size << endl;
		}

		else if (operation == 105)
		{
			// rotate right
			// 1 input ciphertext
			token = strtok(NULL, " ");
			ctxt_one_data = fopen(token, "rb");

			for (int i = 0; i < wordSize; i++)
			{
				import_gate_bootstrapping_ciphertext_fromFile(ctxt_one_data, &ciphertext1[i], params);
			}

			fclose(ctxt_one_data);

			token = strtok(NULL, " ");
			int rot_amount = atoi(token);
			rot_r(result, ciphertext1, rot_amount, wordSize, bk);
		}

		else if (operation == 106)
		{
			// rotate left
			// 1 input ciphertext
			token = strtok(NULL, " ");
			ctxt_one_data = fopen(token, "rb");

			for (int i = 0; i < wordSize; i++)
			{
				import_gate_bootstrapping_ciphertext_fromFile(ctxt_one_data, &ciphertext1[i], params);
			}

			fclose(ctxt_one_data);

			token = strtok(NULL, " ");
			int rot_amount = atoi(token);
			rot_l(result, ciphertext1, rot_amount, wordSize, bk);
		}

		else if (operation == 111)
		{ // divide
		  // 2 input ciphertexts
			token = strtok(NULL, " ");
			ctxt_one_data = fopen(token, "rb");
			token = strtok(NULL, " ");
			ctxt_two_data = fopen(token, "rb");

			for (int i = 0; i < wordSize; i++)
			{
				import_gate_bootstrapping_ciphertext_fromFile(ctxt_one_data, &ciphertext1[i], params);
				import_gate_bootstrapping_ciphertext_fromFile(ctxt_two_data, &ciphertext2[i], params);
			}

			fclose(ctxt_one_data);
			fclose(ctxt_two_data);
			div(result, ciphertext1, ciphertext2, wordSize, bk);
		}

		else if (operation == 112)
		{
			// mod
			// 2 input ciphertexts
			token = strtok(NULL, " ");
			ctxt_one_data = fopen(token, "rb");
			token = strtok(NULL, " ");
			ctxt_two_data = fopen(token, "rb");

			for (int i = 0; i < wordSize; i++)
			{
				import_gate_bootstrapping_ciphertext_fromFile(ctxt_one_data, &ciphertext1[i], params);
				import_gate_bootstrapping_ciphertext_fromFile(ctxt_two_data, &ciphertext2[i], params);
			}

			fclose(ctxt_one_data);
			fclose(ctxt_two_data);
			mod(result, ciphertext1, ciphertext2, wordSize, bk);
		}

		else if (operation == 114)
		{
			// sqrt
			token = strtok(NULL, " ");
			ctxt_one_data = fopen(token, "rb");

			for (int i = 0; i < wordSize; i++)
			{
				import_gate_bootstrapping_ciphertext_fromFile(ctxt_one_data, &ciphertext1[i], params);
			}

			fclose(ctxt_one_data);
			e_sqrt(result, ciphertext1, wordSize, bk);
		}

		else if (operation == 115)
		{
			// Variance
			token = strtok(NULL, " ");
			cout << "RECEIVED: " << token << endl;
			vector<string> dat = split(token, ",");
			int size = stoi(dat[0]);
			string ctxtLst[size];

			// char tmp_buf[MAX];
			// memset(tmp_buf, 0, MAX);
			// tmp_buf[0] = 'O';
			// tmp_buf[1] = 'K';
			// write(tmp_socket, tmp_buf, sizeof(tmp_buf));

			// for (int i = 0; i < size; i++)
			// {
			//   memset(tmp_buf, 0, MAX);

			//   read(tmp_socket, tmp_buf, sizeof(tmp_buf));

			//   token = strtok(tmp_buf, " ");
			//   ctxtLst[i] = token;
			//   cout << "\t\t" << ctxtLst[i] << endl;
			//   tmp_buf[0] = 'O';
			//   tmp_buf[1] = 'K';
			//   write(tmp_socket, tmp_buf, sizeof(tmp_buf));
			// }

			vector<LweSample *> ctxt_lst;
			for (int i = 0; i < size; i++)
			{
				ctxt_one_data = fopen(dat[i + 1].c_str(), "rb");

				for (int i = 0; i < wordSize; i++)
				{
					import_gate_bootstrapping_ciphertext_fromFile(ctxt_one_data, &ciphertext1[i], params);
				}
				ctxt_lst.push_back(ciphertext1);

				fclose(ctxt_one_data);
				ciphertext1 = new_gate_bootstrapping_ciphertext_array(wordSize, params);
			}

			LweSample *c_count = new_gate_bootstrapping_ciphertext_array(wordSize, bk->params);

			for (int i = 0; i < wordSize; i++)
			{
				bootsCONSTANT(&c_count[i], (size >> i) & 1, bk);
			}

			variance(result, ctxt_lst, c_count, size, wordSize, bk);

			delete_gate_bootstrapping_ciphertext_array(wordSize, c_count);
		}

		else if (operation == 116)
		{
			// Mean
			token = strtok(NULL, " ");
			vector<string> dat = split(token, ",");
			int size = stoi(dat[0]);
			string ctxtLst[size];

			vector<LweSample *> ctxt_lst;
			for (int i = 0; i < size; i++)
			{
				ctxt_one_data = fopen(dat[i + 1].c_str(), "rb");

				for (int i = 0; i < wordSize; i++)
				{
					import_gate_bootstrapping_ciphertext_fromFile(ctxt_one_data, &ciphertext1[i], params);
				}
				ctxt_lst.push_back(ciphertext1);

				fclose(ctxt_one_data);
				ciphertext1 = new_gate_bootstrapping_ciphertext_array(wordSize, params);
			}

			LweSample *c_count = new_gate_bootstrapping_ciphertext_array(wordSize, bk->params);

			for (int i = 0; i < wordSize; i++)
			{
				bootsCONSTANT(&c_count[i], (size >> i) & 1, bk);
			}

			mean(result, ctxt_lst, c_count, size, wordSize, bk);

			delete_gate_bootstrapping_ciphertext_array(wordSize, c_count);
		}

		else if (operation == 117)
		{
			// Std
			token = strtok(NULL, " ");
			vector<string> dat = split(token, ",");
			int size = stoi(dat[0]);
			string ctxtLst[size];

			vector<LweSample *> ctxt_lst;
			for (int i = 0; i < size; i++)
			{
				ctxt_one_data = fopen(dat[i + 1].c_str(), "rb");

				for (int i = 0; i < wordSize; i++)
				{
					import_gate_bootstrapping_ciphertext_fromFile(ctxt_one_data, &ciphertext1[i], params);
				}
				ctxt_lst.push_back(ciphertext1);

				fclose(ctxt_one_data);
				ciphertext1 = new_gate_bootstrapping_ciphertext_array(wordSize, params);
			}

			LweSample *c_count = new_gate_bootstrapping_ciphertext_array(wordSize, bk->params);

			for (int i = 0; i < wordSize; i++)
			{
				bootsCONSTANT(&c_count[i], (size >> i) & 1, bk);
			}

			stdev(result, ctxt_lst, c_count, size, wordSize, bk);

			delete_gate_bootstrapping_ciphertext_array(wordSize, c_count);
		}

		else if (operation == 118)
		{
			// relu
			token = strtok(NULL, " ");
			ctxt_one_data = fopen(token, "rb");

			for (int i = 0; i < wordSize; i++)
			{
				import_gate_bootstrapping_ciphertext_fromFile(ctxt_one_data, &ciphertext1[i], params);
			}

			fclose(ctxt_one_data);
			relu(result, ciphertext1, wordSize, bk);
		}

		else if (operation == 119)
		{
			// Max
			token = strtok(NULL, " ");
			vector<string> dat = split(token, ",");
			int size = stoi(dat[0]);
			string ctxtLst[size];

			vector<LweSample *> ctxt_lst;
			for (int i = 0; i < size; i++)
			{
				ctxt_one_data = fopen(dat[i + 1].c_str(), "rb");

				for (int i = 0; i < wordSize; i++)
				{
					import_gate_bootstrapping_ciphertext_fromFile(ctxt_one_data, &ciphertext1[i], params);
				}
				ctxt_lst.push_back(ciphertext1);

				fclose(ctxt_one_data);
				ciphertext1 = new_gate_bootstrapping_ciphertext_array(wordSize, params);
			}

			LweSample *c_count = new_gate_bootstrapping_ciphertext_array(wordSize, bk->params);

			for (int i = 0; i < wordSize; i++)
			{
				bootsCONSTANT(&c_count[i], (size >> i) & 1, bk);
			}

			max(result, ctxt_lst, wordSize, bk);

			delete_gate_bootstrapping_ciphertext_array(wordSize, c_count);
		}

		else if (operation == 120)
		{
			// Min
			token = strtok(NULL, " ");
			vector<string> dat = split(token, ",");
			int size = stoi(dat[0]);
			string ctxtLst[size];

			vector<LweSample *> ctxt_lst;
			for (int i = 0; i < size; i++)
			{
				ctxt_one_data = fopen(dat[i + 1].c_str(), "rb");

				for (int i = 0; i < wordSize; i++)
				{
					import_gate_bootstrapping_ciphertext_fromFile(ctxt_one_data, &ciphertext1[i], params);
				}
				ctxt_lst.push_back(ciphertext1);

				fclose(ctxt_one_data);
				ciphertext1 = new_gate_bootstrapping_ciphertext_array(wordSize, params);
			}

			LweSample *c_count = new_gate_bootstrapping_ciphertext_array(wordSize, bk->params);

			for (int i = 0; i < wordSize; i++)
			{
				bootsCONSTANT(&c_count[i], (size >> i) & 1, bk);
			}

			min(result, ctxt_lst, wordSize, bk);

			delete_gate_bootstrapping_ciphertext_array(wordSize, c_count);
		}

		else if (operation == 121)
		{
			// Blake3
			token = strtok(NULL, " ");
			vector<string> dat = split(token, ",");
			int size = stoi(dat[0]);
			string ctxtLst[size];

			vector<LweSample *> ctxt_lst;
			for (int i = 0; i < size; i++)
			{
				ctxt_one_data = fopen(dat[i + 1].c_str(), "rb");

				for (int i = 0; i < wordSize; i++)
				{
					import_gate_bootstrapping_ciphertext_fromFile(ctxt_one_data, &ciphertext1[i], params);
				}

				ctxt_lst.push_back(ciphertext1);

				fclose(ctxt_one_data);
				ciphertext1 = new_gate_bootstrapping_ciphertext_array(wordSize, params);
			}

			vector<LweSample *> iv;

			int32_t v0 = 0x6a09e667;
			LweSample *c_v0 = new_gate_bootstrapping_ciphertext_array(wordSize, params);
			for (int i = 0; i < wordSize; i++)
			{
				bootsCONSTANT(&c_v0[i], (v0 >> i) & 1, bk);
			}

			int32_t v1 = 0xbb67ae85;
			LweSample *c_v1 = new_gate_bootstrapping_ciphertext_array(wordSize, params);
			for (int i = 0; i < wordSize; i++)
			{
				bootsCONSTANT(&c_v1[i], (v1 >> i) & 1, bk);
			}

			int32_t v2 = 0x3c6ef372;
			LweSample *c_v2 = new_gate_bootstrapping_ciphertext_array(wordSize, params);
			for (int i = 0; i < wordSize; i++)
			{
				bootsCONSTANT(&c_v2[i], (v2 >> i) & 1, bk);
			}

			int32_t v3 = 0xa54ff53a;
			LweSample *c_v3 = new_gate_bootstrapping_ciphertext_array(wordSize, params);
			for (int i = 0; i < wordSize; i++)
			{
				bootsCONSTANT(&c_v3[i], (v3 >> i) & 1, bk);
			}

			iv.push_back(c_v0);
			iv.push_back(c_v1);
			iv.push_back(c_v2);
			iv.push_back(c_v3);

			blake3(result_lst, ctxt_lst, iv, wordSize, bk);

			delete_gate_bootstrapping_ciphertext_array(wordSize, c_v0);
			delete_gate_bootstrapping_ciphertext_array(wordSize, c_v1);
			delete_gate_bootstrapping_ciphertext_array(wordSize, c_v2);
			delete_gate_bootstrapping_ciphertext_array(wordSize, c_v3);
		}
	}

	else
	{
		cout << "Uknown Assembly Instruction ... Code " << operation << endl;
		exit(1);
	}

	char *fileName = (char *)malloc(50);
	fileName = gen_filename();
	FILE *answer_data = fopen(fileName, "wb");
	if (list_of_results)
	{
		for (size_t indx = 0; indx < result_lst.size(); indx++)
		{
			for (int i = 0; i < wordSize; i++)
				export_gate_bootstrapping_ciphertext_toFile(answer_data, &result_lst[indx][i], params);
		}
	}
	else
	{
		for (int i = 0; i < wordSize; i++)
			export_gate_bootstrapping_ciphertext_toFile(answer_data, &result[i], params);
	}
	list_of_results = false;
	fclose(answer_data);

	// export ciphertext filename to ctxtmem.txt
	FILE *ctxt_mem = fopen("ctxtMem.txt", "a");
	fprintf(ctxt_mem, "%s", fileName);
	fprintf(ctxt_mem, "\n");
	fclose(ctxt_mem);

	free(fileName);
	delete_gate_bootstrapping_ciphertext_array(wordSize, result);
	delete_gate_bootstrapping_ciphertext_array(wordSize, ciphertext3);
	delete_gate_bootstrapping_ciphertext_array(wordSize, ciphertext2);
	delete_gate_bootstrapping_ciphertext_array(wordSize, ciphertext1);
}

int listen_for_inst(int sockfd, TFheGateBootstrappingCloudKeySet *bk, const TFheGateBootstrappingParameterSet *params)
{
	tmp_socket = sockfd;
	char buf[MAX];
	for (;;)
	{
		memset(buf, 0, MAX);
		read(sockfd, buf, sizeof(buf));
		cout << "Received: " << buf << endl;
		if (strncmp("exit", buf, 4) == 0)
		{
			cout << "Server Exit..." << endl;
			memset(buf, 0, MAX);
			buf[0] = 'O';
			buf[1] = 'K';
			write(sockfd, buf, sizeof(buf));
			return 0;
		}
		else if (buf[0] == 0)
		{
			cout << "Erroneous message, returning..." << endl;
			return -1;
		}
		op_select(buf, bk, params);
		memset(buf, 0, MAX);
		buf[0] = 'O';
		buf[1] = 'K';
		cout << "Sending OK..." << endl;
		if (write(sockfd, buf, sizeof(buf)) < 0)
		{
			return -1;
		}
	}
}

int main(int argc, char **argv)
{

	// reads the evaluation key from file
	FILE *cloud_key = fopen("clouds.key", "rb");
	TFheGateBootstrappingCloudKeySet *bk = new_tfheGateBootstrappingCloudKeySet_fromFile(cloud_key);
	fclose(cloud_key);

	// if necessary, the params are inside the key
	const TFheGateBootstrappingParameterSet *params = bk->params;

	int sockfd, connfd;
	socklen_t len;
	int error_check = -1;
	struct sockaddr_in servaddr, cli;
	while (error_check < 0)
	{
		// socket create and verification
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1)
		{
			printf("socket creation failed...\n");
			exit(0);
		}
		else
			printf("Socket successfully created..\n");
		bzero(&servaddr, sizeof(servaddr));

		// assign IP, PORT
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port = htons(PORT);

		// Binding newly created socket to given IP and verification
		if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
		{
			printf("socket bind failed...\n");
			exit(0);
		}
		else
			printf("Socket successfully binded..\n");

		// Now server is ready to listen and verification
		if ((listen(sockfd, 5)) != 0)
		{
			printf("Listen failed...\n");
			exit(0);
		}
		else
			printf("Server listening..\n");
		len = sizeof(cli);

		// Accept the data packet from client and verification
		connfd = accept(sockfd, (SA *)&cli, &len);
		if (connfd < 0)
		{
			printf("Server accept failed...\n");
			exit(0);
		}
		else
			printf("Server accepted the client...\n");

		// Function for chatting between client and server
		error_check = listen_for_inst(connfd, bk, params);

		// After chatting close the socket
		close(sockfd);
	}
	// clean up all pointers
	delete_gate_bootstrapping_cloud_keyset(bk);
}
