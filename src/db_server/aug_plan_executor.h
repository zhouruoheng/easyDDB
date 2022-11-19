#include "aug_planner.h"

namespace db::exe{
    class AugPlanExecutor{
    public:
        AugPlanExecutor();
        ~AugPlanExecutor();
        char* execute(AugmentedPlan aug_plan);
        char* send_site_message(int site_id,char* msg_type, char* msg);
        
    };

}
