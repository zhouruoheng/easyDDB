#include <iostream>
#include "query_optimizer.h"
#include "aug_planner.h"
#include "aug_plan_executor.h"
#include "site_task_executor.h"
#include <bits/stdc++.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "executor.h"



char* db::exe::sql_execute(const std::string& msg) {
    db::opt::Tree query_tree = db::opt::build_query_tree();
    db::exe::AugmentedPlan aug_plan = db::exe::build_augmented_plan(query_tree);
    char* result=db::exe::AugPlanExecutor::execute(aug_plan);
    return result;
} 

char* db::exe::config(const std::string& msg) {
    return "config not written";
}


