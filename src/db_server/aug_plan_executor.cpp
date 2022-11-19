#include "aug_plan_executor.h"

namespace db::exe{
    AugPlanExecutor::AugPlanExecutor(){
    }

    AugPlanExecutor::~AugPlanExecutor(){
    }

    char* AugPlanExecutor::execute(AugmentedPlan aug_plan){
        char* msg0=send_site_message(0,"order","0,1,select * from publisher");
        char* msg1=send_site_message(0,"order","1,2,select * from publisher");
        char* msg2=send_site_message(0,"order","2,3,select * from publisher");
        char* msg3=send_site_message(0,"order","3,-1,select * from publisher union all select * from node0 union all select * from node1 union all select * from node2");
        return msg3;
    }

}