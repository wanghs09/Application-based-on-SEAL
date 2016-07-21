#include "SPP.h"
//#include "cryptonets.h"
#include "TopN.h"

int main()
{
	EncryptionParameters optimal_parms= SPP_Parameter_Gen();
	HEsystem hesystem(optimal_parms);
	//test all 943 users with 3 prediction per user
	//./SPP >result.txt
	for(int user_id=1; user_id<=943; user_id++)
	{
		int item_id;
		for(int j=0; j<=2; j++)
			{
				item_id=rand()%n;
				cout<<"user_id:"<<user_id<<", item_id:"<<item_id<<endl;
				SPP(user_id,item_id, optimal_parms, hesystem);
			}
	}

    //Cryptonets();
//   TopN(1);

    return 0;
}

