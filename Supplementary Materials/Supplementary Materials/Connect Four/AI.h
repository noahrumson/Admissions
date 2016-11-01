// Noah Rubin

#ifndef AI_H_INCLUDED
#define AI_H_INCLUDED

#include <unordered_map>
#include <mutex>

#include "Build.h"
#include "Board.h"


class AI
{
public:
	AI();

	int BestMove(const Board& board);

	void Reset();

private:
	enum class ABResultType
	{
		EXACT, LOWER_BOUND, UPPER_BOUND
	};

	typedef int (AI::*EvaluationFunction)(const Board&, int) const;

	struct TTData
	{
		ABResultType type;
		int value;
		const int* moveOrder;
	};

	std::unordered_map<Board, TTData> m_transpositionTable;
	std::mutex m_ttMutex;
	EvaluationFunction m_evalFunc;
	int m_searchDepth;

	int NegaScout(const Board& node, int depth, int alpha, int beta);

	int NegaScoutCache(const Board& node, int depth, int alpha, int beta);

	int ThreadFunc(const Board& board, int col);

	int FastEvaluate(const Board& node, int depth) const;

	int HeuristicEvaluate(const Board& node, int depth) const;

	static const int sm_moveOrderings[7][7];
	
/*	
	static const int sm_inverseOrderings[7][7];

	int AlphabetaNegamax(const Board& node, int depth, int alpha, int beta);

	int AlphabetaNegamaxCache(const Board& node, int depth, int alpha, int beta);

	void NegaScoutIterativeDeepening(const Board& node, int* scores);

	int NegaScoutID(const Board& node, int depth, int alpha, int beta);

	int NegaScoutIDCache(const Board& node, int depth, int alpha, int beta);

	int MTDf(const Board& node, int depth, int guess);
*/
};

#endif
