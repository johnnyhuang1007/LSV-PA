#include "INPUT_SOLVER.h"

using namespace std;

CE_Storage::CE_Storage(int l):size(0),cur_idx(-1),limit_size(l)
{
    data.reserve(l);
}

void CE_Storage::push_back(vector<pair<int, int>> v)
{
    if(size < limit_size || limit_size == -1)
    {
        data.push_back(v);
        size++;
    }
    else
    {
        data[size] = v;
        size++;
    }
}

vector<pair<int, int>>::const_iterator CE_Storage::get_clauses(int idx)
{
    return data[idx].begin();
}

vector<pair<int, int>>& CE_Storage::operator[](int idx){
    return data[idx];
}

void CE_Storage::merge(CE_Storage& other){
    for (int i=0; i<other.get_size(); i++){
        push_back(other.data[i]);
    }
}

void CE_Storage::clear()
{
    data.clear();
    size = 0;
    cur_idx = -1;
    limit_size = -1;
}

void INPUT_SOLVER::gen_constraints_cnf()
{
    // sean: generate constraint CNF for the sat_solver
    // sean: 1. MI part: if aij == 1, then xi == yj; if bij == 1, then xi == !yj; where xi is the ith PI of AIG1, yj is the jth PI of AIG2
    // sean: 2. one PI port of AIG2 must match exactly one PI port of AIG1
    // sean: 3. Counter example: impossible match pairs should be excluded => set as a cnf constraint to exclude them
    // sean: 4. MO part: if cij == 1, then fi == gj; if dij == 1, then fi == !gj; where fi is the ith PO of AIG1, gj is the jth PO of AIG2
    


    // sean: 1. MI part
    for (int j = 0; j < Abc_NtkPiNum(cone2); j++)
    {
        int yj_idx = get_ckt2_pi(Abc_ObjId(Abc_NtkPi(cone2,j))); // sean: get the real index of yj in original AIG
        int yj = cir2_var_id(Aig_ObjId(Aig_ManCi(cone2_aig,j))); // sean: get the node id of xi in current cnf
        lit yj_pron_lit = Abc_Var2Lit(yj, 1);
        lit yj_lit = Abc_Var2Lit(yj, 0);
        
        for (int i = 0; i < Abc_NtkPiNum(cone1); i++)
        {

            lit Lits[3];
            
            int xi_idx = get_ckt1_pi(Abc_ObjId(Abc_NtkPi(cone1,i))); // sean: get the real index of xi in original AIG
            int xi = cir1_var_id(Aig_ObjId(Aig_ManCi(cone1_aig,i))); // sean: get the node id of xi in current cnf
            // int aji = MI[0].size() * yj_idx + xi_idx*2; // aji's variable id
            int aji = get_MI_var_id(yj_idx, xi_idx*2); // aji's variable id
            int bji = aji + 1; // bji's variable id
            
            lit xi_pron_lit = Abc_Var2Lit(xi, 1);
            lit xi_lit = Abc_Var2Lit(xi, 0);
            lit aji_pron_lit = Abc_Var2Lit(aji, 1);
            lit bji_pron_lit = Abc_Var2Lit(bji, 1);
            // sean: 1.1 aij == 1, then xi == yj
            // sean: (-aji + xi + -yj) ^ (-aji + -xi + yj)
            Lits[0] = aji_pron_lit;
            Lits[1] = xi_lit;
            Lits[2] = yj_pron_lit;
            assert(sat_solver_addclause(pSat, Lits, Lits + 3));
            Lits[1] = xi_pron_lit;
            Lits[2] = yj_lit;
            assert(sat_solver_addclause(pSat, Lits, Lits + 3));
            
            // sean: 1.2 bij == 1, then xi == !yj
            // sean: (-bji + xi + yj) ^ (-bji + -xi + -yj)
            Lits[0] = bji_pron_lit;
            Lits[1] = xi_lit;
            Lits[2] = yj_lit;
            assert(sat_solver_addclause(pSat, Lits, Lits + 3));
            Lits[1] = xi_pron_lit;
            Lits[2] = yj_pron_lit;
            assert(sat_solver_addclause(pSat, Lits, Lits + 3));
        }
    }
    // sean: 2. one PI port of AIG2 must match exactly one PI port of AIG1
    for (int j=0; j<this->MI.size(); j++){
        // constraint for at least one PI port of AIG2 must match PI port of AIG1
        
        vector<lit> Lits1(this->MI[j].size(),0);
        for(int k = 0 ; k < this->MI.size()-1 ; k++)
        {
            // constrant for at most one PI port of AIG2 must match PI port of AIG1
            for (int i=k+1; i<this->MI[j].size(); i++){
                // (-nji + -njk)
                lit Lits2[2];
                // int node1 = j*MI[j].size() + i; 
                int node1 = get_MI_var_id(j, i); // 1 0 1 0 0
                int node2 = get_MI_var_id(j, k); 
                lit node1_lit = Abc_Var2Lit(node1, 1);
                lit node2_lit = Abc_Var2Lit(node2, 1);
                Lits2[0] = node1_lit;
                Lits2[1] = node2_lit;
                assert(sat_solver_addclause(pSat, Lits2, Lits2 + 2));
                
            }
            
        }
        // sean: (nj1 + nj2 + ... + njn)
        for(int k = 0 ; k < this->MI.size() ; k++)
            Lits1[k] = Abc_Var2Lit(get_MI_var_id(j, k), 0);
        assert(sat_solver_addclause(pSat, &Lits1[0], &Lits1[0] + this->MI[j].size()));
    }

    // sean: 3. Counter example: impossible match pairs should be excluded => set as a cnf constraint to exclude them
    // sean: add the clauses in CE_cnf into the sat solver
    // cout << "adding all counter example clauses into the sat solver" << endl;
    for (int i=0; i<CE_cnf.get_size(); i++){
        vector<pair<int, int>> CE_clause = CE_cnf[i];
        vector<lit> CE_lit;
        for (int j=0; j<CE_clause.size(); j++){
            lit lit = Abc_Var2Lit(CE_clause[j].first + MI_offset, CE_clause[j].second);
            CE_lit.push_back(lit);
        }
        assert(sat_solver_addclause(pSat, &CE_lit[0], &CE_lit[0] + CE_lit.size()));
    }

    return;
}

void INPUT_SOLVER::gen_counter_ex_cnf()
{
    // sean: generate counter example CNF for the sat_solver for the current solve pairs and add into the sat_solver
    // sean: get the MI part variables and flip them and "or" them together
    // sean: for example: (a*b*c*d') is a counter example
    // sean: then the CNF is (-a + -b + -c + d)
    
    // cout << "generating local counter example CNF" << endl;
    vector<lit> counter_ex_lit;
    vector<pair<int, int>> counter_mi_clause; // sean: store the MI variables based on MI location
    for (int j = 0; j < Abc_NtkPiNum(cone2); j++){
        int yj_idx = get_ckt2_pi(Abc_ObjId(Abc_NtkPi(cone2,j))); // sean: get the real index of yj in original AIG
        for (int i = 0; i < Abc_NtkPiNum(cone1); i++){
            int xi_idx = get_ckt1_pi(Abc_ObjId(Abc_NtkPi(cone1,i))); // sean: get the real index of xi in original AIG
            int aji = get_MI_var_id(yj_idx, xi_idx*2); // aji's variable id
            int bji = aji + 1;
            int aji_value = sat_solver_var_value(pSat,aji);
            int bji_value = sat_solver_var_value(pSat,bji);
            
            counter_ex_lit.push_back(Abc_Var2Lit(aji, aji_value)); // flip the value
            
            counter_mi_clause.push_back(make_pair(aji-MI_offset, aji_value)); // flip the value
            counter_mi_clause.push_back(make_pair(bji-MI_offset, bji_value)); // flip the value
        }
        // sean: deal the const0 and const1
    }

    // sean: add the counter example clause into the sat_solver
    if (counter_ex_lit.size() > 0){
        assert(sat_solver_addclause(pSat, &counter_ex_lit[0], &counter_ex_lit[0] + counter_ex_lit.size()));
    }

    // sean: add the counter example clause into the CE_curr
    CE_curr.push_back(counter_mi_clause);
    
}


Cnf_Dat_t *MergeCnf(Cnf_Dat_t *miter_cnf1, Cnf_Dat_t *miter_cnf2) {
    if (!miter_cnf1 || !miter_cnf2) {
        printf("Error: One or both CNF data are NULL.\n");
        return NULL;
    }

    // sean: Create new cnf structure
    Cnf_Dat_t *merged_cnf = Cnf_DataAlloc(NULL, miter_cnf1->nVars + miter_cnf2->nVars, 
                                           miter_cnf1->nClauses + miter_cnf2->nClauses, 
                                           miter_cnf1->nLiterals + miter_cnf2->nLiterals);


    if (!merged_cnf) {
        printf("Error: Failed to allocate merged CNF.\n");
        return NULL;
    }

    //cout << "allocated and created a new CNF structure for merged_cnf. " << endl;
    
    // sean: merge two cnfs into one
    // sean: lift the variable index of the second CNF
    // sean: so that the variable index of the second CNF is after the first CNF
    Cnf_DataLift(miter_cnf2, miter_cnf1->nVars);

    // sean: below method refer to the Cnf_DataDup function in abc
    // sean: observation: only allocate memory for the pClauses[0], according to the Cnf_DataAlloc function

    // sean: pClauses[0] of the Cnf_Dat_t type is the sum of all literals in the CNF
    // sean: in this case, all literals in the merged CNF is the sum of all literals in the first CNF and the second CNF
    memcpy( merged_cnf->pClauses[0], miter_cnf1->pClauses[0], sizeof(int) * miter_cnf1->nLiterals );
    memcpy( merged_cnf->pClauses[0] + miter_cnf1->nLiterals, miter_cnf2->pClauses[0], sizeof(int) * miter_cnf2->nLiterals );
    
    // sean: copy the clauses of the first CNF to the merged CNF
    for (int i = 1; i < miter_cnf1->nClauses; i++) {
        // sean: the reason to add miter_cnf2->nLiterals is to skip the first clause, which is the sum of all literals
        merged_cnf->pClauses[i] = merged_cnf->pClauses[0] + (miter_cnf1->pClauses[i] - miter_cnf1->pClauses[0]);
    }

    // sean: copy the clauses of the second CNF to the merged CNF
    for (int i = 1; i < miter_cnf2->nClauses; i++) {
        merged_cnf->pClauses[i + miter_cnf1->nClauses] = merged_cnf->pClauses[0] + miter_cnf1->nLiterals + (miter_cnf2->pClauses[i] - miter_cnf2->pClauses[0]);
    }

    return merged_cnf;
}