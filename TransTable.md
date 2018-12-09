# Transposition Table 置换表
置换表的作用是把MinMax过程中搜索到的节点存储起来，下次遇到同样的局面时直接从表中获取该局面的启发评价值和最优分支，避免不必要的重复搜索。因为涉及到快速的存储和查询，很自然地就想到了用哈希表来实现。

## 1.棋盘局面的哈希表示

### 1.1. 低效的字符串哈希
一开始想到的是把整个棋盘表示成一个3进制的数，对于棋盘上每一个格子：

|格子|代表数值|
|:---:|:---:|
|空格子|0|
|黑棋|1|
|白棋|2|

这样的话，一个20*20大小的棋盘可以映射为一个400位的三进制数：001002001120...
编程实现用一个长度为400的字符串来存储，再用常用的字符串哈希方法映射为哈希值。。

真要这样做的话，那么每次存储/查询一个局面时，都得构造这样的一个长度为400的字符串，然后是冗长麻烦的字符串哈希……效率真的很低，很耗时间，所以很快就否定了这种思路。

### 1.2. Zobrist Hash
该方法是Zobrist在1970年的一篇论文中提出的快速求哈希值的方法。主要思路是：
1. 用一个随机数 currentBoardHashVal 表示棋盘的哈希值
2. 为棋盘上的每一个格子赋予一个随机数，用一个三维数组 hashValTable\[player\]\[width\]\[height\] 来表示。如果棋盘位置(x, y)上是黑棋，currentBoardHashVal 就加上 hashValTable\[0\]\[x\]\[y\]。在局面搜索过程中，当(x, y)位置上的黑棋被拿掉时，currentBoardHashVal 就减去 hashValTable\[0\]\[x\]\[y\]
 
Q: 为什么不是用二维随机数数组 hashValTable[width\]\[height\] 呢？

A: 如果是二维数组的话，那么调换棋盘上任意一个黑棋和白棋的位置，该局面的哈希值相同，会被错误地认为是同一局面。

tips: 实际编程中，可以用异或运算来代替哈希值的+-运算。一方面是因为计算机做位运算较快，更重要的是因为异或运算具有这样的性质：a ^ b ^ b = a，等价于 a + b - b = a，即在棋盘上放一个棋子，再拿走，表示棋盘的哈希值不变。

#### 防止哈希值碰撞的策略：校验和
因为整数表示的哈希值容易产生碰撞，即两个表示不同的局面可能有相同的哈希值。为了防止这个情况发生，需要有一个**校验和（checksum）** 来表示该局面。如果两个局面的哈希值相同，而校验和不同，那么它们会被认为是不同局面。这样的话，一个局面可以表示成：

```javascript
{
	 hashVal: 1339368269,
	checksum: 2226023961
}
```

Q: 局面的校验和怎么计算？

A: 那就再加一个三维随机数数组 checksumTabe\[player\]\[width\]\[height\] 呗。

在《PC游戏编程（人机博弈）》中给出的伪代码实现中，hashVal被处理为哈希表的key，而checksum被存储为哈希表的value。我觉得一个更好的实现是哈希表顺便把checksum的实现细节也封装了，即hashVal和checksum都存储在key中。**当哈希表把局面的表示映射为数组下标时，用的是hashVal，当哈希表检查对应下标的 key k1 和当前要查找的 key k2 是相同值时，需要满足 k1.hashVal = k2.hashVal 且 k1.checksum = k2.checksum 的条件。**

Q: 在C++中，怎样实现这样的哈希表呢？

A: 定义数据结构 MultiKeysNode: 
1. MultiKeysNode的hash方法返回值为 hashVal；
2. 重载 == 运算符，return k1.hashVal == k2.hashVal && k1.checksum == k2.checksum; 因为STL哈希表的实现中，检验键是否相同，用的就是 == 运算符。类似的，Java中是用equals方法（可以参考 java.util.HashMap 的源码）

## 2.局面节点的相关数据的存储
上面第一节的内容是关于棋盘局面的哈希值表示，即哈希表的Key，这里说的是哈希表的Value，存储局面节点的数据结构 BoardNode：

```javascript
{
	score: 9999 // 局面的启发评价值
	depth: 4 // 当前局面往下搜索的深度
	move: branch2 // 当前局面的最优分支走法  
	// turn: 0 // 如果有必要，记录当前局面是轮到哪一方走棋
	scoreType: EXACT, LOWER_BOUND 或 UPPER_BOUND // 启发评价值的类型 
}
```

因为搜索的深度越深，得到的启发评价值越可靠。所以记录局面往下搜索的深度，便于对是否使用该启发评价值作出判断：如果当前局面往下搜索的深度大于记录的深度，那么舍弃该值，继续往下搜索。~当然也可以尝试用该启发评价值调整AlphaBeta搜索的窗口 (apha, beta)~

而记录局面的启发评价值类型，是因为在AlphaBeta搜索中，因为剪枝的存在，一个节点的返回值有可能不是精确值。AlphaBeat搜索中存在3种节点：
* Alpha节点：该节点往下搜索的各个分支的启发评价值都小于alpha值，该节点返回的**实际启发评价值**小于alpha值，即alpha值表示着一个上限 UPPER_BOUND
* Beta节点：该节点的某一分支引发剪枝，导致该节点的所有分支没有搜索完，该节点返回的实际启发评价值小于实际的值，代表着一个下限 LOWER_BOUND
* PV节点：该节点返回的启发评价值等于实际上的值

### 3.C++代码

```c++
// TransTable.h

#ifndef TRANSTABLE
#define TRANSTABLE

#include "board.h"
#include "skelet/pisqpipe.h"
#include "evaluation.h"
#include "historytable.h"

#include <unordered_map>
using std::unordered_map;

class MultiKeysNode
{
public:
	MultiKeysNode(size_t hashVal = 0, size_t checksum = 1);
	~MultiKeysNode();

	size_t getHashVal() const;
	bool operator==(const MultiKeysNode& other) const; // 重载 == 运算符
private:
	size_t hashVal;
	size_t checksum;
};

struct MultiKeysNode_hash {
	size_t operator()(const MultiKeysNode& node) const
	{
		return node.getHashVal();
	}
};

enum ScoreType {EXACT, LOWER_BOUND, UPPER_BOUND};

class BoardNode
{
private:
	int depth;
	int score;
	Mov move;
	ScoreType scoreType;
	//int turn; 
	// 对于五子棋来说，只要确定了先手方，根据棋子数目是奇数还是偶数，接下来轮到谁走棋是固定的
	// 所以对于一个局面来说，没有必要记录turn
public:

	BoardNode()
	{

	}

	BoardNode(int score, int depth, Mov move, ScoreType scoreType=EXACT)
	{
		this->depth = depth;
		this->score = score;
		this->move = move;
		this->scoreType = scoreType;
	}

	// 省略一堆set, get
};


class TransTable
{
private:
	unordered_map<MultiKeysNode, BoardNode, MultiKeysNode_hash>* boardNodeMap;

	// 实际编程实现中，没有使用三维数组，使用一维长数组+下标转化方法tableIdxFromBoardIdx，效率更优一些
	int rndTableSize;
	size_t *hashIndexTable;
	size_t *checksumTable;

	size_t currBoardHashVal;
	size_t currBoardChecksum;

	void initRandomTable();
	int tableIdxFromBoardIdx(int player, int x, int y) const;
	MultiKeysNode currKeyNode();

	// 置换表插入次数和查询成功次数，用于衡量置换表的效率
	int insertCnt;
	int searchHitCnt;

protected:
	//static TransTable *instance;
	TransTable(int initCapacity=16);
	//TransTable(const TransTable& other);
	//TransTable& operator=(const TransTable& other);
	
	~TransTable();

public:
	static TransTable& getInstance();

	// 标记棋盘上的棋子
	void MarkMove(int player, int x, int y);
	void UnmarkMove(int player, int x, int y);

	void insertCurrBoardNode(int score, int depth, Mov move = {-1, -1, -10001}, ScoreType scoreType = EXACT);
	bool searchCurrBoardNode();
	BoardNode* getCurrBoardNode(); // 获取当前棋盘节点的BoardNode

	int getSize() const;

	size_t getCurrBoardChecksum() const;
	size_t getCurrBoardHashVal() const;
	
	int getInsertCnt() const;
	int getSearchHitCnt() const;
	void incrementSearchHitCnt();
	double getHitRate() const;
};

#endif // !TransTable
```

```c++
// TransTable.cpp
#include "TransTable.h"
#include <random>
#include <ctime>
#include <math.h>

MultiKeysNode::MultiKeysNode(size_t hashVal, size_t checksum)
{
	this->hashVal = hashVal;
	this->checksum = checksum;
}

MultiKeysNode::~MultiKeysNode()
{

}

// 重写比较运算符，这样哈希表在比较时就会把hashVal相同，但是checksum不同的两个Key当成不相等的两个键
bool MultiKeysNode::operator==(const MultiKeysNode& other) const
{
	return hashVal == other.hashVal && checksum == other.checksum;
}

size_t MultiKeysNode::getHashVal() const
{
	return hashVal;
}

// **************************************************************

TransTable& TransTable::getInstance()
{
	static TransTable TransTableInstance; // 函数的静态变量，直到整个程序的生命周期结束才销毁
	return TransTableInstance;
}

TransTable::TransTable(int initCapcity)
{
	boardNodeMap = new unordered_map<MultiKeysNode, BoardNode, MultiKeysNode_hash>(initCapcity);
	insertCnt = searchHitCnt = 0;
	initRandomTable();
}

TransTable::~TransTable()
{
	delete boardNodeMap;
	boardNodeMap = NULL;
}

void TransTable::initRandomTable()
{
	rndTableSize = width * height * 2;

	hashIndexTable = new size_t[rndTableSize];
	checksumTable = new size_t[rndTableSize];

	std::default_random_engine rnd_engine(static_cast<unsigned int> (time(0)));
	std::uniform_int_distribution<long> rnd_long(LONG_MIN, LONG_MAX);

	for (int i = 0; i < rndTableSize; i++)
	{
		hashIndexTable[i] = rnd_long(rnd_engine);
		checksumTable[i] = rnd_long(rnd_engine);
	}

	currBoardHashVal = rnd_long(rnd_engine);
	currBoardChecksum = rnd_long(rnd_engine);
}

int TransTable::tableIdxFromBoardIdx(int player, int x, int y) const
{
	return x * height + y + player * width * height;
}

MultiKeysNode TransTable::currKeyNode()
{
	return MultiKeysNode(currBoardHashVal, currBoardChecksum);
}

void TransTable::MarkMove(int player, int x, int y)
{
	currBoardHashVal ^= hashIndexTable[tableIdxFromBoardIdx(player, x, y)];
	currBoardChecksum ^= checksumTable[tableIdxFromBoardIdx(player, x, y)];
}

void TransTable::UnmarkMove(int player, int x, int y)
{
	currBoardHashVal ^= hashIndexTable[tableIdxFromBoardIdx(player, x, y)];
	currBoardChecksum ^= checksumTable[tableIdxFromBoardIdx(player, x, y)];
}

void TransTable::insertCurrBoardNode(int score, int depth, Mov move, ScoreType scoreType)
{
	BoardNode node(score, depth, move, scoreType);
	(*boardNodeMap)[currKeyNode()] = node;
	insertCnt++;
}

bool TransTable::searchCurrBoardNode()
{
	return boardNodeMap->find(currKeyNode()) != boardNodeMap->end();
}

BoardNode* TransTable::getCurrBoardNode()
{
	return &(boardNodeMap->at(currKeyNode()));
}

int TransTable::getSize() const
{
	return boardNodeMap->size();
}

size_t TransTable::getCurrBoardChecksum() const
{
	return currBoardChecksum;
}

size_t TransTable::getCurrBoardHashVal() const 
{
	return currBoardHashVal;
}

int TransTable::getInsertCnt() const
{
	return insertCnt;
}

int TransTable::getSearchHitCnt() const
{
	return searchHitCnt;
}

void TransTable::incrementSearchHitCnt()
{
	searchHitCnt++;
}

double TransTable::getHitRate() const
{
	return 1.0 * searchHitCnt / insertCnt;
}

```

