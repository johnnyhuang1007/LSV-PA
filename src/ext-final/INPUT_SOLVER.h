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
#include<set>
#include<stack>
#include<algorithm>
#include<bitset>
#include <sstream>
using namespace std;
extern "C" Aig_Man_t * Abc_NtkToDar( Abc_Ntk_t * pNtk, int fExors, int fRegisters );

// sean: limit 2D vector
// sean: use for storing cnf of counter examples
class CE_Storage{
    public:
        CE_Storage():size(0),cur_idx(-1),limit_size(-1){}
        CE_Storage(int l);
        void push_back(vector<pair<int, int>> v);
        vector<pair<int, int>>::const_iterator get_clauses(int idx);
        void clear();
        void merge(CE_Storage& other);
        // operator overloading [] for accessing the data
        vector<pair<int, int>>& operator[](int idx);
        int get_size(){return size;}
        
    private:
        vector<vector<pair<int, int>>> data; // sean: the pair<int, int> store <location in MI, complement of not>. 1 means complement, 0 means not complement
        int size; // current size
        int cur_idx; // current index
        int limit_size; // limit size
};

class INPUT_SOLVER
{
    private:
        //SAT
        sat_solver_t * pSat;
        
        

        //AIGS
        Abc_Ntk_t * cone1;
        Abc_Ntk_t * cone2;

        Aig_Man_t * cone1_aig;
        Aig_Man_t * cone2_aig;

        Abc_Ntk_t * origin_cir1;
        Abc_Ntk_t * origin_cir2;    //circuit from circuit.v.aig
        vector<int> output1_idxs;   //might be phase changed
        vector<int> output2_idxs;   //o[i] = j : i:the idx of cone, j: the idx of cir
        vector<int> rev;
        
        Abc_Ntk_t * miter;
        Cnf_Dat_t * miter_Cnf;
        Cnf_Dat_t * pCnf;


        //CNF UPDATED
        Cnf_Dat_t* cir1_cnf;
        Cnf_Dat_t* cir2_cnf;
        Cnf_Dat_t* xors_cnf;
        Cnf_Dat_t* MI_cnf;
        Cnf_Dat_t* MO_cnf;
        

        //CNF
        vector<vector<bool>> MI;
        vector<vector<bool>> MO;

        CE_Storage CE_cnf; // sean: counter example cnf of all history
        CE_Storage CE_curr; // sean: current counter example cnf
        
        int cir1_offset;
        int cir2_offset;
        int miter_po_offset;
        int MI_offset;
        int MO_offset;



        stack<pair<Abc_Obj_t*,Abc_Obj_t*>> backtrace;
        vector<int> hGroupId;

        vector<int> cir1Choose; // how many number be chosen by cir2 port
        vector<int> cir2Choose; // choose which cir1 port
        vector<Abc_Obj_t*> Po1_list;
        vector<Abc_Obj_t*> Po2_list;
        map<std::string, Abc_Obj_t*> Po1_map;
        map<std::string, Abc_Obj_t*> Po2_map;
        map<std::string, Abc_Obj_t*> Pi1_map;
        map<std::string, Abc_Obj_t*> Pi2_map;
        unordered_map<string, int> cir1OutputMap, cir2OutputMap;

        
        set<size_t> forbid;

    public:


        INPUT_SOLVER()
        {
            pSat = sat_solver_new();
        }
        INPUT_SOLVER(Abc_Ntk_t * AIG1, Abc_Ntk_t * AIG2);
        ~INPUT_SOLVER()
        {
            sat_solver_delete(pSat);
            Cnf_DataFree(pCnf);
            Aig_ManStop(cone1_aig);
            Aig_ManStop(cone2_aig);
            Abc_NtkDelete(miter);
            Abc_NtkDelete(cone1);
            Abc_NtkDelete(cone2);
        }

        
        bool solver_setup()
        {
            cout<<"SETUP_SOLVER"<<endl;
            gen_miter_cnf();
            cout<<"MITER_CONSTRUCTED"<<endl;
            gen_constraints_cnf();
            // add_counter_ex_cnf();
            // set_miter_cnf_out(0);
            return 1;
        }

        // original version
        // int solve()
        // {
        //     cout<<"SOLVING"<<endl;
        //     cout<<sat_solver_nvars(pSat)<<endl;
        //     int result = sat_solver_solve(pSat, NULL, NULL, 0, 0, 0, 0);
        //     cout<<"RESULT: "<<result<<endl;
        //     return result;
        // }

        // // huang
        // int solve(int miter_out){
        //     lit Lits[1];
        //     Lits[0] = get_miter_out_lit(miter_out);
        //     cout << "SOLVING" << endl;
        //     int result = sat_solver_solve(pSat, Lits, Lits + 1, 0, 0, 0, 0);
        //     cout << "RESULT: " << result << endl;
        //     return result;
        // }

        // sean modified version
        // sean: solve the sat solver and deal with the counter example
        int solve()
        {
            cout<<"SOLVING"<<endl;
            
            CE_curr.clear();
            
            lit Lits[1];
            Lits[0] = get_miter_out_lit(1);

            int fail_time = 0;
            cout << "SAT solving lookin for counter ex" << endl;
            while (sat_solver_solve(pSat, Lits, Lits + 1, 0, 0, 0, 0) == 1){
                // if SAT => add a new constraint to exclude the current solution
                
                gen_counter_ex_cnf();
                if(++fail_time == 10*MO.size()*MO[0].size())
                {
                    sat_clean_up();
                    gen_pair();
                    reverse_MO_phase(output1_idxs.back(),output2_idxs.back());
                    solver_setup();
                }
                if(fail_time == 20*MO.size()*MO[0].size())
                    return -1;
            }
            //cout<<"MATCHED"<<endl;
            Lits[0] = get_miter_out_lit(0);
            int result = sat_solver_solve(pSat, Lits, Lits + 1, 0, 0, 0, 0);
            if (result == 1){
                // sat => match found
                // add the counter example into the CE_cnf
            //    CE_cnf.merge(CE_curr);
            }
            //cout<<"RESULT: "<<result<<endl;
            return result;
        }

        void print();

        bool sat_clean_up()
        {
            sat_solver_delete(pSat);
            pSat = sat_solver_new();
            return 1;
        }



        bool gen_test_pair(pair<int,int> test_pair);
        bool gen_prev_pair(pair<int,int> test_pair);
        bool gen_pair();
        bool Cir2_Phase_Change();
        bool reverse_MO_phase(int i,int j); //MO[j(cir2)][i(cir1)]

        bool gen_miter_cnf();   //consider Mo and invert certain output, update miter
        bool set_miter_cnf_out(int);   //1 for equvilant under phase assignment
        //get the lit of miter output
        lit  get_miter_out_lit(int miter_out){
            return Abc_Var2Lit(MI_offset - 1, !miter_out);
        }


        int AddMiterCnf(Cnf_Dat_t *miter_cnf1, Cnf_Dat_t *miter_cnf2);
        void gen_constraints_cnf(); // generate MI connection and constraints and add into sat solver
        void gen_counter_ex_cnf(); // generate counter example cnf and add into sat solver

        int get_ckt1_pi(int idx)
        {
        
            Abc_Obj_t* node_from_cone1 = Abc_NtkObj(cone1,idx);
            Abc_Obj_t* org_node = Abc_NtkFindCi(origin_cir1, Abc_ObjName(node_from_cone1));
            return Abc_ObjId(org_node)-1;
            // TODO: return the real index of PI in original AIG1
        }

        int get_ckt2_pi(int idx)
        {
            Abc_Obj_t* node_from_cone2 = Abc_NtkObj(cone2,idx);
            Abc_Obj_t* org_node = Abc_NtkFindCi(origin_cir2, Abc_ObjName(node_from_cone2));
            return Abc_ObjId(org_node)-1;
            // TODO: return the real index of PI in original AIG2
        }
        
        int get_MI_var_id(int i, int j)
        {
            // return variable id of MI[i][j]
            return MI_offset + MI[i].size() * i + j;
        }

        int cir1_var_id(int idx) {return cir1_cnf->pVarNums[idx];}
        int cir2_var_id(int idx){return cir2_cnf->pVarNums[idx];}
        int miter_var_id(int idx){return miter_po_offset + idx;}
        int MI_var_id(int i, int j){return MI_offset + i*MI[0].size() + j;}
        int MO_var_id(int i, int j){return MO_offset + i*MO[0].size() + j;}
        
        pair<Abc_Obj_t*,Abc_Obj_t*> output_solver(vector<pair<Abc_Obj_t*,Abc_Obj_t*>>& R); 
        vector<int> generateOutputGroups(vector<string> f, vector<string> g ,  vector<vector<string>>& ,  vector<vector<string>> &);
        pair<Abc_Obj_t*,Abc_Obj_t*> initial_funct(vector<pair<Abc_Obj_t*,Abc_Obj_t*>>& R) ;
        
        unordered_map<string, set<string>> funSupport1map ;
        unordered_map<string, set<string>> funSupport2map ;
        unordered_map<string, set<string>> strSupport1map ;
        unordered_map<string, set<string>> strSupport2map ;
        vector<vector<string>> bus1_vec ; 
        vector<vector<string>> bus2_vec ; 
        vector<vector<string>> Outbus1_vec ; 
        vector<vector<string>> Outbus2_vec ; 
        vector<vector<string>> Inbus1_vec ; 
        vector<vector<string>> Inbus2_vec ; 
        unordered_map<int ,int> cir1OutputBusMatch, cir2OutputBusMatch;
        unordered_map<string, int> Bus1Mapint, Bus2Mapint;
        
        vector<vector<string>> NewGroup1_vec;
        vector<vector<string>> NewGroup2_vec;
        unordered_map<string,Abc_Obj_t*> Out1toPTR ; 
        unordered_map<string,Abc_Obj_t*> Out2toPTR ; 
        bool initial;
        bool enableOutputBus;
        bool BusGroup_enable ;
        
        bool lastResult ;
        int last_idx1 , last_i2 ,last_q;
        pair<Abc_Obj_t*,Abc_Obj_t*> lastPair ;

};

Cnf_Dat_t *MergeCnf(Cnf_Dat_t *miter_cnf1, Cnf_Dat_t *miter_cnf2);