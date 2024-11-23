#include "Chess.h"
//#include "magicbitboards.h"
//#include "Logger.h"

const int AI_PLAYER = 1;
const int HUMAN_PLAYER = -1;

bool forceTrueFunc(const int& a){
    return true;
}

Chess::Chess()
{
}

Chess::~Chess()
{
}

//
// make a chess piece for the player
//
Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    // depending on playerNumber load the "x.png" or the "o.png" graphic
    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("chess/") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;
    //
    // we want white to be at the bottom of the screen so we need to reverse the board
    //
    char piece[2];
    piece[1] = 0;
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            ImVec2 position((float)(pieceSize * x + pieceSize), (float)(pieceSize * (_gameOptions.rowY - y) + pieceSize));
            _grid[y][x].initHolder(position, "boardsquare.png", x, y);
            _grid[y][x].setGameTag(0);
            piece[0] = bitToPieceNotation(y,x);
            _grid[y][x].setNotation(piece);
        }
    }
    //debug
    //QuickPlacePeice(4,4,King,0);
    //QuickPlacePeice(4,4,Queen,0);
    //QuickPlacePeice(4,4,Rook,0);
    //QuickPlacePeice(4,4,Knight,0);
    //QuickPlacePeice(4,4,Bishop,0);
    //QuickPlacePeice(4,4,Pawn,0);
    //QuickPlacePeice(4,1,Pawn,0);
    //QuickPlacePeice(3,5,Pawn,1);
    //QuickPlacePeice(5,5,Pawn,1);
    //_enpassant = 5;
    //QuickPlacePeice(3,4,Pawn,1);
    //QuickPlacePeice(5,4,Pawn,1);
    FENtoPos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    return;
    // set up pawns for both sides
    for (int c = 0; c < 2; c++)
        for (int x = 0; x < _gameOptions.rowX; x++) 
            QuickPlacePeice(x,1+5*c,Pawn,c);
    // set up other peices
    ChessPiece temp[] = {Rook,Knight,Bishop,Queen,King,Bishop,Knight,Rook};
    for (int c = 0; c < 2; c++)
        for (int x = 0; x < _gameOptions.rowX; x++) 
            QuickPlacePeice(x,7*c,temp[x],c);
}

void Chess::QuickPlacePeice(int x, int y, ChessPiece type, bool color){
    if (!type){
        _grid[y][x].destroyBit();
        return;
    }
    Bit* bit = PieceForPlayer(color, type);
    bit->setPosition(_grid[y][x].getPosition());
    bit->setParent(&_grid[y][x]);
    //raise bit 0b10000000 to be a little more descrete to code i commented out
    bit->setGameTag(type|(color<<7));

    _grid[y][x].setBit(bit);
}

bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

//I'd personally just use canBitMoveFromTo but this is probably better for organisation
bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    if (bit.getOwner() == getCurrentPlayer())
        return true;
    else
        return false;
}

//bitboards but like bad though? ;:(
bool Chess::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst)
{
    //general colision check
    int capture = 0;
    if (!dst.empty()){
        if (dst.bit()->getOwner() == bit.getOwner())
            return false;
        capture = 1;
    }

    //find info on current peice
    //get "playing" color
    int c = bit.getOwner()->playerNumber();
    int type = bit.gameTag() & 127;
    //typecast that's in no way dangerous trust me
    int sX = ((ChessSquare*)&src)->getColumn();
    int sY = ((ChessSquare*)&src)->getRow();
    int dX = ((ChessSquare*)&dst)->getColumn();
    int dY = ((ChessSquare*)&dst)->getRow();

    //check and pins
    int tempType = NoPiece;
    if (!dst.empty())
        tempType = dst.bit()->gameTag() & 127;
    //temp modify board to match attempted move
    //QuickPlacePeice(dX,dY,*((ChessPiece*)&type),c);
    //temp move king if selected piece is a king
    //if (type == King){
    //    _kingpos[c*2] = dX;
    //    _kingpos[c*2+1] = dY;
    //}
    //do check for king attacks
    //bool moveIllegal = getPosAttacked(_kingpos[c*2],_kingpos[c*2+1],bit.getOwner(),&src);
    //reset temp changes
    //QuickPlacePeice(dX,dY,*((ChessPiece*)&tempType),!c);
    //if (type == King){
    //    _kingpos[c*2] = sX;
    //    _kingpos[c*2+1] = sY;
    //}
    //if (moveIllegal)
    //    return false;
    
    //Pawn movement
    if (type == Pawn){
        //normal
        if (&dst == &_grid[sY+1-c*2][sX])
            return dst.empty();
        //double
        if (&dst == &_grid[sY+2-c*4][sX])
            return (sY == 6 || sY == 1) && dst.empty() && _grid[sY+1-c*2][sX].empty();
        //capture
        for (int i = -1; i < 2; i += 2)
            if (&dst == &_grid[sY+1-c*2][sX+i])
                return capture || (_enpassant == sX+i && (sY == 3 || sY == 4));
    //Knight movement
    }else if (type == Knight){
        for (int x = -2; x < 3; x ++){
            if (sX+x < 0 || sX+x > 7 || x == 0)
                continue;
            for (int i = -1; i < 2; i += 2){
                int y = (3-abs(x))*i;
                if (sY+y < 0 || sY+y > 7)
                    continue;
                if (&dst == &_grid[sY+y][sX+x])
                    return true;
            }
        }
    //King, Queen, Rook, and Bishop Movement
    }else
        if (bitFromToHelper(sX,sY,type,dst))
            return true;
    //Castling
    if (sX != 4 || type != King)
        return false;
    if ((&dst == &_grid[sY][sX+2] && _castleRights & 1<<(2*c) && _grid[sY][sX+1].empty()) ||
        (&dst == &_grid[sY][sX-2] && _castleRights & 2<<(2*c) && _grid[sY][sX-1].empty()))
        return !capture;
    return false;
}

void Chess::gridToBitboard(){
    for (int i = 0; i < 8; i += 1){
        _bitboards[i] = 0;
    }
    for (int x = 0; x < 8; x ++)
    for (int y = 0; y < 8; y ++){
        if (!_grid[y][x].empty()){
            Bit* temp = _grid[y][x].bit();
            int type = temp->gameTag();
            //set white pieces bitboard
            if (type & 128)
                _bitboards[NoPiece] |= 1ULL<<(y*8+x);
            type = type & 127;
            //set bitboards for each specific piece type
            if (type)
                _bitboards[type] |= 1ULL<<(y*8+x);
        }
    }
}

bool Chess::bitFromToHelper(int sX, int sY, int type, BitHolder& dst){
    for (int x = -1; x < 2; x ++){
        for (int y = -1; y < 2; y ++){
            if ((!y && !x) || (type == Rook && x && y) || (type == Bishop && !(x && y)))
                continue;
            for (int i = 1; sY+y*i < 8 && sY+y*i > -1 && sX+x*i < 8 && sX+x*i > -1; i ++){
                if (&dst == &_grid[sY+y*i][sX+x*i])
                    return true;
                if ((!_grid[sY+y*i][sX+x*i].empty()) || type == King)
                    break;
            }
        }
    }
    return false;
}

bool Chess::getPosAttacked(int sX, int sY, Player* player, BitHolder* ignore){
    for (int x = -2; x < 3; x ++){
        if (sX+x < 0 || sX+x > 7 || x == 0)
            continue;
        for (int i = -1; i < 2; i += 2){
            int y = (3-abs(x))*i;
            if (sY+y < 0 || sY+y > 7)
                continue;
            if (_grid[sY+y][sX+x].empty())
                continue;
            Bit* pinfo = _grid[sY+y][sX+x].bit();
            if (pinfo->getOwner() != player && ((pinfo->gameTag() & 127) == Knight))
                return true;
        }
    }
    for (int x = -1; x < 2; x ++){
        for (int y = -1; y < 2; y ++){
            if ((!y && !x) || sY+y > 7 || sY+y < 0 || sX+x > 7 || sX+x < 0)
                continue;
            //int pinfo = _grid[sY+y][sX+x].bit()->gameTag();
            //if (c == !(pinfo & 128) && ((pinfo & 127) == King))
            //    return true;
            for (int i = 1; sY+y*i < 8 && sY+y*i > -1 && sX+x*i < 8 && sX+x*i > -1; i++){
                if (_grid[sY+y*i][sX+x*i].empty())
                    continue;
                Bit* pinfo = _grid[sY+y][sX+x].bit();
                int type = pinfo->gameTag() & 127;
                if (player == pinfo->getOwner() && (type == Queen || (type == Bishop && x && y) || (type == Rook && !(x && y))))
                    return true;
                if (ignore != &_grid[sY+y*i][sX+x*i])
                    break;
            }
        }
    }
    return false;
}

void Chess::FENtoPos(std::string fen){
    int i = 0;
    int x = 0;
    int y = 7;
    while (fen[i] != ' ') {
        switch(fen[i]){
            case 'P':
                QuickPlacePeice(x,y,Pawn,0);
            break;
            case 'B':
                QuickPlacePeice(x,y,Bishop,0);
            break;
            case 'N':
                QuickPlacePeice(x,y,Knight,0);
            break;
            case 'R':
                QuickPlacePeice(x,y,Rook,0);
            break;
            case 'Q':
                QuickPlacePeice(x,y,Queen,0);
            break;
            case 'K':
                QuickPlacePeice(x,y,King,0);
            break;
            case 'p':
                QuickPlacePeice(x,y,Pawn,1);
            break;
            case 'b':
                QuickPlacePeice(x,y,Bishop,1);
            break;
            case 'n':
                QuickPlacePeice(x,y,Knight,1);
            break;
            case 'r':
                QuickPlacePeice(x,y,Rook,1);
            break;
            case 'q':
                QuickPlacePeice(x,y,Queen,1);
            break;
            case 'k':
                QuickPlacePeice(x,y,King,1);
            break;
            case '/':
                y --;
                x = -1;
            break;
            default:
                x += (fen[i] - '1'); //subtract 1 from x bc its added again later
        }
        i++;
        x++;
        if (i >= fen.length())
            return;
    }
    i ++;
    if (i >= fen.length())
        return;
    if (fen[i] == 'b')
        endTurn();
    i += 2;
    if (i >= fen.length())
        return;
    _castleRights = 0;
    while (fen[i] != ' ') {
        switch(fen[i]){
            case 'q':
                _castleRights |= 0b1000;
            break;
            case 'k':
                _castleRights |= 0b0100;
            break;
            case 'Q':
                _castleRights |= 0b0010;
            break;
            case 'K':
                _castleRights |= 0b0001;
            break;
            default:
            break;
        }
        i ++;
        if (i >= fen.length())
            return;
    }
    i ++;
    if (i < fen.length())
        _enpassant = fen[i] - 'a';
    //last two feilds are ignored (i dont have win condition checks and i don't super care what turn it is)
}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) 
{
    bool c = getCurrentTurnNo()%2;
    int type = bit.gameTag() & 127;
    int sX = ((ChessSquare*)&src)->getColumn();
    int sY = ((ChessSquare*)&src)->getRow();
    int dX = ((ChessSquare*)&dst)->getColumn();
    if (!dst.empty())
        if (dst.bit()->gameTag() & 127 == Rook){
            _castleRights &= (0b0001 << (2*(!c) + (dX==0))) ^ 0b1111;
        }
    switch (type){
        case King:
            _castleRights &= 0b1100 >> 2*c;
            for (int i = -1; i < 2; i += 2)
                if ((&_grid[sY][4+i*2] == &dst)){
                    QuickPlacePeice(4+i,sY,Rook,c);
                    _grid[sY][7*(i==1)].destroyBit();
                }
            _enpassant = -1;
        break;
        case Rook:
            _castleRights &= (0b0001 << (2*c + (sX==0))) ^ 0b1111;
            _enpassant = -1;
        break;
        case Pawn:
            if (sY == 1 + c * 5)
                bit.setGameTag(Queen|(c<<7));
            for (int i = -1; i < 2; i += 2)
                if (sX+i == _enpassant && sY == 4 - c)
                    _grid[sY][sX+i].destroyBit();
            if (sY == (1 + 5*c) && &dst == &_grid[(3 + c)][sX])
                _enpassant = sX;
            else
                _enpassant = -1;
        break;
        default:
            _enpassant = -1;
        break;
    }
    endTurn();
}

void Chess::generateMoveList()
{
    //clear list
    _moveList.remove_if(forceTrueFunc);
    //find piece to move
    for (int x = 0; x < 8; x ++)
    for (int y = 0; y < 8; y ++){
        if (_grid[y][x].bit() == nullptr)
            continue;
        if (!canBitMoveFrom(*_grid[y][x].bit(),_grid[y][x]))
            continue;
        //try to move piece
        for (int dstX = 0; dstX < 8; dstX ++)
        for (int dstY = 0; dstY < 8; dstY ++)
            if (canBitMoveFromTo(*_grid[y][x].bit(),_grid[y][x],_grid[dstY][dstX]))
                _moveList.push_back(y<<9 | x<<6 | dstY<<3 | dstX);
    }
}

//
// free all the memory used by the game on the heap
//
void Chess::stopGame()
{
}

Player* Chess::checkForWinner()
{
    // check to see if either player has won

    return nullptr;
}

bool Chess::checkForDraw()
{
    // check to see if the board is full
    return false;
}



//
// add a helper to Square so it returns out FEN chess notation in the form p for white pawn, K for black king, etc.
// this version is used from the top level board to record moves
//
const char Chess::bitToPieceNotation(int row, int column) const {
    if (row < 0 || row >= 8 || column < 0 || column >= 8) {
        return '0';
    }

    const char* wpieces = { "?PNBRQK" };
    const char* bpieces = { "?pnbrqk" };
    unsigned char notation = '0';
    Bit* bit = _grid[row][column].bit();
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag() & 127];
    } else {
        notation = '0';
    }
    return notation;
}

//
// state strings
//
std::string Chess::initialStateString()
{
    return stateString();
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string Chess::stateString()
{
    std::string s;
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            s += bitToPieceNotation(y, x);
        }
    }
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void Chess::setStateString(const std::string &s)
{
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            int index = y * _gameOptions.rowX + x;
            int playerNumber = s[index] - '0';
            if (playerNumber) {
                _grid[y][x].setBit(PieceForPlayer(playerNumber - 1, Pawn));
            } else {
                _grid[y][x].destroyBit();
            }
        }
    }
}

int Chess::evaluateBoard(){
    return _boardScore;
}

//
// this is the function that will be called by the AI
//
void Chess::updateAI() 
{

}