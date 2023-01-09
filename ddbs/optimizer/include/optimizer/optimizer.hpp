#pragma once
#include <bits/stdc++.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
// include the sql parser
#include <SQLParser.h>

// contains printing utilities
#include <util/sqlhelper.h>
#include <sql/SelectStatement.h>
#include <sql/Table.h>
#include <sql/Expr.h>
#include <optimizer/utils.h>

#define macro_print(a)                             \
	for (auto it = a.begin(); it != a.end(); it++) \
		cout << *it << " ";                        \
	cout << endl;
#define INF 1e9

namespace db::opt
{
	using namespace std;

	struct joinCell
	{
		joinCell(){};
		joinCell(string _ltab, string _lcol, string _rtab, string _rcol);
		string find(string tab, string attr);
		string ltab, rtab, lcol, rcol;
	};

	class treeNode
	{
	public:
		treeNode(string _type, pair<string, int> _fname, int _size, int _site = 1, vector<string> _attr = {}, vector<Condition> _select = {});
		treeNode(string _type, int _site);
		void show_treeNode();
		string getStr();
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

	class Tree
	{
	public:
		void show_tree();
		void createPrintFile();
		vector<treeNode> tr;
		int root;
	};

	class DeleteSql
	{
	public:
		vector<int> sites;
		string query;
	};

	class InsertSql
	{
	public:
		vector<pair<int, string>> siteQuery;
	};

	bool get_Expression(hsql::Expr *expr, vector<string> &All_Attr, vector<Condition> &Predicate, vector<joinCell> &Join, vector<string> fromTable);

	void init_SQL(const hsql::SelectStatement *Statement, vector<string> &fromTable, vector<string> &Select_Attr, vector<string> &All_Attr, vector<Condition> &Predicate, vector<joinCell> &Join);

	vector<Condition> check_condition(vector<Condition> hf_condition, vector<Condition> Predicate, int &cnt);

	vector<metadataTable> getMetadata();

	metadataTable findTable(vector<metadataTable> Tables, string tableName);
	vector<int> findAttr(vector<Condition> hf_condition, string table, string attr);

	void sortJoin(vector<joinCell> &Join);

	bool cmp(const pair<string, int> left, const pair<string, int> right);
	struct siteNum
	{
		siteNum(int _site);
		void add_relation(string table, int _rSize);
		bool operator<(const siteNum &a) const;

		map<string, int> rSize;
		int total;
		int site;
	};

	void get_joinList(pair<string, int> x, map<pair<string, int>, map<pair<string, int>, int>> edge, vector<vector<pair<string, int>>> &JoinList, vector<pair<string, int>> &ans, int sz);

	int find_site_total(vector<siteNum> siteNums, int site);

	siteNum find_site(vector<siteNum> siteNums, int site);

	bool cmp1(const pair<string, int> a, const pair<string, int> b);

	bool check_pattern(vector<vector<int>> a, vector<vector<int>> b, int pos, int sz);

	vector<vector<vector<int>>> get_pattern(vector<vector<pair<string, int>>> joinList, int sz);

	int get_treeNode(map<pair<string, int>, int> mpTr, map<pair<string, int>, int> &cpTr, vector<treeNode> &tr, pair<string, int> x);

	vector<string> get_attr(treeNode x, treeNode y);

	vector<string> get_projection(vector<string> Select_Attr, vector<string> attr, string x);

	Tree build_query_tree(vector<metadataTable> Tables, vector<string> fromTable, vector<string> Select_Attr, vector<string> All_Attr, vector<Condition> Predicate, vector<joinCell> Join);

	Tree SelectProcess(vector<metadataTable> Tables, hsql::SQLParserResult *result);

	string createInsertSql(vector<string> vf_column, vector<string> columns, vector<string> values);

	InsertSql InsertProcess(vector<metadataTable> Tables, hsql::SQLParserResult *result, string query);

	void StrSplit(const string &s, vector<string> &v, const string &c);

	DeleteSql DeleteProcess(vector<metadataTable> Tables, string query);
}
