#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "proof/int/intInt.h"
#include <iostream>
#include <fstream>
#include<list>
#include<vector>
#include<map>
#include<unordered_map>
#include<set>
#include<algorithm>
#include<bitset>
#include "INPUT_SOLVER.h"
#include<string>
#include "INOUT.h"


using namespace std;

extern "C" Aig_Man_t * Abc_NtkToDar( Abc_Ntk_t * pNtk, int fExors, int fRegisters );

static int read_two_aig(Abc_Frame_t* pAbc, int argc, char** argv);

void getBus(char** argv, vector<vector<string>>& bus1, vector<vector<string>>& bus2);

void init_final(Abc_Frame_t* pAbc) {
  Cmd_CommandAdd(pAbc, "LSV", "read_two_aig", read_two_aig, 0);
}

void destroy_final(Abc_Frame_t* pAbc) {}

Abc_FrameInitializer_t frame_initializer_final = {init_final, destroy_final};

struct lsvPackageRegistrationManager_final
{
    lsvPackageRegistrationManager_final() { Abc_FrameAddInitializer(&frame_initializer_final); }
} lsvPackageRegistrationManager_final;

string extract_case_name(const string& path) 
{
    size_t start_pos = path.find("case");
    if (start_pos != string::npos) {
        size_t end_pos = path.find('/', start_pos);
        return path.substr(start_pos, end_pos - start_pos);
    }
    return "unknown_case";
}



void append_to_csv(const string& filepath, const string& case_path, int enableOutputBus, int BusGroup_enable, int matched_num) {
    ofstream file;
    bool file_exists = ifstream(filepath).good();
    file.open(filepath, ios::app);
    if (!file_exists) { 
        file << "Case Path,Enable Output Bus,Bus Group Enable,Matched Number" << endl;
    }
     string case_name = extract_case_name(case_path);
    file << case_name << "," << enableOutputBus << "," << BusGroup_enable << "," << matched_num << endl;

    file.close();
}



int read_two_aig(Abc_Frame_t* pAbc, int argc, char** argv) 
{
    if ( argc != 6 )
    {
        Abc_Print( -2, "Usage: tworead <file1.aig> <file2.aig> <input> <enableOutputBus> <BusGroup_enable> \n" );
        return 1; // 1 = 執行失敗
    }


    int matched_num = 0;
    vector<Abc_Ntk_t *> AIGs = get_AIGs(pAbc,argv);

    INPUT_SOLVER input_solver(AIGs[0],AIGs[1]);
    vector<pair<Abc_Obj_t*,Abc_Obj_t*>> storePair;
    input_solver.initial =  true; 
    input_solver.enableOutputBus = stoi(argv[4]);
    if(stoi(argv[4]))
    {
        cout<<"enableOutputBus success!"<<endl;
    }
    else
    {
        cout<<"enableOutputBus zzz"<<endl;
    }
    input_solver.BusGroup_enable = stoi(argv[5]);
    if(stoi(argv[5]))
    {
        cout<<"BusGroup_enable success!"<<endl;
    }
    else
    {
        cout<<"BusGroup_unable zzz"<<endl;
    }
    input_solver.funSupport1map = funSupport(AIGs[0]);
    input_solver.funSupport2map = funSupport(AIGs[1]);
    input_solver.strSupport1map = strSupport(AIGs[0]);
    input_solver.strSupport2map = strSupport(AIGs[1]); //要store 這個map
    
     getBus(argv,input_solver.bus1_vec , input_solver.bus2_vec);
     set< pair<Abc_Obj_t*,Abc_Obj_t*>> record;



     vector<pair<Abc_Obj_t*,Abc_Obj_t*> > output_pairs;

    input_solver.lastResult = 1;
    double start = clock();
    for(int i=0 ; i<10000000;i++)
    {
        if(clock()-start > 3600.0*1000000)
        {
            cout<<"Time out"<<endl;
            break;
        }
        cout<<"Current iteration"<<i<<endl;
        pair<Abc_Obj_t*,Abc_Obj_t*> test = input_solver.output_solver(storePair); // find new PO pair to input_solver
        if(record.find(test)!= record.end()) // detect repeat pattern
        {
            cout<<"ERROR"<<endl;
        }

        if (test.first == nullptr || test.second == nullptr){
            cout << "early stop at iteration " << i << endl;
            break;
        }
        input_solver.gen_test_pair({test.first->Id, test.second->Id});
        input_solver.solver_setup();

        cout<<"READY To SOLVE"<<endl;  
        if(input_solver.solve() == 1) // SAT
        {
            
            input_solver.lastResult = 1;
            matched_num++;
            cout<<"result:"<<endl;
            // input_solver.print();
            output_pairs.push_back(test);
        }
        else
        {
            input_solver.lastPair = test;
            input_solver.lastResult = 0;
            input_solver.gen_prev_pair({test.first->Id, test.second->Id});
        }
        input_solver.sat_clean_up();


        record.insert(test);
        cout<<"test obj1: "<< Abc_ObjName(test.first)<<endl;
        cout<<"test obj2: "<< Abc_ObjName(test.second)<<endl;

    }
    cout<<"bus1:";
    for(auto test_bus1 : input_solver.bus1_vec) 
    {
        cout<<" ";
        for(auto test_bus1_inner : test_bus1) 
        {
            cout<<test_bus1_inner<<" ";
        }
        cout<<endl;
    }
    cout<<"bus2:";
    for(auto test_bus2 : input_solver.bus2_vec) 
    {
        cout<<" ";
        for(auto test_bus2_inner : test_bus2) 
        {
            cout<<test_bus2_inner<<" ";
        }
        cout<<endl;
    }
    cout << "\nmatched number: " << matched_num << endl;

    string out_data_name = extract_case_name(argv[2]); 
    string csv_path = out_data_name + "_results.csv";
    append_to_csv(csv_path, argv[2], stoi(argv[4]), stoi(argv[5]), matched_num);

    return 0;
}