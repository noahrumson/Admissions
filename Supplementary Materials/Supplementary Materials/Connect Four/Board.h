// Noah Rubin

#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#if _MSC_VER <= 1800
#	define constexpr const	// constexpr isn't implemented in Visual Studio versions before 2015
#endif

#include <functional>

#include "Build.h"

// Black and red must be 0 and 1 because used as array indices
enum Chip
{
	CHIP_BLACK,
	CHIP_RED,
	CHIP_NONE,
};

class Board
{
public:
	typedef unsigned long long Bitboard;

	static constexpr int WIDTH = 7;
	static constexpr int HEIGHT = 6;

	Board();

	bool operator==(const Board& other) const;

	int Drop(int column);

	bool IsColumnFull(int column) const;

	bool IsBoardFull() const;

	Chip GetWinner() const;

	Chip GetThisTurn() const;

	Chip GetNextTurn() const;

	int WeightedOpenThreeInARows(Chip chip) const;

private:
	Bitboard m_boards[2];
	Chip m_thisMove;
	Chip m_winner;

	friend struct std::hash<Board>;

	bool CheckWinner() const;

	int OpenThreeInARows(Chip chip, int (&threats)[16], int& subtract) const;
};


namespace std
{
	template<>
	struct hash<Board>
	{
		inline std::size_t operator()(const Board& board) const
		{
			const char* bytes = (const char*) board.m_boards;
			std::size_t hash = 0;
			for (int i = 0; i < 14; ++i) {
				if (i == 6 || i == 7) continue;
				hash += bytes[i];
				hash += (hash << 10);
				hash ^= (hash >> 6);
			}
			hash += (hash << 3);
			hash ^= (hash >> 11);
			hash += (hash << 15);
			return hash;
		}
	};
}


#endif // !BOARD_H_INCLUDED


