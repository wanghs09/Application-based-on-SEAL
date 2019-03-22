# Application-based-on-SEAL
Privacy preserving Recommender System Training based on Microsoft float point Homomorphic Encryption library SEAL

main.cpp 
//includes the main function.

SPP.h
SPP.cpp
//This is the implementation of Single Prediction Protocol
//Includes: Parameter selection based on the dataset size
//          Time cost for different parties at different stages
//For details please see the paper "Privacy-Preserving Context-Aware Recommender Systems: Analysis and New Solutions".

cryptonets.h
cryptonets.cpp
//The implementation based on the parameters of cryptonets, for the performance test
//"CryptoNets: Applying Neural Networks to Encrypted Data with High Throughput and Accuracy"

Example:
Test data extracted from ML100K.
Friends num: 70
Stranger num: 10
Tested user: 1
Item num: 1682

***************************************************************
********** Implementation:Single Prediciton Protocol **********
***************************************************************

Time for parameter selection:0.054222
Selected parameters:
{ poly_modulus: 1x^4096 + 1
{ coeff_modulus: 3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0000001
{ plain_modulus: 512
{ decomposition_bit_count: 64
{ noise_standard_deviation: 3.19
{ noise_max_deviation: 15.95
Generating keys...
... key generation complete
Time for Key Gen:9.84324
Time for stage1 user:70.22
Time for stage1 Strangers:5.78338
Time for stage2 user:2.92821
Time for stage2 friends:44.3962
Time for stage3 user:0.04172
Time for stage3 server:1.00154


********************************
********** Cryptonets **********
********************************

Encryption parameters specify 8193 coefficients with 383 bits per coefficient
Number of Slots:8192
Plaintext slot contents (slot, value): (0, 2), (1, 3), (2, 5), (3, 7), (4, 11), (5, 13)
Generating keys...
... key generation complete
Encrypting ... done.
Squaring the encrypted polynomial ... done.
multiplication(square):3.16375
Decrypting the squared polynomial ... done.
Squared slot contents (slot, value): (0, 4), (1, 9), (2, 25), (3, 49), (4, 121), (5, 169)
Coefficient slot contents (slot, value): (0, 3), (1, 1), (2, 4), (3, 1), (4, 5), (5, 9)
Multiplying squared slots with the coefficients ...  done.
multiplication with plain time:0.230487
Decrypting the scaled squared polynomial ... done.
Scaled squared slot contents (slot, value): (0, 12), (1, 9), (2, 100), (3, 49), (4, 605), (5, 1521)
Noise in the result: 221/342 bits
