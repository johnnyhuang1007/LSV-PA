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
using namespace std;

unordered_map<string, set<string>> funSupport(Abc_Ntk_t * mainNtk);

unordered_map<string, set<string>> strSupport(Abc_Ntk_t * mainNtk) ;

vector<Abc_Ntk_t *> get_AIGs(Abc_Frame_t* pAbc,char** argv);
