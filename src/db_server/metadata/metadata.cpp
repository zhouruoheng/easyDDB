#include "metadata.h"

/*
存储所有表格的名称 </table, "Publisher,Customer,Book,Order">
存储表格的列名 </table/Publisher/attr, "Publisher.id,Publisher.name,Publisher.nation">
存储每列的属性 </table/Publisher/attr/Publisher.id, int>
			   </table/Publisher/attr/Publisher.name, string>
存储主键 </table/Publisher/key, "Publisher.id">
存储分片类型 </table/Publisher/schema, "hf">
存储分片的数量 </table/Publisher/fragment_num, 4>
存储分片上的列（垂直分片） <table/Customer/fragment/1/columns, "Customer.id,Customer.name">
存储水平分片的条件 </table/Publisher/fragment/1/conditions, "Publisher.id<104000,Publisher.nation=PRC">
存储分片上的数据量 </table/Publisher/fragment/1/size, 100>
存储分片所在的位置 </table/Publisher/fragment/1/pos, 1> 1号分片在1号站点
*/

string EncodeBase64(const string inputStr)
{
    string strEncode;
    //行长,MIME规定Base64编码行长为76字节
    int LineLength=0;

    for(int i=0;i<inputStr.size();i+=3){
		char temp[4];
		memset(temp,0,4);

		temp[0]=inputStr[i];
		temp[1]=inputStr[i+1];
		temp[2]=inputStr[i+2];
		LineLength+=4;

		int len=strlen(temp);
		switch (len){
			case 1:
				strEncode+=base64_key[((int)temp[0] & 0xFC)>>2];
				strEncode+=base64_key[((int)temp[0] & 0x3 )<<4];
				if(LineLength>=76) strEncode+="\r\n";
				strEncode+="==";
				break;
			case 2:
				strEncode+=base64_key[((int)temp[0] & 0xFC)>>2];
				strEncode+=base64_key[((int)temp[0] & 0x3 )<<4 | ((int)temp[1] & 0xF0 )>>4];
				strEncode+=base64_key[((int)temp[1] & 0x0F)<<2];
				if(LineLength>=76) strEncode+="\r\n";
				strEncode+="=";
				break;
			case 3:
				strEncode+=base64_key[((int)temp[0] & 0xFC)>>2];
				strEncode+=base64_key[((int)temp[0] & 0x3)<<4 | ((int)temp[1] & 0xF0)>>4];
				strEncode+=base64_key[((int)temp[1] & 0xF)<<2 | ((int)temp[2] & 0xC0)>>6];
				strEncode+=base64_key[(int)temp[2] & 0x3F];
				if(LineLength>=76) strEncode+="\r\n";
				break;
		}
    }
    return strEncode;
}

string DecodeBase64(const string inputStr)
{
    string strDecode;

    for(int i=0;i<inputStr.size();i+=4)
    {
		char temp[5];
		memset(temp,0,5);

		temp[0]=inputStr[i];
		temp[1]=inputStr[i+1];
		temp[2]=inputStr[i+2];
		temp[3]=inputStr[i+3];

		int asc[4];
		for(int j=0;j<4;j++)
			for(int k=0;k<(int)strlen(base64_key);k++)
				if(temp[j]==base64_key[k]) asc[j]=k;
		if(temp[2] =='='&&temp[3]=='=') strDecode+=(char)(int)(asc[0] << 2 | asc[1] << 2 >> 6);
		else if(temp[3]=='=')
			strDecode+=(char)(int)(asc[0] << 2 | asc[1] << 2 >> 6),
			strDecode+=(char)(int)(asc[1] << 4 | asc[2] << 2 >> 4);
		else
			strDecode+=(char)(int)(asc[0] << 2 | asc[1] << 2 >> 6),
			strDecode+=(char)(int)(asc[1] << 4 | asc[2] << 2 >> 4),
			strDecode+=(char)(int)(asc[2] << 6 | asc[3] << 2 >> 2); 
    }
    return strDecode;
}

vector<string> string2list(string inputStr)
{
    stringstream Str(inputStr);
    vector<string> temp;
    string cur;
    while(getline(Str, cur, ',')) temp.push_back(cur);
    return temp;
}

string string2json(string key, string value)
{
    string cur;
    if (value.size()) cur = "{\"key\": \""+EncodeBase64(key)+"\", \"value\": \""+EncodeBase64(value)+"\"}";
    else cur = "{\"key\":\""+EncodeBase64(key)+"\"}";
    return cur;
}

string json2string(string inputStr)
{
    Value root, node;
    Reader reader;
    FastWriter writer;
    if (!reader.parse(inputStr, root)){
        cout<<"Parse json error"<<endl;
        return "";
    }
    string nodeStr = writer.write(root["kvs"]);
    if (!reader.parse(nodeStr.substr(1,nodeStr.size()-2), node)){
        cout<<"Parse json error"<<endl;
        return "";
    }
    string cur = writer.write(node["value"]);
	//cout<<DecodeBase64(cur.substr(1, cur.size()-3))<<endl;
    return DecodeBase64(cur.substr(1, cur.size()-3));
}

size_t write_data(void *buffer, size_t size, size_t nmemb, void *stream) 
{ 
    strncat((char*)stream,(char*)buffer,size*nmemb);
    return nmemb*size; 
} 

string etcd_opt(string data,string op) 
{
	//cout<<data<<endl;
    string etcd_url = "http://127.0.0.1:2379/v3/kv/";
    if(op == "PUT"){
        etcd_url += "put";
    }
    else if(op == "GET"){
        etcd_url+= "range";
    }
    else{
        cout << "illegal opration !" << endl;
        return "";
    }
    char *ss=(char*)etcd_url.c_str();
    CURLcode return_code;  
    return_code = curl_global_init(CURL_GLOBAL_SSL);  
    if (CURLE_OK != return_code)  
    {  
        cerr << "init libcurl failed." << endl;  
        return "";  
    }  
    // 获取easy handle  
    CURL *easy_handle = curl_easy_init();  
    if (NULL == easy_handle)  
    {  
        cerr << "get a easy handle failed." << endl;  
        curl_global_cleanup();   
        return "";  
    }  
    char szJsonData[1024]; 
    strcpy(szJsonData, data.c_str()); 
    char * buff_p = NULL;  
    char result[5000] = "";
    // 设置easy handle属性  
    curl_slist *plist = curl_slist_append(NULL,   
                "Content-Type:application/json;charset=UTF-8");  
    curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, plist);
    curl_easy_setopt(easy_handle, CURLOPT_URL,ss);  
    curl_easy_setopt(easy_handle, CURLOPT_PORT, 2379); 
    curl_easy_setopt(easy_handle, CURLOPT_POSTFIELDS, szJsonData);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &write_data);  
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &result);  
  
    // 执行数据请求  
    curl_easy_perform(easy_handle);   
  
    // 释放资源  
    curl_easy_cleanup(easy_handle);  
    curl_global_cleanup();  
	//cout<<"sucess!"<<endl;
    return string(result);
}

void putTables()
{
	etcd_opt(string2json("/table","Publisher,customer,book,order"), "PUT");
	etcd_opt(string2json("/table/Publisher/attr","Publisher.id,Publisher.name,Publisher.nation"), "PUT");
	etcd_opt(string2json("/table/Publisher/attr/Publisher.id","int"), "PUT");
	etcd_opt(string2json("/table/Publisher/attr/Publisher.name","string"), "PUT");
	etcd_opt(string2json("/table/Publisher/attr/Publisher.nation","string"), "PUT");
	etcd_opt(string2json("/table/Publisher/key","Publisher.id"), "PUT");
	etcd_opt(string2json("/table/Publisher/schema","hf"), "PUT");
	etcd_opt(string2json("/table/Publisher/fragment_num","4"), "PUT");
	etcd_opt(string2json("/table/Publisher/fragment/1/pos","1"), "PUT");
	etcd_opt(string2json("/table/Publisher/fragment/2/pos","2"), "PUT");
	etcd_opt(string2json("/table/Publisher/fragment/3/pos","3"), "PUT");
	etcd_opt(string2json("/table/Publisher/fragment/4/pos","4"), "PUT");
	etcd_opt(string2json("/table/Publisher/fragment/1/conditions","Publisher.id<104000,Publisher.nation=PRC"), "PUT");
	etcd_opt(string2json("/table/Publisher/fragment/2/conditions","Publisher.id<104000,Publisher.nation=USA"), "PUT");
	etcd_opt(string2json("/table/Publisher/fragment/3/conditions","Publisher.id>=104000,Publisher.nation=PRC"), "PUT");
	etcd_opt(string2json("/table/Publisher/fragment/4/conditions","Publisher.id>=104000,Publisher.nation=USA"), "PUT");
}

vector<metadataTable> getTables()
{
    vector<metadataTable> Tabs;
    vector<string> tab=string2list(json2string(etcd_opt(string2json("/table",""), "GET")));
	int cnt=0;
    for (auto x : tab){
		if (++cnt > 1 ) 
			break;
        string key = json2string(etcd_opt(string2json("/table/"+x+"/key",""), "GET"));
        string type = json2string(etcd_opt(string2json("/table/"+x+"/schema",""), "GET"));
        metadataTable temp=metadataTable(x, type, key);
        temp.attrs = string2list(json2string(etcd_opt(string2json("/table/"+x+"/attr",""), "GET")));
        int fragNum = atoi(json2string(etcd_opt(string2json("/table/"+x+"/fragment_num",""), "GET")).c_str());
        for (int i=1;i<=fragNum;i++){
            string url = "/table/"+x+"/fragment/"+to_string(i)+"/";
            int pos = atoi(json2string(etcd_opt(string2json(url+"pos",""), "GET")).c_str());
            Fragment fr = Fragment(make_pair(x,1), pos, type);
            if (type=="hf") {
                vector<string> cons = string2list(json2string(etcd_opt(string2json(url+"conditions",""), "GET")));
                for (auto con : cons) fr.hf_condition.push_back(Condition(con));
            }
            else fr.vf_column = string2list(json2string(etcd_opt(string2json(url+"columns",""), "GET")));
            temp.frags.push_back(fr);
        }
        Tabs.push_back(temp);
    }
    return Tabs;
}


// int main(){
// 	string t, op, res;
// 	//string t =  "{\"key\": \""+EncodeBase64("test")+"\", \"value\": \""+EncodeBase64("hello etcd")+"\"}";
//     //string op = "PUT";
//     //string res = etcd_opt(t,op);  //res is json string
// 	//cout<<res<<endl;
// 	t = "{\"key\":\""+EncodeBase64("/table/publisher/fragment/4/conditions")+"\"}";
// 	op = "GET";
// 	res = etcd_opt(t, op);
// 	cout<<json2string(res)<<endl;
// 	//putTables();
// 	vector<metadataTable> Tabs=getTables();
// 	return 0;
// }
