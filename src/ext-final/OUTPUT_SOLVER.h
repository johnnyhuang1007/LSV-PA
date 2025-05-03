#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "proof/int/intInt.h"
#include <iostream>
#include<list>
#include<vector>
#include<map>
#include<unordered_map>
#include<set>
#include<algorithm>
#include<bitset>
#include <sstream>
using namespace std;

extern "C" Aig_Man_t * Abc_NtkToDar( Abc_Ntk_t * pNtk, int fExors, int fRegisters );

class OUTPUT_SOLVER
{
    private:
        

    public:
    
        vector<vector<bool> > output_matrix; 
        OUTPUT_SOLVER()
        {
            pSat = sat_solver_new();
        }
        OUTPUT_SOLVER(Abc_Ntk_t * AIG1, Abc_Ntk_t * AIG2);
        ~OUTPUT_SOLVER()
        {
            sat_solver_delete(pSat);
            Cnf_DataFree(pCnf);
            Aig_ManStop(cone1_aig);
            Aig_ManStop(cone2_aig);
            Abc_NtkDelete(miter);
            Abc_NtkDelete(cone1);
            Abc_NtkDelete(cone2);
        }
        

        
};