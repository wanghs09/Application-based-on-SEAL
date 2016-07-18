//patial homomorphic encryption
//ElGamal, Paillier,Goldwasser-Micali
//library: FHE-simon from Lepoint

//  ./tests/TestAddMul_x 80 23 7
#define N_TESTS 1
//#define DEBUG 1

#include "Plaintext.h"
#include "DoubleCRT.h"
#include "Ciphertext.h"
#include "NTL/ZZ_pX.h"
#include <NTL/vector.h>

#define TIMES 100
//GM operations
#include "NTL/ZZ.h"
#include "NTL/ZZ_p.h"

#include <time.h>
#include "FHE-SI.h"

//time cost
#include <ctime>
std::clock_t start;
double duration;

//ns time test
#include <chrono>
typedef std::chrono::high_resolution_clock Clock;

/**************************
//ElGamal HE system
//len: length of params p,q
**************************/
void ElGamal(int len=512){
  ZZ n, n2, p, q, g, x, lamda;
  ZZ p1, q1, p1q1;
  ZZ bA;
  
  ZZ m1, m2, k1, k2;
  ZZ BSm, HEm; //baseline and HE result
  ZZ c, r1, r2, t1, t2, cm1, cm2;
  ZZ r, t;

  //key gen
  start = std::clock();

  GenPrime(q, len);
  RandomBnd(g, q);
  RandomBnd(x, q);
  PowerMod(bA,g,x, q);

  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  cout<<"ElGamal Setup:"<< duration <<'\n';

  //Enc
  RandomBnd(m1, q);
  RandomBnd(m2, q);

  start = std::clock();

  RandomBnd(k1, q); //B
  PowerMod(r1,g,k1, q);
  PowerMod(t1,bA,k1, q);
  MulMod(t1, t1, m1, q);

  RandomBnd(k2, q); //B
  PowerMod(r2,g,k2, q);
  PowerMod(t2,bA,k2, q);
  MulMod(t2, t2, m2, q);

  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  cout<<"ElGamal Enc:"<< duration/2 <<'\n';

  //Evaluation
  start = std::clock();

  MulMod(r,r1,r2,q);
  MulMod(t,t1,t2,q);

  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  cout<<"ElGamal Eval:"<< duration/2 <<'\n';

  //Dec  
  start = std::clock();

  PowerMod(HEm,r,x,q);
  InvMod(HEm, HEm, q);
  MulMod(HEm,HEm,t,q);
  
  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  cout<<"ElGamal Dec:"<< duration/2 <<'\n';

  //baseline
  MulMod(BSm,m1,m2,q);

  assert(BSm==HEm);
}

/**************************
//Paillier HE system
//len: length of params p,q
**************************/
void Paillier(int len=512){
  ZZ n, n2, p, q, g, lamda;
  ZZ p1, q1, p1q1;
  ZZ miu;
  
  ZZ m1, m2;
  ZZ BSm, HEm; //baseline and HE result
  ZZ c, c1, c2, cm1, cm2, r;

  //key gen
  start = std::clock();

  GenPrime(p, len);
  GenPrime(q, len);
  mul(n, p, q);
  mul(n2, n, n);

  sub(p1,p,1);
  sub(q1,q,1);
  GCD(lamda,p1,q1);
  mul(p1q1,p1,q1);
  div(lamda, p1q1, lamda);

  RandomBnd(g, n2);

  PowerMod(miu,g,lamda,n2);
  sub(miu, miu, 1);
  div(miu,miu,n); //should add 1?
  InvMod(miu, miu, n);
  
  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  cout<<"Pailler Setup:"<< duration <<'\n';

  //Enc
  start = std::clock();
  RandomBnd(m1,n);
  RandomBnd(m2,n);

  RandomBnd(r,n); //enc m1
  PowerMod(c1, g,m1,n2);
  PowerMod(c2, r,n,n2);
  MulMod(cm1, c1,c2, n2);

  RandomBnd(r,n); //enc m2
  PowerMod(c1, g,m2,n2);
  PowerMod(c2, r,n,n2);
  MulMod(cm2, c1,c2, n2);

  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  cout<<"Pailler Enc:"<< duration/2 <<'\n';

  //Evaluation
  start = std::clock();
  c=cm1;
  for(int i=0; i<TIMES; i++)
  	MulMod(c,c,cm2,n2);
  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  cout<<"Pailler Eval:"<< duration <<'\n';

  //c=cm2;
  //Dec  
  start = std::clock();
  PowerMod(c,c,lamda,n2);
  sub(c,c,1);
  div(c,c,n); //should add 1?
  MulMod(HEm, c, miu, n);  

  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  cout<<"Pailler Dec:"<< duration <<'\n';

  //baseline
  BSm=m1;
  for(int i=0; i<TIMES; i++)
  	AddMod(BSm,BSm,m2,n);

  assert(BSm==HEm);
}

/**************************
//Goldwasser-Micali HE system
//len: length of params p,q
**************************/
void GM(int len=512){
  ZZ N, p, q, x;

  long m1, m2;
  ZZ BSm, HEm; //baseline and HE result
  ZZ c, c1, c2, cm1, cm2, r;

  //key gen
  start = std::clock();

  GenPrime(p, len);
  GenPrime(q, len);
  mul(N, p, q);
  
  do{
  RandomBnd(x, N);
  } while( (Jacobi(x,p)==1) || (Jacobi(x,q)==1));

  cout<<"Jac:"<<Jacobi(x,p)<<Jacobi(x,q)<<endl;
  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  cout<<"GM Setup:"<< duration <<'\n';

  //Enc
  RandomBnd(m1,2);
  RandomBnd(m2,2);

  start = std::clock();
  RandomBnd(r, N);
  MulMod(cm1,r,r,N);

  if(m1==1)
   MulMod(cm1,cm1,x,N);
  
  RandomBnd(r, N);
  MulMod(cm2,r,r,N);

  if(m2==1)
   MulMod(cm2,cm2,x,N);

  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  cout<<"GM Enc:"<< duration <<'\n';

  //Evaluation
  start = std::clock();
  MulMod(c,cm1,cm2,N);
  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  cout<<"GM Eval:"<< duration <<'\n';

  //Dec  
  start = std::clock();
  HEm=Jacobi(c,p);
  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  cout<<"GM Dec:"<< duration <<'\n';

  if(HEm==1)
    HEm=0;
  else
    HEm=1;

  //baseline
  BSm=m1^m2;  //xor

  //cout<<"plaintext:"<<m1<<m2<<BSm<<endl;
  assert(BSm==HEm);
}

void printpoly(ZZ_pX poly, long phim ){
cout<<"--------print poly:-------"<<endl;
//for (long i=0; i < phim; i++)
for (long i=0; i < 10; i++)
	cout<<poly.rep[i]<<endl;
}

//Comparison protocol based on ""
bool COM(bool disp, long long seed, unsigned p, FHEcontext &context) {
  ZZ seedZZ;
  seedZZ = seed;

  srand48(seed);
  SetSeed(seedZZ);

  FHESISecKey secretKey(context);
  const FHESIPubKey &publicKey(secretKey);
  
  long phim = context.zMstar.phiM();
  
  ZZ_pX ptxt1Poly, ptxt2Poly, sum, sumMult, prod, prod2, sumQuad;
  Plaintext resSum, resSumMult, resProd, resProdSwitch, resProd2, resSumQuad;

  //gen plaintext
  ptxt1Poly.rep.SetLength(phim);
  ptxt2Poly.rep.SetLength(phim);
  for (long i=0; i < phim; i++) {
    ptxt1Poly.rep[i] = RandomBnd(p);
    ptxt2Poly.rep[i] = RandomBnd(p);
  }

  //printpoly(ptxt1Poly, phim);

  ptxt1Poly.normalize();
  ptxt2Poly.normalize();
  
  #ifdef DEBUG
  cout<<"phim:"<<phim<<endl;
  cout<<"p1:"<<endl;
  printpoly(ptxt1Poly, phim);
  cout<<"p2:"<<endl;
  printpoly(ptxt2Poly, phim);
  #endif

  //plaintext operation
  sum = ptxt1Poly + ptxt2Poly;
  sumMult = ptxt2Poly * 7;
  prod = ptxt1Poly * ptxt2Poly;
  prod2 = prod * prod;
  sumQuad = prod2 * prod2 * 9; //\sum_((xy)^4)

  #ifdef DEBUG  
  cout<<"sum:"<<endl;
  printpoly(sum, phim);
  cout<<"prod2:"<<endl;
  printpoly(prod2, phim);
  #endif

  rem(prod, prod, to_ZZ_pX(context.zMstar.PhimX()));
  rem(prod2, prod2, to_ZZ_pX(context.zMstar.PhimX()));
  rem(sumQuad, sumQuad, to_ZZ_pX(context.zMstar.PhimX()));
  
  //encryption
  start = std::clock();
  Plaintext ptxt1(context, ptxt1Poly), ptxt2(context, ptxt2Poly);
  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  duration = duration/2;
  cout<<"Encryption:"<< duration <<'\n';

  Ciphertext ctxt1(publicKey), ctxt2(publicKey);
  publicKey.Encrypt(ctxt1, ptxt1);
  publicKey.Encrypt(ctxt2, ptxt2);

  Ciphertext cSum = ctxt1;
  cSum += ctxt2;

  Ciphertext cSumMult = ctxt2;
  start = std::clock();
  for (int i = 1; i < 7; i++) {
    cSumMult += ctxt2;
  }
  duration = ( std::clock() - start ) / (double) (CLOCKS_PER_SEC);
  duration = duration/6;
  cout<<"Addition:"<< duration <<'\n';

  Ciphertext cProd = ctxt1;
  cProd *= ctxt2;
 
  secretKey.Decrypt(resSum, cSum);
  secretKey.Decrypt(resSumMult, cSumMult);
  
  KeySwitchSI keySwitch(secretKey);
  keySwitch.ApplyKeySwitch(cProd);
  secretKey.Decrypt(resProd, cProd);

  cProd *= cProd;
  Ciphertext tmp = cProd;
  Ciphertext cSumQuad = cProd;
  
  keySwitch.ApplyKeySwitch(cProd);
  secretKey.Decrypt(resProd2, cProd);

  for (int i = 0; i < 8; i++) {
    cSumQuad += tmp;
  }
  keySwitch.ApplyKeySwitch(cSumQuad);  //apply key switch after summing all prod

  start = std::clock();
  cSumQuad *= cProd;
  duration = ( std::clock() - start ) / (double) (CLOCKS_PER_SEC);
  cout<<"HMult without key switch:"<< duration <<'\n';

  keySwitch.ApplyKeySwitch(cSumQuad);

  duration = ( std::clock() - start ) / (double) (CLOCKS_PER_SEC);
  cout<<"HMult with key switch:"<< duration <<'\n';

  start = std::clock();
  secretKey.Decrypt(resSumQuad, cSumQuad);
  duration = ( std::clock() - start ) / (double) (CLOCKS_PER_SEC);
  cout<<"Decryption:"<< duration <<'\n';
  
  //comparison
  bool success = ((resSum.message == sum) && (resSumMult.message == sumMult) &&
                  (resProd.message == prod) && (resProd2.message == prod2) &&
                  (resSumQuad == sumQuad));
  
  if (disp || !success) {
    cout << "Seed: " << seed << endl << endl;
    
    if (resSum.message != sum) {
      cout << "Add failed." << endl;
    }
    if (resSumMult.message != sumMult) {
      cout << "Adding multiple times failed." << endl;
    }
    if (resProd.message != prod) {
      cout << "Multiply failed." << endl;
    }
    if (resProd2.message != prod2) {
      cout << "Squaring failed." << endl;
    }
    if (resSumQuad.message != sumQuad) {
      cout << "Sum and quad failed." << endl;
    }
  }
  
  if (disp || !success) {
    cout << "Test " << (success ? "SUCCEEDED" : "FAILED") << endl;
  }

  return success;
}

int main(int argc, char *argv[]) {

  ElGamal();
  GM();
  Paillier();
  //COM();
  
  /*****************Test HE*********************
  unsigned p, g, logQ;

  if (argc >= 4) {
    logQ = atoi(argv[1]);
    p = atoi(argv[2]);
    g = atoi(argv[3]);
  } else {
    cout << "usage: Test_AddMul_x logQ p generator [seed]" << endl;
    return 1;
  }
  
  cout << "==================================================" << endl
       << "Running add/multiply tests using Brakerski system." << endl
       << "==================================================" << endl;

  FHEcontext context(p-1, logQ, p, g, 3);
  activeContext = &context;
  
  //cout<<"CLOCKS_PER_SEC"<<CLOCKS_PER_SEC<<endl;

  start = std::clock();
  context.SetUpSIContext();
  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  cout<<"Setting context:"<< duration <<'\n';
  //cout << "Finished setting up context." << endl;

  if (argc >= 5) {
    long long seed = atoi(argv[4]);
    bool res = runTest(true, seed, p, context);
    
    if (!res) {
      cout << "Failed test with seed " << seed << endl;
      return 1;
    }
    return 0;
  }

  int testsFailed = 0;

  srand48(time(NULL));
  long long start = lrand48();
  for (int iter = 0; iter < N_TESTS; iter++) {
    if (!runTest(false, start+iter, p, context)) {
      testsFailed++;
    }

    if (iter % 100 == 0) {
      cout << "." << flush;
    }
  }
  cout << endl;

  if (testsFailed == 0) {
    cout << "All tests SUCCEEDED!" << endl;
  } else{
    cout << testsFailed << " of " << N_TESTS << " failed." << endl;
  }

  return testsFailed;
  *******************************************/
}
