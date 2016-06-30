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
    std::clock_t start;
    print_example_banner("Implementation:Single Prediciton Protocol");

    /*
    Here we demonstrate the automatic parameter selection tool. Suppose we want to find parameters
    that are optimized in a way that allows us to evaluate the polynomial 42x^3-27x+1. We need to know
    the size of the input data, so let's assume that x is an integer with base-3 representation of length
    at most 10.
    */
    //cout << "Finding optimized parameters for computing 42x^3-27x+1 ... ";

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


    ChooserPoly RIb = chooser_evaluator.multiply_plain(Ib, chooser_encoder.encode(T*n)); //*n*Rt
    ChooserPoly RIbs = chooser_evaluator.sub_plain(RIb, chooser_encoder.encode(T*n)); //n*rt_avg
    ChooserPoly ntb = chooser_evaluator.multiply(RIbs, qtb); 

    ChooserPoly nT = chooser_evaluator.multiply_plain(ntb, chooser_encoder.encode(Tu)); //Tu*
    ChooserPoly dT = chooser_evaluator.multiply_plain(qtb, chooser_encoder.encode(Tu)); //Tu*

    //Friends
    ChooserPoly Rfb = chooser_evaluator.multiply_plain(Ib, chooser_encoder.encode(T*n)); //*n*Rt
    ChooserPoly Rfbs = chooser_evaluator.sub_plain(Rfb, chooser_encoder.encode(T*n)); //n*rt_avg
    ChooserPoly nfb = chooser_evaluator.multiply(Rfbs, qfb); 
    ChooserPoly nfbw = chooser_evaluator.multiply(nfb, wuf); 

    ChooserPoly nF = chooser_evaluator.multiply_plain(nfbw, chooser_encoder.encode(Fu)); //Fu*
    ChooserPoly dF = chooser_evaluator.multiply_plain(qtb, chooser_encoder.encode(Fu)); //Fu*

    //X Y
    ChooserPoly X1 = chooser_evaluator.multiply(ruavg, dF);
    ChooserPoly X12 = chooser_evaluator.multiply(X1, dT);
    ChooserPoly X123 = chooser_evaluator.multiply_plain(X12, chooser_encoder.encode(alpha+beta));
    ChooserPoly X2 = chooser_evaluator.multiply(nT, dF);
    ChooserPoly X22 = chooser_evaluator.multiply_plain(X2, chooser_encoder.encode(beta));
    ChooserPoly X3 = chooser_evaluator.multiply(nF, dT);
    ChooserPoly X32 = chooser_evaluator.multiply_plain(X3, chooser_encoder.encode(alpha));

    ChooserPoly Xs = chooser_evaluator.add(X123, X22);
    ChooserPoly X = chooser_evaluator.add(Xs, X32);

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
    BigPoly EIb[n], RtIb, tmp, tmps, RtIb2; //Rt are plaintexts
    int Rtavg[Tu]; //plain qtb
    BigPoly Ent[Tu], Eqtb[Tu];

    //user
    start=clock();

    //user encryption
    //select the first item
    encryptor.encrypt( encoder.encode(1), EIb[0] );
    for(i=1; i<n; i++)
        encryptor.encrypt( encoder.encode(0), EIb[i] );
    cout<<"Time for stage1 user:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    //modify later
    //strangers encryption
    for(j=0; j<Tu; j++)
        Rtavg[j]=2;

    for(j=0; j<Tu; j++)
    {
        encryptor.encrypt( encoder.encode(0), Eqtb[j] );
        for(i=0; i<n; i++)
            if(MTu[j][i]!=0)
            {
                BigPoly tmp=evaluator.add(Eqtb[j],EIb[i]);
                Eqtb[j]=tmp;
            }
    }
        

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
            BigPoly tmp=evaluator.add(RtIb,  evaluator.multiply_plain(EIb[i], encoder.encode(MTu[j][i]) )); 
            RtIb=tmp;
        }
        //RtIb2=evaluator.sub_plain(RtIb, encoder.encode(Rtavg[j]) );
        evaluator.multiply(evaluator.sub_plain(RtIb, encoder.encode(Rtavg[j]) ), Eqtb[j], Ent[j] );
    }   
    cout<<"Time for stage1 Strangers:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
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
    BigPoly RfIb, RfIb2, Eqfb2; //Rt are plaintexts
    int Rfavg[Fu];
    BigPoly Enf[Fu], Eqfb[Fu];

    for(j=0; j<Fu; j++)
        {
            encryptor.encrypt( encoder.encode(0), Eqfb[j] );
            for(i=0; i<n; i++)
            if(MFu[j][i]!=0)
            {
                BigPoly tmp=evaluator.add(Eqfb[j],EIb[i]);
                Eqfb[j]=tmp;
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
    }   

    cout<<"Time for stage2 friends:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    //Stage 3
    //modify later, user average
    int rua=2;

    //user encryption
    BigPoly Eruavg=encryptor.encrypt( encoder.encode(rua) );
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

    BigPoly EX=evaluator.add(evaluator.multiply(Eruavg, evaluator.multiply(EdT, evaluator.multiply_plain(EdF, encoder.encode(alpha+beta) ) )), evaluator.add(evaluator.multiply(EnT, evaluator.multiply_plain(EdF, encoder.encode(beta) ) ) , evaluator.multiply(EnF, evaluator.multiply_plain(EdT, encoder.encode(alpha) ) ) ) );
    BigPoly EY=evaluator.multiply(EdT, evaluator.multiply_plain(EdF, encoder.encode(alpha+beta) ) ); 
    cout<<"Time for stage3 server:"<<(clock()-start)/(double)CLOCKS_PER_SEC <<endl;
    start=clock();

    // Finally print the result and noise
    cout << "EX: " << encoder.decode_int64(decryptor.decrypt(EX)) << endl;

    cout << "Noise : " << inherent_noise(EX, optimal_parms, secret_key).significant_bit_count()
        << "/" << inherent_noise_max(optimal_parms).significant_bit_count() << " bits" << endl;

    cout << "EY: " << encoder.decode_int64(decryptor.decrypt(EY)) << endl;

    cout << "Noise : " << inherent_noise(EY, optimal_parms, secret_key).significant_bit_count()
        << "/" << inherent_noise_max(optimal_parms).significant_bit_count() << " bits" << endl;

    /****/
};



//load matrixes from a file
//Tm: target user
//Tu: stranger size
//Fu: friend size
//n: item size
void LoadM(int MTm[Tm][n], int MTu[Tu][n], int MFu[Fu][n])
{
  ifstream fm("target user data .txt");
  ifstream ff("friends data.txt");
  ifstream fs("strangers data.txt");

  for(int i=0; i<n; i++)
  {
    for(int j1=0; j1<Tm; j1++)
        fm>>MTm[i][j1];

    for(int j2=0; j2<Tu; j2++)
        fs>>MTu[i][j2];

    for(int j3=0; j3<Fu; j3++)
        ff>>MFu[i][j3];
  }

  fm.close();
  ff.close();
  fs.close();
}