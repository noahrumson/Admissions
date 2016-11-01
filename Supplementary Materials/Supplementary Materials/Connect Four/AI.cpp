// Noah Rubin

#include <iostream>
#include <algorithm>
#include <vector>
#include <future>

#include "AI.h"
#include "Board.h"

#define WINNING_VALUE 1000000
#define LOSING_VALUE -1000000

#if _MSC_VER <= 1800
#	define constexpr const	// constexpr isn't implemented in Visual Studio versions before 2015
#endif

/*
	The AI uses the minimax algorithm to determine the optimal move. I implemented
	several variations of the algorithm including Negascout, alpha-beta pruning,
	MTDf, iterative deepening, and a transposition table. They all give the same results
	for the same input, however after testing I found that Negascout with a transposition
	table was the fastest.

	In the beginning, the state space of the game is so large that the AI cannot
	look ahead to every possible move. Instead, it begins by looking 9 moves ahead
	and applies a heuristic function (AI::HeuristicEvaluate) which tries to approximate
	how favorable a given board state is for the AI. After a certain number of moves the
	game tree decreases enough that the AI can look ahead to the end of the game, looking
	through all possible moves. At this stage no heuristic function is needed, instead it
	just evaluates if the board state results in a win, loss, or draw.

	A few other optimizations are made, such as move ordering and multithreading. Since moves
	down the middle are typically more beneficial, particularly in the opening of the game, the
	AI checks these moves first so that alpha-beta pruning can eliminate many other possible moves,
	which helps speed up the AI. Also, all of the game trees explored are divided into several threads
	which can run in parallel to each other on multi-processor systems.
*/


namespace
{
	constexpr int STARTING_DEPTH = 9;
	constexpr int MAX_DEPTH = 27;
	constexpr int TTABLE_SIZE = 300000; // three hundred thousand
	constexpr int STARTING_MAX_CACHE_DEPTH = 3;
	constexpr int MAX_CACHE_DEPTH = 12;
	constexpr int moveOrdering[] { 3, 4, 2, 5, 1, 6, 0 };
	int movesMade = 0;
	int maxCacheDepth = STARTING_MAX_CACHE_DEPTH;
}


const int AI::sm_moveOrderings[7][7] {
	{ 0, 3, 4, 2, 5, 1, 6 },
	{ 1, 3, 4, 2, 5, 6, 0 },
	{ 2, 3, 4, 5, 1, 6, 0 },
	{ 3, 4, 2, 5, 1, 6, 0 },
	{ 4, 3, 2, 5, 1, 6, 0 },
	{ 5, 3, 4, 2, 1, 6, 0 },
	{ 6, 3, 4, 2, 5, 1, 0 },
};


AI::AI() :
	m_transpositionTable(),
	m_ttMutex(),
	m_searchDepth(STARTING_DEPTH),
	m_evalFunc(&AI::HeuristicEvaluate)
{
	m_transpositionTable.reserve(TTABLE_SIZE);
}


int AI::ThreadFunc(const Board& board, int col)
{
	Board child(board);
	child.Drop(col);
	return -NegaScoutCache(child, m_searchDepth - 1, LOSING_VALUE - 1, WINNING_VALUE + 1);
}


int AI::BestMove(const Board& board)
{
	int scores[7];
	std::fill_n(scores, 7, std::numeric_limits<int>::min());
	std::vector<std::future<int>> futures(7);
	for (int i = 0; i < Board::WIDTH; ++i) {
		if (!board.IsColumnFull(moveOrdering[i])) {
			futures[i] = std::async(std::launch::async, &AI::ThreadFunc, this, board, moveOrdering[i]);
		}
	}
	for (int i = 0; i < Board::WIDTH; ++i) {
		if (!board.IsColumnFull(moveOrdering[i])) {
			scores[i] = futures[i].get();
		}
	}
	int* ptr = std::max_element(scores, &scores[7]);
	int max = *ptr;
	int bestCol = moveOrdering[ptr - scores];
	if (++movesMade == 8) {
		m_searchDepth = MAX_DEPTH;
		m_evalFunc = &AI::FastEvaluate;
		maxCacheDepth = MAX_CACHE_DEPTH;
	}
	if (m_searchDepth < 13 && movesMade % 2 == 0) {
		m_searchDepth += 2;
	}
	if (max >= WINNING_VALUE - MAX_DEPTH) {
		std::cout << "Winning moves for AI found\n";	// the AI is now guaranteed to win
	}
	else if (max <= LOSING_VALUE + MAX_DEPTH) {
		std::cout << "Winning moves for opponent found\n";	// if the opponent plays perfectly, he will win
	}
	m_transpositionTable.clear();
	return bestCol;
}

void AI::Reset()
{
	m_evalFunc = &AI::HeuristicEvaluate;
	movesMade = 0;
	m_searchDepth = STARTING_DEPTH;
	m_transpositionTable.clear();
	maxCacheDepth = STARTING_MAX_CACHE_DEPTH;
}


int AI::NegaScout(const Board& node, int depth, int alpha, int beta)
{
	if (depth == 0 || node.GetWinner() != CHIP_NONE || node.IsBoardFull()) {
		return (this->*m_evalFunc)(node, depth);
	}
	int m = LOSING_VALUE - 1;
	int n = beta;
	int t;
	int col;
	for (int i = 0; i < Board::WIDTH; ++i) {
		col = moveOrdering[i];
		if (node.IsColumnFull(col)) {
			continue;
		}
		Board child(node);
		child.Drop(col);
		t = -NegaScout(child, depth - 1, -n, -std::max(alpha, m));
		if (t > m) {
			if (n == beta || t >= beta) {
				m = t;
			}
			else {
				m = -NegaScout(child, depth - 1, -beta, -t);
			}
		}
		if (m >= beta) {
			return m;
		}
		n = std::max(alpha, m) + 1;
	}
	return m;
}


int AI::NegaScoutCache(const Board& node, int depth, int alpha, int beta)
{
	const int* thisMoveOrder = sm_moveOrderings[3];
	const int* moveOrder = sm_moveOrderings[3];
	int m = LOSING_VALUE - 1;
	int n = beta;
	int t;
	int col;
	{
		std::lock_guard<std::mutex> lock(m_ttMutex);
		auto it = m_transpositionTable.find(node);
		if (it != m_transpositionTable.end()) {
			switch (it->second.type) {
			case ABResultType::EXACT:
				return it->second.value;
			case ABResultType::LOWER_BOUND:
				m = std::max(m, it->second.value);
				break;
			case ABResultType::UPPER_BOUND:
				n = std::min(beta, it->second.value);
				break;
			}
			if (m >= beta) {
				return it->second.value;
			}
			thisMoveOrder = it->second.moveOrder;
		}
	}
	if (depth == 0 || node.GetWinner() != CHIP_NONE || node.IsBoardFull()) {
		return (this->*m_evalFunc)(node, depth);
	}

	int best = m;

	for (int i = 0; i < Board::WIDTH; ++i) {
		col = thisMoveOrder[i];
		if (node.IsColumnFull(col)) {
			continue;
		}
		Board child(node);
		child.Drop(col);
		t = (depth <= maxCacheDepth ? -NegaScout(child, depth - 1, -n, -std::max(alpha, m)) : -NegaScoutCache(child, depth - 1, -n, -std::max(alpha, m)));
		if (t > m) {
			if (n == beta || t >= beta) {
				m = t;
			}
			else {
				m = (depth <= maxCacheDepth ? -NegaScout(child, depth - 1, -beta, -t) : -NegaScoutCache(child, depth - 1, -beta, -t));
			}
		}

		if (m > best) {
			best = m;
			moveOrder = sm_moveOrderings[col];
		}

		if (m >= beta) {
			break;
		}
		n = std::max(alpha, m) + 1;
	}
	m_ttMutex.lock();
	if (m <= alpha) {
		m_transpositionTable.insert({ node, { ABResultType::UPPER_BOUND, m, moveOrder } });
	}
	else if (m >= beta) {
		m_transpositionTable.insert({ node, { ABResultType::LOWER_BOUND, m, moveOrder } });
	}
	else {
		m_transpositionTable.insert({ node, { ABResultType::EXACT, m, moveOrder } });
	}
	m_ttMutex.unlock();
	return m;
}


int AI::FastEvaluate(const Board& node, int depth) const
{
	if (node.GetWinner() == node.GetThisTurn()) {
		return WINNING_VALUE - MAX_DEPTH + depth;
	}
	else if (node.GetWinner() == node.GetNextTurn()) {
		return LOSING_VALUE + MAX_DEPTH - depth;
	}
	return 0;
}


int AI::HeuristicEvaluate(const Board& node, int depth) const
{
	if (node.GetWinner() == node.GetThisTurn()) {
		return WINNING_VALUE - MAX_DEPTH + depth;
	}
	else if (node.GetWinner() == node.GetNextTurn()) {
		return LOSING_VALUE + MAX_DEPTH - depth;
	}
	return node.WeightedOpenThreeInARows(node.GetThisTurn());
}

/*
const int AI::sm_inverseOrderings[7][7] {
{ 0, 5, 3, 1, 2, 4, 6 },
{ 6, 0, 3, 1, 2, 4, 5 },
{ 6, 4, 0, 1, 2, 3, 5 },
{ 6, 4, 2, 0, 1, 3, 5 },
{ 6, 4, 2, 1, 0, 3, 5 },
{ 6, 4, 3, 1, 2, 0, 5 },
{ 6, 5, 3, 1, 2, 4, 6 },
};
struct TTIDData
{
	AI::ABResultType type;
	int value;
	int depth;
	const int* moveOrder;
};
std::unordered_map<Board, TTIDData> tt;

void AI::NegaScoutIterativeDeepening(const Board& node, int* scores)
{
	static std::function<int(const Board&, int, int)> func([this](const Board& node, int col, int depth) {
		Board child(node);
		child.Drop(col);
		return -NegaScoutIDCache(child, depth, LOSING_VALUE - 1, WINNING_VALUE + 1);
	});
	tt.reserve(TTABLE_SIZE);
	std::vector<std::future<int>> futures(7);
	int best = LOSING_VALUE - 1;
	const int* thisMoveOrdering = sm_moveOrderings[3];
	for (int d = 0; d < m_searchDepth; d += (m_searchDepth == MAX_DEPTH) ? 3 : 2) {
		std::cout << "Depth: " << d << std::endl;
		for (int i = 0; i < Board::WIDTH; ++i) {
			if (!node.IsColumnFull(thisMoveOrdering[i])) {
				//futures[i] = std::async(std::launch::async, &AI::ThreadFunc, this, node, thisMoveOrdering[i], false, d);
				futures[i] = std::async(std::launch::async, func, node, thisMoveOrdering[i], d);
			}
		}
		for (int i = 0; i < Board::WIDTH; ++i) {
			if (!node.IsColumnFull(thisMoveOrdering[i])) {
				scores[sm_inverseOrderings[3][thisMoveOrdering[i]]] = futures[i].get();
				if (scores[i] > best) {
					best = scores[i];
					thisMoveOrdering = sm_moveOrderings[thisMoveOrdering[i]];
				}
			}
		}
		if (m_searchDepth == MAX_DEPTH && d == 24)
			d = MAX_DEPTH - 4;
	}
//	tt.clear();
}


int AI::NegaScoutIDCache(const Board& node, int depth, int alpha, int beta)
{
	const int* thisMoveOrder = sm_moveOrderings[3];
	const int* moveOrder = sm_moveOrderings[3];
	int m = LOSING_VALUE - 1;
	int n = beta;
	int t;
	int col;
	{
		std::lock_guard<std::mutex> lock(m_ttMutex);
		auto it = tt.find(node);
		if (it != tt.end()) {
			if (it->second.depth >= depth) {
				switch (it->second.type) {
				case ABResultType::EXACT:
					return it->second.value;
				case ABResultType::LOWER_BOUND:
					m = std::max(m, it->second.value);
					break;
				case ABResultType::UPPER_BOUND:
					n = std::min(beta, it->second.value);
					break;
				}
			}
			if (m >= beta) {
				return it->second.value;
			}
			thisMoveOrder = it->second.moveOrder;
		}
	}
	if (depth == 0 || node.GetWinner() != CHIP_NONE || node.IsBoardFull()) {
		return (this->*m_evalFunc)(node, depth);
	}


	int best = m;

	for (int i = 0; i < Board::WIDTH; ++i) {
		col = thisMoveOrder[i];
		if (node.IsColumnFull(col)) {
			continue;
		}
		Board child(node);
		child.Drop(col);
		t = (depth <= maxCacheDepth ? -NegaScoutID(child, depth - 1, -n, -std::max(alpha, m)) : -NegaScoutIDCache(child, depth - 1, -n, -std::max(alpha, m)));
		if (t > m) {
			if (n == beta || t >= beta) {
				m = t;
			}
			else {
				m = (depth <= maxCacheDepth ? -NegaScoutID(child, depth - 1, -beta, -t) : -NegaScoutIDCache(child, depth - 1, -beta, -t));
			}
		}

		if (m > best) {
			best = m;
			moveOrder = sm_moveOrderings[col];
		}

		if (m >= beta) {
			break;
		}
		n = std::max(alpha, m) + 1;
	}
	m_ttMutex.lock();
	if (m <= alpha) {
		tt.insert({ node, { ABResultType::UPPER_BOUND, m, depth, moveOrder } });
	}
	else if (m >= beta) {
		tt.insert({ node, { ABResultType::LOWER_BOUND, m, depth, moveOrder } });
	}
	else {
		tt.insert({ node, { ABResultType::EXACT, m, depth, moveOrder } });
	}
	m_ttMutex.unlock();
	return m;
}


int AI::NegaScoutID(const Board& node, int depth, int alpha, int beta)
{
	int m = LOSING_VALUE - 1;
	int n = beta;
	int t;
	int col;
	if (depth == 0 || node.GetWinner() != CHIP_NONE || node.IsBoardFull()) {
		return (this->*m_evalFunc)(node, depth);
	}
	for (int i = 0; i < Board::WIDTH; ++i) {
		col = moveOrdering[i];
		if (node.IsColumnFull(col)) {
			continue;
		}
		Board child(node);
		child.Drop(col);
		t = -NegaScoutID(child, depth - 1, -n, -std::max(alpha, m));
		if (t > m) {
			if (n == beta || t >= beta) {
				m = t;
			}
			else {
				m = -NegaScoutID(child, depth - 1, -beta, -t);
			}
		}
		if (m >= beta) {
			break;
		}
		n = std::max(alpha, m) + 1;
	}
	return m;
}


int AI::MTDf(const Board& node, int depth, int guess)
{
	int upperBound = WINNING_VALUE + 1;
	int lowerBound = LOSING_VALUE - 1;
	int alpha, beta;
	while (lowerBound < upperBound) {
		beta = (guess == lowerBound ? guess + 1 : guess);


		if ((beta > 20 && beta < 100) || (beta < -20 && beta > -100) || beta == WINNING_VALUE || beta == LOSING_VALUE) {
			beta = guess == lowerBound ? WINNING_VALUE - MAX_DEPTH : LOSING_VALUE + MAX_DEPTH;
		}

		guess = NegaScoutCache(node, depth, beta - 1, beta);
		if (guess < beta) {
			upperBound = guess;
		}
		else {
			lowerBound = guess;
		}
	}
	return guess;
}
int AI::AlphabetaNegamax(const Board& node, int depth, int alpha, int beta)
{
	if (depth == 0 || node.GetWinner() != CHIP_NONE || node.IsBoardFull()) {
		return (this->*m_evalFunc)(node, depth);
	}
	int score;
	int col;
	for (int i = 0; i < Board::WIDTH; ++i) {
		col = moveOrdering[i];
		if (node.IsColumnFull(col)) {
			continue;
		}
		Board child(node);
		child.Drop(col);
		score = -AlphabetaNegamax(child, depth - 1, -beta, -alpha);
		if (score >= beta) {
			return score;
		}
		if (score > alpha) {
			alpha = score;
		}
	}
	return alpha;
}


int AI::AlphabetaNegamaxCache(const Board& node, int depth, int alpha, int beta)
{
	const int* thisMoveOrder = sm_moveOrderings[3];
	const int* moveOrder = sm_moveOrderings[3];
	int score;
	int col;
	int alphaOrig = alpha;
	{
		std::lock_guard<std::mutex> lock(m_ttMutex);
		auto it = m_transpositionTable.find(node);
		if (it != m_transpositionTable.end()) {
			switch (it->second.type) {
			case ABResultType::EXACT:
				return it->second.value;
			case ABResultType::LOWER_BOUND:
				alpha = std::max(alpha, it->second.value);
				thisMoveOrder = it->second.moveOrder;
				break;
			case ABResultType::UPPER_BOUND:
				beta = std::min(beta, it->second.value);
				thisMoveOrder = it->second.moveOrder;
				break;
			}
			if (alpha >= beta) {
				return it->second.value;
			}
		}
	}
	if (depth == 0 || node.GetWinner() != CHIP_NONE || node.IsBoardFull()) {
		return (this->*m_evalFunc)(node, depth);
	}
	for (int i = 0; i < Board::WIDTH; ++i) {
		col = thisMoveOrder[i];
		if (node.IsColumnFull(col)) {
			continue;
		}
		Board child(node);
		child.Drop(col);
		score = (depth <= maxCacheDepth ? -AlphabetaNegamax(child, depth - 1, -beta, -alpha) : -AlphabetaNegamaxCache(child, depth - 1, -beta, -alpha));
		if (score > alpha) {
			moveOrder = sm_moveOrderings[col];
			alpha = score;
		}
		if (alpha >= beta) {
			break;
		}
	}
	m_ttMutex.lock();
	if (alpha <= alphaOrig) {
		m_transpositionTable.insert({ node,{ ABResultType::UPPER_BOUND, alpha, moveOrder } });
	}
	else if (alpha >= beta) {
		m_transpositionTable.insert({ node,{ ABResultType::LOWER_BOUND, alpha, moveOrder } });
	}
	else {
		m_transpositionTable.insert({ node,{ ABResultType::EXACT, alpha, moveOrder } });
	}
	m_ttMutex.unlock();
	return alpha;
}*/