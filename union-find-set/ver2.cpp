#include<iostream>
#include<map>
#include<vector>
#include<stdio.h>
using namespace std;

typedef struct UFS_KEY_S {
	long Id;//busi_id或者serv_id
	//int recordId;//该结构存的是busi_id时，recordId为有效值;为的是防止出现busiid和servid相同并且type也相同的情况导致错误
	int type;//标志id的类型
	bool operator!=(const struct UFS_KEY_S &t)
	{
		return (Id!=t.Id)||(type!=t.type);	
	}
	bool operator==(const struct UFS_KEY_S &t)
	{
		return (Id==t.Id)&&(type==t.type);//(recordId==t.recordId)&&	
	}
}UfsKey;//the key of union-find set
bool operator <(const UfsKey &key1,const UfsKey &key2)  
{  
    if((key1.Id<key2.Id)||((key1.Id==key2.Id)&&(key1.type<key2.type)))  
        return true;  
    else  
        return false;  
}  

typedef struct UFS_NODE_S {
	UfsKey followId;//并查集 被跟随者ID，初始化时为跟随自身
	vector<int>* pRecordIds;
	int rank;//被跟随者有意义，代表多少人跟随（优化，可根据效率来判断是否需要）
}UfsNode;//the node of union-find set

map<UfsKey, UfsNode> set;

UfsKey find(UfsKey &x) {
	if (x != set[x].followId) {
		//set[x].followId = find(set[x].followId).followId;
		set[x].followId = find(set[x].followId);
	}
	return set[x].followId;
}
void printNode(UfsNode& node) {
	printf("node.Id:%ld\t"
			"node.type:%d\t"
			"node.rank:%d\n",
			node.followId.Id,
			node.followId.type,
			node.rank);
	if(node.pRecordIds != NULL) {
		vector<int>::iterator vit = node.pRecordIds->begin();
		for(;vit!=node.pRecordIds->end();vit++) {
			printf("\t%d",*vit);
		}
		printf("\n");
	}
}
void merge(int recordId,UfsKey &k1, UfsKey &k2) {
	UfsKey x = find(k1);
	UfsKey y = find(k2);
	if ((x == y)) {
		return;
	}
	UfsNode &setx = set[x];//多次使用，引用加快访问速度
	UfsNode &sety = set[y];//多次使用，引用加快访问速度
	
	//指针的引用是存在的
	vector<int>* &pxRec = setx.pRecordIds;//多次使用，引用加快访问速度
	vector<int>* &pyRec = sety.pRecordIds;//多次使用，引用加快访问速度
	if (setx.rank>sety.rank) {
		sety.followId = x;
		setx.rank += sety.rank;
		//由于rank是初始值为1，x.rank必定大于等于2，此时recordIds中必定有数据，直接判断y中是否含有recordid即可
		if (pyRec != NULL) {
			pxRec->assign(pyRec->begin(), pyRec->end());
			delete pyRec;
			pyRec = NULL;
		} 
		pxRec->push_back(recordId);
	}
	else { //x.rank<=y.rank
		setx.followId = y;
		sety.rank += setx.rank;
		if (pyRec == NULL) {
			pyRec = new vector<int>;
		} else if (pxRec != NULL)  {
			pyRec->assign(pxRec->begin(), pxRec->end());
			delete pxRec;
			pxRec = NULL;
		} 
		pyRec->push_back(recordId);
	}
} 
void printSet()
{
	map<UfsKey, UfsNode>::iterator it;
	UfsNode node;
	UfsKey key;
	printf("size:%d\n\n",set.size());
	for(it=set.begin();it!=set.end();it++)
	{
		key = it->first;
		node = it->second;
		printf("key.Id:%ld\t"
				"key.type:%d\n"
				"node.Id:%ld\t"
				"node.type:%d\t"
				"node.rank:%d\n",
				key.Id,(int)key.type,
				node.followId.Id,node.followId.type,node.rank);
		if(node.pRecordIds != NULL) {
			vector<int>::iterator vit = node.pRecordIds->begin();
			for(;vit!=node.pRecordIds->end();vit++) {
				printf("\t%d",*vit);
			}
			printf("\n");
		}
		printf("\n");
	}
}
//mapStudent.insert(pair<string, string>("r000", "student_zero"));
int main() {
	UfsKey key;
	UfsNode node;
	long busiId;
	long servId;
	int type;
	int recordId;
	vector<UfsKey> v1;
	vector<UfsKey> v2;
	vector<int> recordIds;
	int n;
	int i;
	while(scanf("%d",&n)!=EOF) {
		for (i=0;i<n;i++) {
			scanf("%d %ld %ld %d",&recordId,&busiId,&servId,&type);
			
			key.Id = busiId;
			key.type = (type/100*100);
			node.followId = key;
			node.rank = 1;
			node.pRecordIds = NULL;
			set.insert(pair<UfsKey, UfsNode>(key, node));
			v1.push_back(key);

			key.Id = servId;
			key.type = (type%10);
			node.followId = key;
			node.rank = 1;
			node.pRecordIds = NULL;
			set.insert(pair<UfsKey, UfsNode>(key, node));
			v2.push_back(key);

			recordIds.push_back(recordId);
		}
		int size = v1.size();
		for (i=0;i<size;i++) {
			merge(recordIds[i],v1[i],v2[i]);
		}
		printSet();
		v1.clear();
		v2.clear();
		set.clear();
	}
	return 0;
}
/*
4
1 1 2 101
2 1 2 201
3 4 3 301
4 4 5 301
4
1 1 2 101
2 1 3 201
3 4 3 301
4 4 5 301
4
1 1 2 101
2 1 3 101
3 4 3 301
4 4 5 301
*/
