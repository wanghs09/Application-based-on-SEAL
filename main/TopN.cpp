#include "TopN.h"
#include "SPP.h"


//TDSC implementation
//TopN Protocol
void TopN()
{
    std::clock_t start;
    print_example_banner("Implementation:TopN Protocol");

    //parameter selection
    start= clock();

    ChooserEncoder chooser_encoder;
    ChooserEvaluator chooser_evaluator;
    
    //input: Max poly size, max every coefficient size
    //7=x^2+x+1
    ChooserPoly qtb(1, 1);  
    ChooserPoly ntb(4, 1);  
    ChooserPoly qfb(1, 1);  
    ChooserPoly nfb(4, 1); 
    ChooserPoly wuf(4, 1); 
    ChooserPoly mx(1, 1); 
    ChooserPoly my(1, 1); 

    // Compute the first term
    //Strangers

    //Friends
    ChooserPoly nfbw = chooser_evaluator.multiply(nfb, wuf); 

    ChooserPoly nT = chooser_evaluator.multiply_plain(nfbw, chooser_encoder.encode(Tu)); //Fu*
    ChooserPoly dT = chooser_evaluator.multiply_plain(qtb, chooser_encoder.encode(Tu)); //Fu*
    ChooserPoly nF = chooser_evaluator.multiply_plain(nfbw, chooser_encoder.encode(Fu)); //Fu*
    ChooserPoly dF = chooser_evaluator.multiply_plain(qtb, chooser_encoder.encode(Fu)); //Fu*

    //X Y
    ChooserPoly X = chooser_evaluator.add(chooser_evaluator.multiply_plain(chooser_evaluator.multiply(nT, dF), chooser_encoder.encode(beta)), chooser_evaluator.multiply_plain(chooser_evaluator.multiply(nF, dT), chooser_encoder.encode(alpha)));
    ChooserPoly Y = chooser_evaluator.multiply_plain(chooser_evaluator.multiply(nT, dF), chooser_encoder.encode(beta+alpha));

    ChooserPoly MxX = chooser_evaluator.multiply(X, mx); //Fu*
    ChooserPoly MyY = chooser_evaluator.multiply(Y, my); //Fu*
    cout<<"here1"<<endl;
    // To find an optimized set of parameters, we use ChooserEvaluator::select_parameters(...).
    EncryptionParameters optimal_parms;
    chooser_evaluator.select_parameters(MyY, optimal_parms);

    cout<<"Time for parameter selection:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    // Let's print these to see what was recommended
    cout << "Selected parameters:" << endl;
    cout << "{ poly_modulus: " << optimal_parms.poly_modulus().to_string() << endl;
    cout << "{ coeff_modulus: " << optimal_parms.coeff_modulus().to_string() << endl;
    cout << "{ plain_modulus: " << optimal_parms.plain_modulus().to_dec_string() << endl;
    cout << "{ decomposition_bit_count: " << optimal_parms.decomposition_bit_count() << endl;
    cout << "{ noise_standard_deviation: " << optimal_parms.noise_standard_deviation() << endl;
    cout << "{ noise_max_deviation: " << optimal_parms.noise_max_deviation() << endl;

    // Let's try to actually perform the homomorphic computation using the recommended parameters.
    // Generate keys.
    cout << "Generating keys..." << endl;
    KeyGenerator generator(optimal_parms);
    generator.generate();
    cout << "... key generation complete" << endl;
    BigPoly public_key = generator.public_key();
    BigPoly secret_key = generator.secret_key();
    EvaluationKeys evaluation_keys = generator.evaluation_keys();
    
    cout<<"Time for Key Gen:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    // Create the encoding/encryption tools
    BalancedEncoder encoder(optimal_parms.plain_modulus());
    Encryptor encryptor(optimal_parms, public_key);
    Evaluator evaluator(optimal_parms, evaluation_keys);
    Decryptor decryptor(optimal_parms, secret_key);

    
    // Now perform the computations on real encrypted data.
    //load users, strangers, friends, matrix
    int MTm[Tm][n], MTu[Tu][n], MFu[Fu][n];
    LoadM(MTm, MTu, MFu);

    int i,j;
    //int Su=0; //select a specific user i=1
    //stage1, strangers
    BigPoly RtIb, tmp, tmps, RtIb2; //Rt are plaintexts
    //int Rtavg[Tu]; //plain qtb
    //BigPoly Ent[Tu][n], Eqtb[Tu][n];

    BigPoly *Ent=new BigPoly[Tu*n];
    BigPoly *Eqtb=new BigPoly[Tu*n];

    // encryptor.encrypt( encoder.encode(1), Ent[0] );
    //cout<<"size:"<<sizeof(Ent[0])<<endl;
    //cout<<Ent[0].to_string()<<endl;

    //user
    start=clock();
    encryptor.encrypt( encoder.encode(1), Ent[0] );
    cout<<"Time for enc:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    start=clock();
    encryptor.encrypt( encoder.encode(20), Ent[1] );
    cout<<"Time for enc:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    start=clock();
    evaluator.multiply(Ent[0],  Ent[1], Ent[2] );
    evaluator.multiply(Ent[0],  Ent[2], Ent[4] );
    cout<<"Time for multply:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    cout << "res: " << encoder.decode_int64(decryptor.decrypt(Ent[2])) << endl;
    start=clock();

    start=clock();
    evaluator.multiply_plain(Ent[0],  encoder.encode(3), Ent[2] );
    cout<<"Time for multply_plain:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    cout << "res: " << encoder.decode_int64(decryptor.decrypt(Ent[2])) << endl;
    start=clock();
    
    start=clock();
    evaluator.add(Ent[0],  Ent[1], Ent[2] );
    cout<<"Time for add:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    start=clock();
    evaluator.add_plain(Ent[0],  encoder.encode(3), Ent[2] );
    cout<<"Time for add_plain:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();
/**
    //user encryption
    //select the first item
    for(i=0; i<Tu; i++)
        for(j=0; j<n; j++)
        {
            encryptor.encrypt( encoder.encode(1), Ent[n*i+j] );
            encryptor.encrypt( encoder.encode(3), Eqtb[n*i+j] );
        }
        cout<<"test here";
    cout<<"Time for stage1 strangers:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    //stage2, friends
    //user encryption
    //similarity
    int w[Fu];
    //modify later
    for(j=0; j<Fu; j++) 
        w[j]=1;

    BigPoly Ew[Fu];
    for(i=0; i<Fu; i++) 
        encryptor.encrypt( encoder.encode(w[i] ), Ew[i] );

    cout<<"Time for stage2 user:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    //friends encryption
    //BigPoly RfIb; //Rt are plaintexts
    //int Rfavg[Fu];
    //BigPoly Enf[Fu][n], Eqfb[Fu][n];
    BigPoly *Enf=new BigPoly[Fu*n];
    BigPoly *Eqfb=new BigPoly[Fu*n];

    //BigPoly *pEnf = new BigPoly[210000];
    //int ax = 0;
    //delete[] pEnf;


    for(i=0; i<Fu; i++)
        for(j=0; j<n; j++)
        {
            encryptor.encrypt( encoder.encode(1), Eqfb[n*i+j] );
            evaluator.multiply_plain(Ew[i],  encoder.encode(3), Enf[n*i+j] );
        }

    cout<<"Time for stage2 friends:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();


    
    //Stage 3
    //modify later, user average
    //int rua=2;

    //user encryption
    //BigPoly EMx[n][n], EMy[n][n];
    BigPoly *EMx=new BigPoly[n*n];
    BigPoly *EMy=new BigPoly[n*n];

    for(i=0; i<n; i++)
        for(j=0; j<n; j++)
        {
            encryptor.encrypt( encoder.encode(1), EMx[n*i+j] );
            encryptor.encrypt( encoder.encode(1), EMy[n*i+j] );
        }
    cout<<"Time for stage3 user:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    //RS server
    //sum all nt
    BigPoly EnT[n], EdT[n], EnF[n], EdF[n], EX[n], EY[n];

    for(j=0; j<n; j++)
    {
        encryptor.encrypt( encoder.encode(0), EnT[j] );
        encryptor.encrypt( encoder.encode(0), EdT[j] );

        //BigPoly EnT=Ent[0][j], EdT=Eqtb[0][j];

        for(i=0; i<Tu; i++)
        {
            BigPoly tmp1=evaluator.add(EnT[j], Ent[n*i+j] );
            EnT[j]=tmp1;

            BigPoly tmp2=evaluator.add(EdT[j], Eqtb[n*i+j] );        
            EdT[j]=tmp2;
        }
    
        encryptor.encrypt( encoder.encode(0), EnF[j] );
        encryptor.encrypt( encoder.encode(0), EdF[j] );
        for(i=0; i<Fu; i++)
        {
            BigPoly tmp1=evaluator.add(EnF[j], Enf[n*i+j] );
            EnF[j]=tmp1;

            BigPoly tmp2=evaluator.add(EdF[j], Eqfb[n*i+j] );        
            EdF[j]=tmp2;      
        }

        evaluator.add(evaluator.multiply(EnT[j], evaluator.multiply_plain(EdF[j], encoder.encode(beta) ) ) , evaluator.multiply(EnF[j], evaluator.multiply_plain(EdT[j], encoder.encode(alpha) ) ) , EX[j]);
        evaluator.multiply(EdT[j], evaluator.multiply_plain(EdF[j], encoder.encode(alpha+beta) ) , EY[j]); 
    }

    BigPoly EMxX[n], EMyY[n];

    for(i=0; i<n; i++)
    for(j=0; j<n; j++)
    {
        BigPoly tmp1=evaluator.multiply(EMx[n*i+j], EX[i] );
        EMx[n*i+j]=tmp1;

        BigPoly tmp2=evaluator.multiply(EMy[n*i+j], EY[i] );
        EMy[n*i+j]=tmp2;
    }

    for(i=0; i<n; i++)
    {
        encryptor.encrypt( encoder.encode(0), EMxX[0] );
        encryptor.encrypt( encoder.encode(0), EMyY[0] );
        for(j=0; j<n; j++)
        {
            BigPoly tmp1=evaluator.add(EMx[n*i+j], EMxX[j] );
            EMxX[j]=tmp1;

            BigPoly tmp2=evaluator.add(EMy[n*i+j], EMyY[j] );       
            EMyY[j]=tmp2;
        }
    }
    
    delete[] Enf;
    delete[] Eqfb;
    delete[] EMx;
    delete[] EMy;
    delete[] Ent;
    delete[] Eqtb;


    cout<<"Time for stage3 server:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    // Finally print the result and noise
    
    cout << "EX: " << encoder.decode_int64(decryptor.decrypt(EX)) << endl;

    cout << "Noise : " << inherent_noise(EMxX[0][0], optimal_parms, secret_key).significant_bit_count()
        << "/" << inherent_noise_max(optimal_parms).significant_bit_count() << " bits" << endl;

    cout << "EY: " << encoder.decode_int64(decryptor.decrypt(EY)) << endl;

    cout << "Noise : " << inherent_noise(EMyY[0][0], optimal_parms, secret_key).significant_bit_count()
        << "/" << inherent_noise_max(optimal_parms).significant_bit_count() << " bits" << endl;

    **/
};
