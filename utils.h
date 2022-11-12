#include <vector>

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
		Condition(string _opt,string _table,string _attr){
			opt=_opt;
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

class Fragment{
	public:
		pair<string,int> fname;
		int site;
		string type; // vf/hf
		vector<string> vf_column;
		vector<Condition> hf_condition;
};

class Attribute{
	public:
		string type,attr;
		bool is_key;
		pair<string,int> fname;
		int site;
};

class metadataTable{
	public:
		string name,type;
		int size;
		vector<Fragment> frags;
		vector<Attribute> attrs;
};
