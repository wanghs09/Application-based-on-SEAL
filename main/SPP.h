#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <cstdlib>

#include "seal.h"

using namespace std;
using namespace seal;


#define Tm 1 
#define Tu 10 
#define Fu 70 
#define n 1682 
#define T 5 //rating size 0-5
#define alpha 8 
#define beta 2 

class HEsystem{

public:
	HEsystem(EncryptionParameters optimal_parms1){
		optimal_parms=optimal_parms1;

		// Generate keys
		cout << "Generating keys..." << endl;
	    KeyGenerator generator(optimal_parms1);
	    generator.generate();
	    cout << "... key generation complete" << endl;
	    public_key = generator.public_key();
	    secret_key = generator.secret_key();
	    evaluation_keys = generator.evaluation_keys();
	}

	EncryptionParameters optimal_parms;
	BigPoly public_key;
	BigPoly secret_key;
	EvaluationKeys evaluation_keys;
};


void print_example_banner(string title);

EncryptionParameters SPP_Parameter_Gen();
void Cryptonets();
void SPP(int user_num, int item_num, EncryptionParameters params,  HEsystem hesystem);
void LoadM(int MTu[Tu][n], int MFu[Fu][n], int sim[Fu], int user_num);
void Ttest(Encryptor encryptor,Decryptor decryptor,Evaluator evaluator, BalancedEncoder encoder, clock_t start);




