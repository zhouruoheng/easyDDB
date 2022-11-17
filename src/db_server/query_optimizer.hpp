
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