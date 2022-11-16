#include <bits/stdc++.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
// include the sql parser
#include "hsql/SQLParser.h"

// contains printing utilities
#include "hsql/util/sqlhelper.h"
#include "hsql/sql/SelectStatement.h"
#include "hsql/sql/Table.h"
#include "hsql/sql/Expr.h"
#include "utils.h"

#define print(a) for (auto it=a.begin();it!=a.end();it++) cout<<*it<<" "; cout<<endl; 
#define INF 1e9

using namespace std;

vector<string> fromTable, Select_Attr, All_Attr; //All_Attr所有涉及到的Attr
vector<Condition> Predicate; //where中的非join条件
vector<pair<string,string>> Join;  //存储所有的join条件

class treeNode{
    public:
	    treeNode(string _type, int _site=1, vector<string> _attr={}, vector<Condition> _select={}){
		   type=_type;
		   site=_site;
		   attr=_attr;
		   select=_select;
		}
		int parent;
		vector<int> child;
		string type; // Fragment,Union,Join
		int site;
		pair<string,int> fname;
		string join;
		vector<string> attr, projection;
		vector<Condition> select;
};

class Tree{
    public:
        vector<treeNode> tr;
        int root;
};

bool get_Expression(hsql::Expr* expr)
{
	if (expr->type==hsql::kExprOperator) {
		bool mark= get_Expression(expr->expr)&get_Expression(expr->expr2);
		if (mark&&expr->opType!=hsql::kOpAnd){
			if (expr->expr2->type==hsql::kExprColumnRef){
				//printf("join\n");
				string left_attr=string(expr->expr->table)+'.'+string(expr->expr->name);
				string right_attr=string(expr->expr2->table)+'.'+string(expr->expr2->name);
				Join.push_back(make_pair(left_attr, right_attr));
				All_Attr.push_back(left_attr);
				All_Attr.push_back(right_attr);
			}
			else{
				Condition cond=Condition(expr->opType,string(expr->expr->table),string(expr->expr->name));
				All_Attr.push_back(string(expr->expr->table)+'.'+string(expr->expr->name));
				if (expr->expr2->type==hsql::kExprLiteralString)
					//printf("string\n");
					cond.type="string",cond.sval=expr->expr2->name;
				else
					//printf("int\n");
					cond.type="int",cond.ival=expr->expr2->ival;
				Predicate.push_back(cond);
			}
		}
		return false;
	}
	else 
		return true;
}

void init_SQL(const hsql::SelectStatement* Statement)
{
	//printf("init\n");
	//cout<<Statement->fromTable->name<<endl;
	if (Statement->fromTable != nullptr){
		if (Statement->fromTable->type==hsql::kTableName)
			fromTable.push_back(Statement->fromTable->name);
		else
			for (hsql::TableRef* tbl : *Statement->fromTable->list)
				fromTable.push_back(tbl->name);
	}
	//for (int i=0;i<Table.size();i++)
	//	cout<<Table[i]<<" ";
	//cout<<endl;
	printf("from\n");
	if (Statement->selectList != nullptr){
		for (hsql::Expr* expr : *Statement->selectList)
			switch (expr->type){
				case hsql::kExprStar:
					Select_Attr.push_back("*");
					break;
				case hsql::kExprColumnRef:
					string table=expr->table;
					string column=expr->name;
					Select_Attr.push_back(table+'.'+column);
					break;
			}
	}
	//printf("select & from finished!\n");
	All_Attr=Select_Attr;
	if (Statement->whereClause != nullptr) get_Expression(Statement->whereClause);
	sort(All_Attr.begin(),All_Attr.end());
	All_Attr.erase(unique(All_Attr.begin(),All_Attr.end()),All_Attr.end());
	print(All_Attr);
}


vector<Condition> check_condition(vector<Condition> vf_condition, int &cnt)
{
	vector<Condition> intersection;
	for (auto x : Predicate){
		for (auto y : vf_condition){
			int a1,b1,a2,b2;
			//cout<<x.table<<" "<<x.attr<<" "<<y.table<<" "<<y.attr<<endl;
			if (x.table != y.table || x.attr != y.attr) continue;
			if (x.type == "string"){ 
				if (x.sval == y.sval) intersection.push_back(y), cnt++;
			}
			x.get_section(a1,b1);
			y.get_section(a2,b2);
			//cout<<"["<<a1<<" "<<b1<<"]"<<endl;
			//cout<<"["<<a2<<" "<<b2<<"]"<<endl;
			int a=max(a1,a2), b=min(b1,b2);
			if (a==a2&&b==b2) cnt++;
			if (a<=b){
				Condition inSec=y;
				if (a!=-INF&&b!=INF&&a!=b) {
					intersection.push_back(Condition(x.table, x.attr, a, INF));
					intersection.push_back(Condition(x.table, x.attr, -INF, b));
				}
				else intersection.push_back(Condition(x.table, x.attr, a,b));
			}
		}
	}
	return intersection;
}

Tree build_query_tree()
{
	vector<metadataTable> Tables;
	metadataTable Publisher=metadataTable("publisher","vf","id");
	Publisher.attrs.push_back("id");
	Publisher.attrs.push_back("name");
	Publisher.attrs.push_back("nation");
	Fragment frag1=Fragment(make_pair("publisher",1),1);
	frag1.vf_condition.push_back(Condition("publisher.id<104000"));
	frag1.vf_condition.push_back(Condition("publisher.nation=PRC"));
	Fragment frag2=Fragment(make_pair("publisher",2),2);
	frag2.vf_condition.push_back(Condition("publisher.id<104000"));
	frag2.vf_condition.push_back(Condition("publisher.nation=USA"));
	Publisher.frags.push_back(frag1);
	Publisher.frags.push_back(frag2);
	Tables.push_back(Publisher);
	Tree query_tree;
	for (auto table : Tables){
		if (find(fromTable.begin(), fromTable.end(), table.name)==fromTable.end())
			continue;
		if (table.type=="vf"){
			for (auto frag : table.frags){
				int cnt=0;
				vector<Condition> conditions=check_condition(frag.vf_condition, cnt);
				//cout<<cnt<<endl;
				if (conditions.size()==0&&Predicate.size()!=0) continue;
				if (cnt==frag.vf_condition.size()) conditions={};
				treeNode node=treeNode("Fragment", frag.site, table.attrs, conditions);
				for (auto attr : table.attrs)
					if (find(All_Attr.begin(), All_Attr.end(), table.name+"."+attr)!=All_Attr.end())
						node.projection.push_back(attr);
				//print(table.attrs)
				print(node.projection);
				if (node.projection.size()==table.attrs.size())
					node.projection={};
				query_tree.tr.push_back(node);
				cout<<frag.site<<endl;
			}
		}
		else{
			printf("hf");
		}
	}	
	//join
	//union
	treeNode root = treeNode("Union", 1);
	int num = query_tree.tr.size();
	for (int i=0;i<num;i++) root.child.push_back(i), query_tree.tr[i].parent=num;
	query_tree.tr.push_back(root);
	query_tree.root=num;
	return query_tree;
}



int main() {
    freopen("sql.in","r",stdin);
    std::string query;
    getline(std::cin,query);
    transform(query.begin(), query.end(), query.begin(), ::tolower);
    std::cout<<query;
    
    // parse a given query
    hsql::SQLParserResult result;
    hsql::SQLParser::parse(query, &result);

    // check whether the parsing was successful

    if (result.isValid()) {
        printf("Parsed successfully!\n");
		hsql::printStatementInfo(result.getStatement(0));
		//cout<<"!!!"<<endl;
        const hsql::SelectStatement* Statement = (const hsql::SelectStatement*)result.getStatement(0);
		//printf("!!!!!!!!\n");
		init_SQL(Statement);
		Tree query_tree=build_query_tree();
        return 0;
    } else {
        fprintf(stderr, "Given string is not a valid SQL query.\n");
        fprintf(stderr, "%s (L%d:%d)\n",
                result.errorMsg(),
                result.errorLine(),
                result.errorColumn());
        return -1;
    }
}
