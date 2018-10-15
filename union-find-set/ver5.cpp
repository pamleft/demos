#include<iostream>
#include<map>
#include<vector>
#include<list>
#include<stdio.h>
using namespace std;

typedef struct UFS_KEY_S {
	long Id;//busi_id����serv_id
	//int recordId;//�ýṹ�����busi_idʱ��recordIdΪ��Чֵ;Ϊ���Ƿ�ֹ����busiid��servid��ͬ����typeҲ��ͬ��������´���
	int type;//��־id������
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
	UfsKey followId;//���鼯 ��������ID����ʼ��ʱΪ��������
	list<int>* pRecordIds;
	int rank;//�������������壬��������˸��棨�Ż����ɸ���Ч�����ж��Ƿ���Ҫ��
}UfsNode;//the node of union-find set

typedef struct GRPSERV
{
	long id;//����ID
	long lGrpId;//����ID
	long lServId;//�û�ID
	long pos;//����������λ��,��ʼ��Ϊ0
	int type;//����ҵ������
	int oldtype;
	long lNewGrpId;//����ID
	long lSnbr;// �汾�ţ���ʼ��Ϊ0
	char sCityId[8];//������
	int flag;
	long lGrpCount;//����ҵ�����ID����
	long lServCount;//�û���
    long lPartRefCount;  //��ӳ�䵽����������Բο������ݣ����ݸ��ֶε�ֵ���ȷֲ�����ͬ�ķ�����
    long fromGrpId;     // ��¼Ǩ����
    long toGrpId;       // ��¼Ǩ����
}SGRPSERV;

class CGroupAlgorithm { //����Ҫ����vectSortedAllGrpServ���ó�Ա
public:
	CGroupAlgorithm(vector<GRPSERV>& vectSortedAllGrpServ);
	void mergeAll();
	void printSet();
	void printNode(UfsNode& node);
	void init();
	void destroy();
	void stampGroupIdAndFree(long &lStartGroupId);//in-out
private:
	void merge(int recordId,UfsKey &k1, UfsKey &k2);
	UfsKey find(UfsKey &x);//���ﲻ�ܷ����������ͣ���ֹ��mergeʱ����
	void GrpServ2BusiKey(const GRPSERV &record,UfsKey &k);
	void GrpServ2ServKey(const GRPSERV &record,UfsKey &k);
	void initNode(const UfsKey &k,UfsNode &node);
	void stampGroupId(const list<int> &recordIds,long groupId);//����groupId
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
	UfsNode &setx = set[x];//���ʹ�ã����üӿ�����ٶ�
	UfsNode &sety = set[y];//���ʹ�ã����üӿ�����ٶ�
	
	//ָ��������Ǵ��ڵ�
	list<int>* &pxRec = setx.pRecordIds;//���ʹ�ã����üӿ�����ٶ�
	list<int>* &pyRec = sety.pRecordIds;//���ʹ�ã����üӿ�����ٶ�
	if (setx.rank>sety.rank) {
		sety.followId = x;
		setx.rank += sety.rank;
		//����rank�ǳ�ʼֵΪ1��x.rank�ض����ڵ���2����ʱrecordIds�бض������ݣ�ֱ���ж�y���Ƿ���recordid����
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

void CGroupAlgorithm::destroy() {
	map<UfsKey, UfsNode>::iterator it = set.begin();
	map<UfsKey, UfsNode>::iterator end = set.end();
	
	for(;it!=end;it++) {
		list<int>* &pRecordIds = it->second.pRecordIds;
		if(pRecordIds != NULL) {
			delete pRecordIds;
			pRecordIds = NULL;
		}
	}
}

void CGroupAlgorithm::stampGroupId(const list<int> &recordIds,long groupId) {
	list<int>::const_iterator it = recordIds.begin();
	list<int>::const_iterator end = recordIds.end();

	for(;it!=end;it++) {
		m_vectAllGrpServ[*it].lNewGrpId = groupId;
	}
}

void CGroupAlgorithm::stampGroupIdAndFree(long &lStartGroupId) {
	map<UfsKey, UfsNode>::iterator it = set.begin();
	map<UfsKey, UfsNode>::iterator end = set.end();
	//lStartGroupId��������
	for(;it!=end;it++) {
		list<int>* &pRecordIds = it->second.pRecordIds;
		if(pRecordIds != NULL) {
			stampGroupId(*pRecordIds, lStartGroupId);
			lStartGroupId++;
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