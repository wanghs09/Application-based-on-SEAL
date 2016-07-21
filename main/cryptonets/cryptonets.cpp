#include "cryptonets.h"
#include "SPP.h"

void Cryptonets()
{
    std::clock_t start;
    double duration;

    print_example_banner("Cryptonets");

    // Create encryption parameters
    EncryptionParameters parms;

    parms.poly_modulus() = "1x^8192 + 1";
    parms.coeff_modulus() = "7ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe00000001";
    //parms.coeff_modulus() = ChooserEvaluator::default_parameter_options().at(4096);
    parms.plain_modulus() = 1099511922689;//t2=1099512004609

    parms.decomposition_bit_count() = 32;
    parms.noise_standard_deviation() = ChooserEvaluator::default_noise_standard_deviation();
    parms.noise_max_deviation() = ChooserEvaluator::default_noise_max_deviation();

    cout << "Encryption parameters specify " << parms.poly_modulus().significant_coeff_count() << " coefficients with "
        << parms.coeff_modulus().significant_bit_count() << " bits per coefficient" << endl;

    // Create the PolyCRTBuilder
    PolyCRTBuilder crtbuilder(parms.plain_modulus(), parms.poly_modulus());
    size_t slot_count = crtbuilder.get_slot_count();

    cout<<"Number of Slots:" <<slot_count<<endl;
    
    // Create a vector of values that are to be stored in the slots. We initialize all values to 0 at this point.
    vector<BigUInt> values(slot_count, BigUInt(parms.plain_modulus().bit_count(), static_cast<uint64_t>(0)));

    // Set the first few entries of the values vector to be non-zero
    values[0] = 2;
    values[1] = 3;
    values[2] = 5;
    values[3] = 7;
    values[4] = 11;
    values[5] = 13;

    // Now compose these into one polynomial using PolyCRTBuilder
    cout << "Plaintext slot contents (slot, value): ";
    for (size_t i = 0; i < 6; ++i)
    {
        string to_write = "(" + to_string(i) + ", " + values[i].to_dec_string() + ")";
        to_write += (i != 5) ? ", " : "\n";
        cout << to_write;
    }
    BigPoly plain_composed_poly = crtbuilder.compose(values);

    // Let's do some homomorphic operations now. First we need all the encryption tools.
    // Generate keys.
    cout << "Generating keys..." << endl;
    KeyGenerator generator(parms);
    generator.generate();
    cout << "... key generation complete" << endl;
    BigPoly public_key = generator.public_key();
    BigPoly secret_key = generator.secret_key();
    EvaluationKeys evaluation_keys = generator.evaluation_keys();

    // Create the encryption tools
    Encryptor encryptor(parms, public_key);
    Evaluator evaluator(parms, evaluation_keys);
    Decryptor decryptor(parms, secret_key);

    // Encrypt plain_composed_poly
    cout << "Encrypting ... ";
    BigPoly encrypted_composed_poly = encryptor.encrypt(plain_composed_poly);
    cout << "done." << endl;

    // Let's square the encrypted_composed_poly
    start = std::clock();

    cout << "Squaring the encrypted polynomial ... ";
    BigPoly encrypted_square = evaluator.exponentiate(encrypted_composed_poly, 2);
    cout << "done." << endl;

    //mul time
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    cout<<"multiplication(square):"<< duration <<'\n';

    cout << "Decrypting the squared polynomial ... ";
    BigPoly plain_square = decryptor.decrypt(encrypted_square);
    cout << "done." << endl;
    
    // Print the squared slots
    cout << "Squared slot contents (slot, value): ";
    for (size_t i = 0; i < 6; ++i)
    {
        string to_write = "(" + to_string(i) + ", " + crtbuilder.get_slot(plain_square, i).to_dec_string() + ")";
        to_write += (i != 5) ? ", " : "\n";
        cout << to_write;
    }

    // Now let's try to multiply the squares with the plaintext coefficients (3, 1, 4, 1, 5, 9, 0, 0, ..., 0).
    // First create the coefficient vector
    vector<BigUInt> plain_coeff_vector(slot_count, BigUInt(parms.plain_modulus().bit_count(), static_cast<uint64_t>(0)));
    plain_coeff_vector[0] = 3;
    plain_coeff_vector[1] = 1;
    plain_coeff_vector[2] = 4;
    plain_coeff_vector[3] = 1;
    plain_coeff_vector[4] = 5;
    plain_coeff_vector[5] = 9;

    // Use PolyCRTBuilder to compose plain_coeff_vector into a polynomial
    BigPoly plain_coeff_poly = crtbuilder.compose(plain_coeff_vector);

    // Print the coefficient vector
    cout << "Coefficient slot contents (slot, value): ";
    for (size_t i = 0; i < 6; ++i)
    {
        string to_write = "(" + to_string(i) + ", " + crtbuilder.get_slot(plain_coeff_poly, i).to_dec_string() + ")";
        to_write += (i != 5) ? ", " : "\n";
        cout << to_write;
    }

    // Now use multiply_plain to multiply each encrypted slot with the corresponding coefficient
    start = std::clock();

    cout << "Multiplying squared slots with the coefficients ... ";
    BigPoly encrypted_scaled_square = evaluator.multiply_plain(encrypted_square, plain_coeff_poly);
    cout << " done." << endl;
    
    //mul time
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    cout<<"multiplication with plain time:"<< duration <<'\n';

    cout << "Decrypting the scaled squared polynomial ... ";
    BigPoly plain_scaled_square = decryptor.decrypt(encrypted_scaled_square);
    cout << "done." << endl;

    // Print the scaled squared slots
    cout << "Scaled squared slot contents (slot, value): ";
    for (size_t i = 0; i < 6; ++i)
    {
        string to_write = "(" + to_string(i) + ", " + crtbuilder.get_slot(plain_scaled_square, i).to_dec_string() + ")";
        to_write += (i != 5) ? ", " : "\n";
        cout << to_write;
    }

    // How much noise did we end up with?
    cout << "Noise in the result: " << inherent_noise(encrypted_scaled_square, parms, secret_key).significant_bit_count()
        << "/" << inherent_noise_max(parms).significant_bit_count() << " bits" << endl;
}