#include<iostream>
#include<map>
#include<vector>
#include<list>
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
	list<int>* pRecordIds;
	int rank;//被跟随者有意义，代表多少人跟随（优化，可根据效率来判断是否需要）
}UfsNode;//the node of union-find set

typedef struct GRPSERV
{
	long id;//关联ID
	long lGrpId;//关联ID
	long lServId;//用户ID
	long pos;//反向索引的位置,初始化为0
	int type;//关联业务类型
	int oldtype;
	long lNewGrpId;//分组ID
	long lSnbr;// 版本号，初始化为0
	char sCityId[8];//本地网
	int flag;
	long lGrpCount;//分组业务关联ID数量
	long lServCount;//用户数
    long lPartRefCount;  //组映射到分区均衡策略参考的数据，根据该字段的值均匀分布到不同的分区上
    long fromGrpId;     // 记录迁出组
    long toGrpId;       // 记录迁入组
}SGRPSERV;

class CGroupAlgorithm { //估计要增加vectSortedAllGrpServ引用成员
public:
	CGroupAlgorithm(vector<GRPSERV>& vectSortedAllGrpServ);
	void mergeAll();
	void printSet();
	void printNode(UfsNode& node);
	void init();
	void destroy();
private:
	void merge(int recordId,UfsKey &k1, UfsKey &k2);
	UfsKey find(UfsKey &x);//这里不能返回引用类型，防止在merge时出错
	void GrpServ2BusiKey(const GRPSERV &record,UfsKey &k);
	void GrpServ2ServKey(const GRPSERV &record,UfsKey &k);
	void initNode(const UfsKey &k,UfsNode &node);
private:
	map<UfsKey, UfsNode> set;
	vector<GRPSERV>& m_vectAllGrpServ;
};

CGroupAlgorithm::CGroupAlgorithm(vector<GRPSERV>& vectSortedAllGrpServ):m_vectAllGrpServ(vectSortedAllGrpServ){}

UfsKey CGroupAlgorithm::find(UfsKey &x) {
	if (x != set[x].followId) {
		//set[x].followId = find(set[x].followId).followId;
		set[x].followId = find(set[x].followId);
	}
	return set[x].followId;
}
void CGroupAlgorithm::printNode(UfsNode& node) {
	printf("node.Id:%ld\t"
			"node.type:%d\t"
			"node.rank:%d\n",
			node.followId.Id,
			node.followId.type,
			node.rank);
	if(node.pRecordIds != NULL) {
		list<int>::iterator vit = node.pRecordIds->begin();
		for(;vit!=node.pRecordIds->end();vit++) {
			printf("\t%d",*vit);
		}
		printf("\n");
	}
}
void CGroupAlgorithm::merge(int recordId,UfsKey &k1, UfsKey &k2) {
	UfsKey x = find(k1);
	UfsKey y = find(k2);
	if ((x == y)) {
		return;
	}
	UfsNode &setx = set[x];//多次使用，引用加快访问速度
	UfsNode &sety = set[y];//多次使用，引用加快访问速度
	
	//指针的引用是存在的
	list<int>* &pxRec = setx.pRecordIds;//多次使用，引用加快访问速度
	list<int>* &pyRec = sety.pRecordIds;//多次使用，引用加快访问速度
	if (setx.rank>sety.rank) {
		sety.followId = x;
		setx.rank += sety.rank;
		//由于rank是初始值为1，x.rank必定大于等于2，此时recordIds中必定有数据，直接判断y中是否含有recordid即可
		if (pyRec != NULL) {
			pxRec->splice(pxRec->end(), *pyRec);
			delete pyRec;
			pyRec = NULL;
		} 
		pxRec->push_back(recordId);
	}
	else { //x.rank<=y.rank
		setx.followId = y;
		sety.rank += setx.rank;
		if (pyRec == NULL) {
			pyRec = new list<int>;
		} else if (pxRec != NULL)  {
			pyRec->splice(pyRec->end(), *pxRec);
			delete pxRec;
			pxRec = NULL;
		} 
		pyRec->push_back(recordId);
	}
} 
void CGroupAlgorithm::destroy()
{
	map<UfsKey, UfsNode>::iterator it;
	map<UfsKey, UfsNode>::iterator end = set.end();
	
	for(it=set.begin();it!=end;it++)
	{
		list<int>* &pRecordIds = it->second.pRecordIds;
		if(pRecordIds != NULL) {
			delete pRecordIds;
			pRecordIds = NULL;
		}
	}
}

inline void CGroupAlgorithm::GrpServ2BusiKey(const GRPSERV &record,UfsKey &k) {
	k.Id = record.lGrpId;
	k.type = (record.type/100*100);
}

inline void CGroupAlgorithm::GrpServ2ServKey(const GRPSERV &record,UfsKey &k) {
	k.Id = record.lServId;
	k.type = (record.type%10);
}
inline void CGroupAlgorithm::initNode(const UfsKey &k,UfsNode &node) {
	node.followId = k;
	node.rank = 1;
	node.pRecordIds = NULL;
}

void CGroupAlgorithm::init() {
	int size = m_vectAllGrpServ.size();
	UfsKey key;
	UfsNode node;
	for (int i=0; i<size; i++) {
		GrpServ2BusiKey(m_vectAllGrpServ[i], key);
		initNode(key, node);
		set.insert(pair<UfsKey, UfsNode>(key, node));

		GrpServ2ServKey(m_vectAllGrpServ[i], key);
		initNode(key, node);
		set.insert(pair<UfsKey, UfsNode>(key, node));
	}
}

void CGroupAlgorithm::mergeAll() {
	int size = m_vectAllGrpServ.size();
	UfsKey key1;
	UfsKey key2;
	int i;
	for (i=0;i<size;i++) {
		GrpServ2BusiKey(m_vectAllGrpServ[i], key1);
		GrpServ2ServKey(m_vectAllGrpServ[i], key2);
		merge(i,key1,key2);
	}
}

void CGroupAlgorithm::printSet() {
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
			list<int>::iterator vit = node.pRecordIds->begin();
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
	vector<GRPSERV> vectSortedAllGrpServ;
	GRPSERV grp;
	long busiId;
	long servId;
	int type;
	int n;
	int i;
	while(scanf("%d",&n)!=EOF) {
		for (i=0;i<n;i++) {
			scanf("%ld %ld %d",&busiId,&servId,&type);
			
			grp.lGrpId = busiId;
			grp.lServId = servId;
			grp.type = type;
			vectSortedAllGrpServ.push_back(grp);
		}
		CGroupAlgorithm cg(vectSortedAllGrpServ);
		cg.init();
		cg.mergeAll();
		cg.printSet();
		cg.destroy();
		vectSortedAllGrpServ.clear();
	}
	return 0;
}
/*
4
1 2 101
1 2 201
4 3 301
4 5 301
4
1 2 101
1 3 201
4 3 301
4 5 301
4
1 2 101
1 3 101
4 3 301
4 5 301
*/
