#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "opt/sim/sim.h"
#include "proof/int/intInt.h"
#include <iostream>
#include<list>
#include<vector>
#include<map>
#include<unordered_map>
#include<set>
#include<algorithm>
#include<bitset>
#include "INPUT_SOLVER.h"  // 假設你的 header 是這樣命名

#include<stack>
#include "INOUT.h"


pair<Abc_Obj_t*,Abc_Obj_t*> INPUT_SOLVER::output_solver(vector<pair<Abc_Obj_t*,Abc_Obj_t*>>& R) 
{
    if(initial)
    {


    int circuitPo1  = Abc_NtkPoNum(origin_cir1);
    int circuitPo2  = Abc_NtkPoNum(origin_cir2); 

    MO = vector<vector<bool>>(circuitPo2, vector<bool>(circuitPo1 * 2, true));

    // 建 MO: 大小= Po2_list.size() x (Po1_list.size()*2)

    Abc_Obj_t* pObj1;
    int i1;
    Abc_NtkForEachPo(origin_cir1, pObj1, i1) {
        Po1_list.push_back(pObj1);
        std::string objName = Abc_ObjName(pObj1);
         Out1toPTR[objName] = pObj1;
        cout<<"test funct support circuit1: "<<Abc_ObjName(pObj1)<<" "<<this->funSupport1map[Abc_ObjName(pObj1)].size()<<endl;;
        Po1_map[objName] = pObj1;
    }

    Abc_Obj_t* pObj2;
    int i2;
    Abc_NtkForEachPo(origin_cir2, pObj2, i2) {
        Po2_list.push_back(pObj2);
        std::string objName = Abc_ObjName(pObj2);
        Out2toPTR[objName] = pObj2;
        cout<<"test funct support circuit2: "<<Abc_ObjName(pObj2)<<" "<<this->funSupport2map[Abc_ObjName(pObj2)].size()<<endl;;
        Po2_map[objName] = pObj2; 
    }




    for (const auto& bus : bus1_vec) 
    {
        if (bus.empty()) 
        {
            continue;
        }

        const std::string& firstElement = bus[0]; // 取得第一個元素
        if (Po1_map.find(firstElement) != Po1_map.end()) 
        {
           Outbus1_vec.push_back(bus);
        } 
        else 
        {
           Inbus1_vec.push_back(bus);
        }
    }
    for (const auto& bus : bus2_vec) 
    {
        if (bus.empty()) 
        {
            continue;
        }

        const std::string& firstElement = bus[0]; // 取得第一個元素
        if (Po2_map.find(firstElement) != Po2_map.end()) 
        {
           Outbus2_vec.push_back(bus);
        } 
        else 
        {
           Inbus2_vec.push_back(bus);
        }
    }




    

    printf("Outputs from Network 1:\n");
    for (auto pObj : Po1_list) {
        printf("Output: %s\n", Abc_ObjName(pObj));
    }

    printf("Outputs from Network 2:\n");
    for (auto pObj : Po2_list) {
        printf("Output: %s\n", Abc_ObjName(pObj));
    }

   std::sort(Po1_list.begin(), Po1_list.end(), [&](Abc_Obj_t* pObjA, Abc_Obj_t* pObjB) {
    int supportSizeA = this->funSupport1map[Abc_ObjName(pObjA)].size();
    int supportSizeB = this->funSupport1map[Abc_ObjName(pObjB)].size();

    return supportSizeA < supportSizeB;
    });
    std::sort(Po2_list.begin(), Po2_list.end(), [&](Abc_Obj_t* pObjA, Abc_Obj_t* pObjB) {
    int supportSizeA =  this->funSupport2map[Abc_ObjName(pObjA)].size();
    int supportSizeB =  this->funSupport2map[Abc_ObjName(pObjB)].size();

    return supportSizeA < supportSizeB;
    });


    for(auto & Bus1inner : Outbus1_vec)
    {
        std::sort(Bus1inner.begin(), Bus1inner.end(), [&](string pObjA, string pObjB) {
        int supportSizeA = this->funSupport1map[pObjA].size();
        int supportSizeB = this->funSupport1map[pObjB].size();

        return supportSizeA < supportSizeB;
        });
    }
    
    for(auto & Bus2inner : Outbus2_vec)
    {
        std::sort(Bus2inner.begin(), Bus2inner.end(), [&](string pObjA, string pObjB) {
        int supportSizeA = this->funSupport1map[pObjA].size();
        int supportSizeB = this->funSupport1map[pObjB].size();

        return supportSizeA < supportSizeB;
        });
    }


    if(BusGroup_enable)
    {
        cout<<"Before ungroup Bus size: "<< Outbus1_vec.size() << " and "<<Outbus2_vec.size()<<endl;
        for (int i = 0; i < Outbus1_vec.size(); ++i)
        {
        cout<<"OUTER: "<<i<<endl;
        for (int j = 0; j < Outbus2_vec.size(); ++j)
        {
            if (Outbus1_vec[i].size() == Outbus2_vec[j].size())
            {
                NewGroup1_vec.clear();
                NewGroup2_vec.clear();
                generateOutputGroups(Outbus1_vec[i], Outbus2_vec[j], NewGroup1_vec, NewGroup2_vec);
                cout<<"inner new group1 size: "<<NewGroup1_vec.size()<<endl; 
                cout<<"inner new group2 size: "<<NewGroup2_vec.size()<<endl; 

                Outbus1_vec.erase(Outbus1_vec.begin() + i);
                Outbus2_vec.erase(Outbus2_vec.begin() + j);

                Outbus1_vec.insert(Outbus1_vec.begin() + i, NewGroup1_vec.begin(), NewGroup1_vec.end());
                Outbus2_vec.insert(Outbus2_vec.begin() + j, NewGroup2_vec.begin(), NewGroup2_vec.end());

                i += NewGroup1_vec.size() - 1;
                j += NewGroup2_vec.size() - 1;
            }
        }
        }      
        cout<<"After ungroup Bus size: "<< Outbus1_vec.size() << " and "<<Outbus2_vec.size()<<endl;
    }


        for(int i = 0 ; i < Inbus1_vec.size() ; i++){
            for(auto& name : Inbus1_vec[i]){
                Bus1Mapint[name] = i;
            }
        }
        for(int i = 0 ; i < Inbus2_vec.size() ; i++){
            for(const auto& name : Inbus2_vec[i]){
                Bus2Mapint[name] = i;
            }
        }
        for(int i = 0 ; i < Outbus1_vec.size() ; i++){
            for(const auto& name : Outbus1_vec[i]){
                cout<<"O1 No"<<i<<" :"<<name<<endl;
                Bus1Mapint[name] = i;
            }
        }
        for(int i = 0 ; i < Outbus2_vec.size() ; i++){
            for(const auto& name : Outbus2_vec[i]){
                cout<<"O2 No"<<i<<" :"<<name<<endl;
                Bus2Mapint[name] = i;
            }
        }


        //bus 檢查 (若有需要)
        if (enableOutputBus) 
        {
            cout<<"enableOutputBus success"<<endl;
            for (int i2 = 0; i2 < (int)Po2_list.size(); i2++) {
                for (int q = 0; q < (int)Po1_list.size() * 2; q += 2) {
                    auto name1 = string(Abc_ObjName(Po1_list[q/2]));
                    auto name2 = string(Abc_ObjName(Po2_list[i2]));

                    bool cir1InBus = (Bus1Mapint.find(name1) != Bus1Mapint.end());
                    bool cir2InBus = (Bus2Mapint.find(name2) != Bus2Mapint.end());
                    if (cir1InBus && cir2InBus) {
                        // 簡易示範: 若 bus id 不同 => prune
                        int busId1 = Bus1Mapint[name1];
                        int busId2 = Bus2Mapint[name2];
                        // 若 bus 大小不合, 就禁止
                        // (真實程式可另外存vector<bus>來看大小, 這裡略)
                        if (Outbus1_vec[busId1].size() != Outbus2_vec[busId2].size()) 
                        {
                            cout<<name1 << " & "<<name2<<"can't match"<<endl;
                            MO[i2][q]   = false;
                            MO[i2][q+1] = false;
                        }
                    }
                    else {
                        // 只有一方在 bus => prune
                        if (cir1InBus ^ cir2InBus) {
                            cout<<name1 << " & "<<name2<<"can't match"<<endl;
                            MO[i2][q]   = false;
                            MO[i2][q+1] = false;
                        }
                    }
                }
            }
        }

        // cir1Choose, cir2Choose
        cir1Choose.assign(Po1_list.size(), 0);
        cir2Choose.assign(Po2_list.size(), -1);

        initial = false;

        return output_solver(R);
    }
    else
    {
        if (lastResult == 0)
        {
            cout<<"---------RECOVER---------"<<endl;
            cout<<"RECOVER PAIR: "<<Abc_ObjName(lastPair.first)<<" "<< Abc_ObjName(lastPair.second)<<endl;
            if (lastPair.first != nullptr && lastPair.second != nullptr)
            {
                MO[last_i2][last_q]   = false;
                MO[last_i2][last_q+1]   = false;
                // 找出它們在 Po1_list, Po2_list 對應的 index
                auto it1 = std::find(Po1_list.begin(), Po1_list.end(), lastPair.first);
                auto it2 = std::find(Po2_list.begin(), Po2_list.end(), lastPair.second);

                if (it1 != Po1_list.end())
                {
                    int idx1 = std::distance(Po1_list.begin(), it1);
                    cir1Choose[idx1] = 0;
                }

                if (it2 != Po2_list.end())
                {
                    int idx2 = std::distance(Po2_list.begin(), it2);
                    
                    cout<<"Recover i2"<<idx2<<endl;
                    cir2Choose[idx2] = -1;
                }
                if (enableOutputBus)
                {
                    const std::string &name1 = Abc_ObjName(lastPair.first);
                    const std::string &name2 = Abc_ObjName(lastPair.second);
                    if (Bus1Mapint.find(name1) != Bus1Mapint.end() &&
                        Bus2Mapint.find(name2) != Bus2Mapint.end())
                    {
                        int busId1 = Bus1Mapint[name1];
                        int busId2 = Bus2Mapint[name2];
                        auto itBus1 = cir1OutputBusMatch.find(busId1);
                        if (itBus1 != cir1OutputBusMatch.end() && itBus1->second == busId2)
                        {
                            cir1OutputBusMatch.erase(itBus1);
                        }
                        auto itBus2 = cir2OutputBusMatch.find(busId2);
                        if (itBus2 != cir2OutputBusMatch.end() && itBus2->second == busId1)
                        {
                            cir2OutputBusMatch.erase(itBus2);
                        }
                    }
                }
                auto itR = std::find(R.begin(), R.end(), lastPair);
                if (itR != R.end())
                {
                    R.erase(itR);
                }
                lastPair = {nullptr, nullptr};
            }

        }
        else
        {
            cout<<"Last Round Success"<<endl;
            cout<<"R size:"<<R.size()<<endl;
        }


        // 先把目前 R 裏的 CurrentPair 收集到 nowSelect
        std::set<pair<Abc_Obj_t*,Abc_Obj_t*>> nowSelect;
        for (auto & select : R) {
            nowSelect.insert(select);
        }
        
            cout<<"Now select: "<<nowSelect.size()<<endl;
        // 試著找一個 cir2 的輸出(尚未匹配) -> cir1 的輸出
        for (int i2 = 0; i2 <Po2_list.size(); i2++) 
        {
            if (cir2Choose[i2] == -1) 
            {
                
                cout<<"new select i2"<<i2<<endl;
                cout<<"Po2_list's size: "<<Po2_list.size()<<endl;
                // cir2 第 i2 個輸出尚未匹配
                for (int q = 0; q < Po1_list.size()*2; q++) {
                    int idx1 = q / 2;
                    bool isPosPhase = (q % 2 == 0);
                    if ( cir1Choose[idx1] != 0) 
                    {
                        continue;
                    }
                    if (!MO[i2][q]) {
                        continue;
                    }
                    std::string out1Name = Abc_ObjName(Po1_list[idx1]);
                    std::string out2Name = Abc_ObjName(Po2_list[i2]);
                    pair<Abc_Obj_t*,Abc_Obj_t*> re(Po1_list[idx1], Po2_list[i2]);

                    if (nowSelect.find(re) != nowSelect.end()) {
                        continue;
                    }

                    if (enableOutputBus) 
                    {
                        bool busConflict = 0;
                        auto name1 = Abc_ObjName(re.first);
                        auto name2 = Abc_ObjName(re.second);
                        if (Bus1Mapint.find(name1) == Bus1Mapint.end() ||
                            Bus2Mapint.find(name2) == Bus2Mapint.end()) 
                        {
                            busConflict = 0; // false
                        }

                        int busId1 = Bus1Mapint[name1];
                        int busId2 = Bus2Mapint[name2];
                        if (cir1OutputBusMatch.find(busId1) != cir1OutputBusMatch.end()) {
                            if (cir1OutputBusMatch[busId1] != busId2) {
                                busConflict = 1;
                            }
                        }
                        if (cir2OutputBusMatch.find(busId2) != cir2OutputBusMatch.end()) {
                            if (cir2OutputBusMatch[busId2] != busId1) {
                                busConflict = 1;
                            }
                        }

                        if (busConflict) {
                            continue;
                        }
                    }

                    nowSelect.insert(re);
                    auto name1 = Abc_ObjName(re.first);
                    auto name2 = Abc_ObjName(re.second);
                    if(Bus1Mapint.find(name1) != Bus1Mapint.end() && Bus2Mapint.find(name2) != Bus2Mapint.end()){
                            cir1OutputBusMatch[Bus1Mapint[name1]] = Bus2Mapint[name2];
                            cir2OutputBusMatch[Bus2Mapint[name2]] = Bus1Mapint[name1];
                    }

                    cir1Choose[idx1]++;
                    cir2Choose[i2] = q;
                    cout<<"New Chose: "<<name1<< " "<<name2<<endl;
                    last_i2 = i2;
                    last_q = q;
                    return re;
                }
            }
        }

        return pair<Abc_Obj_t*,Abc_Obj_t*>(NULL,NULL);
        }
}

vector<int> INPUT_SOLVER::generateOutputGroups(
    vector<string> bus1,
    vector<string> bus2,
    vector<vector<string>> &newGroup1,
    vector<vector<string>> &newGroup2
) 
{
    newGroup1.clear();
    newGroup2.clear();
    newGroup1.emplace_back(); 
    newGroup2.emplace_back();

    int n = (int)bus1.size();
    for (int i = 0; i < n; i++)
    {
        int k = n - 1 - i;
        std::cout << "i: " << i << "   => k: " << k << std::endl;
        newGroup1.back().push_back(bus1[k]);
        newGroup2.back().push_back(bus2[k]);
        if (k > 0)
        {
            if (funSupport1map[ bus1[k] ].size() > funSupport2map[ bus2[k - 1] ].size())
            {
                cout<<"Divide"<<endl;
                newGroup1.emplace_back();
                newGroup2.emplace_back();
            }
        }
    }
    return {};
}