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
    cir2_cnf = NULL;
    cir1_cnf = NULL;
    origin_cir1 = AIG1;
    origin_cir2 = AIG2;
    output1_idxs.reserve(Abc_NtkPoNum(AIG1));
    output2_idxs.reserve(Abc_NtkPoNum(AIG2));
    rev.reserve(Abc_NtkPoNum(AIG2));
    
}

int serach_Po_idx(Abc_Ntk_t * cir, int idx)
{
    for(int i = 0 ; i < Abc_NtkPoNum(cir) ; i++)
    {
        if(Abc_ObjId(Abc_NtkPo(cir,i)) == idx)
            return i;
    }
    return -1;
}

bool INPUT_SOLVER::gen_test_pair(pair<int,int> test_pair)
{
    
    output1_idxs.push_back(serach_Po_idx(origin_cir1,test_pair.first));
    output2_idxs.push_back(serach_Po_idx(origin_cir2,test_pair.second));
    rev.push_back(0);
    Vec_Ptr_t* Po1_List = Vec_PtrAlloc(output1_idxs.size());
    Vec_Ptr_t* Po2_List = Vec_PtrAlloc(output2_idxs.size());

    vector<int> o1_tmp = output1_idxs;
    sort(o1_tmp.begin(), o1_tmp.end());
    auto it = unique(o1_tmp.begin(), o1_tmp.end());
    o1_tmp.erase(it, o1_tmp.end());
    for(int i = 0 ; i < o1_tmp.size(); i++)
        Vec_PtrPush(Po1_List,Abc_NtkObj(origin_cir1,o1_tmp[i]));  //obtain

    for(int i = 0 ; i < output2_idxs.size(); i++)
        Vec_PtrPush(Po2_List,Abc_NtkObj(origin_cir2,output2_idxs[i]));


    if(cone1!=NULL) 
        Abc_NtkDelete(cone1);
    if(cone2!=NULL)
        Abc_NtkDelete(cone2);

    if(cone1_aig!=NULL)
        Aig_ManStop(cone1_aig);
    if(cone2_aig!=NULL)
        Aig_ManStop(cone2_aig);

    
    cone1 = Abc_NtkCreateConeArray(origin_cir1,Po1_List,0);
    cone2 = Abc_NtkCreateConeArray(origin_cir2,Po2_List,0);
        

    cone1_aig = Abc_NtkToDar(cone1,0,0);
    cone2_aig = Abc_NtkToDar(cone2,0,0);
    Vec_PtrFree(Po1_List);
    Vec_PtrFree(Po2_List);

    Cir2_Phase_Change();

    return 1;
}

bool INPUT_SOLVER::gen_prev_pair(pair<int,int> test_pair)
{
    int idx2 = serach_Po_idx(origin_cir2,test_pair.second);
    for(int i = 0 ; i < output2_idxs.size() ; i++)
    {
        if(output2_idxs[i] == idx2)
        {
            output1_idxs[i] = output1_idxs.back();
            output2_idxs[i] = output2_idxs.back();
            rev[i] = rev.back();
            output1_idxs.pop_back();
            output2_idxs.pop_back();
            rev.pop_back();
            break;
        }
    }
    
    Vec_Ptr_t* Po1_List = Vec_PtrAlloc(output1_idxs.size());
    Vec_Ptr_t* Po2_List = Vec_PtrAlloc(output2_idxs.size());

    vector<int> o1_tmp = output1_idxs;
    sort(o1_tmp.begin(), o1_tmp.end());
    auto it = unique(o1_tmp.begin(), o1_tmp.end());
    o1_tmp.erase(it, o1_tmp.end());
    for(int i = 0 ; i < o1_tmp.size(); i++)
        Vec_PtrPush(Po1_List,Abc_NtkObj(origin_cir1,o1_tmp[i]));
    for(int i = 0 ; i < output2_idxs.size(); i++)
        Vec_PtrPush(Po2_List,Abc_NtkObj(origin_cir2,output2_idxs[i]));
    
    if(cone1!=NULL) 
        Abc_NtkDelete(cone1);
    if(cone2!=NULL)
        Abc_NtkDelete(cone2);

    if(cone1_aig!=NULL)
        Aig_ManStop(cone1_aig);
    if(cone2_aig!=NULL)
        Aig_ManStop(cone2_aig);

    cone1 = Abc_NtkCreateConeArray(origin_cir1,Po1_List,0);
    cone2 = Abc_NtkCreateConeArray(origin_cir2,Po2_List,0);

    Cir2_Phase_Change();

    cone1_aig = Abc_NtkToDar(cone1,0,0);
    cone2_aig = Abc_NtkToDar(cone2,0,0);

    return 1;
}

bool INPUT_SOLVER::gen_pair()
{

    Vec_Ptr_t* Po1_List = Vec_PtrAlloc(output1_idxs.size());
    Vec_Ptr_t* Po2_List = Vec_PtrAlloc(output2_idxs.size());

    vector<int> o1_tmp = output1_idxs;
    sort(o1_tmp.begin(), o1_tmp.end());
    auto it = unique(o1_tmp.begin(), o1_tmp.end());
    o1_tmp.erase(it, o1_tmp.end());
    for(int i = 0 ; i < o1_tmp.size(); i++)
        Vec_PtrPush(Po1_List,Abc_NtkObj(origin_cir1,o1_tmp[i]));
    for(int i = 0 ; i < output2_idxs.size(); i++)
        Vec_PtrPush(Po2_List,Abc_NtkObj(origin_cir2,output2_idxs[i]));
    
    if(cone1!=NULL) 
        Abc_NtkDelete(cone1);
    if(cone2!=NULL)
        Abc_NtkDelete(cone2);

    if(cone1_aig!=NULL)
        Aig_ManStop(cone1_aig);
    if(cone2_aig!=NULL)
        Aig_ManStop(cone2_aig);

    cone1 = Abc_NtkCreateConeArray(origin_cir1,Po1_List,0);
    cone2 = Abc_NtkCreateConeArray(origin_cir2,Po2_List,0);

    Cir2_Phase_Change();

    cone1_aig = Abc_NtkToDar(cone1,0,0);
    cone2_aig = Abc_NtkToDar(cone2,0,0);

    return 1;
}

bool INPUT_SOLVER::reverse_MO_phase(int i,int j)
{
    for(int jdx = 0 ; jdx < output2_idxs.size() ; jdx++)
    {
        if(output2_idxs[jdx] == j && output1_idxs[jdx] == i)
        {
            rev[jdx] = !rev[jdx];
            break;
        }
    }
    return 1;
}

bool INPUT_SOLVER::Cir2_Phase_Change()  //call once cone2 is created
{

    for(int i = 0 ; i < output2_idxs.size() ; i++)
    {
        if(rev[i])
        {
            char* name2 = Abc_ObjName(Abc_NtkObj(origin_cir2,output2_idxs[i]));
            int idx_cone2 = Abc_ObjId(Abc_NtkFindCo(cone2,name2));
            idx_cone2 = serach_Po_idx(cone2,idx_cone2);
            char* name1 = Abc_ObjName(Abc_NtkObj(origin_cir1,output1_idxs[i]));
            int idx_cone1 = Abc_ObjId(Abc_NtkFindCo(cone1,name1));
            idx_cone1 = serach_Po_idx(cone1,idx_cone1);
            Abc_ObjXorFaninC(Abc_NtkPo(cone2,idx_cone2),0);
        }
    }
    return 1;
}


bool INPUT_SOLVER::gen_miter_cnf()
{


    if(miter!=NULL) 
        Abc_NtkDelete(miter);
    
    if(cir1_cnf!=NULL)
        Cnf_DataFree(cir1_cnf);
    if(cir2_cnf!=NULL)
        Cnf_DataFree(cir2_cnf);
    

    cir1_cnf = Cnf_Derive(cone1_aig, Abc_NtkPoNum(cone1));
    cir2_cnf = Cnf_Derive(cone2_aig, Abc_NtkPoNum(cone2));


    AddMiterCnf(cir1_cnf,cir2_cnf);

    return 1;
}

int INPUT_SOLVER::AddMiterCnf(Cnf_Dat_t *miter_cnf1, Cnf_Dat_t *miter_cnf2) {   //update miter cnf and offset of each solver
    // sean: return 0 if success, 1 if failed
    if (!miter_cnf1 || !miter_cnf2) {
        printf("Error: One or both CNF data are NULL.\n");
        return 1;
    }

    int var_offset_ckt1 = 0;
    cir1_offset = var_offset_ckt1;
    int var_offset_ckt2 = var_offset_ckt1 + miter_cnf1->nVars;
    cir2_offset = var_offset_ckt2;
    miter_po_offset = cir2_offset + miter_cnf2->nVars;

    // sean: augments the variable of the sat solver
    sat_solver_setnvars(pSat, miter_cnf1->nVars + miter_cnf2->nVars + sat_solver_nvars(pSat));
    
    // sean: lift the variable index of the CNF of cnf1 and cnf2
    Cnf_DataLift(miter_cnf1, var_offset_ckt1);
    Cnf_DataLift(miter_cnf2, var_offset_ckt2);
    
    // sean: add the clauses of the first CNF to the sat solver
    for (int i = 0; i < miter_cnf1->nClauses; i++) {
        if (!sat_solver_addclause(pSat, miter_cnf1->pClauses[i], miter_cnf1->pClauses[i+1])) {
            printf("Error: Failed to add clause %d of the first CNF to the sat solver.\n", i);
            return 1;
        }
    }

    // sean: add the clauses of the second CNF to the sat solver
    for (int i = 0; i < miter_cnf2->nClauses; i++) {
        if (!sat_solver_addclause(pSat, miter_cnf2->pClauses[i], miter_cnf2->pClauses[i+1])) {
            printf("Error: Failed to add clause %d of the second CNF to the sat solver.\n", i);
            return 1;
        }
    }
    
    // sean: create cnf of the XOR part and OR part to the sat solver
    // sean: augment the variable index of the XOR part and OR part, number of matched co
    int output_offset = sat_solver_nvars(pSat);
    sat_solver_setnvars(pSat, sat_solver_nvars(pSat) + Abc_NtkPoNum(cone1) + 1); // +1 for or var
    // traverse the co id of cone1_aig and cone2_aig
    int idx = 0;
    Aig_Obj_t *pObj1, *pObj2;
    lit *ORLits = new int[Abc_NtkPoNum(cone1)]; // OR all the xor parts
    Aig_ManForEachCo(cone1_aig , pObj1, idx ) {
        pObj2 = Aig_ManCo(cone2_aig, idx);
        int cnf_idx0 = miter_cnf1->pVarNums[Aig_ObjId(pObj1)];
        int cnf_idx1 = miter_cnf2->pVarNums[Aig_ObjId(pObj2)];
        
        // sean: add the XOR part to the sat solver
        if (!Cnf_DataAddXorClause(pSat, cnf_idx0, cnf_idx1, output_offset + idx)){
            printf("Error: Failed to add XOR clause %d to the sat solver.\n", idx);
            return 1;
        }

        // sean: add the OR part to the sat solver
        ORLits[idx] = Abc_Var2Lit(output_offset + idx, 0);

    }
    //or : a + b + c <-> d --> (a + b + c + d')*(a' + d)*(b' + d)*(c' + d)
    //shorter one
    int OR_var = output_offset + Abc_NtkPoNum(cone1);
    for(int i = 0 ; i < Abc_NtkPoNum(cone1) ; i++)
    {
        lit Lits[2];
        Lits[0] = Abc_Var2Lit(OR_var, 0);   //d
        Lits[1] = Abc_Var2Lit(output_offset + i, 1); //a'

        if (!sat_solver_addclause(pSat, Lits, Lits + 2)) {
            printf("Error: Failed to add OR clause %d to the sat solver.\n", i);
            return 1;
        }
    }
    
    //(a + b + c + d')
    vector<lit> or_long(Abc_NtkPoNum(cone1)+1,0);
    for(int i = 0 ; i < Abc_NtkPoNum(cone1) ; i++)
    {
        or_long[i] = Abc_Var2Lit(output_offset + i, 0);
    }
    or_long[Abc_NtkPoNum(cone1)] = Abc_Var2Lit(OR_var, 1);
    if (!sat_solver_addclause(pSat, &or_long[0], &or_long[0] + Abc_NtkPoNum(cone1)+1)) {
        printf("Error: Failed to add final or clause %d to the sat solver.\n");
        return 1;
    }

    
    
    MI_offset = sat_solver_nvars(pSat);
    MO_offset = MI_offset + MI.size()*MI[0].size();
    sat_solver_setnvars(pSat,MO_offset + MO.size()*MO[0].size());
    return 0;
}

bool INPUT_SOLVER::set_miter_cnf_out(int miter_out)
{

    lit Lit[1];
    Lit[0] = Abc_Var2Lit(MI_offset-1, !miter_out);
    return sat_solver_addclause(pSat, Lit, Lit + 1);
}