/*
Date: 2016-07-15
Husen.wang@uni.lu 
University of Luxembourg & SnT
*/

#include "SPP.h"

void print_example_banner(string title)
{
    if (!title.empty())
    {
        size_t title_length = title.length();
        size_t banner_length = title_length + 2 + 2 * 10;
        string banner_top(banner_length, '*');
        string banner_middle = string(10, '*') + " " + title + " " + string(10, '*');

        cout << endl
            << banner_top << endl
            << banner_middle << endl
            << banner_top << endl
            << endl;
    }
}

//TDSC implementation
//Single Prediciton Protocol
void SPP()
{

    /**
    { poly_modulus: 1x^4096 + 1
    { coeff_modulus: 3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0000001
    { plain_modulus: 400
    { decomposition_bit_count: 95
    { noise_standard_deviation: 3.19
    { noise_max_deviation: 15.95

    { poly_modulus: 1x^8192 + 1
    { coeff_modulus: 7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE00000001
    { plain_modulus: 2000
    { decomposition_bit_count: 192
    { noise_standard_deviation: 3.19
    { noise_max_deviation: 15.95

    Time for Key Gen:72.0671
    Time for stage1 user:372.461
    Time for stage2 user:15.5459
    Time for stage2 friends:1304.81
    Time for stage3 user:0
    Time for stage3 server:5.01636
    **/

    std::clock_t start;
    print_example_banner("Implementation:Single Prediciton Protocol");

    start= clock();

    ChooserEncoder chooser_encoder;
    ChooserEvaluator chooser_evaluator;
    
    //input: Max poly size, max every coefficient size
    //7=x^2+x+1
    ChooserPoly Ib(1, 1);
    ChooserPoly qtb(1, 1); //q_{t,b}
    ChooserPoly wuf(5, 1); //how to get it?
    ChooserPoly qfb(1, 1); //q_{f,b}
    ChooserPoly ruavg(3, 1); //r_u

    // Compute the first term
    //Strangers
    ChooserPoly test = chooser_evaluator.multiply_plain(Ib, qtb);

    ChooserPoly RIb = chooser_evaluator.multiply_plain(Ib, chooser_encoder.encode(1)); //*n*Rt, only one Ib element is 1
    ChooserPoly RIbs = chooser_evaluator.sub_plain(RIb, ruavg); //n*rt_avg
    ChooserPoly ntb = chooser_evaluator.multiply(RIbs, qtb); 

    ChooserPoly nT(1, 1);
    ChooserPoly dT(1, 1);
    for(int i=0; i<0.5*Tu; i++)
    {
        //cannot use this multply_plain to emulate multiple additions, since the plaintext modulus for multiple poly mult and poly add are different.
        // The decryption will module every poly coefficient with plaintext modulus and cause error when plaintext modulus is small.
        // but if the plaintext modulus is too large, the mult noise increase will be very fast

        //ChooserPoly nT = chooser_evaluator.multiply_plain(ntb, chooser_encoder.encode(Tu)); //Tu*
        //ChooserPoly dT = chooser_evaluator.multiply_plain(qtb, chooser_encoder.encode(Tu)); //Tu*
        nT=chooser_evaluator.add(nT, ntb); //Tu*
        dT=chooser_evaluator.add(dT, qtb); //Tu*
    }
    

    //Friends
    ChooserPoly Rfb = chooser_evaluator.multiply_plain(Ib, chooser_encoder.encode(1)); //*n*Rt
    ChooserPoly Rfbs = chooser_evaluator.sub_plain(Rfb, ruavg); //n*rt_avg
    ChooserPoly nfb = chooser_evaluator.multiply(Rfbs, qfb); 
    ChooserPoly nfbw = chooser_evaluator.multiply(nfb, wuf); 

    ChooserPoly nF(1, 1);
    ChooserPoly dF(1, 1);
    for(int i=0; i<0.5*Fu; i++)
    {
        //ChooserPoly nT = chooser_evaluator.multiply_plain(ntb, chooser_encoder.encode(Tu)); //Tu*
        //ChooserPoly dT = chooser_evaluator.multiply_plain(qtb, chooser_encoder.encode(Tu)); //Tu*
        nF=chooser_evaluator.add(nF, nfb); //Fu*
        dF=chooser_evaluator.add(dF, qfb); //Fu*
    }
    //X Y
    ChooserPoly X1 = chooser_evaluator.multiply(ruavg, dF);
    ChooserPoly X12 = chooser_evaluator.multiply(X1, dT);
    ChooserPoly X123 = chooser_evaluator.multiply_plain(X12, chooser_encoder.encode(alpha+beta));
    ChooserPoly X2 = chooser_evaluator.multiply(nT, dF);
    ChooserPoly X22 = chooser_evaluator.multiply_plain(X2, chooser_encoder.encode(beta));
    ChooserPoly X3 = chooser_evaluator.multiply(nF, dT);
    ChooserPoly X32 = chooser_evaluator.multiply_plain(X3, chooser_encoder.encode(alpha));

    //ChooserPoly Xs = chooser_evaluator.add(X123, X22);
    ChooserPoly X = chooser_evaluator.add(X22, X32);

    ChooserPoly Ys = chooser_evaluator.multiply(dF, dT);
    ChooserPoly Y = chooser_evaluator.multiply_plain(Ys, chooser_encoder.encode(alpha+beta));

    // To find an optimized set of parameters, we use ChooserEvaluator::select_parameters(...).
    EncryptionParameters optimal_parms;
    chooser_evaluator.select_parameters(X, optimal_parms);

    cout<<"Time for parameter selection:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    // Let's print these to see what was recommended
    cout << "Selected parameters:" << endl;
    cout << "{ poly_modulus: " << optimal_parms.poly_modulus().to_string() << endl;
    cout << "{ coeff_modulus: " << optimal_parms.coeff_modulus().to_string() << endl;
    cout << "{ plain_modulus: " << optimal_parms.plain_modulus().to_string() << endl; //to_dec_string
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
    int MTu[Tu][n], MFu[Fu][n];//modify, use new*
    LoadM(MTu, MFu);

    int i,j;
    //int Su=0; //select a specific user i=1
    //stage1, strangers
    BigPoly EIb[n], RtIb, tmp, tmps, RtIb2; //Rt are plaintexts
    int Rtavg[Tu]; //plain qtb
    BigPoly Ent[Tu], Eqtb[Tu];

    //test
    /*
    encryptor.encrypt( encoder.encode(-15), Ent[0] );
    encryptor.encrypt( encoder.encode(-8), Ent[1] );
    evaluator.multiply(Ent[0], Ent[1], Ent[2]);
    cout << "test: " << encoder.decode_int64(decryptor.decrypt(Ent[2])) << endl;
    */

    //user
    start=clock();

    //user encryption
    //select the first item, target id=0
    encryptor.encrypt( encoder.encode(1), EIb[0] );
    for(i=1; i<n; i++)
        encryptor.encrypt( encoder.encode(0), EIb[i] );
    cout<<"Time for stage1 user:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    //modify later
    //strangers encryption
    for(j=0; j<Tu; j++)
    {
        Rtavg[j]=0;
        for(i=0; i<n; i++)
            Rtavg[j]+=MTu[j][i];
        Rtavg[j]=Rtavg[j]/n;
    }
        //cout<<"Time for stage1 Strangers 1:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;   

    for(j=0; j<Tu; j++)
    {
        int k=0;
        while((MTu[j][k]==0)&&(k<n)) //find the first non-zero rating to save encrypt(0) operation
        {               k++;
            }

        if(k==n) //no rating
        {
            cout<<"here"<<endl;
            encryptor.encrypt( encoder.encode(0), Eqtb[j] );
        }
        else
            evaluator.add_plain(EIb[k], encoder.encode(0), Eqtb[j] );

        for(i=k+1; i<n; i++)
            if(MTu[j][i]!=0)
            {
                evaluator.add(Eqtb[j],EIb[i], Eqtb[j]);
                //Eqtb[j]=tmp;
            }
    }
        
        //cout<<"Time for stage1 Strangers 2:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;  
    for(j=0; j<Tu; j++)
    {
    	//cout<<j<<endl;
        //Rt*Ib
        evaluator.multiply_plain(EIb[0], encoder.encode(MTu[j][0]), RtIb ); 
        for(i=1; i<n; i++)
        {      	 
        	//tmps=evaluator.multiply_plain(EIb[i], encoder.encode(MTu[j][i]) );
        	//tmps=evaluator.multiply_plain(EIb[0], encoder.encode(MTu[j][0]) ); 

            //cannot accumulate to one variable, since the input to add is defined const
            //cout<<"Time for stage1 Strangers 2:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;  

            BigPoly tmp=evaluator.add(RtIb,  evaluator.multiply_plain(EIb[i], encoder.encode(MTu[j][i]) )); 
            //cout<<"Time for stage1 Strangers 2:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;  
            
            RtIb=tmp;
        }
        //cout<<"Time for stage1 Strangers 2:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;  
        //RtIb2=evaluator.sub_plain(RtIb, encoder.encode(Rtavg[j]) );
        evaluator.multiply(evaluator.sub_plain(RtIb, encoder.encode(Rtavg[j]) ), Eqtb[j], Ent[j] );

        //cout << "Ent: " << encoder.decode_int64(decryptor.decrypt(Ent[j])) << endl;
    }   
    cout<<"Time for stage1 Strangers:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    //stage2, friends
    //user encryption
    //similarity
    int w[Fu];
    for(j=0; j<Fu; j++) 
        w[j]=8;

    BigPoly Ew[Fu];
    for(i=0; i<Fu; i++) 
        encryptor.encrypt( encoder.encode(w[i] ), Ew[i] );

    cout<<"Time for stage2 user:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    //friends encryption
    BigPoly RfIb, RfIb2, Eqfb2; //Rt are plaintexts
    int Rfavg[Fu];
    BigPoly Enf[Fu], Eqfb[Fu];

    for(j=0; j<Fu; j++)
    {
        Rfavg[j]=0;
        for(i=0; i<n; i++)
            Rfavg[j]+=MFu[j][i];
        Rfavg[j]=Rfavg[j]/n;
        //cout<<"Rfavg[j]:"<<Rfavg[j]<<endl;
    }

    for(j=0; j<Fu; j++)
        {
            //encryptor.encrypt( encoder.encode(0), Eqfb[j] );

            int k=0;
            while((MFu[j][k]==0)&&(k<n))
            {           k++;
            } 
            

            if(k==n) //no rating
                encryptor.encrypt( encoder.encode(0), Eqfb[j] );
            else
                evaluator.add_plain(EIb[k], encoder.encode(0), Eqfb[j] );

            for(i=k+1; i<n; i++)
            {
                if(MFu[j][i]!=0)
                        {
                            BigPoly tmp=evaluator.add(Eqfb[j],EIb[i]);
                            Eqfb[j]=tmp;
                        }
            }
        }

    for(j=0; j<Fu; j++)
    {
        //Rt*Ib
        evaluator.multiply_plain(EIb[0], encoder.encode(MFu[j][0]), RfIb ); 
        for(i=1; i<n; i++)
        {
            BigPoly tmp=evaluator.add(RfIb, evaluator.multiply_plain(EIb[i], encoder.encode(MFu[j][i]) ) );
            RfIb=tmp;
        }
        //RfIb2=evaluator.sub_plain(RfIb, encoder.encode(Rfavg[j]) );
        //Eqfb2=evaluator.multiply(Eqfb[j], Ew[j] ); //qfb*w
        evaluator.multiply(evaluator.sub_plain(RfIb, encoder.encode(Rfavg[j]) ), evaluator.multiply(Eqfb[j], Ew[j] ),  Enf[j]);

        /*
        BigPoly Enf1=evaluator.sub_plain(RfIb, encoder.encode(Rfavg[j]) );
        BigPoly Enf2=evaluator.multiply(Eqfb[j], Ew[j] );
        BigPoly Enf3=evaluator.multiply(Enf2, encryptor.encrypt(encoder.encode(-5)) );

        cout << "Enf1: " << decryptor.decrypt(Enf1).to_string() << endl;
        cout << "Enf2: " << decryptor.decrypt(Enf2).to_string() << endl;
        cout << "Enf3: " << decryptor.decrypt(Enf3).to_string() << endl;

        cout << "Noise : " << inherent_noise(Enf1, optimal_parms, secret_key).significant_bit_count()
        << "/" << inherent_noise_max(optimal_parms).significant_bit_count() << " bits" << endl;

        cout << "Noise : " << inherent_noise(Enf2, optimal_parms, secret_key).significant_bit_count()
        << "/" << inherent_noise_max(optimal_parms).significant_bit_count() << " bits" << endl;

        cout << "Noise : " << inherent_noise(Enf3, optimal_parms, secret_key).significant_bit_count()
        << "/" << inherent_noise_max(optimal_parms).significant_bit_count() << " bits" << endl;
        */
        //cout << "Enf: " << decryptor.decrypt(Enf[j]).to_string() << endl;
        cout << "Enf: " << encoder.decode_int64(decryptor.decrypt(Enf[j])) << endl;
    }   

    cout<<"Time for stage2 friends:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    //Stage 3
    //modify later, user average
    //int rua=2;

    //user encryption
    //BigPoly Eruavg=encryptor.encrypt( encoder.encode(rua) );
    cout<<"Time for stage3 user:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    //RS server
    //sum all nt
    BigPoly EnT=Ent[0], EdT=Eqtb[0];
    for(i=1; i<Tu; i++)
    {
        BigPoly tmp1=evaluator.add(EnT, Ent[i] );
        EnT=tmp1;

        BigPoly tmp2=evaluator.add(EdT, Eqtb[i] );        
        EdT=tmp2;
    }

    BigPoly EnF=Enf[0], EdF=Eqfb[0];
    for(i=1; i<Fu; i++)
    {
        BigPoly tmp1=evaluator.add(EnF, Enf[i] );
        EnF=tmp1;

        BigPoly tmp2=evaluator.add(EdF, Eqfb[i] );        
        EdF=tmp2;      
    }

    //BigPoly EX=evaluator.add(evaluator.multiply(Eruavg, evaluator.multiply(EdT, evaluator.multiply_plain(EdF, encoder.encode(alpha+beta) ) )), evaluator.add(evaluator.multiply(EnT, evaluator.multiply_plain(EdF, encoder.encode(beta) ) ) , evaluator.multiply(EnF, evaluator.multiply_plain(EdT, encoder.encode(alpha) ) ) ) );
    BigPoly EX=evaluator.add(evaluator.multiply_plain(evaluator.multiply(EdF, EnT), encoder.encode(beta) )  , evaluator.multiply_plain(evaluator.multiply(EdT, EnF), encoder.encode(alpha) )  );
    BigPoly EY=evaluator.multiply_plain(evaluator.multiply(EdT, EdF), encoder.encode(alpha+beta) ) ; 
    
    cout<<"Time for stage3 server:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    BigPoly EY1=evaluator.multiply_plain(EdF, encoder.encode(alpha+beta) ) ;
    BigPoly EY2=evaluator.multiply(EdT, EY1 ) ;

    BigPoly EX1=evaluator.multiply(EdF, EnT);
    BigPoly EX2=evaluator.multiply_plain(EX1, encoder.encode(beta) )  ;

    BigPoly EX3=evaluator.multiply(EdT, EnF);
    BigPoly EX4=evaluator.multiply_plain(EX3, encoder.encode(alpha) )  ;

    // Finally print the result and noise
    cout << "EnT: " << encoder.decode_int64(decryptor.decrypt(EnT)) << endl;
    cout << "EdT: " << encoder.decode_int64(decryptor.decrypt(EdT)) << endl;    
    cout << "EnF: " << encoder.decode_int64(decryptor.decrypt(EnF)) << endl;
    cout << "EdF: " << encoder.decode_int64(decryptor.decrypt(EdF)) << endl;

    cout << "EX1: " << encoder.decode_int64(decryptor.decrypt(EX1)) << ", "<<decryptor.decrypt(EX1).to_string() <<endl;
    cout << "EY1: " << encoder.decode_int64(decryptor.decrypt(EY1)) << ", "<<decryptor.decrypt(EY1).to_string() <<endl;
    cout << "EX2: " << encoder.decode_int64(decryptor.decrypt(EX2)) << ", "<<decryptor.decrypt(EX2).to_string() <<endl;
    cout << "EY2: " << encoder.decode_int64(decryptor.decrypt(EY2)) << ", "<<decryptor.decrypt(EY2).to_string() <<endl;
    
    cout << "EX3: " << encoder.decode_int64(decryptor.decrypt(EX3)) << ", "<<decryptor.decrypt(EX3).to_string() <<endl;
    cout << "EX4: " << encoder.decode_int64(decryptor.decrypt(EX4)) << ", "<<decryptor.decrypt(EX4).to_string() <<endl;

    cout << "Noise : " << inherent_noise(EX3, optimal_parms, secret_key).significant_bit_count()
        << "/" << inherent_noise_max(optimal_parms).significant_bit_count() << " bits" << endl;
    cout << "Noise : " << inherent_noise(EX4, optimal_parms, secret_key).significant_bit_count()
        << "/" << inherent_noise_max(optimal_parms).significant_bit_count() << " bits" << endl;

    cout << "EX: " << encoder.decode_int64(decryptor.decrypt(EX).to_string()) << endl;
    cout << "EY: " << encoder.decode_int64(decryptor.decrypt(EY).to_string()) << endl;
    /**

    cout << "EX1: " <<decryptor.decrypt(EX1).to_string() <<endl;
    cout << "EY1: " <<decryptor.decrypt(EY1).to_string() <<endl;
    cout << "EX2: " <<decryptor.decrypt(EX2).to_string() <<endl;
    cout << "EY2: " <<decryptor.decrypt(EY2).to_string() <<endl;

    cout << "EX: " << decryptor.decrypt(EX).to_string() << endl;
    cout << "EY: " << decryptor.decrypt(EY).to_string() << endl;
    **/
    cout << "Noise : " << inherent_noise(EX, optimal_parms, secret_key).significant_bit_count()
        << "/" << inherent_noise_max(optimal_parms).significant_bit_count() << " bits" << endl;

    //cout << "EY: " << encoder.decode_int64(decryptor.decrypt(EY)) << endl;

    cout << "Noise : " << inherent_noise(EY, optimal_parms, secret_key).significant_bit_count()
        << "/" << inherent_noise_max(optimal_parms).significant_bit_count() << " bits" << endl;

    /****/
};



//load matrixes from a file
//Tm: target user
//Tu: stranger size
//Fu: friend size
//n: item size
void LoadM(int MTu[Tu][n], int MFu[Fu][n])
{
  //ifstream fm("../python/target user data .txt");
  ifstream ff("../python/friends.txt");
  ifstream fs("../python/strangers.txt");

  for(int j=0; j<Fu; j++)
  for(int i=0; i<n; i++)
        ff>>MFu[j][i];

  for(int j=0; j<Tu; j++)
  for(int i=0; i<n; i++)
        fs>>MTu[j][i];

  ff.close();
  fs.close();
}