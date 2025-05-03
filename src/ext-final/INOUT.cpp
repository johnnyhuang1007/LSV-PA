#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "proof/int/intInt.h"

#include "opt/sim/sim.h"
#include <iostream>
#include<list>
#include<vector>
#include<map>
#include<unordered_map>

#include <fstream>
#include<set>
#include<algorithm>
#include<bitset>
#include "INOUT.h"
using namespace std;

unordered_map<string, set<string>> strSupport(Abc_Ntk_t * mainNtk) {
    Abc_Obj_t * pNode, * pNodeCi;
    unordered_map<string, set<string> > re;
    int i, v;
    Vec_Ptr_t * vSuppStr = Sim_ComputeStrSupp( mainNtk );
    Abc_NtkForEachCo( mainNtk, pNode, i )
        Abc_NtkForEachCi( mainNtk, pNodeCi, v ){
            if(Sim_SuppStrHasVar( vSuppStr, pNode, v ) != 0){
                string outputName = Abc_ObjName(pNode);
                string inputName = Abc_ObjName(pNodeCi);

                if(outputName == inputName)continue;
                re[outputName].insert(inputName);
                re[inputName].insert(outputName);
            }
        }
    if ( vSuppStr )     Sim_UtilInfoFree( vSuppStr);
    return re;
}

unordered_map<string, set<string>> funSupport(Abc_Ntk_t * mainNtk){
    Abc_Obj_t * pNode, * pNodeCi;
    unordered_map<string, set<string> > re;
    int i, v;
    Vec_Ptr_t * vSuppFun = Sim_ComputeFunSupp( mainNtk, 0 );
    Abc_NtkForEachCo( mainNtk, pNode, i )
        Abc_NtkForEachCi( mainNtk, pNodeCi, v ){
            if(Sim_SuppFunHasVar( vSuppFun, i, v ) != 0){
                string outputName = Abc_ObjName(pNode);
                string inputName = Abc_ObjName(pNodeCi);
                if(outputName == inputName)continue;
                re[outputName].insert(inputName);
                re[inputName].insert(outputName);
            }
        }
    if ( vSuppFun )     Sim_UtilInfoFree( vSuppFun );
    return re;
}


void getBus(char** argv , vector<vector<string>>& bus1, vector<vector<string>>& bus2)
{
    ifstream in;
    string dummy;
    int busNum = 0;
    int businner = 0;
    in.open(argv[3]);
     if (!in.is_open()) { // 檢查檔案是否成功開啟
        std::cerr << "Error: Failed to open file " << argv[3] << "." << std::endl;
        exit(1);
    }

    in >> dummy >> busNum;
    for(int i = 0 ; i < busNum ; i++)
    {
        in >> businner;
        vector<string> busVector;
        for(int q = 0 ; q < businner ; q++){
            string busName;
            in >> busName;
            busVector.push_back(busName);
        }
        bus1.push_back(busVector);
    }
    in >> dummy >> busNum;
    for(int i = 0 ; i < busNum ; i++)
    {
        in >> businner;
        vector<string> busVector;
        for(int q = 0 ; q < businner ; q++){
            string busName;
            in >> busName;
            busVector.push_back(busName);
        }
        bus2.push_back(busVector);
    }
}



vector<Abc_Ntk_t *> get_AIGs(Abc_Frame_t* pAbc,char** argv)
{
    const char * pFile1 = argv[1];
    const char * pFile2 = argv[2];

    {
        char cmdRead1[1024];
        sprintf(cmdRead1, "read %s", pFile1);
        if ( Cmd_CommandExecute( pAbc, cmdRead1 ) ) {
            Abc_Print( -2, "Error: Fail to read %s\n", pFile1 );
            return vector<Abc_Ntk_t *>{};
        }
    }

    Abc_Ntk_t * pNtk1Global = Abc_FrameReadNtk( pAbc );
    if ( pNtk1Global == NULL ) {
        Abc_Print( -2, "Error: pNtk1Global is NULL\n" );
        return vector<Abc_Ntk_t *>{};
    }
    Abc_Ntk_t * pNtk1 = Abc_NtkDup( pNtk1Global );
    Abc_Print( 1, "Successfully read AIG from: %s\n", pFile1 );


    {
        char cmdRead2[1024];
        sprintf(cmdRead2, "read %s", pFile2);
        if ( Cmd_CommandExecute( pAbc, cmdRead2 ) ) {
            Abc_Print( -2, "Error: Fail to read %s\n", pFile2 );
            Abc_NtkDelete( pNtk1 ); // 釋放第一個網路
            return vector<Abc_Ntk_t *>{};
        }
    }

    Abc_Ntk_t * pNtk2Global = Abc_FrameReadNtk( pAbc );
    if ( pNtk2Global == NULL ) {
        Abc_Print( -2, "Error: pNtk2Global is NULL\n" );
        Abc_NtkDelete( pNtk1 ); // 釋放第一個網路
        return vector<Abc_Ntk_t *>{};
    }
    Abc_Ntk_t * pNtk2 = Abc_NtkDup( pNtk2Global );
    Abc_Print( 1, "Successfully read AIG from: %s\n", pFile2 );


    Abc_Print( 1, "pNtk1 #PI = %d, #PO = %d, #Obj = %d\n",
        Abc_NtkPiNum(pNtk1), Abc_NtkPoNum(pNtk1), Abc_NtkObjNum(pNtk1) );
    Abc_Print( 1, "pNtk2 #PI = %d, #PO = %d, #Obj = %d\n",
        Abc_NtkPiNum(pNtk2), Abc_NtkPoNum(pNtk2), Abc_NtkObjNum(pNtk2) );

    


    vector<Abc_Ntk_t *> AIGs;
    AIGs.push_back(pNtk1);
    AIGs.push_back(pNtk2);
    return AIGs;
}
