// Noah Rubin

#include <cmath>
#include <intrin.h>
#include <iostream>
#include <algorithm>

#include "Board.h"
#include "Build.h"

#define BITBOARD_FIRST_COLUMN		0x0101010101010101ULL
#define BITBOARD_MAIN_DIAGONAL		0x0102040810204080ULL
#define BITBOARD_MAIN_ANTIDIAGONAL	0x8040201008040201ULL
#define BITBOARD_ALL_BITS			0xFFFFFFFFFFFFFFFFULL
#define BITBOARD_LEAST_BYTE			0x7F
#define BITBOARD_FULL				0x7F7F7F7F7F7FULL

/*
	The bitboard represents the game board like this:

		0  1  2  3  4  5  6
		8  9  10 11 12 13 14
		16 17 18 19 20 21 22
		24 25 26 27 28 29 30
		32 33 34 35 36 37 38
		40 41 42 43 44 45 46

	Each byte represents a row, with the least-significant byte representing the top.
	The last bit of each byte is unused and always zero.
*/


namespace
{
	Board::Bitboard insertTable[7][6];


	struct InitTableHelper
	{
		InitTableHelper()
		{
			for (int c = 0; c < Board::WIDTH; ++c) {
				for (int r = 0; r < Board::HEIGHT; ++r) {
					insertTable[c][r] = 1ULL << (8ULL * r + c);
				}
			}
		}
	};

	InitTableHelper dummy;


	inline void Insert(Board::Bitboard& board, unsigned long col, unsigned long row)
	{
		board |= insertTable[col][row];
	}


	inline unsigned long GetColumn(const Board::Bitboard& board, int col)
	{
		return ((board & (BITBOARD_FIRST_COLUMN << col)) >> col) * BITBOARD_MAIN_DIAGONAL >> 56ULL;
	}
}


Board::Board() :
	m_boards(),
	m_thisMove(CHIP_BLACK),
	m_winner(CHIP_NONE)
{
	m_boards[0] = 0uLL;
	m_boards[1] = 0uLL;
}


bool Board::operator==(const Board& other) const
{
	return !std::memcmp(m_boards, other.m_boards, 16);
}


#ifdef BUILD_64

int Board::Drop(int column)
{
	static constexpr Bitboard cols[]{
		BITBOARD_FIRST_COLUMN,
		BITBOARD_FIRST_COLUMN << 1ULL,
		BITBOARD_FIRST_COLUMN << 2ULL,
		BITBOARD_FIRST_COLUMN << 3ULL,
		BITBOARD_FIRST_COLUMN << 4ULL,
		BITBOARD_FIRST_COLUMN << 5ULL,
		BITBOARD_FIRST_COLUMN << 6ULL,
	};

	unsigned long firstSet;
	if (!_BitScanForward64(&firstSet, (m_boards[0] | m_boards[1]) & cols[column])) {
		firstSet = 48 + column;
	}
	m_boards[m_thisMove] |= 1ULL << (firstSet - 8);
	if (CheckWinner()) {
		m_winner = m_thisMove;
	}
	m_thisMove = GetNextTurn();
	return firstSet / 8 - 1;
}

#else

int Board::Drop(int column)
{
	unsigned long firstSet;
	if (!_BitScanForward(&firstSet, GetColumn(m_boards[0] | m_boards[1], column))) {
		firstSet = HEIGHT;
	}
	Insert(m_boards[m_thisMove], column, firstSet - 1);
	if (CheckWinner()) {
		m_winner = m_thisMove;
	}
	m_thisMove = GetNextTurn();
	return firstSet - 1;
}

#endif


bool Board::CheckWinner() const
{
	const Board::Bitboard& board = m_boards[m_thisMove];

	Board::Bitboard checkRows = board & (board >> 1ULL);
	checkRows &= checkRows >> 2ULL;
	if (checkRows) { return true; }

	Board::Bitboard checkCols = board & (board >> 8ULL);
	checkCols &= checkCols >> 16ULL;
	if (checkCols) { return true; }

	Board::Bitboard checkDiag = board & (board >> 7ULL);
	checkDiag &= checkDiag >> 14ULL;
	if (checkDiag) { return true; }

	Board::Bitboard checkAdiag = board & (board >> 9ULL);
	checkAdiag &= checkAdiag >> 18ULL;
	if (checkAdiag) { return true; }

	return false;
}


int Board::OpenThreeInARows(Chip chip, int(&threats)[16], int& subtract) const
{
	static constexpr int diagonalStarts[]{ 24, 32, 40, 41, 42, 43 };
	static constexpr int diagonalEnds[]{ 3, 4, 5, 6, 14, 22 };
	static constexpr int antidiagonalStarts[]{ 0, 1, 2, 3, 8, 16 };
	static constexpr int antidiagonalEnds[]{ 45, 46, 38, 30, 44, 43 };

	subtract = 0;
	const Bitboard& board = m_boards[chip];
	const Bitboard& other = m_boards[chip ^ 1];
	int oddEven = (chip == CHIP_BLACK ? 1 : 0);
	int found = 0;
	for (int r = 0, count = 0; r < HEIGHT; ++r, count = 0) {
		for (int i = 8 * r; i < 8 * r + WIDTH; ++i) {
			if (board & (1ULL << i)) {
				if (++count == 3) {
					if (i + 1 < 8 * r + WIDTH && !(other & (1ULL << (i + 1)))) {
						threats[found++] = i + 1;
					}
					if (i - 3 >= 8 * r && !(other & (1ULL << (i - 3)))) {
						threats[found++] = i - 3;
					}
				}
			}
			else {
				if (count == 2) {
					if (i + 1 < 8 * r + WIDTH && board & (1ULL << (i + 1)) && !(other & (1ULL << i))) {
						threats[found++] = i;
					}
					if (i - 4 >= 8 * r && board & (1ULL << (i - 4)) && !(other & (1ULL << (i - 3)))) {
						threats[found++] = i - 3;
					}
				}
				count = 0;
			}
		}
	}
	// test columns
	for (int c = 0, count = 0; c < HEIGHT; ++c, count = 0) {
		for (int i = c; i <= c + 40; i += 8) {
			if (board & (1ULL << i)) {
				if (++count == 3) {
					if (i - 24 >= c && !(other & (1ULL << (i - 24)))) {
						threats[found++] = i - 24;
						if (((i - 24) / 8) % 2 == oddEven) {
							subtract += 2;
						}
					}
				}
			}
			else {
				count = 0;
			}
		}
	}
	// test diagonals
	for (int i = 0, count = 0; i < 6; ++i, count = 0) {
		for (int j = diagonalStarts[i]; j >= diagonalEnds[i]; j -= 7) {
			if (board & (1ULL << j)) {
				if (++count == 3) {
					if (j - 7 >= diagonalEnds[i] && !(other & (1ULL << (j - 7)))) {
						threats[found++] = j - 7;
					}
					if (j + 21 <= diagonalStarts[i] && !(other & (1ULL << (j + 21)))) {
						threats[found++] = j + 21;
					}
				}
			}
			else {
				if (count == 2) {
					if (j - 7 >= diagonalEnds[i] && board & (1ULL << (j - 7)) && !(other & (1ULL << j))) {
						threats[found++] = j;
					}
					if (j + 28 <= diagonalStarts[i] && board & (1ULL << (j + 28)) && !(other & (1ULL << (j + 21)))) {
						threats[found++] = j + 21;
					}
				}
				count = 0;
			}
		}
	}
	// test antidiagonals
	for (int i = 0, count = 0; i < 6; ++i, count = 0) {
		for (int j = antidiagonalStarts[i]; j <= antidiagonalEnds[i]; j += 9) {
			if (board & (1ULL << j)) {
				if (++count == 3) {
					if (j + 9 <= antidiagonalEnds[i] && !(other & (1ULL << (j + 9)))) {
						threats[found++] = j + 9;
					}
					if (j - 27 >= antidiagonalStarts[i] && !(other & (1ULL << (j - 27)))) {
						threats[found++] = j - 27;
					}
				}
			}
			else {
				if (count == 2) {
					if (j + 9 <= antidiagonalEnds[i] && board & (1ULL << (j + 9)) && !(other & (1ULL << j))) {
						threats[found++] = j;
					}
					if (j - 36 >= antidiagonalStarts[i] && board & (1ULL << (j - 36)) && !(other & (1ULL << (j - 27)))) {
						threats[found++] = j - 27;
					}
				}
				count = 0;
			}
		}
	}
	return found;
}


bool Board::IsColumnFull(int column) const
{
	// truncation to long is needed and should be ok because top row is bits 0-7
	long all = m_boards[0] | m_boards[1];	// might store this so no recalculation needed
	return _bittest(&all, column);
}


bool Board::IsBoardFull() const
{
	return (m_boards[0] | m_boards[1]) == BITBOARD_FULL;
}


Chip Board::GetWinner() const
{
	return m_winner;
}


Chip Board::GetThisTurn() const
{
	return m_thisMove;
}


Chip Board::GetNextTurn() const
{
	return (Chip) ((m_thisMove) ^ 1);
}


int Board::WeightedOpenThreeInARows(Chip chip) const
{
	int blackThreats[16];
	int redThreats[16];
	int blackSub;
	int redSub;
	int blackFound = OpenThreeInARows(CHIP_BLACK, blackThreats, blackSub);
	int redFound = OpenThreeInARows(CHIP_RED, redThreats, redSub);
	blackFound = std::unique(blackThreats, blackThreats + blackFound) - blackThreats;
	redFound = std::unique(redThreats, redThreats + redFound) - redThreats;
	int blackScore = -blackSub / 2;
	int redScore = -redSub / 2;
	for (int i = 0; i < blackFound; ++i) {
		bool unblocked = true;
		for (int j = 0; j < redFound; ++j) {
			if (blackThreats[i] == redThreats[j] - 8) {
				unblocked = false;
				break;
			}
		}
		if (unblocked && (blackThreats[i] / 8) % 2 && blackThreats[i] / 8 != 5) {
			++blackScore;
		}
	}
	int redOddThreats = 0;
	for (int i = 0; i < redFound; ++i) {
		bool unblocked = true;
		for (int j = 0; j < blackFound; ++j) {
			if (redThreats[i] == blackThreats[j] - 8) {
				unblocked = false;
				break;
			}
		}
		if (unblocked) {
			if ((redThreats[i] / 8) % 2 == 0 && redThreats[i] / 8 != 5) {
				++redScore;
			}
			else {
				++redOddThreats;
			}
		}
	}
	if (redOddThreats > 1) {
		redScore += redOddThreats / 2;
	}
	int score = blackScore - redScore;
	return chip == CHIP_BLACK ? score : -score;
}

