#pragma once

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
	bool operator==(const MultiKeysNode& other) const; // ���� == �����
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
	int turn;
	ScoreType scoreType;
public:

	BoardNode()
	{

	}

	BoardNode(int score, int depth, int turn, Mov move, ScoreType scoreType=EXACT)
	{
		this->depth = depth;
		this->score = score;
		this->turn = turn;
		this->move = move;
		this->scoreType = scoreType;
	}

	void setDepth(int depth)
	{
		this->depth = depth;
	}

	void setScore(int score)
	{
		this->score = score;
	}

	int getScore() const
	{
		return score;
	}

	int getDepth() const
	{
		return depth;
	}

	int getTurn() const
	{
		return turn;
	}

	Mov getMove() const
	{
		return move;
	}

	ScoreType getScoreType() const
	{
		return scoreType;
	}
	
	void setScoreType(ScoreType scoreType)
	{
		this->scoreType = scoreType;
	}
};


class TransTable
{
private:
	unordered_map<MultiKeysNode, BoardNode, MultiKeysNode_hash>* boardNodeMap;

	// ʵ�ʱ��ʵ���У�û��ʹ����ά���飬ʹ��һά������+�±�ת������tableIdxFromBoardIdx��Ч�ʸ���һЩ
	int rndTableSize;
	size_t *hashIndexTable;
	size_t *checksumTable;

	size_t currBoardHashVal;
	size_t currBoardChecksum;

	void initRandomTable();
	int tableIdxFromBoardIdx(int player, int x, int y) const;
	MultiKeysNode currKeyNode();

	// �û����������Ͳ�ѯ�ɹ����������ں����û����Ч��
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

	// ��������ϵ�����
	void MarkMove(int player, int x, int y);
	void UnmarkMove(int player, int x, int y);

	void insertCurrBoardNode(int score, int depth, int turn, Mov move, ScoreType scoreType = EXACT);
	bool searchCurrBoardNode();
	BoardNode* getCurrBoardNode(); // ��ȡ��ǰ���̽ڵ��BoardNode

	int getSize() const;

	size_t getCurrBoardChecksum() const;
	size_t getCurrBoardHashVal() const;
	
	int getInsertCnt() const;
	int getSearchHitCnt() const;
	void incrementSearchHitCnt();
};

#endif // !TransTable

