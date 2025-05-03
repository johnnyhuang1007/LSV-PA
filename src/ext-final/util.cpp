#include "INPUT_SOLVER.h"

using namespace std;

void INPUT_SOLVER::print()
{
    cout<<sat_solver_nvars(pSat)<<endl;
    cout<<"CIR1 RES:"<<endl;
    for(int i = 0 ; i < cir2_offset ; i++)
    {
        cout<<sat_solver_var_value(pSat,i)<<"   ";
    }
    cout<<endl;
    Abc_Obj_t * pObj;
    int i ;
    cout<<Abc_NtkCiNum(cone1)<<endl;
    Abc_NtkForEachCi(cone1, pObj, i)
    {
        cout<<Abc_ObjName(pObj)<<" "<<cir1_cnf->pVarNums[Abc_ObjId(pObj)] <<endl;
    }
    cout<<endl<<endl;

    cout<<"CIR1 PO:"<<endl;
    Aig_Obj_t * obj;
    Aig_ManForEachCi(cone1_aig, obj, i)
    {
        if(cir1_cnf->pVarNums[Aig_ObjId(obj)]!=-1)
            cout<<Aig_ObjId(obj)<<" "<<sat_solver_var_value(pSat,cir1_cnf->pVarNums[Aig_ObjId(obj)])<<"   "<<endl;
    }
    cout<<endl;
    Aig_ManForEachNode(cone1_aig, obj, i)
    {
        if(cir1_cnf->pVarNums[Aig_ObjId(obj)]!=-1)
            cout<<Aig_ObjId(obj)<<" "<<sat_solver_var_value(pSat,cir1_cnf->pVarNums[Aig_ObjId(obj)])<<"   "<<endl;
    }
    cout<<endl;
    Aig_ManForEachCo(cone1_aig, obj, i)
    {
        if(cir1_cnf->pVarNums[Aig_ObjId(obj)]!=-1)
            cout<<Aig_ObjId(obj)<<" "<<sat_solver_var_value(pSat,cir1_cnf->pVarNums[Aig_ObjId(obj)])<<"   ";
    }
    cout<<endl<<endl;

    cout<<"CIR2 RES:"<<endl;
    for(int i = cir2_offset ; i < miter_po_offset ; i++)
    {
        cout<<sat_solver_var_value(pSat,i)<<"   ";
    }
    cout<<endl<<endl;

    cout<<"CIR2 PO:"<<endl;
    Aig_ManForEachCi(cone2_aig, obj, i)
    {
        if(cir2_cnf->pVarNums[Aig_ObjId(obj)]!=-1)
            cout<<Aig_ObjId(obj)<<" "<<sat_solver_var_value(pSat,cir2_cnf->pVarNums[Aig_ObjId(obj)])<<"   "<<endl;
    }
    cout<<endl;
    Aig_ManForEachNode(cone2_aig, obj, i)
    {
        if(cir2_cnf->pVarNums[Aig_ObjId(obj)]!=-1)
            cout<<Aig_ObjId(obj)<<" "<<sat_solver_var_value(pSat,cir2_cnf->pVarNums[Aig_ObjId(obj)])<<"   "<<endl;
    }
    cout<<endl;
    Aig_ManForEachCo(cone2_aig, obj, i)
    {
        if(cir2_cnf->pVarNums[Aig_ObjId(obj)]!=-1)
            cout<<Aig_ObjId(obj)<<" "<<sat_solver_var_value(pSat,cir2_cnf->pVarNums[Aig_ObjId(obj)])<<"   "<<endl;
    }
    cout<<endl<<endl;

    cout<<"MITER RES"<<endl;
    for(int i = miter_po_offset ; i < MI_offset ; i++)
    {
        cout<<sat_solver_var_value(pSat,i)<<"   ";
    }
    cout<<endl<<endl;

    cout<<"MI:"<<endl;
    for(int i = MI_offset ; i < MO_offset ; i++)
    {
        cout<<sat_solver_var_value(pSat,i)<<"   ";
        if(!((1+i-MI_offset)%MI[0].size()))
            cout<<endl;
    }
    cout<<endl<<endl;

    cout<<"MO:"<<endl;
    for(int i = MO_offset ; i <sat_solver_nvars(pSat) ; i++)
    {
        cout<<sat_solver_var_value(pSat,i)<<"   ";
        if(!((1+i-MO_offset)%MO[0].size()))
            cout<<endl;
    }
    cout<<endl<<endl;

    cout<<"OFFSET:"<<endl;
    cout<<cir1_offset<<endl;
    cout<<cir2_offset<<endl;
    cout<<miter_po_offset<<endl;
    cout<<MI_offset<<endl;
    cout<<MO_offset<<endl;
    cout<<sat_solver_nvars(pSat)<<endl;
}