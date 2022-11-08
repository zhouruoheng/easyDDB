#include <bits/stdc++.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
// include the sql parser
#include "hsql/SQLParser.h"

// contains printing utilities
#include "hsql/util/sqlhelper.h"
#include "hsql/sql/SelectStatement.h"
#include "hsql/sql/Table.h"
#include "hsql/sql/Expr.h"

using namespace std;


static const std::map<const hsql::OperatorType, const std::string> operatorToToken = {
      {hsql::kOpEquals, "="},      {hsql::kOpNotEquals, "!="},
      {hsql::kOpLess, "<"},        {hsql::kOpLessEq, "<="},
      {hsql::kOpGreater, ">"},     {hsql::kOpGreaterEq, ">="},
      {hsql::kOpAnd, "AND"}
};


class Condition{
    public:
		Condition(hsql::OperatorType _opt,string _table,string _attr){
			auto found = operatorToToken.find(_opt);
			opt=(*found).second;
			table=_table;
			attr=_attr;
		}
        string table, attr, opt, type; //type->int/string
        int ival;
        string sval;
		void print(){
			cout<<opt<<" "<<table<<" "<<attr<<" ";
			if (type=="int")
				cout<<ival<<endl;
			else
				cout<<sval<<endl;
		}
};

vector<string> Table, Attr;
vector<Condition> Predicate;
vector<pair<string,string>> Join;  //存储所有的join条件

class treeNode{
    public:
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
		printf("AND\n");
		bool mark= get_Expression(expr->expr)&get_Expression(expr->expr2);
		printf(expr->name);
		if (mark&&expr->opType!=hsql::kOpAnd){
			if (expr->expr2->type==hsql::kExprColumnRef)
				//printf("join\n");
				Join.push_back(make_pair(string(expr->expr->table)+'.'+string(expr->expr->name),string(expr->expr2->table)+'.'+string(expr->expr2->name)));
			else{
				Condition cond=Condition(expr->opType,string(expr->expr->table),string(expr->expr->name));
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
	for (hsql::TableRef* tbl : *Statement->fromTable->list)
		Table.push_back(tbl->name);
	//for (int i=0;i<Table.size();i++)
	//	cout<<Table[i]<<" ";
	//cout<<endl;
	for (hsql::Expr* expr : *Statement->selectList)
		switch (expr->type){
			case hsql::kExprStar:
				Attr.push_back("*");
				break;
			case hsql::kExprColumnRef:
				string table=expr->table;
				string column=expr->name;
				Attr.push_back(table+'.'+column);
				break;
		}
	//for (int i=0;i<Attr.size();i++)
	//	cout<<Attr[i]<<" ";
	//cout<<endl;
	//hsql::printExpression(Statement->whereClause,0);
	get_Expression(Statement->whereClause);
	for (int i=0;i<Predicate.size();i++)
		Predicate[i].print();
	for (int i=0;i<Join.size();i++)
		cout<<Join[i].first<<" "<<Join[i].second<<endl;
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
		//hsql::printStatementInfo(result.getStatement(0));
        const hsql::SelectStatement* Statement = (const hsql::SelectStatement*)result.getStatement(0);
		init_SQL(Statement);
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
