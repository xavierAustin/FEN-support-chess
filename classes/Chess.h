#pragma once
#include "Game.h"
#include "ChessSquare.h"

#include <list>

const int pieceSize = 64;

enum ChessPiece {
    //unnessesary notation but useful for readability
    NoPiece = 0,
    Pawn    = 1,
    Knight  = 2,
    Bishop  = 3,
    Rook    = 4,
    Queen   = 5,
    King    = 6
};

//
// the main game class
//
class Chess : public Game
{
public:
    Chess();
    ~Chess();

    // set up the board
    void        setUpBoard() override;

    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() override;
    void        setStateString(const std::string &s) override;
    bool        actionForEmptyHolder(BitHolder& holder) override;
    bool        canBitMoveFrom(Bit& bit, BitHolder& src) override;
    bool        canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;
    void        bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;

    void        stopGame() override;
    BitHolder& getHolderAt(const int x, const int y) override { return _grid[y][x]; }

	void        updateAI() override;
    bool        gameHasAI() override { return true; }
    void        QuickPlacePeice(int x, int y, ChessPiece type, bool color);
    bool        bitFromToHelper(int sX, int sY, int type, BitHolder& dst);
    void        generateMoveList();
    bool        getPosAttacked(int pX, int pY, Player* player, BitHolder* ignore);
    void        gridToBitboard();
    int         evaluateBoard();
    void        FENtoPos(std::string fen);
private:
    Bit *       PieceForPlayer(const int playerNumber, ChessPiece piece);
    const char  bitToPieceNotation(int row, int column) const;

    ChessSquare _grid[8][8];
    //not really implemented yet
    uint64_t _bitboards[8] = {};

    int _boardScore = 1;
    //0b111000000000 src x of possible move
    //0b000111000000 src y of possible move
    //0b000000111000 dst x of possible move
    //0b000000000111 dst y of possible move
    std::list<uint16_t>  _moveList = {};
    
    //0 x of white king
    //1 y of white king
    //3 y of black king
    char _kingpos[4] = {4,7,4,0};

    //0b1100 Black Castle Possible (Both Ways)
    //0b1010 Queenside Castle Possible (Both Colors)
    char        _castleRights = 0b1111;
    char        _enpassant = -1;
};

