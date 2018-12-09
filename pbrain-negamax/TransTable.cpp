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

void TransTable::insertCurrBoardNode(int score, int depth, int turn, Mov move, ScoreType scoreType)
{
	BoardNode node(score, depth, turn, move, scoreType);
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

