//  Dalton Voth Othello
//  Problem: Play Othello with minimax search and alpha beta pruning
//  Solution: Implemented minimax search using a greater number of moves and corner occupation
//			  evaluation functions. While these are better than choosing random moves, they 
//			  could use more heuristics to determine better moves, such as more strategic spots on the board.
//  main.cpp
//
//  Due: 3/07/2019
//
//  PERFORMANCE SUMMARY (Times in microseconds)
//		Maxdepth: 3
//		Time: 540ms
//
//		Maxdepth: 6
//		Time: 1146ms

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "gamecomm.h"
#include <time.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

const int maxturn = 1;
const int minturn = -1;
const int blank = 0;
int nodecount = 0;
const int maxsucc = 3;
const int VS = -1000000;
const int VL = 1000000;
const int rowWidth = 8;
const int columnWidth = 8;
const int lateGame = 50;

struct board
{
	int m[8][8];	// 1, 0, -1
	int r, c, turn, h;	// the move that gets to this board
	board(int n[][8], int row = 8, int column = 8, int t = 1)
	{
		for (int k = 0; k<8; k++)
			for (int l = 0; l<8; l++)
				m[k][l] = n[k][l];
		r = row; c = column; turn = t;
	}
	board() {

	}
};

typedef board *state_t;
state_t best = NULL;

int max(int a, int b)
{
	return a > b ? a : b;
}

int min(int a, int b)
{
	return a > b ? b : a;
}

void swap(int &a, int &b)
{
	int tmp = a;
	a = b;
	b = tmp;
}

int other(int k)
{
	return k == 1 ? -1 : 1;
}

// flip pieces in the direction being moves while they are the opponent's piece
void flipPieces(state_t board, int x, int y, int xDirection, int yDirection, int turn)
{
	while (board->m[x][y] == turn * -1)
	{
		board->m[x][y] = turn;
		x += xDirection;
		y += yDirection;
	}
}

// Pretty much the same as checking if legal, but if the move is legal flip the pieces
void makeMove(state_t s, int row, int column, int turn)
{
	int r, c;
	s->m[row][column] = turn;

	// check up the column - is there a capture from above?
	for (row = row - 1; row >= 0 && s->m[row][column] == other(turn); row--);
	if (row < row - 1 && row >= 0 && s->m[row][column] == turn)
	{
		flipPieces(s, row, column + 1, 0, 1, turn);
	}
	// check down the column
	for (row = row + 1; row < 8 && s->m[row][column] == other(turn); row++);
	if (row > row + 1 && row < 8 && s->m[row][column] == turn)
	{
		flipPieces(s, row, column - 1, 0, -1, turn);
	}
	// check to the left in the row
	for (c = column - 1; c >= 0 && s->m[row][c] == other(turn); c--);
	if (c < column - 1 && c >= 0 && s->m[row][c] == turn)
	{
		flipPieces(s, row - 1, column, -1, 0, turn);
	}
	// check to the right in the row
	for (c = column + 1; c < 8 && s->m[row][c] == other(turn); c++);
	if (c > column + 1 && c < 8 && s->m[row][c] == turn)
	{
		flipPieces(s, row + 1, column, 1, 0, turn);
	}
	// check NW
	for (c = column - 1, row = row - 1; c >= 0 && row >= 0 && s->m[row][c] == other(turn); c--, row--);
	if (c < column - 1 && c >= 0 && row >= 0 && s->m[row][c] == turn)
	{
		flipPieces(s, row - 1, column + 1, -1, 1, turn);
	}
	// check NE
	for (c = column + 1, row = row - 1; c < 8 && row>0 && s->m[row][c] == other(turn); c++, row--);
	if (c > column + 1 && c < 8 && row >= 0 && s->m[row][c] == turn)
	{
		flipPieces(s, row + 1, column + 1, 1, 1, turn);
	}
	// check SE
	for (c = column + 1, row = row + 1; c < 8 && row < 8 && s->m[row][c] == other(turn); c++, row++);
	if (c > column + 1 && c < 8 && row < 8 && s->m[row][c] == turn)
	{
		flipPieces(s, row + 1, column - 1, 1, -1, turn);
	}

	// check SW
	for (c = column - 1, row = row + 1; c >= 0 && row < 8 && s->m[row][c] == other(turn); c--, row++);
	if (c < column - 1 && c >= 0 && row < 8 && s->m[row][c] == turn)
	{
		flipPieces(s, row - 1, column - 1, -1, -1, turn);
	}
}

bool legal(state_t s, int row, int column, int turn)
{

	// check up if it's open
	if (s->m[row][column] != 0)
		return false;


	int r, c;

	// check up the column - is there a capture from above?
	for (r = row - 1; r >= 0 && s->m[r][column] == other(turn); r--);
	if (r < row - 1 && r >= 0 && s->m[r][column] == turn)
	{
		return true;
	}
	// check down the column
	for (r = row + 1; r < 8 && s->m[r][column] == other(turn); r++);
	if (r > row + 1 && r < 8 && s->m[r][column] == turn)
	{
		return true;
	}
	// check to the left in the row
	for (c = column - 1; c >= 0 && s->m[row][c] == other(turn); c--);
	if (c < column - 1 && c >= 0 && s->m[row][c] == turn)
	{
		return true;
	}
	// check to the right in the row
	for (c = column + 1; c < 8 && s->m[row][c] == other(turn); c++);
	if (c > column + 1 && c < 8 && s->m[row][c] == turn)
	{
		return true;
	}
	// check NW
	for (c = column - 1, r = row - 1; c >= 0 && r >= 0 && s->m[r][c] == other(turn); c--, r--);
	if (c < column - 1 && c >= 0 && r >= 0 && s->m[r][c] == turn)
	{
		return true;
	}
	// check NE
	for (c = column + 1, r = row - 1; c < 8 && r>0 && s->m[r][c] == other(turn); c++, r--);
	if (c > column + 1 && c < 8 && r >= 0 && s->m[r][c] == turn)
	{
		return true;
	}
	// check SE
	for (c = column + 1, r = row + 1; c < 8 && r < 8 && s->m[r][c] == other(turn); c++, r++);
	if (c > column + 1 && c < 8 && r < 8 && s->m[r][c] == turn)
	{
		return true;
	}

	// check SW
	for (c = column - 1, r = row + 1; c >= 0 && r < 8 && s->m[r][c] == other(turn); c--, r++);
	if (c < column - 1 && c >= 0 && r < 8 && s->m[r][c] == turn)
	{
		return true;
	}

	return false;
}

void copyBoard(state_t oldBoard, state_t newBoard) {
	for (int x = 0; x < 8; x++) {
		for (int y = 0; y < 8; y++)
		{
			newBoard->m[x][y] = oldBoard->m[x][y];
		}
	}
}

void expand(state_t state, state_t successor[], int moveX[], int moveY[], int &sn, int turn)
{
	sn = 0;
	for (int x = 0; x < 8; x++) {
		for (int y = 0; y < 8; y++)
		{
			if (legal(state, x, y, turn))
			{
				successor[sn] = new board();
				copyBoard(successor[sn], state);
				makeMove(successor[sn], x, y, turn);
				turn *= -1;
				moveX[sn] = x;
				moveY[sn] = y;
				sn++;
			}
		}
	}

	if (sn == 0) {
		successor[sn] = new board();
		copyBoard(successor[sn], state);
	}
}

int piecesOnBoard(state_t board) {
	int pieces = 0;
	for (int i = 0; i < rowWidth; i++) {
		for (int j = 0; j < columnWidth; j++) {
			if (board->m[i][j] == 1 || board->m[i][j] == -1) {
				pieces++;
			}
		}
	}

	return pieces;
}

// piece of an evaluation function that returns the ratio of maxturns to minturns or minturns to maxturns 
// based on the current turn
int movesAvailable(state_t board, int turn) {
	int minMoves = 0;
	int maxMoves = 0;

	for (int i = 0; i < rowWidth; i++) {
		for (int j = 0; j < columnWidth; j++) {
			if (legal(board, i, j, minturn)) {
				minMoves++;
			}
		}
	}

	for (int i = 0; i < rowWidth; i++) {
		for (int j = 0; j < columnWidth; j++) {
			if (legal(board, i, j, maxturn)) {
				maxMoves++;
			}
		}
	}

	// Returns ratio of respective max vs. min moves to weigh how good or bad the board is for us
	if (turn == maxturn) {
		return 100 * (maxMoves - minMoves) / (maxMoves + minMoves + 1);
	}
	else {
		return 100 * (minMoves - maxMoves) / (maxMoves + minMoves + 1);
	}
}

int corners(state_t board, int turn) {
	int maxCorners = 0;
	int minCorners = 0;

	return 0;

	for (int c = 0; c < 4; c++) {
		if (board->m[0][7] == maxturn) {
			maxCorners++;
		}
		else if (board->m[0][7] == minturn) {
			minCorners++;
		}
	}

	// Returns ratio of respective max vs. min corners to weigh how good or bad the board is for us
	if (maxturn) {
		return 100 * (maxCorners - minCorners) / (maxCorners + minCorners + 1);
	}
	else {
		return 100 * (minCorners - maxCorners) / (maxCorners + minCorners + 1);
	}
}

int eval(state_t s, int turn)
{
	if (piecesOnBoard(s) <= lateGame)
		return 10 * movesAvailable(s, turn) + VL * corners(s, turn); // moves available are good to give more options, corners are said to be best you can get
	else 
		return VL * corners(s, turn); // Moves available are limited late game and don't really matter
}

bool isterminal(state_t board)
{
	for (int i = 0; i < rowWidth; i++) {
		for (int j = 0; j < columnWidth; j++) {
			if (board->m[i][j] == 0) {
				return false;
			}
		}
	}
	return true;
}

int alphabeta(state_t state, int maxDepth, int curDepth,
	int alpha, int beta, int &r, int &c)
{
	int moveX[60], moveY[60];
	nodecount++;
	// cout << "entetring " << state << endl;
	int succnum, turn;
	if (curDepth % 2 == 0) // This is a MAX node 
						   // since MAX has depth of: 0, 2, 4, 6, ...
		turn = maxturn;
	else
		turn = minturn;

	if (curDepth == maxDepth || isterminal(state)) // CUTOFF test
	{
		int UtilV = eval(state, turn);
		//cout << state << " [" << UtilV << "]\t";
		return UtilV;  // eval returns the heuristic value of state
	}

	state_t successor[maxsucc];

	expand(state, successor, moveX, moveY, succnum, turn); // find all successors of state

	if (turn == maxturn) // This is a MAX node 
						 // since MAX has depth of: 0, 2, 4, 6, ...
	{
		alpha = VS; // initialize to some very small value 
		for (int k = 0; k<succnum; k++)
		{
			state_t tempState = new board();
			copyBoard(tempState, successor[k]);
			// recursively find the value of each successor
			int curvalue = alphabeta(tempState, maxDepth, curDepth + 1, alpha, beta, r, c);
			//alpha = max(alpha,curvalue); // update alpha
			if (curvalue>alpha || curvalue == alpha && time(0) % 2 == 0)
			{
				alpha = curvalue;
				if (curDepth == 0) {
					best = successor[k];
					r = moveX[k];
					c = moveY[k];
				}
			}
			if (alpha >= beta)
			{
				//cout << state << " [" << alpha << "]\n";
				return alpha; // best = successor[k];
			}

		}
		//cout << state << " [" << alpha << "]\n";
		return alpha;
	}
	else // A MIN node
	{
		beta = VL;  // initialize to some very large value
		for (int k = 0; k<succnum; k++)
		{
			state_t tempState = new board();
			copyBoard(tempState, successor[k]);
			// recursively find the value of each successor
			int curvalue = alphabeta(tempState, maxDepth, curDepth + 1, alpha, beta, r, c);
			beta = min(beta, curvalue); // update beta
			if (alpha >= beta) {
				//cout << state << " [" << beta << "]\n";

				r = moveX[k];
				c = moveY[k];
				return beta; // best = successor[k];
			}
		}
		//cout << state << " [" << beta << "]\n";
		return beta;
	}
}

void printboard(int m[][8])
{
for (int r=0;r<8;r++)
{
for (int c=0;c<8;c++)
{
if (m[r][c]==minturn)
cout << 'b';
else if (m[r][c]==maxturn)
cout << 'w';
else
cout << '_';
cout << " ";
}
cout << endl;
}
cout << endl << endl;
}


int main()
{
	int curDepth = 0, r = -1, c = -1;
	int n[8][8];
	getGameBoard(n);
	state_t s = new board(n), succ = NULL;

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	alphabeta(s, maxsucc, curDepth, VS, VL, r, c);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	auto duration = duration_cast<microseconds>(t2 - t1).count();

	cout << endl << "Maxdepth: " << maxsucc << endl;
	cout << "Time: " << duration << "ms";

	putMove(r, c);
}