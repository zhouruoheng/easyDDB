#include <server/select_executor.hpp>
#include <thread>
#include <iostream>

namespace server
{

    vector<int> AugmentedPlan::get_children_list(int node_id)
    {
        vector<int> children_list;
        for (int i = 0; i < augplannodes.size(); i++)
        {
            if (augplannodes[i].parent_id == node_id)
            {
                children_list.push_back(augplannodes[i].node_id);
            }
        }
        return children_list;
    }
    json AugmentedPlan::to_json()
    {
        json j;
        j["node_num"] = node_num;
        for (int i = 0; i < node_num; i++)
        {
            j[std::to_string(i)]["sql"] = augplannodes[i].sql;
            j[std::to_string(i)]["parent_id"] = augplannodes[i].parent_id;
            j[std::to_string(i)]["execute_site"] = augplannodes[i].execute_site;
            json columns;
            for (auto column : augplannodes[i].columns)
            {
                json col;
                col["name"] = column.name;
                col["type"] = column.type;
                columns.push_back(col);
            }
            j[std::to_string(i)]["columns"] = columns;
        }
        return j;
    }
    AugmentedPlan::AugmentedPlan(json planjson)
    {
        node_num = planjson["node_num"];
        for (int i = 0; i < node_num; i++)
        {
            AugPlanNode node;
            node.sql = planjson[std::to_string(i)]["sql"];
            node.node_id = i;
            node.parent_id = planjson[std::to_string(i)]["parent_id"];
            node.execute_site = planjson[std::to_string(i)]["execute_site"];
            json &columns = planjson[std::to_string(i)]["columns"];
            for (auto column : columns)
            {
                columnzy col;
                col.name = column["name"];
                col.type = column["type"];
                node.columns.push_back(col);
            }
        }
    }

    int AugmentedPlan::find_root_id()
    {
        for (int i = 0; i < augplannodes.size(); i++)
        {
            if (augplannodes[i].parent_id == -1)
            {
                return augplannodes[i].node_id;
            }
        }
        return -1;
    }}
//     SelectExecutor::SelectExecutor(hsql::SQLParserResult *result, SitesManager &sitesManager, std::string localsitename)
//         : manager(sitesManager), localsitename(localsitename)
//     {
//         db::opt::Tree queryTree;
//         vector<metadataTable> Tables = db::opt::getMetadata();
//         queryTree = db::opt::SelectProcess(Tables, result);
//         AugmentedPlan aug_plan = build_augmented_plan(queryTree);
//         this->plan = aug_plan;
//     }
//     SelectExecutor::SelectExecutor(SitesManager &sitesManager, std::string localsitename, AugmentedPlan plan)
//         : manager(sitesManager), localsitename(localsitename), plan(plan)
//     {
//     }
//     std::string SelectExecutor::execute()
//     {
//         save_plan_to_etcd(plan);
//         // read all site_name from manager.site_dict
//         std::vector<std::string> site_names;
//         for (auto it = manager.channelMap.begin(); it != manager.channelMap.end(); it++)
//         {
//             site_names.push_back(it->first);
//         }

//         // send plan to all site
//         for (int i = 0; i < site_names.size(); i++)
//         {
//             std::string site_name = site_names[i];
//             std::string msg = "plan";
//             // int pid = fork();
//             // if (pid == 0)
//             // {
//             //     manager.send_site_message("site_execute", site_name, msg);
//             //     exit(0);
//             // }

//             // create thread to run function manager.send_site_message("site_execute", site_name, msg);
//             std::thread t(&SitesManager::send_site_message, &manager, "site_execute", site_name, msg);
//             t.detach();
//         } // 发送所有的plan到所有的site，并直接返回ok
//         // now mtx is locked,wating for it to be unlocked
//         std::string result = get_result();
//         etcd_set("result", "");
//         etcd_set("node_num", "0");
//         return result;
//     }

//     // void save_plan_to_etcd(AugmentedPlan plan)
//     // {
//     //     std::string key = "node_num";
//     //     std::string value = std::to_string(plan.node_num);
//     //     etcd_set(key, value);
//     //     for (int i = 0; i < plan.node_num; i++)
//     //     {
//     //         key = std::to_string(i) + "parent_id";
//     //         value = std::to_string(plan.augplannodes[i].parent_id);
//     //         etcd_set(key, value);
//     //         key = std::to_string(i) + "execute_site";
//     //         value = plan.augplannodes[i].execute_site;
//     //         etcd_set(key, value);
//     //         key = std::to_string(i) + "sql";
//     //         value = plan.augplannodes[i].sql;
//     //         etcd_set(key, value);
//     //         key = std::to_string(i) + "save_sql";
//     //         value = plan.augplannodes[i].save_sql;
//     //         etcd_set(key, value);
//     //         key = std::to_string(i) + "is_finished";
//     //         value = std::to_string(plan.augplannodes[i].is_finished);
//     //         etcd_set(key, value);
//     //     }

//     // } // 将plan存到etcd里面
//     // AugmentedPlan get_plan_from_etcd()
//     // {
//     //     AugmentedPlan plan;
//     //     plan.node_num = stoi(etcd_get("plan_num"));
//     //     for (int i = 0; i < plan.node_num; i++)
//     //     {
//     //         AugPlanNode node;
//     //         node.node_id = i;
//     //         node.sql = etcd_get(std::to_string(i) + "sql");
//     //         node.save_sql = etcd_get(std::to_string(i) + "save_sql");
//     //         node.is_finished = stoi(etcd_get(std::to_string(i) + "is_finished"));
//     //         node.parent_id = stoi(etcd_get(std::to_string(i) + "parent_id"));
//     //         plan.augplannodes.push_back(node);
//     //     }
//     //     return plan;
//     // } // 从etcd里面获取plan

//     std::string SelectExecutor::get_result()
//     {

//         clock_t start = clock();
//         // create a semaphore to wait for all site finish

//         while (etcd_get("is_finished") != "1")
//         {
//             std::this_thread::sleep_for(std::chrono::milliseconds(100));
//         }
//         std::string result_flie_name = etcd_get("result");
//         // 读取result文件
//         std::ifstream in(result_flie_name);
//         std::string result;
//         std::string line;
//         while (getline(in, line))
//         {
//             result += line;
//         }
//         // 删除result文件
//         remove(result_flie_name.c_str());
//         clock_t end = clock();
//         int seconds = (end - start) / CLOCKS_PER_SEC;
//         std::cout << "total execution time:" << seconds << "s" << std::endl;
//         return result;
//     }

//     // std::string SelectExecutor::site_execute(std::string msg)
//     // {
//     //     return plan_scan();
//     // } // 每一个站点接受到执行指令之后执行一次plan_scan
//     // std::string SelectExecutor::plan_scan()
//     // {
//     //     AugmentedPlan plan = get_plan_from_etcd();
//     //     vector<int> to_execute;
//     //     for (int i = 0; i < plan.node_num; i++)
//     //     {
//     //         if (plan.augplannodes[i].is_finished == 0)
//     //         {
//     //             int child_num = 0;
//     //             int child_finished_num = 0;
//     //             for (int j = 0; j < plan.node_num; j++)
//     //             {
//     //                 if (plan.augplannodes[j].parent_id == i)
//     //                 {
//     //                     child_num++;
//     //                     if (plan.augplannodes[j].is_finished == 1)
//     //                     {
//     //                         child_finished_num++;
//     //                     }
//     //                 }
//     //             }
//     //             if (child_num == child_finished_num)
//     //             {
//     //                 if (plan.augplannodes[i].execute_site == localsitename)
//     //                 {
//     //                     to_execute.push_back(i);
//     //                 }
//     //             }
//     //         }
//     //     }
//     //     for (int i = 0; i < to_execute.size(); i++)
//     //     {
//     //         int node_id = to_execute[i];
//     //         if (plan.augplannodes[node_id].parent_id == -1)
//     //         {
//     //             data_push(node_id, plan.augplannodes[node_id].sql, localsitename, plan, 1); // r若为根节点，则结果直接输出即可
//     //         }
//     //         else
//     //         {
//     //             data_push(node_id, plan.augplannodes[node_id].sql, plan.augplannodes[plan.augplannodes[node_id].parent_id].execute_site, plan, 0); // 若不为根节点，则需要将结果发送给父节点
//     //         }
//     //     }
//     //     return "ok";
//     // }

//     // std::string SelectExecutor::data_push(int node_id, std::string sql, std::string target_site_name, AugmentedPlan plan, int is_root)
//     // {
//     //     std::string datamsg = std::to_string(node_id) + "," + db::mysql::select_sql(localsitename, sql); // 获取数据并打包
//     //     if (is_root == 1)
//     //     {
//     //         // file_name is current time
//     //         time_t now = time(0);
//     //         tm *ltm = localtime(&now);
//     //         std::string file_name = std::to_string(1900 + ltm->tm_year) + std::to_string(1 + ltm->tm_mon) + std::to_string(ltm->tm_mday) + std::to_string(ltm->tm_hour) + std::to_string(ltm->tm_min) + std::to_string(ltm->tm_sec) + ".txt";
//     //         std::ofstream out(file_name);
//     //         out << datamsg.substr(datamsg.find(",") + 1);
//     //         etcd_set("result", file_name);
//     //         etcd_set(std::to_string(node_id) + "is_finished", "1");
//     //         etcd_set("is_finished", "1");
//     //     }
//     //     else
//     //     {
//     //         if (target_site_name == localsitename)
//     //         {
//     //             // 这里存表的sql语句要改一下
//     //             std::string create_sql = "create table " + to_string(node_id) + plan.augplannodes[node_id].save_sql;
//     //             std::string save_sql = "insert into " + std::to_string(node_id) + " values " + datamsg.substr(datamsg.find(",") + 1);
//     //             db::mysql::create_insert_delete_sql(localsitename, create_sql);
//     //             db::mysql::create_insert_delete_sql(localsitename, save_sql);
//     //             // 这里要标记某个数据已经执行完毕
//     //             etcd_set(std::to_string(node_id) + "is_finished", "1");
//     //             // 这里要新建一个线程,用于执行plan_scan函数
//     //             std::thread t(&SelectExecutor::plan_scan, this);
//     //             t.detach();
//     //         }
//     //         else
//     //         {
//     //             std::string result = manager.send_site_message("data_push", target_site_name, datamsg);
//     //         }
//     //     } // 发送数据

//     //     for (int i = 0; i < plan.node_num; i++)
//     //     {
//     //         if (plan.augplannodes[i].parent_id == node_id)
//     //         {
//     //             delete_data(i);
//     //             // 这里要删除表
//     //         }
//     //     }
//     //     return "ok";
//     // } // 该函数在current_site上执行一条sql，然后将其push到target_site上，返回ok或者error
//     // void SelectExecutor::delete_data(int node_id)
//     // {
//     //     std::string sql = "delete from  " + std::to_string(node_id);
//     //     db::mysql::create_insert_delete_sql(localsitename, sql);
//     // }
//     // std::string SelectExecutor::data_receive(std::string msg)
//     // {
//     //     // 这里要存表
//     //     AugmentedPlan plan = get_plan_from_etcd(); // 这里要从etcd里面读取plan
//     //     int node_id = stoi(msg.substr(0, msg.find(",")));
//     //     std::string data = msg.substr(msg.find(",") + 1);
//     //     std::string create_sql = "create table node" + to_string(node_id) + plan.augplannodes[node_id].save_sql;
//     //     std::string save_sql = "insert into " + std::to_string(node_id) + " values " + data;
//     //     db::mysql::create_insert_delete_sql(localsitename, create_sql);
//     //     db::mysql::create_insert_delete_sql(localsitename, save_sql);
//     //     // 这里要标记某个数据已经执行完毕
//     //     etcd_set(std::to_string(node_id) + "is_finished", "1");
//     //     // 这里要新建一个线程,用于执行plan_scan函数
//     //     std::thread t(&SelectExecutor::plan_scan, this);
//     //     t.detach();
//     //     // 这里要返回ok
//     //     return "ok";
//     // } // 这个函数用于存表并开启下一轮执行，data_receive的result一般是ok
//     // // 或许可以把plan存到etcd里面，然后每个节点都去读取，然后执行，这样就不用每次都传输plan了

// //     AugmentedPlan build_augmented_plan(db::opt::Tree query_tree)
// //     {
// //         AugmentedPlan plan;
// //         // get the total num of query_tree.tr
// //         plan.node_num = query_tree.tr.size();
// //         for(int i =0; i<plan.node_num; i++)
// //         {
// //             plan.augplannodes.push_back(transfer_to_AugNode(query_tree, i, plan));
// //         }
// //         return plan;

// //     }
    
// //     std::vector<int> find_node_to_build(std::vector<int> &already_built, db::opt::Tree query_tree)
// //     {
// //         std::vector<int> node_to_build;
// //         for (int i = 0; i < query_tree.tr.size(); i++)
// //         {
// //             // build a node if it's child are all built
// //             for (int j = 0; j < query_tree.tr[i].child.size(); j++)
// //             {
// //                 if (already_built[query_tree.tr[i].child[j]] == 0)
// //                 {
// //                     break;
// //                 }
// //                 if (j == query_tree.tr[i].child.size() - 1)
// //                 {
// //                     node_to_build.push_back(i);
// //                 }
// //             }
// //         }
// //         return node_to_build;
// //     }
// //     AugPlanNode transfer_to_AugNode(db::opt::Tree query_tree, int i, AugmentedPlan plan)
// //     {
// //         AugPlanNode node;
// //         node.node_id = i;
// //         node.parent_id = query_tree.tr[i].parent;
// //         node.execute_site = "ddb" + std::to_string(query_tree.tr[i].site);
        
// //         vector<std::string> projection_attr;
// //         vector<std::string> selection_attr;
// //         std::string select_what;
// //         std::string where_what;
// //         std::string from_what;
// //         from_what = query_tree.tr[i].fname.first;
// //         if (query_tree.tr[i].type == "Fragment")
// //         {
// //             if (query_tree.tr[i].projection.size())
// //             {
// //                 for (int j = 0; j < query_tree.tr[i].projection.size(); j++)
// //                 {
// //                     projection_attr.push_back(query_tree.tr[i].projection[j]);
// //                 }
// //                 for (int j = 0; j < projection_attr.size(); j++)
// //                 {
// //                     select_what += projection_attr[j];
// //                     if (j != projection_attr.size() - 1)
// //                     {
// //                         select_what += ",";
// //                     }
// //                 }
// //             } // if projection is not empty
// //             else
// //             {
// //                 select_what = "*";
// //             } // if projection is empty

// //             if (query_tree.tr[i].select.size())
// //             {
// //                 for (int j = 0; j < query_tree.tr[i].select.size(); j++)
// //                 {
// //                     selection_attr.push_back(query_tree.tr[i].select[j].getStr());
// //                 }
// //                 for (int j = 0; j < selection_attr.size(); j++)
// //                 {
// //                     where_what += query_tree.tr[i].fname.first + "/" + selection_attr[j];
// //                     if (j != selection_attr.size() - 1)
// //                     {
// //                         where_what += " and ";
// //                     }
// //                 }
// //                 node.sql = "select " + select_what + " from " + from_what + " where " + where_what;
// //             }
// //             else
// //             {
// //                 node.sql = "select " + select_what + " from " + from_what;
// //             }
// //             vector<std::string> all_attr;
// //             std::string save_sql = "(";
// //             if (query_tree.tr[i].projection.size() > 0)
// //             {
// //                 all_attr = query_tree.tr[i].projection;
// //             }
// //             else
// //             {
// //                 all_attr = query_tree.tr[i].attr;
// //             }
// //             for (auto attr : all_attr)
// //             {
// //                 std::string attr_name = attr.substr(attr.find(".") + 1);
// //                 std::string attr_type = etcd_get(attr);
// //                 save_sql += attr_name + " " + attr_type + ",";
// //             }
// //             save_sql += ")";
// //             node.save_sql = save_sql;
// //         }
// //         else if (query_tree.tr[i].type == "Join")
// //         {
// //             std::string node_sql;
// //             std::string save_sql;
// //             std::string select_what;
// //             std::string where_what;
// //             std::string from_what;
// //             for (int j = 0; j < query_tree.tr[i].child.size(); j++)
// //             {
// //                 from_what += query_tree.tr[i].child[j];
// //                 if (j != query_tree.tr[i].child.size() - 1)
// //                 {
// //                     from_what += ",";
// //                 }
// //             }
// //             std::string ltab = query_tree.tr[i].join.ltab;
// //             std::string lcol = query_tree.tr[i].join.lcol;
// //             std::string rtab = query_tree.tr[i].join.rtab;
// //             std::string rcol = query_tree.tr[i].join.rcol;
// //             where_what = query_tree.tr[i].child[0] + "." + lcol + "=" + std::to_string(query_tree.tr[i].child[1]) + "." + rcol;
// //             for (int j = 0; j < query_tree.tr[i].projection.size(); j++)
// //             {
// //                 if (std::find(query_tree.tr[query_tree.tr[i].child[0]].attr.begin(), query_tree.tr[query_tree.tr[i].child[0]].attr.end(), query_tree.tr[i].projection[j]) != query_tree.tr[query_tree.tr[i].child[0]].attr.end())
// //                 {
// //                     select_what += query_tree.tr[i].child[0] + "." + query_tree.tr[i].projection[j];
// //                 }
// //                 else if (std::find(query_tree.tr[query_tree.tr[i].child[0]].projection.begin(), query_tree.tr[query_tree.tr[i].child[0]].projection.end(), query_tree.tr[i].projection[j]) != query_tree.tr[query_tree.tr[i].child[0]].projection.end())
// //                 {
// //                     select_what += query_tree.tr[i].child[0] + "." + query_tree.tr[i].projection[j];
// //                 }
// //                 else
// //                 {
// //                     select_what += query_tree.tr[i].child[1] + "." + query_tree.tr[i].projection[j];
// //                 }
// //                 if (j != query_tree.tr[i].projection.size() - 1)
// //                 {
// //                     select_what += ",";
// //                 }
// //             }
// //             node_sql = "select " + select_what + " from " + from_what + " where " + where_what;
// //             node.sql = node_sql;
// //             vector<std::string> all_attr;
// //             save_sql = "(";
// //             if (query_tree.tr[i].projection.size() > 0)
// //             {
// //                 all_attr = query_tree.tr[i].projection;
// //             }
// //             else
// //             {
// //                 all_attr = query_tree.tr[i].attr;
// //             }
// //             for (auto attr : all_attr)
// //             {
// //                 std::string attr_name = attr.substr(attr.find(".") + 1);
// //                 std::string attr_type = etcd_get(attr);
// //                 save_sql += attr_name + " " + attr_type + ",";
// //                 etcd_set(attr_name, attr_type);
// //             }
// //             save_sql += ")";
// //             node.save_sql = save_sql;
// //         }
// //         else if (query_tree.tr[i].type == "Union")
// //         {
// //             std::string node_sql;
// //             for (int j = 0; j < query_tree.tr[i].child.size(); j++)
// //             {
// //                 node_sql += "select * from " + query_tree.tr[i].child[j];
// //                 if (j != query_tree.tr[i].child.size() - 1)
// //                 {
// //                     node_sql += " union all ";
// //                 }
// //             }
// //             node.sql = node_sql;
// //             node.save_sql = plan.augplannodes[query_tree.tr[i].child[0]].save_sql;
// //         }
// //         return node;
// //     }
// //     std::string deal_with_order(const std::string &msg, SitesManager &sitesManager, string localsitename)
// //     {
// //         AugmentedPlan plan = get_plan_from_etcd();
// //         SelectExecutor selectexecutor = SelectExecutor(sitesManager, localsitename, plan);
// //         return selectexecutor.site_execute(msg);
// //     }
// //     std::string deal_with_data(const std::string &msg, SitesManager &sitesManager, string localsitename)
// //     {
// //         AugmentedPlan plan = get_plan_from_etcd();
// //         SelectExecutor selectexecutor = SelectExecutor(sitesManager, localsitename, plan);
// //         return selectexecutor.data_receive(msg);
// //     }

// // }
