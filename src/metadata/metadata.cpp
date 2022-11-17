#include "metadata.h"

/*
存储所有表格的名称 </table, "publisher,customer,book,order">
存储表格的列名 </table/publisher/attr, "id,name,nation">
存储每列的属性 </table/publisher/attr/id, int>
			   </table/publisher/attr/name, string>
存储主键 </table/publisher/key, "id">
存储分片类型 </table/publisher/schema, "hf">
存储分片的数量 </table/publisher/fragment_num, 4>
存储分片上的列（垂直分片） <table/customer/fragment/site/1, "id,name">
存储水平分片的条件 </table/publisher/fragment/site/1, "id<104000 and nation=PRC">
存储分片上的数据量 </table/publisher/fragment/site/size, 100>
存储分片所在的位置 </table/publisher/fragment/site/1, 1> 1号分片在1号站点
*/


int main(){
	string str = EncodeBase64("hellosite1");
	cout<<str<<endl;
	cout<<DecodeBase64(str)<<endl;
	string t =  "{\"key\": \""+EncodeBase64("test")+"\", \"value\": \""+EncodeBase64("hello etcd")+"\"}";
    string op = "PUT";
    string res = etcd_opt(t,op);  //res is json string
	cout<<res<<endl;
	t = "{\"key\":\""+EncodeBase64("test")+"\"}";
	op = "GET";
	res = etcd_opt(t, op);
	cout<<res<<endl;
	return 0;
}