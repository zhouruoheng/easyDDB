#pragma once
#include <vector>
#include "SQLParser.h"
#define INF 1e9

using namespace std;

static const std::map<const hsql::OperatorType, const std::string> operatorToToken = {
	{hsql::kOpEquals, "="}, {hsql::kOpNotEquals, "!="}, {hsql::kOpLess, "<"}, {hsql::kOpLessEq, "<="}, {hsql::kOpGreater, ">"}, {hsql::kOpGreaterEq, ">="}, {hsql::kOpAnd, "AND"}};

int ddbstr2int(string num)
{
	int ans = 0;
	for (auto str : num)
		if (str >= 48 && str <= 57)
			ans = ans * 10 + str - 48;
		else
			return -1;
	return ans;
}

class Condition
{
public:
	Condition(hsql::OperatorType _opt, string _table, string _attr)
	{
		auto found = operatorToToken.find(_opt);
		opt = (*found).second;
		table = _table;
		attr = _attr;
	}
	Condition(string _opt, string _table, string _attr)
	{
		opt = _opt;
		table = _table;
		attr = _attr;
	}
	Condition(string cond)
	{
		vector<string> opts{"=", ">", "<", ">=", "<=", "!="};
		for (auto opti : opts)
			if (cond.find(opti) != -1)
			{
				opt = opti;
				string cur = cond.substr(0, cond.find(opti));
				table = cur.substr(0, cur.find("."));
				attr = cur.substr(cur.find(".") + 1);
				cur = cond.substr(cond.find(opti) + opti.size());
				ival = ddbstr2int(cur);
				if (ival == -1)
					type = "string", sval = cur;
				else
					type = "int";
			}
	}
	Condition(string _table, string _attr, int a, int b)
	{
		table = _table;
		attr = _attr;
		type = "int";
		if (a == -INF)
			opt = "<=", ival = b;
		else if (b == INF)
			opt = ">=", ival = a;
		else
			opt = "=", ival = a;
	}
	void get_section(int &a, int &b)
	{
		if (opt == "=")
			a = ival, b = ival;
		if (opt == ">")
			a = ival + 1, b = INF;
		if (opt == "<")
			a = -INF, b = ival - 1;
		if (opt == ">=")
			a = ival, b = INF;
		if (opt == "<=")
			a = -INF, b = ival;
	}
	string table, attr, opt, type; // type->int/string
	int ival;
	string sval;
	void print()
	{
		cout << opt << " " << table << " " << attr << " ";
		if (type == "int")
			cout << ival << endl;
		else
			cout << sval << endl;
	}
};

class Fragment
{
public:
	Fragment(pair<string, int> _fname, int _site, string _type)
	{
		fname = _fname;
		site = _site;
		type = _type;
	}
	pair<string, int> fname;
	int site;
	string type; // vf/hf
	vector<string> vf_column;
	vector<Condition> hf_condition;
};

class Attribute
{
public:
	string type, attr;
	bool is_key;
	pair<string, int> fname;
	int site;
};

class metadataTable
{
public:
	metadataTable(string _name, string _type, string _key)
	{
		name = _name;
		type = _type;
		key_column = _key;
	}
	string name, type;
	int size;
	string key_column;
	vector<Fragment> frags;
	// vector<Attribute> attrs;
	vector<string> attrs;
};