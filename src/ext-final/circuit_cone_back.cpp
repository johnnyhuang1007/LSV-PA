#include "INPUT_SOLVER.h"

using namespace std;

INPUT_SOLVER::INPUT_SOLVER(Abc_Ntk_t * AIG1, Abc_Ntk_t * AIG2)
{
    pSat = sat_solver_new();
    MI.resize(Abc_NtkPiNum(AIG2),vector<bool>(Abc_NtkPiNum(AIG1)*2+2,0));
    MO.resize(Abc_NtkPoNum(AIG2),vector<bool>(Abc_NtkPoNum(AIG1)*2,0));
    cone1 = NULL;
    cone2 = NULL;
    cone1_aig = NULL;
    cone2_aig = NULL;
    miter = NULL;
    pCnf = NULL;
    origin_cir1 = AIG1;
    origin_cir2 = AIG2;
    output1_idxs.reserve(Abc_NtkPoNum(AIG1));
    output2_idxs.reserve(Abc_NtkPoNum(AIG2));
    
    sat_solver_setnvars(pSat, MI.size()*MI[0].size() + MO.size()*MO[0].size() + sat_solver_nvars(pSat));
}

bool INPUT_SOLVER::gen_test_pair(pair<int,int> test_pair)
{
    output1_idxs.push_back(test_pair.first);
    output2_idxs.push_back(test_pair.second);
    Vec_Ptr_t* Po1_List = Vec_PtrAlloc(output1_idxs.size());
    Vec_Ptr_t* Po2_List = Vec_PtrAlloc(output2_idxs.size());
    for(int i = 0 ; i < output1_idxs.size(); i++)
        Vec_PtrPush(Po1_List,Abc_NtkPo(origin_cir1,output1_idxs[i]));
    for(int i = 0 ; i < output2_idxs.size(); i++)
        Vec_PtrPush(Po2_List,Abc_NtkPo(origin_cir2,output2_idxs[i]));
    cout<<Po1_List->nSize<<endl;
    cout<<"OBTAINED PO LIST"<<endl;

    if(cone1!=NULL) 
        Abc_NtkDelete(cone1);
    if(cone2!=NULL)
        Abc_NtkDelete(cone2);

    if(cone1_aig!=NULL)
        Aig_ManStop(cone1_aig);
    if(cone2_aig!=NULL)
        Aig_ManStop(cone2_aig);

    

    cout<<"GEN AIG"<<endl;

    cout<<Po1_List->nSize<<endl;

    cone1 = Abc_NtkCreateConeArray(origin_cir1,Po1_List,1);
    cone2 = Abc_NtkCreateConeArray(origin_cir2,Po2_List,1);
        

    cone1_aig = Abc_NtkToDar(cone1,0,0);
    cone2_aig = Abc_NtkToDar(cone2,0,0);
    Vec_PtrFree(Po1_List);
    Vec_PtrFree(Po2_List);
    cout<<"DONE"<<endl;
    return 1;
}

bool INPUT_SOLVER::gen_prev_pair(pair<int,int> test_pair)
{
    for(int i = 0 ; i < output1_idxs.size(); i++)
    {
        if(output1_idxs[i] == test_pair.first)
        {
            output1_idxs[i] = output1_idxs.back();
            output1_idxs.pop_back();
            break;
        }
    }
    for(int i = 0 ; i < output2_idxs.size(); i++)
    {
        if(output2_idxs[i] == test_pair.second)
        {
            output2_idxs[i] = output2_idxs.back();
            output2_idxs.pop_back();
            break;
        }
    }
    Vec_Ptr_t* Po1_List = Vec_PtrAlloc(output1_idxs.size());
    Vec_Ptr_t* Po2_List = Vec_PtrAlloc(output2_idxs.size());
    for(int i = 0 ; i < output1_idxs.size(); i++)
        Vec_PtrPush(Po1_List,Abc_NtkPo(origin_cir1,output1_idxs[i]));
    for(int i = 0 ; i < output2_idxs.size(); i++)
        Vec_PtrPush(Po2_List,Abc_NtkPo(origin_cir2,output2_idxs[i]));
    
    if(cone1!=NULL) 
        Abc_NtkDelete(cone1);
    if(cone2!=NULL)
        Abc_NtkDelete(cone2);

    if(cone1_aig!=NULL)
        Aig_ManStop(cone1_aig);
    if(cone2_aig!=NULL)
        Aig_ManStop(cone2_aig);

    cone1 = Abc_NtkCreateConeArray(origin_cir1,Po1_List,1);
    cone2 = Abc_NtkCreateConeArray(origin_cir2,Po2_List,1);
    
    cone1_aig = Abc_NtkToDar(cone1,0,0);
    cone2_aig = Abc_NtkToDar(cone2,0,0);

    return 1;
}




bool INPUT_SOLVER::gen_miter()
{

    cout<<"COPY CONE"<<endl;
    Abc_Ntk_t * cone1_dup = Abc_NtkDup(cone1);
    Abc_Ntk_t * cone2_dup = Abc_NtkDup(cone2);

    

    cout<<"INVERT OUTPUT"<<endl;
    for(int i = 0 ; i < output2_idxs.size() ; i++)
    {
        
        for(int j = 0 ; j < MO[output2_idxs[i]].size() ; j++)
        {
            if(MO[output2_idxs[i]][j])
            {
                if(j%2 != 0)   
                    Abc_ObjXorFaninC(Abc_NtkPo(cone2_dup,output2_idxs[i]),0);   //invert if bi,j = 1
            }
        }
    }

    cout<<"GENERATE MITER"<<endl;
    if(miter!=NULL) 
        Abc_NtkDelete(miter);
    

    Aig_Man_t * pMan1 = Abc_NtkToDar( cone1_dup, 0, 0 );
    Cnf_Dat_t * miter_cnf = Cnf_Derive(pMan1, 3);
    Aig_Man_t * pMan2 = Abc_NtkToDar( cone2_dup, 0, 0 );
    Cnf_Dat_t * miter_cnf2 = Cnf_Derive(pMan2, 3);
    


    Abc_NtkDelete(cone1_dup);
    Abc_NtkDelete(cone2_dup);

    if (!Abc_NtkIsStrash(miter)) {
        std::cout << "[Error] Miter is not in AIG (strashed) form.\n";
        return 1;
    }
    if (!Abc_NtkIsComb(miter)) {
        std::cout << "[Error] Miter is not a combinational network.\n";
        return 1;
    }
    if (!Abc_NtkCheck(miter)) {
        std::cout << "[Error] Miter network failed consistency check.\n";
        return 1;
    }
    cout<<"GENERATE CNF"<<endl;
    if(pCnf!=NULL)
        Cnf_DataFree(pCnf);
    cout<<"START"<<endl;
    Aig_Man_t * miter_aig = Abc_NtkToDar(miter,0,0);
    cout<<Aig_ManCoNum(miter_aig)<<endl;
    pCnf = Cnf_Derive(miter_aig, Aig_ManCoNum(miter_aig));
    Aig_ManStop(miter_aig);

    return 1;
}

Cnf_Dat_t *MergeCnf(Cnf_Dat_t *miter_cnf1, Cnf_Dat_t *miter_cnf2) {
    if (!miter_cnf1 || !miter_cnf2) {
        printf("Error: One or both CNF data are NULL.\n");
        return NULL;
    }

    // 創建新的 CNF 結構
    Cnf_Dat_t *merged_cnf = Cnf_DataAlloc(miter_cnf1->nVars + miter_cnf2->nVars, 
                                           miter_cnf1->nClauses + miter_cnf2->nClauses);
    if (!merged_cnf) {
        printf("Error: Failed to allocate merged CNF.\n");
        return NULL;
    }
    
    // 複製第一個 CNF 的內容
    int var_offset = 0; // 第一個 CNF 的變數無需偏移
    for (int i = 0; i < miter_cnf1->nClauses; i++) {
        Cnf_AddClause(merged_cnf, miter_cnf1->pClauses[i], miter_cnf1->pClauses[i + 1], var_offset);
    }
    merged_cnf->nVars = miter_cnf1->nVars;

    // 複製第二個 CNF 的內容，需調整變數編號
    var_offset = miter_cnf1->nVars;
    for (int i = 0; i < miter_cnf2->nClauses; i++) {
        Cnf_AddClause(merged_cnf, miter_cnf2->pClauses[i], miter_cnf2->pClauses[i + 1], var_offset);
    }
    merged_cnf->nVars += miter_cnf2->nVars;

    return merged_cnf;
}