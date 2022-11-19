#include <bits/stdc++.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
// include the sql parser
#include "sql-parser/src/SQLParser.h"

// contains printing utilities
#include "sql-parser/src/util/sqlhelper.h"
#include "sql-parser/src/sql/SelectStatement.h"
#include "sql-parser/src/sql/Table.h"
#include "sql-parser/src/sql/Expr.h"
#include "utils.h"



namespace db::opt
{
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
        Tree(char* sql){}
        Tree(){}
};
db::opt::Tree build_query_tree();
} // namespace db::opt
