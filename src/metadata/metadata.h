#include <bits/stdc++.h>
#include <curl/curl.h>
#include <json/json.h>

using namespace std;
using namespace Json;


const char *base64_key = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
//三个字节为一组，编码成四个字节
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

size_t write_data(void *buffer, size_t size, size_t nmemb, void *stream) 
{ 
    strncat((char*)stream,(char*)buffer,size*nmemb);
    return nmemb*size; 
} 

string etcd_opt(string &data,string &op) 
{
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

    return string(result);
}
