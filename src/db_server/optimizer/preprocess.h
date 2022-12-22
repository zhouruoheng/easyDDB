#pragma once
#include <bits/stdc++.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
// include the sql parser
#include "SQLParser.h"

// contains printing utilities
#include "util/sqlhelper.h"
#include "sql/SelectStatement.h"
#include "sql/Table.h"
#include "sql/Expr.h"
#include "utils.h"

#define print(a)                                   \
	for (auto it = a.begin(); it != a.end(); it++) \
		cout << *it << " ";                        \
	cout << endl;
#define INF 1e9

using namespace std;
namespace db::opt{
struct joinCell
{
	joinCell(){};
	joinCell(string _ltab, string _lcol, string _rtab, string _rcol){
		ltab = _ltab;
		rtab = _rtab;
		lcol = _lcol;
		rcol = _rcol;
	}
	string find(string tab, string attr){
		if (ltab==tab && lcol==attr) return rtab+"."+rcol;
		if (rtab==tab && rcol==attr) return ltab+"."+lcol;
		return "";
	}
	string ltab, rtab, lcol, rcol;
};

class treeNode{
	public:
		treeNode(string _type, pair<string, int> _fname, int _size, int _site = 1, vector<string> _attr = {}, vector<Condition> _select = {})
		{
			type = _type;
			fname = _fname;
			size = _size;
			site = _site;
			attr = _attr;
			select = _select;
			parent = -1;
		}
		treeNode(string _type, int _site)
		{
			type = _type;
			site = _site;
			parent = -1;
			size = 0;
			attr = {};
			select = {};
		}
		void show_treeNode(){
			cout<<"-------------------------------"<<endl;
			cout<<"Father: "<<parent<<endl;
			cout<<"Type: "<<type<<endl;
			cout<<"Size: "<<size<<endl;
			cout<<"Site: "<<site<<endl;
			cout<<"Fname: "<<fname.first<<" "<<fname.second<<endl;
			cout<<"Child: "<<endl;
			print(child);
			cout<<"Attributes: "<<endl;
			print(attr);
			cout<<"Projection: "<<endl;
			print(projection);
			cout<<"Select: "<<endl;
			for (auto x : select) x.show_Condition();
			cout<<endl;
		}
		string getStr(){
			string res;
			if (type=="Fragment") res=type + " " + fname.first + to_string(fname.second) + "\t";
			else res = type + "\t";
			bool mark = false;
			if (projection.size()){
				res = res + "Projection(";
				int cnt = 0;
				for (auto x : projection){
					cnt++; 
					res += x+",";
					if (cnt%2==0) res += "\t";
				}
				while (res.back()=='\t' || res.back()==',') res.pop_back();
				res = res + ") \t";
				mark = true;
			}
			if (select.size()) {
				res = res + "Select(";
				for (auto x : select)
					res += x.getStr()+",\t";
				res.pop_back();
				res.pop_back();
				res = res + ") \t";
				mark = true;
			}
			res = res + "|" + to_string(site) + "|";
			for (auto x : child) res += to_string(x) + ",";
			if (res[res.size()-1]==',') res.pop_back(); 
			return res;
		}
		int parent;
		vector<int> child;
		string type; // Fragment,Union,Join
		int site;
		int size;
		pair<string, int> fname;
		joinCell join; // join条件
		vector<string> attr, projection;
		vector<Condition> select;
};

class Tree{
	public:
		void show_tree(){
			cout<<"Root: "<<root<<endl;
			for (int i=0;i<tr.size();i++) {
				cout<<"treeNode id: "<<i<<endl;
				tr[i].show_treeNode();
			}
		}
		void createPrintFile(){
			ofstream fout;
			fout.open("print.txt");
			for (int i=0;i<tr.size();i++)
				fout<<i<<"|"<<tr[i].getStr()<<endl;
			system("python3 printTree.py");
		}
		vector<treeNode> tr;
		int root;
};

class DeleteSql{
	public:
		vector<int> sites;
		string query;
};

class InsertSql{
	public:
		vector<pair<int,string>> siteQuery;
};

class Query
{

};

bool get_Expression(hsql::Expr *expr, vector<string> &All_Attr, vector<Condition> &Predicate, vector<joinCell> &Join)
{
	if (expr->type == hsql::kExprOperator)
	{
		bool mark = get_Expression(expr->expr, All_Attr, Predicate, Join) & get_Expression(expr->expr2, All_Attr, Predicate, Join);
		if (mark && expr->opType != hsql::kOpAnd)
		{
			if (expr->expr2->type == hsql::kExprColumnRef)
			{
				printf("join\n");
				string left_attr = string(expr->expr->table) + '.' + string(expr->expr->name);
				string right_attr = string(expr->expr2->table) + '.' + string(expr->expr2->name);
				cout<<left_attr<<" "<<right_attr<<endl;
				Join.push_back(joinCell(string(expr->expr->table), string(expr->expr->name), string(expr->expr2->table), string(expr->expr2->name)));
				All_Attr.push_back(left_attr);
				All_Attr.push_back(right_attr);
			}
			else
			{
				cout<<"predicate"<<endl;
				Condition cond = Condition(expr->opType, string(expr->expr->table), string(expr->expr->name));
				All_Attr.push_back(string(expr->expr->table) + '.' + string(expr->expr->name));
				cout<<string(expr->expr->table) + '.' + string(expr->expr->name)<<endl;
				if (expr->expr2->type == hsql::kExprLiteralString)
					// printf("string\n");
					cond.type = "string", cond.sval = expr->expr2->name;
				else
					// printf("int\n");
					cond.type = "int", cond.ival = expr->expr2->ival;
				Predicate.push_back(cond);
			}
		}
		return false;
	}
	else
		return true;
}

void init_SQL(const hsql::SelectStatement *Statement, vector<string> &fromTable, vector<string> &Select_Attr, vector<string> &All_Attr, vector<Condition> &Predicate, vector<joinCell> &Join)
{
	// printf("init\n");
	// cout<<Statement->fromTable->name<<endl;
	if (Statement->fromTable != nullptr)
	{
		if (Statement->fromTable->type == hsql::kTableName)
			fromTable.push_back(Statement->fromTable->name);
		else
			for (hsql::TableRef *tbl : *Statement->fromTable->list)
				fromTable.push_back(tbl->name);
	}
	// for (int i=0;i<Table.size();i++)
	//	cout<<Table[i]<<" ";
	// cout<<endl;
	printf("from\n");
	if (Statement->selectList != nullptr)
	{
		for (hsql::Expr *expr : *Statement->selectList)
			switch (expr->type)
			{
				case hsql::kExprStar:
					Select_Attr.push_back("*");
					break;
				case hsql::kExprColumnRef:
					string table = expr->table;
					string column = expr->name;
					Select_Attr.push_back(table + "." + column);
					break;
			}
	}
	printf("select & from finished!\n");
	All_Attr = Select_Attr;
	if (Statement->whereClause != nullptr)
		get_Expression(Statement->whereClause, All_Attr, Predicate, Join);
	sort(All_Attr.begin(), All_Attr.end());
	All_Attr.erase(unique(All_Attr.begin(), All_Attr.end()), All_Attr.end());
	print(All_Attr);
}

vector<Condition> check_condition(vector<Condition> hf_condition, vector<Condition> Predicate, int &cnt)
{
	vector<Condition> intersection;
	printf("check condition!\n");
	for (auto x : Predicate)
	{
		bool flag = false;
		for (auto y : hf_condition)
		{
			int a1, b1, a2, b2;
			cout<<x.table<<" "<<x.attr<<" "<<y.table<<" "<<y.attr<<" "<<x.type<<endl;
			if (x.table != y.table || x.attr != y.attr)
				continue;
		    flag = true;
			if (x.type == "string")
			{
				cout<<x.sval<<" "<<y.sval<<endl;
				if (x.sval == y.sval) intersection.push_back(y), cnt++;
				else return {};
				continue;	
			}
			x.get_section(a1, b1);
			y.get_section(a2, b2);
			cout<<"["<<a1<<" "<<b1<<"]"<<endl;
			cout<<"["<<a2<<" "<<b2<<"]"<<endl;
			int a = max(a1, a2), b = min(b1, b2);
			if (a == a2 && b == b2)
				cnt++;
			if (a <= b)
			{
				Condition inSec = y;
				if (a != -INF && b != INF && a != b)
				{
					intersection.push_back(Condition(x.table, x.attr, a, INF));
					intersection.push_back(Condition(x.table, x.attr, -INF, b));
				}
				else
					intersection.push_back(Condition(x.table, x.attr, a, b));
			}
			else return {};
		}
		if (!flag&&x.table==hf_condition[0].table) intersection.push_back(x);
	}
	return intersection;
}

vector<metadataTable> getMetadata()
{
	vector<metadataTable> Tables;
	metadataTable Publisher = metadataTable("publisher", "hf", "id");
	Publisher.attrs.push_back("publisher.id");
	Publisher.attrs.push_back("publisher.name");
	Publisher.attrs.push_back("publisher.nation");
	Fragment frag1 = Fragment(make_pair("publisher", 1), 1,"hf", 46621);
	frag1.hf_condition.push_back(Condition("publisher.id<104000"));
	frag1.hf_condition.push_back(Condition("publisher.nation=PRC"));
	Fragment frag2 = Fragment(make_pair("publisher", 2), 2,"hf", 222);
	frag2.hf_condition.push_back(Condition("publisher.id<104000"));
	frag2.hf_condition.push_back(Condition("publisher.nation=USA"));
	Fragment frag3 = Fragment(make_pair("publisher", 3), 3,"hf", 12190);
	frag3.hf_condition.push_back(Condition("publisher.id>=104000"));
	frag3.hf_condition.push_back(Condition("publisher.nation=PRC"));
	Fragment frag4 = Fragment(make_pair("publisher", 4), 4,"hf", 444);
	frag4.hf_condition.push_back(Condition("publisher.id>=104000"));
	frag4.hf_condition.push_back(Condition("publisher.nation=USA"));
	Publisher.frags.push_back(frag1);
	Publisher.frags.push_back(frag2);
	Publisher.frags.push_back(frag3);
	Publisher.frags.push_back(frag4);
	Tables.push_back(Publisher);
	metadataTable Customer = metadataTable("customer", "vf", "id");
	Customer.attrs.push_back("customer.id");
	Customer.attrs.push_back("customer.name");
	Customer.attrs.push_back("customer.rank");
	frag1 = Fragment(make_pair("customer", 1), 1,"vf", 105000);
	frag1.vf_column.push_back("customer.id");
	frag1.vf_column.push_back("customer.name");
	frag2 = Fragment(make_pair("customer", 2), 2,"vf", 105000);
	frag2.vf_column.push_back("customer.id");
	frag2.vf_column.push_back("customer.rank");
	Customer.frags.push_back(frag1);
	Customer.frags.push_back(frag2);
	Tables.push_back(Customer);
	metadataTable Book = metadataTable("book", "hf", "id");
	Book.attrs = {"book.id", "book.title", "book.authors", "book.publisher_id", "book.copies"};
	frag1 = Fragment(make_pair("book", 1), 1, "hf", 118776);
	frag1.hf_condition.push_back(Condition("book.id<205000"));
	frag2 = Fragment(make_pair("book", 2), 2, "hf", 118800);
	frag2.hf_condition.push_back(Condition("book.id>=205000"));
	frag2.hf_condition.push_back(Condition("book.id<210000"));
	frag3 = Fragment(make_pair("book", 3), 3, "hf", 950423);
	frag3.hf_condition.push_back(Condition("book.id>=210000"));
	Book.frags = {frag1, frag2, frag3};
	Tables.push_back(Book);
	metadataTable Orders = metadataTable("orders", "hf", "");
	Orders.attrs = {"orders.customer_id", "orders.book_id", "orders.quantity"};
	frag1 = Fragment(make_pair("orders", 1), 1, "hf", 1000);
	frag1.hf_condition.push_back(Condition("orders.customer_id<307000"));
	frag1.hf_condition.push_back(Condition("orders.book_id<215000"));
	frag2 = Fragment(make_pair("orders", 2), 2, "hf", 1000);
	frag2.hf_condition.push_back(Condition("orders.customer_id<307000"));
	frag2.hf_condition.push_back(Condition("orders.book_id>=215000"));
	frag3 = Fragment(make_pair("orders", 3), 3, "hf", 184796);
	frag3.hf_condition.push_back(Condition("orders.customer_id>=307000"));
	frag3.hf_condition.push_back(Condition("orders.book_id<215000"));
	frag4 = Fragment(make_pair("orders", 4), 4, "hf", 435846);
	frag4.hf_condition.push_back(Condition("orders.customer_id>=307000"));
	frag4.hf_condition.push_back(Condition("orders.book_id>=215000"));
	Orders.frags = {frag1, frag2, frag3, frag4};
	Tables.push_back(Orders);
	return Tables;
}

metadataTable findTable(vector<metadataTable> Tables, string tableName)
{
	for (auto x : Tables){
		cout<<x.name<<" "<<tableName<<endl;
		if (x.name == tableName) 
			return x;
	}
}

vector<int> findAttr(vector<Condition> hf_condition, string table, string attr)
{
	vector<int> ans;
	for (int i=0;i<hf_condition.size();i++)
		if (hf_condition[i].table==table && hf_condition[i].attr==attr)
			ans.push_back(i);
	if (ans.size()==0) return {-1};
	return ans;
}

void sortJoin(vector<joinCell> &Join)
{
	if (Join.size()<=1) return;
	for (int i=0;i<Join.size()-1;i++)
		for (int j=i+1;j<Join.size();j++){
			if (Join[i].rtab==Join[j].ltab) {
				swap(Join[j], Join[i+1]);
				break;
			}
			if (Join[i].rtab==Join[j].rtab) {
				swap(Join[j], Join[i+1]);
				swap(Join[i+1].ltab, Join[i+1].rtab);
				swap(Join[i+1].lcol, Join[i+1].rcol);
				break;
			}
		}	
}

bool cmp(const pair<string, int> left, const pair<string, int> right)
{
	return left.second < right.second;
}

struct siteNum
{
	siteNum(int _site){
		total = 0;
		site = _site;
		vector<string> tabs = {"publisher", "customer", "book", "orders"};
		for (auto x : tabs) rSize[x] = 0;
	}
	void add_relation(string table, int _rSize)
	{
		rSize[table] = _rSize;
		total = total + _rSize;
	}
	map<string, int> rSize;
	int total;
	int site;
	bool operator < (const siteNum &a) const{
		return total > a.total;
	}
};

void get_joinList(pair<string, int> x, map<pair<string,int>, map<pair<string,int>, int>> edge, vector<vector<pair<string,int>>> &JoinList, vector<pair<string,int>> &ans, int sz)
{
	if (ans.size()==sz) {
		JoinList.push_back(ans);
		return;
	}
	for (auto cur : edge[x])
		if (cur.second==1){
			ans.push_back(cur.first);
			get_joinList(cur.first, edge, JoinList, ans, sz);
			ans.pop_back();
		}
}

int find_site_total(vector<siteNum> siteNums, int site)
{
	for (auto x : siteNums)
		if (x.site == site)
			return x.total;
}

siteNum find_site(vector<siteNum> siteNums, int site)
{
	for (auto x : siteNums)
		if (x.site == site)
			return x;
}

bool cmp1(const pair<string, int> a, const pair<string,int> b)
{
	return a.first<b.first || a.first==b.first && a.second<b.second;
}

bool check_pattern(vector<vector<int>> a, vector<vector<int>> b, int pos, int sz)
{
	for (int i=0;i<sz;i++)
		if (i!=pos && a[i]!=b[i]) return false;
	return true;
}

vector<vector<vector<int>>> get_pattern(vector<vector<pair<string,int>>> joinList, int sz)
{
	vector<vector<vector<int>>> pattern;
	for (auto x : joinList){
		vector<vector<int>> cur;
		cur.resize(sz);
		for (int i=0;i<sz;i++) cur[i]={x[i].second};
		pattern.push_back(cur);
	}
	for (int i=sz-1;i>=0;i--){
		int n = pattern.size();
		bool mark[300];
		memset(mark, 0, sizeof(mark));
		vector<vector<vector<int>>> ans(pattern);
		for (int j=0;j<n;j++){
			if (mark[j]) continue;
			for (int k=j+1;k<n;k++)
				if (check_pattern(ans[j], ans[k], i, sz))
					ans[j][i].insert(ans[j][i].end(), ans[k][i].begin(), ans[k][i].end()),
					mark[k]=true;
		}
		pattern = {};
		for (int j=0;j<n;j++)
			if (!mark[j]) pattern.push_back(ans[j]);
	}
	cout<<"Pattern:"<<endl;
	for (auto x : pattern){
		for (auto y : x){
			for (auto z : y)
				cout<<z<<"|";
			cout<<",";
		}
		cout<<endl;
	}		
	return pattern;
}

int get_treeNode(map<pair<string,int>, int> mpTr, map<pair<string,int>, int> &cpTr, vector<treeNode> &tr, pair<string,int> x)
{
	if (!cpTr[x]) {
		cpTr[x] = 1;
		return mpTr[x];
	}
	treeNode node = tr[mpTr[x]];
	node.child = {};
	node.parent = -1;
	tr.push_back(node);
	cout<<"newNode"<<" "<<tr.size()-1<<endl;
	return tr.size()-1;
}

vector<string> get_attr(treeNode x, treeNode y)
{
	vector<string> res;
	if (x.projection.size()) res = x.projection;
	else res = x.attr;
	if (y.projection.size()) res.insert(res.end(), y.projection.begin(), y.projection.end());
	else res.insert(res.end(), y.attr.begin(), y.attr.end());
	sort(res.begin(), res.end());
	res.erase(unique(res.begin(),res.end()),res.end());	
	return res;
}

vector<string> get_projection(vector<string> Select_Attr, vector<string> attr, string x)
{
	auto iter = find(Select_Attr.begin(), Select_Attr.end(), x);
	vector<string> projection;
	if (iter==Select_Attr.end()) 
		projection = attr,
		projection.erase(remove(projection.begin(), projection.end(), x) ,projection.end());
	return projection;
}				

Tree build_query_tree(vector<metadataTable> Tables, vector<string> fromTable, vector<string> Select_Attr, vector<string> All_Attr, vector<Condition> Predicate, vector<joinCell> Join)
{
	Tree query_tree;
	if (Select_Attr[0]=="*") {
		metadataTable res = findTable(Tables, fromTable[0]);
		Select_Attr.pop_back();
		All_Attr.erase(find(All_Attr.begin(), All_Attr.end(), "*"));
		for (auto x : res.attrs) {
			Select_Attr.push_back(x);
			if (find(All_Attr.begin(), All_Attr.end(), x) == All_Attr.end())
				All_Attr.push_back(x);
		}
	}
	print(Select_Attr);
	print(All_Attr);
	vector<Condition> addPred;
	vector<string> Join_Attr;
	for (auto x : Join)
		Join_Attr.push_back(x.ltab+"."+x.lcol),
		Join_Attr.push_back(x.rtab+"."+x.rcol);
	map<pair<string,int>, int> mpTr;
	map<pair<string,int>, int> cpTr;
	for (auto p : Predicate)
		for (auto x : Join){
			string tab,attr,res;
			res = x.find(p.table, p.attr);
			if (res.size()==0) continue;
			tab = res.substr(0, res.find('.'));
			attr = res.substr(res.find('.')+1);
			Condition cur = Condition(p.opt, tab, attr);
			cur.type = p.type;
			if (p.type=="int") cur.ival = p.ival;
			else cur.sval = p.sval;
			addPred.push_back(cur);
		}
	Predicate.insert(Predicate.end(), addPred.begin(), addPred.end());
	map<string, vector<Fragment>> pruning;
	map<string, int> relationSize;
	vector<siteNum> siteNums;
	vector<pair<string, int>> allFrag;
	for (int i=1;i<=4;i++) siteNums.push_back(siteNum(i));
	for (auto table : Tables)
	{
		if (find(fromTable.begin(), fromTable.end(), table.name) == fromTable.end())
			continue;
		if (table.type == "hf")
		{
			relationSize[table.name] = 0;
			for (auto frag : table.frags)
			{
				int cnt = 0;
				vector<Condition> conditions = check_condition(frag.hf_condition, Predicate, cnt);
				// cout<<cnt<<endl;
				if (!conditions.size()&& Predicate.size())
					continue;
				if (cnt == frag.hf_condition.size())
					conditions = {};
				treeNode node = treeNode("Fragment", frag.fname, frag.size, frag.site, table.attrs, conditions);
				for (auto attr : table.attrs){
					if (find(Select_Attr.begin(), Select_Attr.end(), attr) != Select_Attr.end())
						node.projection.push_back(attr);
					else 
						if (find(Join_Attr.begin(), Join_Attr.end(), attr) != Join_Attr.end())
							node.projection.push_back(attr);
				}
				// print(table.attrs)
				print(node.projection);
				if (node.projection.size() == table.attrs.size())
					node.projection = {};
				query_tree.tr.push_back(node);
				pruning[table.name].push_back(frag);
				relationSize[table.name] = relationSize[table.name] + frag.size;
				siteNums[frag.site-1].add_relation(table.name, frag.size);
				allFrag.push_back(frag.fname);
				mpTr[frag.fname] = query_tree.tr.size()-1;
				cout << frag.site << endl;
			}
		}
		else
		{
			relationSize[table.name] = table.frags[0].size;
			printf("vf");
			vector<Condition> res;
			for (auto x : Predicate)
				if (x.table==table.name) res.push_back(x);
			int cnt=0;
			bool pruningFrag=false;
			for (auto frag : table.frags){
				vector<string> vf_attr;
				vector<Condition> sel;
				bool mark=false;
				for (auto x : frag.vf_column){
					for (auto p : res)
						if (p.table+"."+p.attr==x) sel.push_back(p);
					if (find(All_Attr.begin(), All_Attr.end(), x)!=All_Attr.end()) {
						if (find(Select_Attr.begin(), Select_Attr.end(), x)!=Select_Attr.end() || find(Join_Attr.begin(), Join_Attr.end(), x)!=Join_Attr.end())
							vf_attr.push_back(x);
					}
					else 
						continue;
					if (x!=table.name+"."+table.key_column) mark=true;
				}
				
				if (mark) {
					cnt++;
					treeNode node = treeNode("Fragment", frag.fname, frag.size, frag.site, frag.vf_column, sel);
					if (frag.vf_column != vf_attr) node.projection = vf_attr;
					query_tree.tr.push_back(node);
					pruning[table.name].push_back(frag);
				}
			}
			if (cnt > 1) {
				int sz = query_tree.tr.size();
				int mx=0, mx_ans;
				vector<string> vf_attr;
				for (int i=1;i<=cnt;i++){
					if (query_tree.tr[sz-i].projection.size() && find(query_tree.tr[sz-i].projection.begin(), query_tree.tr[sz-i].projection.end(), table.name+"."+table.key_column)==query_tree.tr[sz-i].projection.end()) {
						query_tree.tr[sz-i].projection.push_back(table.name+"."+table.key_column);
						if (query_tree.tr[sz-i].projection.size() == query_tree.tr[sz-i].attr.size())
							query_tree.tr[sz-i].projection={};
					}
					if (query_tree.tr[sz-i].size>=mx) mx=query_tree.tr[sz-i].size, mx_ans=query_tree.tr[sz-i].site;
					vf_attr.insert(vf_attr.end(), query_tree.tr[sz-i].attr.begin(), query_tree.tr[sz-i].attr.end());
					pruning[table.name].pop_back();
				}
				sort(vf_attr.begin(),vf_attr.end());
				vf_attr.erase(unique(vf_attr.begin(),vf_attr.end()),vf_attr.end());	
				treeNode node=treeNode("Join", table.frags[0].fname, table.frags[0].size, mx_ans, vf_attr);
				for (auto x : vf_attr)
					if (find(Select_Attr.begin(), Select_Attr.end(), x)!=Select_Attr.end() || find(Join_Attr.begin(), Join_Attr.end(), x)!=Join_Attr.end())
						node.projection.push_back(x);
				if (node.projection == node.attr) node.projection = {};
				node.join = joinCell(table.name, table.key_column, table.name, table.key_column);
				for (int i=1;i<=cnt;i++) 
					node.child.push_back(sz-i),
					query_tree.tr[sz-i].parent=sz;
				query_tree.tr.push_back(node);
				pruning[table.name].push_back(table.frags[0]);
				siteNums[0].add_relation(table.name, table.frags[0].size);
				allFrag.push_back(table.frags[0].fname);
				mpTr[table.frags[0].fname] =  query_tree.tr.size()-1;
			}
			if (cnt==1) {
				treeNode node = query_tree.tr.back();
				siteNums[node.site-1].add_relation(table.name, node.size);
				allFrag.push_back(node.fname);
				mpTr[node.fname] =  query_tree.tr.size()-1;
			}
			if (!cnt) {
				treeNode node = treeNode("Fragment", table.frags[0].fname, table.frags[0].size, 1, table.frags[0].vf_column, res);
				node.projection.push_back(table.key_column);
				query_tree.tr.push_back(node);
				pruning[table.name].push_back(table.frags[0]);
				siteNums[0].add_relation(table.name, table.frags[0].size);
				allFrag.push_back(table.frags[0].fname);
				mpTr[table.frags[0].fname] =  query_tree.tr.size()-1;
			}
		}
	}
	// join
	// 建边
	sort(siteNums.begin(), siteNums.end());
	map<pair<string,int>, map<pair<string,int>, int>> edge;
	map<pair<string,int>, map<pair<string,int>, int>> conflict;
	for (auto i : allFrag)
		for (auto j : allFrag)
			conflict[i][j] = 0;
	sortJoin(Join);
	vector<string> relation;
	for (auto p : Join){
		relation.push_back(p.ltab);
		cout<<p.ltab<<"."<<p.lcol<<" "<<p.rtab<<" "<<p.rcol<<endl;
		for (auto x : pruning[p.ltab])
			for (auto y : pruning[p.rtab]) {
				//cout<<x.type<<" "<<y.type<<endl;
				if (x.type != y.type) edge[x.fname][y.fname] = 1;
				else {
					vector<int> idxs = findAttr(x.hf_condition, p.ltab, p.lcol);
					vector<int> idys = findAttr(y.hf_condition, p.rtab, p.rcol);
					//cout<<idx<<" "<<idy<<endl;
					if (min(idxs[0],idys[0])==-1) {
						edge[x.fname][y.fname] = 1;
						continue;
					}
					bool mark = true;
					for (auto idx : idxs)
						for (auto idy : idys){
							Condition conx = x.hf_condition[idx];
							Condition cony = y.hf_condition[idy];
							//cout<<conx.type<<endl;
							if (conx.type=="string"){
								if (conx.sval!=cony.sval) mark=false;
								continue;
							}
							int a1,b1,a2,b2;
							conx.get_section(a1, b1);
							cony.get_section(a2, b2);
							cout<<"["<<a1<<" "<<b1<<"]"<<endl;
							cout<<"["<<a2<<" "<<b2<<"]"<<endl;
							int a = max(a1, a2), b = min(b1, b2);
							if (a > b) mark=false;
						}
					if (mark) edge[x.fname][y.fname] = 1;
					else conflict[x.fname][y.fname] = 1,conflict[y.fname][x.fname] = 1;
				}
			}
	}
	joinCell lastR = Join.back();
	relation.push_back(lastR.rtab);
	vector<vector<pair<string,int>>> JoinList;
	for (auto x : pruning[Join[0].ltab]){
		vector<pair<string,int>> ans = {x.fname};
		get_joinList(x.fname, edge, JoinList, ans, Join.size()+1);
	}
	cout<<"JoinList:"<<endl;
	for (auto x : JoinList){
		for (auto y : x)
			cout<<y.first<<y.second<<",";
		cout<<endl;
	}
	auto iter = max_element(relationSize.begin(), relationSize.end(), cmp);
	string Rp = iter->first;
	cout<<"Rp:"<<Rp<<endl;
	vector<int> processSite;
	for (auto x : siteNums){
		int sum = 0;
		for (auto tab : Tables) {
			//cout<<relationSize[tab.name] - x.rSize[tab.name]<<endl;
			if (tab.name != Rp) sum += relationSize[tab.name] - x.rSize[tab.name];
			for (auto frag : pruning[tab.name]) //某些frag与Rp在当前site上的frag冲突则不应算入sum
				if (frag.site!=x.site && conflict[make_pair(Rp, x.site)][frag.fname])
					sum -= frag.size;
		}
		cout<<sum<<endl;
		if (sum > x.rSize[Rp]) 
			break;
		else
			processSite.push_back(x.site);
	}
	if (processSite.size()==0) processSite.push_back(siteNums[0].site);
	for (int i=0;i<processSite.size();i++)
		if (find_site_total(siteNums, processSite[i]) > find_site_total(siteNums, processSite[0]))
			swap(processSite[0], processSite[i]);
	print(processSite);
	map<int,vector<vector<pair<string,int>>>> JoinSite;
	for (auto x : JoinList){
		int cnt = 0;
		int mx = 0;
		int mxSite = 0;
		for (auto y : x)
			if (find(processSite.begin(), processSite.end(), y.second)!=processSite.end()){
				cnt++;
				siteNum cur = find_site(siteNums, y.second);
				//cout << cur.rSize[y.first] <<endl;
				if (cur.rSize[y.first] > mx)
					mx = cur.rSize[y.first], mxSite = y.second;
			}
		if (!cnt) mxSite = processSite[0];
		//cout<<mxSite<<endl;
		JoinSite[mxSite].push_back(x);
	}
	for (int i=1;i<=4;i++){
		if (JoinSite[i].size()==0)
			continue;
		cout<<"!!!!"<<endl;
		vector<vector<vector<int>>> res=get_pattern(JoinSite[i], Join.size()+1);
		for (auto plist : res){
			vector<int> tr;
			for (int j=0;j<=Join.size();j++){
				cout<<plist[j].size()<<endl;
				if (plist[j].size()==1) {
					tr.push_back(get_treeNode(mpTr, cpTr, query_tree.tr, make_pair(relation[j], plist[j][0])));
					continue;
				}
				cout<<"!!!!"<<endl;
				treeNode node=treeNode("Union", i);
				query_tree.tr.push_back(node);
				int par = query_tree.tr.size()-1;
				for (auto x : plist[j]){
					int idx = get_treeNode(mpTr, cpTr, query_tree.tr, make_pair(relation[j], x));
					query_tree.tr[par].child.push_back(idx);
					if (query_tree.tr[idx].projection.size()) query_tree.tr[par].attr = query_tree.tr[idx].projection;
					else query_tree.tr[par].attr = query_tree.tr[idx].attr;
					query_tree.tr[idx].parent = par;
				}
				tr.push_back(par);
			}
			treeNode node = treeNode("Join", i);
			node.join = Join[0];
			node.child ={tr[0], tr[1]};
			node.attr = get_attr(query_tree.tr[tr[0]], query_tree.tr[tr[1]]);
			node.projection = get_projection(Select_Attr, node.attr, Join[0].ltab+"."+Join[0].lcol);
			if (tr.size()==2 || Join[0].rtab+"."+Join[0].rcol != Join[1].ltab+"."+Join[1].lcol)
				node.projection = get_projection(Select_Attr, node.projection, Join[0].rtab+"."+Join[0].rcol);
			query_tree.tr.push_back(node);
			int par = query_tree.tr.size()-1;
			query_tree.tr[tr[0]].parent = par;
			query_tree.tr[tr[1]].parent = par;
			for (int j=2;j<tr.size();j++){
				treeNode cur = treeNode("Join", i);
				cur.join = Join[j-1];
				cur.child = {par, tr[j]};
				cur.attr = get_attr(query_tree.tr[par], query_tree.tr[tr[j]]);
				cur.projection = get_projection(Select_Attr, cur.attr, Join[j-1].ltab+"."+Join[j-1].lcol);
				if (j==tr.size()-1 || Join[j-1].rtab+"."+Join[j-1].rcol != Join[j].ltab+"."+Join[j].lcol) 
					cur.projection = get_projection(Select_Attr, cur.projection, Join[j-1].rtab+"."+Join[j-1].rcol);
				query_tree.tr.push_back(cur);
				int cpar = query_tree.tr.size()-1;
				query_tree.tr[par].parent = cpar;
				query_tree.tr[tr[j]].parent = cpar;
				par = cpar;
			}
		}
	}
	// union
	int num = query_tree.tr.size();
	int cnt=0;
	for (int i = 0; i < num; i++)
		if (query_tree.tr[i].parent==-1) cnt++;
	if (cnt>1){
		treeNode root = treeNode("Union", processSite[0]);
		for (int i = 0; i < num; i++)
			if (query_tree.tr[i].parent==-1)
				root.child.push_back(i), query_tree.tr[i].parent = num,
				root.attr = get_attr(root, query_tree.tr[i]);
		if (root.attr.size() != Select_Attr.size()) root.projection = Select_Attr;
		query_tree.tr.push_back(root);
		query_tree.root = num;
	}
	else query_tree.root = num-1;
	query_tree.show_tree();
	query_tree.createPrintFile();
	return query_tree;
}

Tree SelectProcess(vector<metadataTable> Tables, hsql::SQLParserResult *result)
{
	vector<string> fromTable, Select_Attr, All_Attr; // All_Attr所有涉及到的Attr
	vector<Condition> Predicate;					 // where中的非join条件
	vector<joinCell> Join;	
	hsql::printStatementInfo(result->getStatement(0));
	// cout<<"!!!"<<endl;
	const hsql::SelectStatement *Statement = (const hsql::SelectStatement *)result->getStatement(0);
	// printf("!!!!!!!!\n");
	init_SQL(Statement, fromTable, Select_Attr, All_Attr, Predicate, Join);
	Tree query_tree = build_query_tree(Tables, fromTable, Select_Attr, All_Attr, Predicate, Join);
	return query_tree;
}

string createInsertSql(vector<string> vf_column, vector<string> columns, vector<string> values)
{
	string col="(", val="(";
	for (auto x : vf_column){
		int index = find(columns.begin(), columns.end(), x) - columns.begin();
		col = col + columns[index] + ",";
		val = val + values[index] + ",";
	}
	col.pop_back();
	col = col + ")";
	val.pop_back();
	val = val + ")";
	return col + " values" + val + ";"; 
}

InsertSql InsertProcess(vector<metadataTable> Tables, hsql::SQLParserResult *result, string query)
{
	hsql::printStatementInfo(result->getStatement(0));
	const hsql::InsertStatement *Statement = (const hsql::InsertStatement *)result->getStatement(0);
	metadataTable res = findTable(Tables, Statement->tableName);
	vector<string> columns, values;
	vector<Condition> pred;
	for (char* col_name : *Statement->columns)	columns.push_back(string(col_name));
	for (hsql::Expr* expr : *Statement->values) 
		switch (expr->type){
			case hsql::kExprLiteralString:
				values.push_back(expr->name);
				break;
			case hsql::kExprLiteralInt:
				values.push_back(to_string(expr->ival));
				break;
		}
	print(columns);
	print(values);
	for (int i=0;i<columns.size();i++) pred.push_back(Condition(string(Statement->tableName)+"."+columns[i]+"="+values[i]));
	printf("Finish create predicate!\n");
	InsertSql ans;
	if (res.type == "hf"){
		printf("hf\n");
		for (auto frag : res.frags){
			int cnt=0;
			vector<Condition> conditions = check_condition(frag.hf_condition, pred, cnt);
			cout<<cnt<<endl;
			if (!conditions.size()) continue;
			ans.siteQuery.push_back(make_pair(frag.site, query));
		}
	}
	else {
		printf("vf\n");
		for (auto frag : res.frags) ans.siteQuery.push_back(make_pair(frag.site, "insert into "+string(Statement->tableName)+createInsertSql(frag.vf_column, columns, values)));
	}
	for (auto x : ans.siteQuery)
		cout<<x.first<<" "<<x.second<<endl;
	return ans;
}

void StrSplit(const string& s, vector<string>& v, const string& c)
{
	string::size_type pos1 = 0, pos2 = s.find(c);
	while (string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));
		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length()) v.push_back(s.substr(pos1));
}

DeleteSql DeleteProcess(vector<metadataTable> Tables, string query)
{
	vector<Condition> Predicate;
	query.pop_back();
	string temp = query.substr(query.find("from")+5);
	cout<<temp<<endl;
	string tableName = temp.substr(0,temp.find(' '));
	string whereClause = temp.substr(temp.find("where")+6);
	whereClause.erase(remove(whereClause.begin(), whereClause.end(), ' '), whereClause.end());
	whereClause.erase(remove(whereClause.begin(), whereClause.end(), '\''), whereClause.end());
	cout<<tableName<<":"<<whereClause<<endl;
	vector<string> pred;
	StrSplit(whereClause, pred, "and");
	print(pred);
	for (auto x : pred) Predicate.push_back(Condition(x));
	DeleteSql ans;
	ans.query = query;
	metadataTable res = findTable(Tables, tableName);
	if (res.type == "hf"){
		printf("hf\n");
		for (auto frag : res.frags){
			int cnt=0;
			vector<Condition> conditions = check_condition(frag.hf_condition, Predicate, cnt);
			cout<<cnt<<endl;
			if (!conditions.size()) continue;
			ans.sites.push_back(frag.site);
			cout<<frag.site<<endl;
		}
	}
	else  for (auto frag : res.frags) ans.sites.push_back(frag.site); // vf的delete最好是执行的时候先将所有的fragment合并，delete后再进行拆分写回
	return ans;
}
}
// int main()
// {
// 	freopen("sql.in", "r", stdin);
// 	std::string query;
// 	getline(std::cin, query);
// 	transform(query.begin(), query.end(), query.begin(), ::tolower);
// 	std::cout << query;

// 	// parse a given query
// 	hsql::SQLParserResult result;
// 	hsql::SQLParser::parse(query, &result);

// 	// check whether the parsing was successful

// 	if (result.isValid())
// 	{
// 		printf("Parsed successfully!\n");
// 		hsql::printStatementInfo(result.getStatement(0));
// 		// cout<<"!!!"<<endl;
// 		const hsql::SelectStatement *Statement = (const hsql::SelectStatement *)result.getStatement(0);
// 		// printf("!!!!!!!!\n");
// 		init_SQL(Statement);
// 		Tree query_tree = build_query_tree();
// 		return 0;
// 	}
// 	else
// 	{
// 		fprintf(stderr, "Given string is not a valid SQL query.\n");
// 		fprintf(stderr, "%s (L%d:%d)\n",
// 				result.errorMsg(),
// 				result.errorLine(),
// 				result.errorColumn());
// 		return -1;
// 	}
// }
