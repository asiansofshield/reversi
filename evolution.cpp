#include "reversi.h"
#include "neuralNet.h"
#include "evolution.h"
#include <sstream>
#include <cfloat>
#include <algorithm>
double max(Board reversiBoard, Organism org, int lastMove, int depth, player mover);
double min(Board reversiBoard, Organism org, int lastMove, int depth, player mover);
double maxAlphaBeta(Board reversiBoard, Organism org, int lastMove, int depth, player mover, double alpha, double beta);
double minAlphaBeta(Board reversiBoard, Organism org, int lastMove, int depth, player mover, double alpha, double beta);
vector<int> wins;
vector<int> ties;
Organism::Organism(vector<unsigned int> layerSizes, ActFunc myFunction, int depth)
{
   brain = NeuralNet(layerSizes);
   fitness = 0;
   myFunc = myFunction;
   this->layerSizes = layerSizes;
   depthLimit = depth;
}

Organism::Organism(vector<vector<vector<double> > > netweights, ActFunc myFunction, int depth)
{
   brain = NeuralNet(netweights);
   myFunc = myFunction;
   depthLimit = depth;
}

Organism::Organism()
{
   fitness = 0;
   layerSizes.clear();
   depthLimit = 0;
}

Population::Population(vector<vector<Organism> > newPop)
{
   pop = newPop;
}

Population::Population(vector<unsigned int> layerSizes, int popSize, ActFunc myFunction, int depthLim)
{
   Organism currentGuy = Organism(); 
   int i, j;
   vector<Organism> row;
   for(i = 0; i < popSize; i++)
   {
      for(j = 0; j < popSize; j++)
      {
         currentGuy = Organism(layerSizes, myFunction, depthLim);
         row.push_back(currentGuy);
      }
      pop.push_back(row);
      row.clear();
   }
}

Population::Population()
{
   pop.clear();
}

void Population::playNeighbors()
{
   int games;
   games = 0;
   int i, j, rowInc, colInc, row, column;
   int size;
   size = this->pop.size();
   pair<int, int> fitnessUpdate;
   for(i = 0; i < size; i++)
   {
      for(j = 0; j < size; j++)
      {
         for(rowInc = -1; rowInc <= 1; rowInc++)
         {
            for(colInc = -1; colInc <= 1; colInc++)
            {
               if(rowInc != colInc)
               {
                  games++;
                  row = (i + size + rowInc) % size;
                  column = (j + size + colInc) % size;
                  pop.at(i).at(j);
                  pop.at(row).at(column);
                  fitnessUpdate = playGame(pop.at(i).at(j), pop.at(row).at(column));
                  pop.at(i).at(j).updateFitness(fitnessUpdate.first);
                  pop.at(row).at(column).updateFitness(fitnessUpdate.second);
               }
            }
         }
      }
   }
}

pair<int, int> Population::playGame(Organism blackPlayer, Organism whitePlayer)
{
   srandom(time(0));
   // black and then white for this pair
   pair<int, int> fitnesses;
   pair<int, int> finalScore;
   int size, blackMove, whiteMove, emptyPieces, depthLimitWhite, depthLimitBlack;
   fitnesses.first = 0;
   fitnesses.second = 0;
   depthLimitBlack = blackPlayer.getDepth();
   depthLimitWhite = whitePlayer.getDepth();
   Board reversiBoard = Board(8);
   size = reversiBoard.getSize();
    while(!reversiBoard.isGameOver())
   {
      
      // player A goes first, if they still have valid moves,
      // Otherwise, pass to black
      if(reversiBoard.getValidMoves(Black).size() != 0)
      {
         if(depthLimitBlack == 0)
         {
            blackMove = reversiAgentOneMove(reversiBoard, Black, blackPlayer);
         }
         else
         {
            blackMove = reversiAgentMiniMax(reversiBoard, Black, blackPlayer);
         }
         //if it is an invalid move, B automatically wins
         if(!reversiBoard.isValidMove(blackMove, Black))
         {
            cout << blackMove << " : " << "Black made an invalid move" << endl;
            cerr << "Black made an invalid move" << endl;
            exit(1);
            finalScore = reversiBoard.getScore();
            // First Organism gets 64 points for winning,
            fitnesses.first += size * size;
            // see how many empty pieces r on the board
            emptyPieces = size * size - (finalScore.first + finalScore.second);
            // winner takes credit for empty spaces.
            fitnesses.first = fitnesses.first + emptyPieces + finalScore.first;
            // loser only gets the number of his tiles
            fitnesses.second += finalScore.second;
            return fitnesses;
         }
         // Update the board for player A
         else
         {
            //cout << "make black's move" << endl;
            reversiBoard.makeMove(blackMove, Black);
            //reversiBoard.printBoard();
         }

         // if playerA made a move that won them the game then return result
         if(reversiBoard.isGameOver())
         {
            finalScore = reversiBoard.getScore();
            // black is first in this pair.
            //cout << "White: " << finalScore.second << endl;
            //cout << "Black: " << finalScore.first << endl;
            // WINNER TAKES EMPTY SPACES
            // if black wins
            if(finalScore.first > finalScore.second)
            {
               // First Organism gets 64 points for winning,
               fitnesses.first += size * size;
               // see how many empty pieces r on the board
               emptyPieces = size * size - (finalScore.first + finalScore.second);
               // winner takes credit for empty spaces.
               fitnesses.first = fitnesses.first + emptyPieces + finalScore.first;
               // loser only gets the number of his tiles
               fitnesses.second += finalScore.second;
            }
            // if white wins
            else if (finalScore.first < finalScore.second)
            {
               fitnesses.second += size * size;
               emptyPieces = size * size - (finalScore.second + finalScore.first);
               fitnesses.second = fitnesses.second + emptyPieces + finalScore.second;
               fitnesses.first += finalScore.first;
            }
            // if there is a tie
            else
            {
               fitnesses.first += (size * size) / 2 + finalScore.first;
               fitnesses.second += (size * size) / 2 + finalScore.second;
            }
         }
      }
      
      if(reversiBoard.getValidMoves(White).size() != 0)
      {
         if(depthLimitWhite == 0)
         {
            whiteMove = reversiAgentOneMove(reversiBoard, White, whitePlayer);
         }
         else
         {
            whiteMove = reversiAgentMiniMax(reversiBoard, White, whitePlayer);
         }
         // if it is an invalid move, A automatically wins
         if(!reversiBoard.isValidMove(whiteMove, White))
         {
            finalScore = reversiBoard.getScore();
            cout << whiteMove << " : "<<"White made an invalid move" << endl;
            cerr << "White made an invalid move" << endl;
            exit(0);
            fitnesses.second += size * size;
            emptyPieces = size * size - (finalScore.second + finalScore.first);
            fitnesses.second = fitnesses.second + emptyPieces + finalScore.second;
            fitnesses.first += finalScore.first;
            return fitnesses;
         }
         // Update the board for player
         else
         {
            reversiBoard.makeMove(whiteMove, White);
            //reversiBoard.printBoard();
         }

         // if playerBlack made a move that won them the game then return result
         if(reversiBoard.isGameOver())
         {
            
            finalScore = reversiBoard.getScore();
            // WINNER TAKES EMPTY SPACES
            // if black wins
            if(finalScore.first > finalScore.second)
            {
               // First Organism gets 64 points for winning,
               fitnesses.first += size * size;
               // see how many empty pieces r on the board
               emptyPieces = size * size - (finalScore.first + finalScore.second);
               // winner takes credit for empty spaces.
               fitnesses.first = fitnesses.first + emptyPieces + finalScore.first;
               // loser only gets the number of his tiles
               fitnesses.second += finalScore.second;
            }
            // if white wins
            else if (finalScore.first < finalScore.second)
            {
               fitnesses.second += size * size;
               emptyPieces = size * size - (finalScore.second + finalScore.first);
               fitnesses.second = fitnesses.second + emptyPieces + finalScore.second;
               fitnesses.first += finalScore.first;
            }
            // if there is a tie
            else
            {
               fitnesses.first += size * size;
               fitnesses.second += size * size;
            }
         }
      }
      else 
      {
         //cout << endl << "White has no moves, and passes to Black" << endl;
      }

   }

   
   return fitnesses;
}

pair<int, int> Population::playGamePrint(Organism blackPlayer, Organism whitePlayer)
{
   srandom(time(0));
   // white and then black for this pair
   pair<int, int> fitnesses;
   pair<int, int> finalScore;
   int emptyPieces;
   fitnesses.first = 0;
   fitnesses.second = 0;
   int size;
   Board reversiBoard = Board(8);
   size = reversiBoard.getSize();
    while(!reversiBoard.isGameOver())
   {
      
      // player A goes first, if they still have valid moves,
      // Otherwise, pass to black
      if(reversiBoard.getValidMoves(Black).size() != 0)
      {
         int playerBlack_move = reversiAgentOneMove(reversiBoard, Black, blackPlayer);
         
         //if it is an invalid move, B automatically wins
         if(!reversiBoard.isValidMove(playerBlack_move, Black))
         {
            cout << playerBlack_move << " : " << "Black made an invalid move" << endl;
            reversiBoard.printBoard();
            finalScore = reversiBoard.getScore();
            // First Organism gets 64 points for winning,
            fitnesses.first += size * size;
            // see how many empty pieces r on the board
            emptyPieces = size * size - (finalScore.first + finalScore.second);
            // winner takes credit for empty spaces.
            fitnesses.first = fitnesses.first + emptyPieces + finalScore.first;
            // loser only gets the number of his tiles
            fitnesses.second += finalScore.second;
            return fitnesses;
         }
         // Update the board for player A
         else
         {
            //cout << "make black's move" << endl;
            reversiBoard.makeMove(playerBlack_move, Black);
            reversiBoard.printBoard();
         }

         // if playerA made a move that won them the game then return result
         if(reversiBoard.isGameOver())
         {
            finalScore = reversiBoard.getScore();
            // black is first in this pair.
            //cout << "White: " << finalScore.second << endl;
            //cout << "Black: " << finalScore.first << endl;
            // WINNER TAKES EMPTY SPACES
            // if black wins
            if(finalScore.first > finalScore.second)
            {
               // First Organism gets 64 points for winning,
               fitnesses.first += size * size;
               // see how many empty pieces r on the board
               emptyPieces = size * size - (finalScore.first + finalScore.second);
               // winner takes credit for empty spaces.
               fitnesses.first = fitnesses.first + emptyPieces + finalScore.first;
               // loser only gets the number of his tiles
               fitnesses.second += finalScore.second;
            }
            // if white wins
            else if (finalScore.first < finalScore.second)
            {
               fitnesses.second += size * size;
               emptyPieces = size * size - (finalScore.second + finalScore.first);
               fitnesses.second = fitnesses.second + emptyPieces + finalScore.second;
               fitnesses.first += finalScore.first;
            }
            // if there is a tie
            else
            {
               fitnesses.first += (size * size) / 2 + finalScore.first;
               fitnesses.second += (size * size) / 2 + finalScore.second;
            }
         }
      }
      else 
      {
         //cout << endl << "Black has no valid moves and passes to White"  << endl;
      }
      if(reversiBoard.getValidMoves(White).size() != 0)
      {
         int playerWhite_move = reversiAgentOneMove(reversiBoard, White, whitePlayer);
         // if it is an invalid move, A automatically wins
         if(!reversiBoard.isValidMove(playerWhite_move, White))
         {
            finalScore = reversiBoard.getScore();
            //cout << playerWhite_move << " : "<<"White made an invalid move" << endl;
            cout << playerWhite_move << " : "<<"White made an invalid move" << endl;
            reversiBoard.printBoard();
            fitnesses.second += size * size;
            emptyPieces = size * size - (finalScore.second + finalScore.first);
            fitnesses.second = fitnesses.second + emptyPieces + finalScore.second;
            fitnesses.first += finalScore.first;
            return fitnesses;
         }
         // Update the board for player
         else
         {
            reversiBoard.makeMove(playerWhite_move, White);
            reversiBoard.printBoard();
         }

         // if playerBlack made a move that won them the game then return result
         if(reversiBoard.isGameOver())
         {
            finalScore = reversiBoard.getScore();
            //cout << "Black: " << finalScore.first << endl;
            //cout << "White: " << finalScore.second << endl;
            // WINNER TAKES EMPTY SPACES
            // if black wins
            if(finalScore.first > finalScore.second)
            {
               // First Organism gets 64 points for winning,
               fitnesses.first += size * size;
               // see how many empty pieces r on the board
               emptyPieces = size * size - (finalScore.first + finalScore.second);
               // winner takes credit for empty spaces.
               fitnesses.first = fitnesses.first + emptyPieces + finalScore.first;
               // loser only gets the number of his tiles
               fitnesses.second += finalScore.second;
            }
            // if white wins
            else if (finalScore.first < finalScore.second)
            {
               fitnesses.second += size * size;
               emptyPieces = size * size - (finalScore.second + finalScore.first);
               fitnesses.second = fitnesses.second + emptyPieces + finalScore.second;
               fitnesses.first += finalScore.first;
            }
            // if there is a tie
            else
            {
               fitnesses.first += size * size;
               fitnesses.second += size * size;
            }
         }
      }
      else 
      {
         //cout << endl << "White has no moves, and passes to Black" << endl;
      }

   }

   
   return fitnesses;
}

vector<vector<int> > Population::getAllFitnesses()
{
   vector<int> row;
   vector<vector<int> > fitnessVec;
   int size = this->pop.size();
   int i,j;
   for(i = 0; i < size; i++)
   {
      for(j = 0; j < size; j++)
      {
         row.push_back(pop.at(i).at(j).getFitness());
      }
      fitnessVec.push_back(row);
      row.clear();
   }
   return fitnessVec;
}

int reversiAgentAlphaBeta(Board reversiBoard, player mover, Organism org)
{
   vector<double> inputs;
   Board copyBoard;
   int bestMove, lastMove;
   unsigned int i;
   double maxHeuristic, heuristic;
   NeuralNet network = org.getNet();
   vector<int> availableMoves;
   availableMoves = reversiBoard.getValidMoves(mover); 
   lastMove = availableMoves.at(0);
   maxHeuristic = -DBL_MAX;
   bestMove = -1;
   vector<double> heuristicVals;
   if(availableMoves.size() == 1)
   {
      return availableMoves.at(0);
   }
   for(i = 0; i < availableMoves.size(); i++)
   {
      lastMove = availableMoves.at(i);
      heuristic = maxAlphaBeta(reversiBoard, org, lastMove, 0, mover, -DBL_MAX, DBL_MAX);
      heuristicVals.push_back(heuristic);
      if(heuristic > maxHeuristic)
      {
         bestMove = availableMoves.at(i);
         maxHeuristic = heuristic;
      }
   }
   if(!reversiBoard.isValidMove(bestMove, mover))
   {
      for(i = 0; i < heuristicVals.size(); i++)
      {
         cout << heuristicVals.at(i) << endl;
      }
   }
   return bestMove;
}

double maxAlphaBeta(Board reversiBoard, Organism org, int lastMove, int depth, player mover, double alpha, double beta)
{
   Board copyBoard;
   unsigned int i;
   int depthLimit;
   double maxHeuristic;
   player enemy;
   NeuralNet network;
   ActFunc currentAct;
   vector<int> availableMoves;
   // Take the last move considered;
   copyBoard = reversiBoard;
   copyBoard.makeMove(lastMove, mover);
   depthLimit = org.getDepth();
   currentAct = org.getActFunc();
   vector<double> inputs;
   maxHeuristic = -DBL_MAX;
   if(copyBoard.isGameOver())
   {
      pair<int, int> score;
      int fitness;
      score = copyBoard.getScore();
      fitness = 0;
      if(mover == Black)
      {
         if(score.first > 32)
         {
            fitness += 64;
         }
         if(score.first == 32)
         {
            fitness += 32;
         }
         fitness += score.first;
      }
      else// if(mover == White)
      {
         if(score.second > 32)
         {
            fitness += 64;
         }
         if(score.second == 32)
         {
            fitness += 32;
         }
         fitness += score.second;
      }
      return fitness;
   }
   if(depth >= depthLimit)
   {
      inputs = copyBoard.boardToInput(mover);
      return org.getNet().calculateNet(inputs, currentAct);
   }
   if(mover == White)
   {
      enemy = Black;
   }
   else 
   {
      enemy = White;
   }
   availableMoves = reversiBoard.getValidMoves(mover);
   for(i = 0; i < availableMoves.size(); i++)
   {
      lastMove = availableMoves.at(i);
      if(copyBoard.isValidMove(lastMove, mover))
      {
         // find how good the best move we've seen so far is.
         maxHeuristic = max(maxHeuristic, minAlphaBeta(copyBoard, org, lastMove, depth + 1, enemy, alpha, beta));
         alpha = max(alpha, maxHeuristic);
         if(alpha >= beta)
         {
            //cout << "Max - Depth: " << depth << "   Heuristic: " << maxHeuristic << endl;
            return maxHeuristic;
         }
      }
   }
   if(availableMoves.size() == 0)
   {
      return minAlphaBeta(copyBoard, org, lastMove, depth + 1, enemy, alpha, beta);
   }
   
   //cout << "Max - Depth: " << depth << "   Heuristic: " << maxHeuristic << endl;
      return maxHeuristic;
}

double minAlphaBeta(Board reversiBoard, Organism org, int lastMove, int depth, player mover, double alpha, double beta)
{
   Board copyBoard;
   unsigned int i;
   int depthLimit;
   double minHeuristic;
   player enemy;
   NeuralNet network;
   ActFunc currentAct;
   vector<int> availableMoves;
   // Take the last move considered;
   copyBoard = reversiBoard;
   copyBoard.makeMove(lastMove, mover);
   depthLimit = org.getDepth();
   currentAct = org.getActFunc();
   vector<double> inputs;
   minHeuristic = DBL_MAX;
   if(copyBoard.isGameOver())
   {
      pair<int, int> score;
      int fitness;
      score = copyBoard.getScore();
      fitness = 0;
      if(mover == Black)
      {
         if(score.first > 32)
         {
            fitness += 64;
         }
         if(score.first == 32)
         {
            fitness += 32;
         }
         fitness += score.first;
      }
      else// if(mover == White)
      {
         if(score.second > 32)
         {
            fitness += 64;
         }
         if(score.second == 32)
         {
            fitness += 32;
         }
         fitness += score.second;
      }
      return fitness;
   }
   if(depth >= depthLimit)
   {
      inputs = copyBoard.boardToInput(mover);
      return org.getNet().calculateNet(inputs, currentAct);
   }
   if(mover == White)
   {
      enemy = Black;
   }
   else 
   {
      enemy = White;
   }
   availableMoves = reversiBoard.getValidMoves(mover);
   for(i = 0; i < availableMoves.size(); i++)
   {
      lastMove = availableMoves.at(i);
      if(copyBoard.isValidMove(lastMove, mover))
      {
         // find how good the best move we've seen so far is.
         minHeuristic = min(minHeuristic, maxAlphaBeta(copyBoard, org, lastMove, depth + 1, enemy, alpha, beta));
         beta = min(beta, minHeuristic);
         if(alpha >= beta)
         {
            //cout << "Min - Depth: " << depth << "   Heuristic: " << minHeuristic << endl;
            return minHeuristic;
         }
      }
   }
   //cout << "Min - Depth: " << depth << "   Heuristic: " << minHeuristic << endl;
   return minHeuristic;
}

int reversiAgentMiniMax(Board reversiBoard, player mover, Organism org)
{
   vector<double> inputs;
   Board copyBoard;
   int bestMove, lastMove;
   unsigned int i;
   double maxHeuristic, heuristic;
   NeuralNet network = org.getNet();
   vector<int> availableMoves;
   availableMoves = reversiBoard.getValidMoves(mover); 
   lastMove = availableMoves.at(0);
   maxHeuristic = -DBL_MAX;
   bestMove = -1;
   if(availableMoves.size() == 1)
   {
      return availableMoves.at(0);
   }
   //bestMove = availableMoves.at(0);
   for(i = 0; i < availableMoves.size(); i++)
   {
      lastMove = availableMoves.at(i);
      heuristic = max(reversiBoard, org, lastMove, 0, mover);
      if(heuristic > maxHeuristic)
      {
         bestMove = availableMoves.at(i);
         maxHeuristic = heuristic;
      }
   }
  
   return bestMove;
}

double max(Board reversiBoard, Organism org, int lastMove, int depth, player mover)
{
   Board copyBoard;
   unsigned int i;
   int depthLimit;
   double maxHeuristic, heuristic;
   player enemy;
   NeuralNet network;
   ActFunc currentAct;
   vector<int> availableMoves;
   // Take the last move considered;
   copyBoard = reversiBoard;
   copyBoard.makeMove(lastMove, mover);
   depthLimit = org.getDepth();
   currentAct = org.getActFunc();
   vector<double> inputs;
   maxHeuristic = -DBL_MAX;
   if(mover == White)
   {
      enemy = Black;
   }
   else 
   {
      enemy = White;
   }
   if(copyBoard.isGameOver())
   {
      pair<int, int> score;
      int fitness;
      score = copyBoard.getScore();
      fitness = 0;
      if(mover == Black)
      {
         if(score.first > 32)
         {
            fitness += 64;
         }
         if(score.first == 32)
         {
            fitness += 32;
         }
         fitness += score.first;
      }
      else// if(mover == White)
      {
         if(score.second > 32)
         {
            fitness += 64;
         }
         if(score.second == 32)
         {
            fitness += 32;
         }
         fitness += score.second;
      }
      return fitness;
   }
   if(depth >= depthLimit)
   {
      inputs = copyBoard.boardToInput(mover);
      heuristic = org.getNet().calculateNet(inputs, currentAct);
      //cout << "Max - Depth: " << depth << "   Heuristic: " << heuristic << endl;
      return heuristic;
   }
   availableMoves = reversiBoard.getValidMoves(mover);
      for(i = 0; i < availableMoves.size(); i++)
      {
         lastMove = availableMoves.at(i);
         if(copyBoard.isValidMove(lastMove, mover))
         {
            // find how good the best move we've seen so far is.
            heuristic = min(copyBoard, org, lastMove, depth + 1, enemy);
            if(heuristic > maxHeuristic)
            {
               //bestMove = lastMove;
               maxHeuristic = heuristic;
            }
         }
      }
      if(availableMoves.size() == 0)
      {
         return min(copyBoard, org, lastMove, depth + 1, enemy);
      }
      
      //cout << "Max - Depth: " << depth << "   Heuristic: " << heuristic << endl;
      return maxHeuristic;
}

double min(Board reversiBoard, Organism org, int lastMove, int depth, player mover)
{
   Board copyBoard;
   unsigned int i;
   int depthLimit;
   double minHeuristic, heuristic;
   player enemy;
   NeuralNet network;
   ActFunc currentAct;
   vector<int> availableMoves;
   // Take the last move considered;
   copyBoard = reversiBoard;
   copyBoard.makeMove(lastMove, mover);
   depthLimit = org.getDepth();
   currentAct = org.getActFunc();
   vector<double> inputs;
   minHeuristic = DBL_MAX;
   if(mover == White)
   {
      enemy = Black;
   }
   else 
   {
      enemy = White;
   }
   if(copyBoard.isGameOver())
   {
      pair<int, int> score;
      int fitness;
      score = copyBoard.getScore();
      fitness = 0;
      if(mover == Black)
      {
         if(score.first > 32)
         {
            fitness += 64;
         }
         if(score.first == 32)
         {
            fitness += 32;
         }
         fitness += score.first;
      }
      else// if(mover == White)
      {
         if(score.second > 32)
         {
            fitness += 64;
         }
         if(score.second == 32)
         {
            fitness += 32;
         }
         fitness += score.second;
      }
      return fitness;
   }
   if(depth >= depthLimit)
   {
      inputs = copyBoard.boardToInput(mover);
      heuristic = org.getNet().calculateNet(inputs, currentAct);
      //cout << "Min - Depth: " << depth << "   Heuristic: " << heuristic << endl;
      return heuristic;
   }
   availableMoves = reversiBoard.getValidMoves(mover);
   for(i = 0; i < availableMoves.size(); i++)
   {
      lastMove = availableMoves.at(i);
      if(copyBoard.isValidMove(lastMove, mover))
      {
         // find how good the best move we've seen so far is.
         heuristic = max(copyBoard, org, lastMove, depth + 1, enemy);
         if(heuristic < minHeuristic)
         
            minHeuristic = heuristic;
      }
   }
   if(availableMoves.size() == 0)
   {
      return max(copyBoard, org, lastMove, depth + 1, enemy);
   }
   //cout << "Min - Depth: " << depth << "   Heuristic: " << heuristic << endl;
   return minHeuristic;
}

/*
double miniMax(Board reversiBoard, Organism org, int lastMove, int depth, player mover)
{
   // Take the last move considered;
   Board copyBoard = reversiBoard;
   copyBoard.makeMove(lastMove, mover);
   unsigned int i;
   int depthLimit;
   player enemy;
   depthLimit = org.getDepth();
   NeuralNet network = org.getNet();
   ActFunc currentFunction = org.getActFunc();
   vector<int> availableMoves;
   vector<double> inputs = copyBoard.boardToInput(mover);
   if(mover == White)
   {
      enemy = Black;
   }
   else 
   {
      enemy = White;
   }
   if(reversiBoard.isGameOver() || depth > depthLimit)
   {
      return network.calculateNet(inputs, currentAct);
   }
      
   // If we are maxing the depth is even
   if(depth % 2 == 0)
   {
      availableMoves = reversiBoard.getValidMoves(mover);
      for(i = 0; i < availableMoves.size(); i++)
      {
         lastMove = availableMoves.at(i);
         if(copyBoard.isValidMove(lastMove))
         {
            // find how good the best move we've seen so far is.
            
            if(miniMax(reversiBoard, lastMove, bestMove, depth + 1) == DBL_MAX)
            {
               bestMove = last;
               return DBL_MAX;
            }
         }
      }
      return -DBL_MAX;
   }
   // minimizing
   else
   {
      if(reversiBoard.isGameOver())
      {
         return -DBL_MAX;
      }
      availableMoves = reversiBoard.getValidMoves(enemy);
      for(i = 0; i < availableMoves.size(); i++)
      {
         lastMove = availableMoves.at(i);
         if(copyBoard.isValidMove(lastMove))
         {
            if(miniMax(reversiBoard, org, lastMove, bestMove, depth + 1, ) == -DBL_MAX)
            {
               return -DBL_MAX;
            }
         }
      }
      return DBL_MAX;
   }
}
*/
int reversiAgentOneMove(Board reversiBoard, player mover, Organism org)
{
   vector< double> inputs;
   Board copyBoard;
   int bestMove;
   unsigned int i;
    double maxHeuristic, heuristic;
   maxHeuristic = -DBL_MAX;
   bestMove = -1;
   heuristic = 0;
   NeuralNet network = org.getNet();
   ActFunc currentFunction = org.getActFunc();
   //vector<double> heuristicVals;
   vector<int> availableMoves;
   availableMoves = reversiBoard.getValidMoves(mover);
   //cout << "num of avail moves : "  << availableMoves.size() << endl;
   if(availableMoves.size() == 1)
   {
      return availableMoves[0];
   }
   for(i = 0; i < availableMoves.size(); i++)
   {
      
      // only evaluating one move ahead so it should be reset each time.
      copyBoard = reversiBoard;
      if(copyBoard.isValidMove(availableMoves.at(i), mover))
      {
         copyBoard.makeMove(availableMoves.at(i), mover);
         inputs = copyBoard.boardToInput(mover);
         heuristic = network.calculateNet(inputs, currentFunction);
         //heuristicVals.push_back(heuristic);
         //cout << "Heuristic: " << heuristic << endl;
         if(heuristic > maxHeuristic)
         {
            //cout << "new Max found" << endl;
            maxHeuristic = heuristic;
           
            bestMove = availableMoves.at(i);
            //cout << "Best move = " << bestMove << endl;
         }
      }
   }
   if(bestMove != -1)
   {
      
      return bestMove;
   }
   cout << "no move found" << endl;
   /*
      for(i = 0; i < heuristicVals.size(); i++)
      {
         cout << heuristicVals.at(i) << endl;
      }*/
   return bestMove; // return something else maybe?
}

Population Population::createNextGen()
{
   //cout << "creating next Gen" << endl;
   int parentsFound = 0;
   int i, j, rowInc, colInc, row, column;
   int size;
   srandom(time(0));
   NeuralNet child, mom, dad;
   size = this->pop.size();
   Population oldGen = *this;
   vector<unsigned int> layers = oldGen.pop.at(0).at(0).getLayerSizes();
   ActFunc function = oldGen.pop.at(0).at(0).getActFunc();
   int depth = oldGen.pop.at(0).at(0).getDepth();
   Population nextGen(layers, size, function, depth);
   Organism parentOrg(layers, function, depth);
   vector<Organism> parentCandidates;
   Organism father;
   pair<Organism, Organism> chosenParents;
   //vector<int> parentRow;
   //vector<int> parentColumn;
   vector<int> fitnessProb;
   //cout << "finding candidates" << endl;
   for(i = 0; i < size; i++)
   {
      for(j = 0; j < size; j++)
      {
         // center organism
         father = oldGen.pop.at(i).at(j);
         for(rowInc = -1; rowInc <= 1; rowInc++)
         {
            for(colInc = -1; colInc <= 1; colInc++)
            {
               if(rowInc != colInc)
               {
                  
                  row = (i + size + rowInc) % size;
                  column = (j + size + colInc) % size;
                  //cout << "candidate found" << endl;
                  parentCandidates.push_back(oldGen.pop.at(row).at(column));
               }
            }
         }
         //cout << "Calling parentSel" << endl;
         parentsFound++;
         chosenParents = parentSelection(parentCandidates, father);
         //cout << "mom and dad init" << endl;
         mom = chosenParents.first.getNet();
         dad = chosenParents.second.getNet();
         //cout << "calling crossover" << endl;
         //cout << "mom: " << endl;
         //mom.printWeights();
         //cout << "dad: " << endl;
         //dad.printWeights();
         child = mom.crossover(dad, layers);
         //cout << "assigning new pop" << endl;
         nextGen.pop.at(i).at(j).setNet(child);
         nextGen.pop.at(i).at(j).setActFunc(function);
         nextGen.pop.at(i).at(j).setDepth(depth);
         parentCandidates.clear();
      }
   }
   //cout << "Parents found: " << parentsFound << endl;
   return nextGen;
}

pair<Organism, Organism> parentSelection(vector<Organism> parentCandidates, Organism father)
{
   return mother6FitProb(parentCandidates, father);
}

pair<Organism, Organism> mother6FitProb(vector<Organism> parentCandidates, Organism father)
{
   unsigned int i;
   int lowestFit, currentFit, total, condition, randomNum;
   bool parentFound;
   vector<int> fitnessProb;
   pair<Organism, Organism> parents;
   lowestFit = INT_MAX;
   total = 0;
   condition = 0;
   parentFound = false;
   for(i = 0; i < parentCandidates.size(); i++)
   {
      currentFit = parentCandidates.at(i).getFitness();
      if(currentFit < lowestFit)
      {
         lowestFit = currentFit;
      }
   }
   for(i = 0; i < parentCandidates.size(); i++)
   {
      currentFit = parentCandidates.at(i).getFitness();
      fitnessProb.push_back(currentFit - lowestFit);
      total += currentFit - lowestFit;
   }
   randomNum = randomInt(total - 1);
   //cout << "random " << randomNum << endl;
   for(i = 0; i < fitnessProb.size() && !parentFound; i++)
   {
      condition += fitnessProb.at(i);
      //cout << condition << endl;
      if(randomNum <= condition)
      {
         parentFound = true;
         //cout << "chose parent " << i << endl;
         parents.first = parentCandidates.at(i);
      }
   }
   parents.second = father;
   return parents;
}

pair<Organism, Organism> mother7FitProb(vector<Organism> parentCandidates, Organism father)
{
   unsigned int i;
   int lowestFit, currentFit, total, condition, randomNum;
   bool parentFound;
   vector<int> fitnessProb;
   pair<Organism, Organism> parents;
   lowestFit = INT_MAX;
   total = 0;
   condition = 0;
   parentFound = false;
   parentCandidates.push_back(father);
   for(i = 0; i < parentCandidates.size(); i++)
   {
      currentFit = parentCandidates.at(i).getFitness();
      if(currentFit < lowestFit)
      {
         lowestFit = currentFit;
      }
   }
   for(i = 0; i < parentCandidates.size(); i++)
   {
      currentFit = parentCandidates.at(i).getFitness();
      fitnessProb.push_back(currentFit - lowestFit);
      total += currentFit - lowestFit;
   }
   randomNum = randomInt(total - 1);
   //cout << "random " << randomNum << endl;
   for(i = 0; i < fitnessProb.size() && !parentFound; i++)
   {
      condition += fitnessProb.at(i);
      //cout << condition << endl;
      if(randomNum <= condition)
      {
         parentFound = true;
         //cout << "chose parent " << i << endl;
         parents.first = parentCandidates.at(i);
         parentCandidates.erase(parentCandidates.begin() + i);
      }
   }
   parents.second = father;
   return parents;
}

// change to choosing from 7 but can be the same

pair<Organism, Organism> both7FitProb(vector<Organism> parentCandidates, Organism father)
{
   unsigned int i;
   int lowestFit, currentFit, total, condition, randomNum;
   bool parentFound;
   vector<int> fitnessProb;
   pair<Organism, Organism> parents;
   lowestFit = INT_MAX;
   total = 0;
   condition = 0;
   parentFound = false;
   parentCandidates.push_back(father);
   for(i = 0; i < parentCandidates.size(); i++)
   {
      currentFit = parentCandidates.at(i).getFitness();
      if(currentFit < lowestFit)
      {
         lowestFit = currentFit;
      }
   }
   for(i = 0; i < parentCandidates.size(); i++)
   {
      currentFit = parentCandidates.at(i).getFitness();
      fitnessProb.push_back(currentFit - lowestFit);
      total += currentFit - lowestFit;
   }
   randomNum = randomInt(total - 1);
   //cout << "random " << randomNum << endl;
   for(i = 0; i < fitnessProb.size() && !parentFound; i++)
   {
      condition += fitnessProb.at(i);
      //cout << condition << endl;
      if(randomNum <= condition)
      {
         parentFound = true;
         //cout << "chose parent " << i << endl;
         parents.first = parentCandidates.at(i);
         parentCandidates.erase(parentCandidates.begin() + i);
      }
   }
   parentFound = false;
   fitnessProb.clear();
   lowestFit = INT_MAX;
   condition = 0;
   total = 0;
   for(i = 0; i < parentCandidates.size(); i++)
   {
      currentFit = parentCandidates.at(i).getFitness();
      if(currentFit < lowestFit)
      {
         lowestFit = currentFit;
      }
   }
   for(i = 0; i < parentCandidates.size(); i++)
   {
      currentFit = parentCandidates.at(i).getFitness();
      fitnessProb.push_back(currentFit - lowestFit);
      total += currentFit - lowestFit;
   }
   randomNum = randomInt(total - 1);
   //cout << "random " << randomNum << endl;
   for(i = 0; i < fitnessProb.size() && !parentFound; i++)
   {
      condition += fitnessProb.at(i);
      //cout << condition << endl;
      if(randomNum <= condition)
      {
         parentFound = true;
         //cout << "chose parent " << i << endl;
         parents.second = parentCandidates.at(i);
         //parentCandidates.erase(parentCandidates.begin() + i);
      }
   }
   return parents;

}

pair<Organism, Organism> bestMother6(vector<Organism> parentCandidates, Organism father)
{
   unsigned int i;
   int highestFit, currentFit, highestIndex;
   pair<Organism, Organism> parents;
   highestFit = -INT_MAX;
   highestIndex = -1;
   for(i = 0; i < parentCandidates.size(); i++)
   {
      currentFit = parentCandidates.at(i).getFitness();
      //cout << currentFit << endl;
      if(currentFit > highestFit)
      {
         highestFit = currentFit;
         highestIndex = i;
      }
   }
   //cout << "highest " << highestFit << " at " << highestIndex << endl;
   parents.first = parentCandidates.at(highestIndex);
   parents.second = father;
   return parents;
}

pair<Organism, Organism> bestMother7(vector<Organism> parentCandidates, Organism father)
{
   parentCandidates.push_back(father);
   unsigned int i;
   int highestFit, currentFit, highestIndex;
   pair<Organism, Organism> parents;
   highestFit = -INT_MAX;
   highestIndex = -1;
   for(i = 0; i < parentCandidates.size(); i++)
   {
      currentFit = parentCandidates.at(i).getFitness();
      if(currentFit > highestFit)
      {
         highestFit = currentFit;
         highestIndex = i;
      }
   }
   parents.first = parentCandidates.at(highestIndex);
   parents.second = father;
   return parents;
}

pair<Organism, Organism> bestBoth7(vector<Organism> parentCandidates, Organism father)
{
   parentCandidates.push_back(father);
   unsigned int i;
   int highestFit, currentFit, highestIndex;
   pair<Organism, Organism> parents;
   highestFit = -INT_MAX;
   highestIndex = -1;
   for(i = 0; i < parentCandidates.size(); i++)
   {
      currentFit = parentCandidates.at(i).getFitness();
      //cout << currentFit << endl;
      if(currentFit > highestFit)
      {
         highestFit = currentFit;
         highestIndex = i;
      }
   }
   parents.first = parentCandidates.at(highestIndex);
   //cout << "highest " << highestFit << " at " << highestIndex << endl;
   parentCandidates.erase(parentCandidates.begin() + highestIndex);
   highestFit = -INT_MAX;
   highestIndex = -1;
   for(i = 0; i < parentCandidates.size(); i++)
   {
      currentFit = parentCandidates.at(i).getFitness();
      //cout << currentFit << endl;
      if(currentFit > highestFit)
      {
         highestFit = currentFit;
         highestIndex = i;
      }
   }
   parents.second = parentCandidates.at(highestIndex);
   //cout << "highest " << highestFit << " at " << highestIndex << endl;
   return parents;
}
      
void Population::assignFitnesses()
{
   int i, j;
   int size, newFitness;
   size = this->pop.size();
   srandom(time(0));
   for(i = 0; i < size; i++)
   {
      for(j = 0; j < size; j++)
      {
         newFitness = randomInt(768);
         pop.at(i).at(j).updateFitness(newFitness);
      }
   }
}

Population& Population::operator=(const Population &rhs)
{
   this->pop.clear();
   this->pop = rhs.getPop();
   return *this;
}

void Population::printPopulation()
{
   //cout << "inside Print population" << endl;
   Population populus = *this;
   int size, i, j;
   size = populus.pop.size();
   NeuralNet network;
   cout << "size " << size << endl;
   for(i = 0; i < size; i++)
   {
      for(j = 0; j < size; j++)
      {
         //cout << "Organism " << i << " , " << j << " : " << endl;
         network = populus.pop.at(i).at(j).getNet();
         network.printWeights();
      }
   }
}

void Population::resetFitnesses()
{ 
   Population populus = *this;
   int i, j;
   int size = populus.pop.size();
   for(i = 0; i < size; i++)
   {
      for(j = 0; j < size; j++)
      {
         populus.pop.at(i).at(j).setFitness(0);
      }
   }
}

Population Population::runGenerations(int generations, vector<unsigned int> layerSizes, int size, int offset, string qualifier)
{
   int i;
   Population currentGen = *this;
   int depth = currentGen.pop.at(0).at(0).getDepth();
   ActFunc function = currentGen.pop.at(0).at(0).getActFunc();
   Population nextGen(layerSizes, size, function, depth);
   stringstream progress;
   progress << qualifier << "Progress.txt";
   ofstream prog;
   prog.open(progress.str());
   for(i = 0; i < generations; i++)
   {
      if(i % 500 == 0)
      {
         currentGen.savePopulation(i, offset, qualifier);
      }
      if(i % 10 == 0)
      {
         prog << "Generation " << i << endl;
      }
      currentGen.playNeighbors();
      nextGen = currentGen.createNextGen();
      currentGen = nextGen;
   }
   return currentGen;
   // save after about 100 generations;
}

void Population::printRep(string fileName)
{
   Population populus = *this;
   //int size = populus.pop.size();
   ofstream fout;
   fout.open(fileName.c_str());
   //cout << "getting rep" << endl;
   //int mid = size / 2;
   Organism rep = populus.pop.at(0).at(0);
   vector<vector<vector<double> > > testWeights;
   testWeights = rep.getNet().getWeights();
   fout.precision(5);
   unsigned int i, j, k;
   //cout << "printing weights" << endl;
   fout << testWeights.at(0).at(0).at(0) << endl;
   for(i = 0; i < testWeights.size(); i++)
   {
      for(j = 0; j < testWeights.at(i).size(); j++)
      {
         for (k = 1; k < testWeights.at(i).at(j).size(); k++)
         {
            if(k % 8 == 1)
            {
               fout << endl;
            }
            fout << fixed << setw(7) << testWeights.at(i).at(j).at(k) << " , ";
         }
         fout << endl;
      }
      fout << endl;
   }
}

void Population::savePopulation(int genNum, int offset, string qualifier)
{
   //cout << "Saving" << endl;
   //https://stackoverflow.com/questions/13108973/creating-file-names-automatically-c
   Population populus = *this;
   ActFunc currentFunc = populus.getOrganism(0,0).getActFunc();
   string fileString = "";
   genNum += offset;
   stringstream genNumString; 
   genNumString << "/";
   string functionName;
   if(currentFunc == Sigmoid)
   {
      genNumString << "Sigmoid";
      functionName = "Sigmoid";
   }
   else if(currentFunc == Softsign)
   {
      genNumString << "Softsign";
      functionName = "Softsign";
   }
   else if(currentFunc == Threshold)
   {
      genNumString << "Threshold";
      functionName = "Threshold";
   }
   else if(currentFunc == Softplus)
   {
      genNumString << "Softplus";
      functionName = "Softplus";
   }
   else if(currentFunc == Rectifier)
   {
      genNumString << "Rectifier";
      functionName = "Rectifier";
   }
   genNumString << "/" << "Gen_" << genNum;
   //genNumString.str();
   //cout << genNumString.str() << endl;
   fileString += "baseline" + genNumString.str() + qualifier + ".txt";
   
   ofstream fout; 
   // name of directory forward slash name of  file
   //cout << fileString.c_str() << endl;
   fout.open(fileString.c_str());
   int i, j, size;
   size = populus.pop.size();
   NeuralNet network;
   fout << "size" << endl << size << endl;
   fout << "ActFunc" << endl << functionName << endl;
   int depthLim = populus.getOrganism(0,0).getDepth();
   fout << "Depth" << endl << depthLim << endl;
   for(i = 0; i < size; i++)
   {
      for(j = 0;  j < size; j++)
      {
         network = populus.getOrganism(i, j).getNet();
         network.saveWeights(fout);
      }
   }
   fout.close();
}

// asked Michael for help on this function;
Population Population::loadPopulation(string fileName)
{
   ifstream input;
   stringstream strstr;
   strstr << fileName;
   string fileString;
   fileString += strstr.str();
   input.open(fileString.c_str());
   string token, weightString, numString;
   int lastDig, size;
   size = 0; // to keep compuler from warning me
   double weight;
   Population newPop;
   Organism placeholder;
   ActFunc currentFunction = Threshold;
   int depth = 0;
   vector<vector<Organism> > organismVecs;
   vector<Organism> tempOrganisms;
   vector<vector<vector<double> > > netWeights;
   vector<vector<double> > tempLayer;
   vector<double> tempNeuron;
   if(input.is_open())
   {
      while(getline(input, token))
      {
         if(token == "size")
         {
            getline(input, token);
            //cout << "Token is: " << token << endl;
            size = atof(token.c_str());
         }
         if(token == "ActFunc")
         {
            getline(input, token);
            if(token == "Sigmoid")
            {
               currentFunction = Sigmoid;
            }
            else if (token == "Rectifier")
            {
               currentFunction = Rectifier;
            }
            else if(token == "Softsign")
            {
               currentFunction = Softsign;
            }
            else if(token == "Softplus")
            {
               currentFunction = Softplus;
            }
            else if (token == "Threshold")
            {
               currentFunction = Threshold;
            }
         }
         if(token == "Depth")
         {
            getline(input, token);
            depth = atof(token.c_str());
         }
         if(token == "newNeuron")
         {
            tempNeuron.clear();
            getline(input, weightString);
            while(weightString.size() > 0)
            {
               lastDig = weightString.find(",");
               numString = weightString.substr(0, lastDig);
               weight = atof(numString.c_str());
               tempNeuron.push_back(weight);
               weightString.erase(0, lastDig + 1);
            }
            tempLayer.push_back(tempNeuron);
         }
         else if(token == "newLayer")
         {  
            if(tempLayer.size() == 0)
            {
               netWeights.clear();
               tempLayer.clear();
            }
            else
            {
               netWeights.push_back(tempLayer);
               tempLayer.clear();
            }
         }
         else if(token == "endOfOrganism")
         {
            netWeights.push_back(tempLayer);
            //cout << currentFunction << endl;
            placeholder = Organism(netWeights, currentFunction, depth);
            tempOrganisms.push_back(placeholder);
            netWeights.clear();
            tempLayer.clear();
         }
      }
   }
   vector<Organism> tempRow;
   for(int i = 0; i < size; i++)
   {
      for(int j = 0; j < size; j++)
      {
         tempRow.push_back(tempOrganisms.at(i * size + j));
      }
      organismVecs.push_back(tempRow);
      tempRow.clear();
   }  
   input.close();
   Population generatedPop = Population(organismVecs);
   return generatedPop;
}

string Population::popVSpop(Population teamB)
{
   Population teamA = *this;
   int sizeOfA, sizeOfB, rowA, colA, rowB, colB, i, j, temp;
   sizeOfA = teamA.pop.size();
   sizeOfB = teamB.pop.size();
   int winsAsWhiteA[sizeOfA * sizeOfA], winsAsWhiteB[sizeOfB * sizeOfB];
   int winsAsBlackA[sizeOfA * sizeOfA], winsAsBlackB[sizeOfB * sizeOfB];
   int lossesAsWhiteA[sizeOfA * sizeOfA], lossesAsWhiteB[sizeOfB * sizeOfB];
   int lossesAsBlackA[sizeOfA * sizeOfA], lossesAsBlackB[sizeOfB * sizeOfB];
   int tiesAsWhiteA[sizeOfA * sizeOfA], tiesAsWhiteB[sizeOfB * sizeOfB];
   int tiesAsBlackA[sizeOfA * sizeOfA], tiesAsBlackB[sizeOfB * sizeOfB];
   int piecesControlledA[sizeOfA * sizeOfA], piecesControlledB[sizeOfB * sizeOfB];
   int orderA[sizeOfA * sizeOfA], orderB[sizeOfB * sizeOfB];
   string agentStrA[sizeOfA * sizeOfA], agentStrB[sizeOfB * sizeOfB];
   //int fitnessA[sizeOfA * sizeOfA], fitnessB[sizeOfB * sizeOfB];
   pair <int, int> fitnessUpdate;
   int gamesPlayed = 0;
   for(rowA = 0; rowA < sizeOfA; rowA++)
   {
      for(colA = 0; colA < sizeOfA; colA++)
      {
         winsAsWhiteA[rowA * sizeOfA + colA] = 0;
         winsAsBlackA[rowA * sizeOfA + colA] = 0;
         lossesAsWhiteA[rowA * sizeOfA + colA] = 0;
         lossesAsBlackA[rowA * sizeOfA + colA] = 0;
         tiesAsWhiteA[rowA * sizeOfA + colA] = 0;
         tiesAsBlackA[rowA * sizeOfA + colA] = 0;
         piecesControlledA[rowA * sizeOfA + colA] = 0;
         stringstream strstr;
         strstr << rowA << "," << colA;
         agentStrA[rowA * sizeOfA + colA] = strstr.str();
         strstr.clear();
      }
   }
   for(rowB = 0; rowB < sizeOfB; rowB++)
   {
      for(colB = 0; colB < sizeOfB; colB++)
      {
         winsAsWhiteB[rowB * sizeOfB + colB] = 0;
         winsAsBlackB[rowB * sizeOfB + colB] = 0;
         lossesAsWhiteB[rowB * sizeOfB + colB] = 0;
         lossesAsBlackB[rowB * sizeOfB + colB] = 0;
         tiesAsWhiteB[rowB * sizeOfB + colB] = 0;
         tiesAsBlackB[rowB * sizeOfB + colB] = 0;
         piecesControlledB[rowB * sizeOfB + colB] = 0;
         stringstream strstr;
         strstr << rowB << " , " << colB;
         agentStrB[rowB * sizeOfB + colB] = strstr.str();
         strstr.clear();
      }
   }
   for(rowA = 0; rowA < sizeOfA; rowA++)
   {
      for(colA = 0; colA < sizeOfA; colA++)
      {
         for(rowB = 0; rowB < sizeOfB; rowB++)
         {
            for(colB = 0; colB < sizeOfB; colB++)
            {
               // black and then white
               fitnessUpdate = playGame(teamA.getOrganism(rowA, colA), teamB.getOrganism(rowB, colB));
               teamA.pop.at(rowA).at(colA).updateFitness(fitnessUpdate.first);
               teamB.pop.at(rowB).at(colB).updateFitness(fitnessUpdate.second);
               gamesPlayed++;
               if(fitnessUpdate.first > fitnessUpdate.second)
               {
                  winsAsBlackA[rowA * sizeOfA + colA] += 1;
                  lossesAsWhiteB[rowB * sizeOfB + colB] += 1;
               }
               else if (fitnessUpdate.second > fitnessUpdate.first)
               {
                  winsAsWhiteB[rowB * sizeOfB + colB] += 1;
                  lossesAsBlackA[rowA * sizeOfA + colA] += 1;
               }
               else
               {
                  tiesAsBlackA[rowA * sizeOfA + colA] += 1;
                  tiesAsWhiteB[rowB * sizeOfB + colB] += 1;
               }
               fitnessUpdate = playGame(teamB.getOrganism(rowB, colB), teamB.getOrganism(rowA, colA));
               teamB.pop.at(rowB).at(colB).updateFitness(fitnessUpdate.first);
               teamA.pop.at(rowA).at(colA).updateFitness(fitnessUpdate.second);
               gamesPlayed++;
               if(fitnessUpdate.first > fitnessUpdate.second)
               {
                  winsAsBlackB[rowB * sizeOfB + colB] += 1;
                  lossesAsWhiteA[rowA * sizeOfA + colA] += 1;
               }
               else if (fitnessUpdate.second > fitnessUpdate.first)
               {
                  winsAsWhiteA[rowA * sizeOfA + colA] += 1;
                  lossesAsBlackB[rowB * sizeOfB + colB] += 1;
               }
               else
               {
                  tiesAsBlackB[rowB * sizeOfB + colB] += 1;
                  tiesAsWhiteA[rowA * sizeOfA + colA] += 1;
               }
            }
         }
      }
   }
   int totalFit;
   int pieces;
   // CHANGE IF WE CHANGE TO 6X6
   for(rowA = 0; rowA < sizeOfA; rowA++)
   {
      for(colA = 0; colA < sizeOfA; colA++)
      {
         totalFit = teamA.pop.at(rowA).at(colA).getFitness();
         //fitnessA[rowA * sizeOfA + colA] = totalFit;
         pieces = totalFit - (winsAsBlackA[rowA * sizeOfA + colA] + winsAsWhiteA[rowA * sizeOfA + colA]) * 64 - (tiesAsBlackA[rowA * sizeOfA + colA] + tiesAsWhiteA[rowA * sizeOfA + colA]) * 32;
         piecesControlledA[rowA * sizeOfA + colA] = pieces;
      }
   }
   for(rowB = 0; rowB < sizeOfB; rowB++)
   {
      for(colB = 0; colB < sizeOfB; colB++)
      {
         totalFit = teamB.pop.at(rowB).at(colB).getFitness();
         //fitnessB[rowB * sizeOfB + colB] = totalFit;
         pieces = totalFit - (winsAsBlackB[rowB * sizeOfB + colB] + winsAsWhiteB[rowB * sizeOfB + colB]) * 64 - (tiesAsBlackB[rowB * sizeOfB + colB] + tiesAsWhiteB[rowB * sizeOfB + colB]) * 32;
         piecesControlledB[rowB * sizeOfB + colB] = pieces;
      }
   }
   for (i = 0; i < sizeOfA * sizeOfA; i += 1)
   {
      orderA[i] = i;
   }
   for(i = 0; i < sizeOfB * sizeOfB; i+= 1)
   {
      orderB[i] = i;
   }
   for (i = 0; i < sizeOfA * sizeOfA - 1; i += 1)
   {
      for (j = i + 1; j < sizeOfA * sizeOfA; j += 1)
      {
      if (winsAsWhiteA[orderA[i]] + winsAsBlackA[orderA[i]] < winsAsWhiteA[orderA[j]] + winsAsBlackA[orderA[j]] || (winsAsWhiteA[orderA[i]] + winsAsBlackA[orderA[i]] == winsAsWhiteA[orderA[j]] + winsAsBlackA[orderA[j]] && piecesControlledA[orderA[i]] > piecesControlledA[orderA[j]]) || (piecesControlledA[orderA[i]] == piecesControlledA[orderA[j]]  && agentStrA[orderA[i]] >= agentStrA[orderA[j]])) // sort the agents from best to worst
         {
            temp = orderA[i];
            orderA[i] = orderA[j];
            orderA[j] = temp;
         }
      }
   }
   for (i = 0; i < sizeOfB * sizeOfB - 1; i += 1)
   {
      for (j = i + 1; j < sizeOfB * sizeOfB; j += 1)
      {
         if (winsAsWhiteB[orderB[i]] + winsAsBlackB[orderB[i]] < winsAsWhiteB[orderB[j]] + winsAsBlackB[orderB[j]] || (winsAsWhiteB[orderB[i]] + winsAsBlackB[orderB[i]] == winsAsWhiteB[orderB[j]] + winsAsBlackB[orderB[j]] && piecesControlledB[orderB[i]] > piecesControlledB[orderB[j]]) || (piecesControlledB[orderB[i]] == piecesControlledB[orderB[j]]  && agentStrB[orderB[i]] >= agentStrB[orderB[j]])) // sort the agents from best to worst
         {
            temp = orderB[i];
            orderB[i] = orderB[j];
            orderB[j] = temp;
         }
      }
   }
   cout << "\n" << "Team A: " << "                                                                                                 TeamB: \n"
        << "Overall:           all             as Black          as White     " 
        << "    Overall:             all             as Black          as White     \n"
        << "                W    L    T       W    L    T       W    L    T  "
        << "                      W    L    T       W    L    T       W    L    T  \n";
   for (i = 0; i < sizeOfA * sizeOfA; i += 1) 
   {
      /*double ratioA, ratioB;
      if (lossesAsBlackA[orderA[i]] + lossesAsWhiteA[orderA[i]] == 0) 
      {
         ratioA = 1;
      }
      else 
      {
         ratioA = (static_cast<double>(winsAsWhiteA[orderA[i]]) + static_cast<double>(winsAsBlackA[orderA[i]]))/(static_cast<double>((lossesAsWhiteA[orderA[i]]) + static_cast<double>(lossesAsBlackA[orderA[i]])));
      }
      if(lossesAsBlackB[orderB[i]] + lossesAsWhiteB[orderB[i]] == 0) 
      {
         ratioB = 1;
      }
      else 
      {
         ratioB = (static_cast<double>(winsAsWhiteB[orderB[i]]) + static_cast<double>(winsAsBlackB[orderB[i]]))/(static_cast<double>((lossesAsWhiteB[orderB[i]]) + static_cast<double>(lossesAsBlackB[orderB[i]])));
      }*/
      cout << setw(12) << left << agentStrA[orderA[i]]
           << " " << setw(4) << right << winsAsWhiteA[orderA[i]] + winsAsBlackA[orderA[i]]
           //totalWinsA += winsAsWhiteA[orderA[i]] + winsAsBlackA[orderA[i]];
           << " " << setw(4) << right << lossesAsWhiteA[orderA[i]] + lossesAsBlackA[orderA[i]]
            //totalLossesA += lossesAsWhiteA[orderA[i]] + lossesAsBlackA[orderA[i]];
           << " " << setw(4) << right << tiesAsWhiteA[orderA[i]] + tiesAsBlackA[orderA[i]]
           << " " << setw(7) << right << winsAsBlackA[orderA[i]]
           << " " << setw(4) << right << lossesAsBlackA[orderA[i]]
           << " " << setw(4) << right << tiesAsBlackA[orderA[i]]
           << " " << setw(7) << right << winsAsWhiteA[orderA[i]]
           << " " << setw(4) << right << lossesAsWhiteA[orderA[i]]
           << " " << setw(4) << right << tiesAsWhiteA[orderA[i]]
           //<< " " << setw(8) << right << piecesControlledA[orderA[i]]
           //<< " " << setw(8) << right << numInvalid[orderA[i]] 
           //<< " " << setprecision(5) << setw(11) << right << ratioA
          // << " " << setw(9) << right << fitnessA[orderA[i]]
           << "       "
           << setw(12) << left << agentStrB[orderB[i]]
           << " " << setw(4) << right << winsAsWhiteB[orderB[i]] + winsAsBlackB[orderB[i]]
           << " " << setw(4) << right << lossesAsWhiteB[orderB[i]] + lossesAsBlackB[orderB[i]]
           << " " << setw(4) << right << tiesAsWhiteB[orderB[i]] + tiesAsBlackB[orderB[i]]
           << " " << setw(7) << right << winsAsBlackB[orderB[i]]
           << " " << setw(4) << right << lossesAsBlackB[orderB[i]]
           << " " << setw(4) << right << tiesAsBlackB[orderB[i]]
           << " " << setw(7) << right << winsAsWhiteB[orderB[i]]
           << " " << setw(4) << right << lossesAsWhiteB[orderB[i]]
           << " " << setw(4) << right << tiesAsWhiteB[orderB[i]] << "\n";
           //<< " " << setw(8) << right << piecesControlledB[orderB[i]]
           //<< " " << setw(8) << right << numInvalid[orderA[i]] 
           //<< " " << setprecision(5) << setw(11) << right << ratioB 
           //<< " " << setw(9) << right << fitnessB[orderB[i]] << "\n";
   }
   /*
   cout << "\n" << "Team B: " << "\n"
        << "Overall standings:       all             as Black          as White      Total #     W / L     Fitness\n"
        << "                      W    L    T       W    L    T       W    L    T    pieces      Ratio      Score\n";
  
   for (i = 0; i < sizeOfB * sizeOfB; i += 1)
   {
      double ratio; 
      if(lossesAsBlackB[orderB[i]] + lossesAsWhiteB[orderB[i]] == 0) 
      {
         ratio = 1;
      }
      else 
      {
         ratio = (static_cast<double>(winsAsWhiteB[orderB[i]]) + static_cast<double>(winsAsBlackB[orderB[i]]))/(static_cast<double>((lossesAsWhiteB[orderB[i]]) + static_cast<double>(lossesAsBlackB[orderB[i]])));
      }
      cout << setw(15) << left << agentStrB[orderB[i]]
           << " " << setw(4) << right << winsAsWhiteB[orderB[i]] + winsAsBlackB[orderB[i]]
           << " " << setw(4) << right << lossesAsWhiteB[orderB[i]] + lossesAsBlackB[orderB[i]]
           << " " << setw(4) << right << tiesAsWhiteB[orderB[i]] + tiesAsBlackB[orderB[i]]
           << " " << setw(7) << right << winsAsBlackB[orderB[i]]
           << " " << setw(4) << right << lossesAsBlackB[orderB[i]]
           << " " << setw(4) << right << tiesAsBlackB[orderB[i]]
           << " " << setw(7) << right << winsAsWhiteB[orderB[i]]
           << " " << setw(4) << right << lossesAsWhiteB[orderB[i]]
           << " " << setw(4) << right << tiesAsWhiteB[orderB[i]]
           << " " << setw(8) << right << piecesControlledB[orderB[i]]
           //<< " " << setw(8) << right << numInvalid[orderA[i]] 
           << " " << setprecision(5) << setw(11) << right << ratio 
           << " " << setw(9) << right << fitnessB[orderB[i]] << "\n";
   }*/
   cout << endl;
   int totalWinsA, totalLossesA, totalWinsB, totalLossesB, totalTies;
   totalWinsA = 0;
   totalLossesA = 0;
   totalWinsB = 0;
   totalLossesB = 0;
   totalTies = 0;
   stringstream returnstring;
   for(i = 0; i < sizeOfA * sizeOfA; i += 1)
   {
      totalWinsA += winsAsBlackA[orderA[i]] + winsAsWhiteA[orderA[i]];
      totalLossesA += lossesAsBlackA[orderA[i]] + lossesAsWhiteA[orderA[i]];
      totalTies += tiesAsBlackA[orderA[i]] + tiesAsWhiteA[orderA[i]];
   }
   for(i = 0; i < sizeOfB * sizeOfB; i += 1)
   {
      totalWinsB += winsAsBlackB[orderA[i]] + winsAsWhiteB[orderA[i]];
      totalLossesB += lossesAsBlackB[orderA[i]] + lossesAsWhiteB[orderA[i]];
   }
   wins.push_back(totalWinsB);
   ties.push_back(totalTies);
   returnstring << endl << "Team A Total Wins: " << totalWinsA << endl;// << "Team A Total Losses: " << totalLossesA << endl;
   returnstring << "Team B Total Wins: " << totalWinsB << endl;// << "Team B Total Losses: " << totalLossesB << endl << endl;
   returnstring << "Ties: " << totalTies << endl;
   cout << endl << "Team A Total Wins: " << totalWinsA << endl;// << "Team A Total Losses: " << totalLossesA << endl;
   cout << "Team B Total Wins: " << totalWinsB << endl;// << "Team B Total Losses: " << totalLossesB << endl << endl;
   cout << "Ties: " << totalTies << endl;
   return returnstring.str();
}
string Population::popVSpopTeamA(Population teamB)
{
   Population teamA = *this;
   int sizeOfA, sizeOfB, rowA, colA, rowB, colB, i, j, temp;
   sizeOfA = teamA.pop.size();
   sizeOfB = teamB.pop.size();
   int winsAsWhiteA[sizeOfA * sizeOfA];
   int winsAsBlackA[sizeOfA * sizeOfA];
   int lossesAsWhiteA[sizeOfA * sizeOfA];
   int lossesAsBlackA[sizeOfA * sizeOfA];
   int tiesAsWhiteA[sizeOfA * sizeOfA];
   int tiesAsBlackA[sizeOfA * sizeOfA];
   int piecesControlledA[sizeOfA * sizeOfA];
   int orderA[sizeOfA * sizeOfA];
   string agentStrA[sizeOfA * sizeOfA];
   pair <int, int> fitnessUpdate;
   for(rowA = 0; rowA < sizeOfA; rowA++)
   {
      for(colA = 0; colA < sizeOfA; colA++)
      {
         winsAsWhiteA[rowA * sizeOfA + colA] = 0;
         winsAsBlackA[rowA * sizeOfA + colA] = 0;
         lossesAsWhiteA[rowA * sizeOfA + colA] = 0;
         lossesAsBlackA[rowA * sizeOfA + colA] = 0;
         tiesAsWhiteA[rowA * sizeOfA + colA] = 0;
         tiesAsBlackA[rowA * sizeOfA + colA] = 0;
         piecesControlledA[rowA * sizeOfA + colA] = 0;
         stringstream strstr;
         strstr << rowA << "," << colA;
         agentStrA[rowA * sizeOfA + colA] = strstr.str();
         strstr.clear();
      }
   }

   //cout << "SizeA" << sizeOfA << endl;
   //cout << "SizeB" << sizeOfB << endl;
   for(rowA = 0; rowA < sizeOfA; rowA++)
   {
      for(colA = 0; colA < sizeOfA; colA++)
      {
         for(rowB = 0; rowB < sizeOfB; rowB++)
         {
            for(colB = 0; colB < sizeOfB; colB++)
            {
               // black and then white
               fitnessUpdate = playGame(teamA.getOrganism(rowA, colA), teamB.getOrganism(rowB, colB));
               //cout << "playing Game" << endl;
               teamA.pop.at(rowA).at(colA).updateFitness(fitnessUpdate.first);
               if(fitnessUpdate.first > fitnessUpdate.second)
               {
                  winsAsBlackA[rowA * sizeOfA + colA] += 1;
               }
               else if (fitnessUpdate.second > fitnessUpdate.first)
               {
                  lossesAsBlackA[rowA * sizeOfA + colA] += 1;
               }
               else
               {
                  tiesAsBlackA[rowA * sizeOfA + colA] += 1;
               }
               fitnessUpdate = playGame(teamB.getOrganism(rowB, colB), teamB.getOrganism(rowA, colA));
               teamA.pop.at(rowA).at(colA).updateFitness(fitnessUpdate.second);
               if(fitnessUpdate.first > fitnessUpdate.second)
               {
                  lossesAsWhiteA[rowA * sizeOfA + colA] += 1;
               }
               else if (fitnessUpdate.second > fitnessUpdate.first)
               {
                  winsAsWhiteA[rowA * sizeOfA + colA] += 1;
               }
               else
               {
                  tiesAsWhiteA[rowA * sizeOfA + colA] += 1;
               }
            }
         }
      }
   }
   for (i = 0; i < sizeOfA * sizeOfA; i += 1)
   {
      orderA[i] = i;
   }
   for (i = 0; i < sizeOfA * sizeOfA - 1; i += 1)
   {
      for (j = i + 1; j < sizeOfA * sizeOfA; j += 1)
      {
      if  (winsAsWhiteA[orderA[i]] + winsAsBlackA[orderA[i]] < winsAsWhiteA[orderA[j]] + winsAsBlackA[orderA[j]] || (winsAsWhiteA[orderA[i]] + winsAsBlackA[orderA[i]] == winsAsWhiteA[orderA[j]] + winsAsBlackA[orderA[j]] && piecesControlledA[orderA[i]] > piecesControlledA[orderA[j]]) || (piecesControlledA[orderA[i]] == piecesControlledA[orderA[j]]  && agentStrA[orderA[i]] >= agentStrA[orderA[j]])) // sort the agents from best to worst
         {
            temp = orderA[i];
            orderA[i] = orderA[j];
            orderA[j] = temp;
         }
      }
   }

   cout << "\n" << "Team A: \n"
        << "Overall:           all             as Black          as White     \n" 
        << "                W    L    T       W    L    T       W    L    T  \n";
   cout << "size: " << sizeOfA * sizeOfA << endl;
   int num = 0;
   for (i = 0; i < sizeOfA * sizeOfA; i += 1)
   {
      
      cout << setw(12) << left << agentStrA[orderA[i]]
           << " " << setw(4) << right << winsAsWhiteA[orderA[i]] + winsAsBlackA[orderA[i]]
           << " " << setw(4) << right << lossesAsWhiteA[orderA[i]] + lossesAsBlackA[orderA[i]]
           << " " << setw(4) << right << tiesAsWhiteA[orderA[i]] + tiesAsBlackA[orderA[i]]
           << " " << setw(7) << right << winsAsBlackA[orderA[i]]
           << " " << setw(4) << right << lossesAsBlackA[orderA[i]]
           << " " << setw(4) << right << tiesAsBlackA[orderA[i]]
           << " " << setw(7) << right << winsAsWhiteA[orderA[i]]
           << " " << setw(4) << right << lossesAsWhiteA[orderA[i]]
           << " " << setw(4) << right << tiesAsWhiteA[orderA[i]] << endl;
           num++;
   }
   cout << num << endl;
   cout << endl;
   int totalWinsA, totalLossesA, totalTies;
   totalWinsA = 0;
   totalLossesA = 0;
   totalTies = 0;
   stringstream returnstring;
   for(i = 0; i < sizeOfA * sizeOfA; i += 1)
   {
      totalWinsA += winsAsBlackA[orderA[i]] + winsAsWhiteA[orderA[i]];
      totalLossesA += lossesAsBlackA[orderA[i]] + lossesAsWhiteA[orderA[i]];
      totalTies += tiesAsBlackA[orderA[i]] + tiesAsWhiteA[orderA[i]];
   }
   wins.push_back(totalWinsA);
   ties.push_back(totalTies);
   returnstring << endl << "Team A Total Wins: " << totalWinsA << endl;// << "Team A Total Losses: " << totalLossesA << endl;
   returnstring << "Ties: " << totalTies << endl;
   cout << endl << "Team A Total Wins: " << totalWinsA << endl;// << "Team A Total Losses: " << totalLossesA << endl;
   cout << "Ties: " << totalTies << endl;
   return returnstring.str();
}

void Population::printFitnesses()
{
   Population populus = *this;
   int size, i, j;
   size = populus.pop.size();
   vector<vector<int> > fitnessVec;
   fitnessVec = populus.getAllFitnesses();
   for(i = 0; i < size; i++)
   {
      for(j = 0; j < size; j++)
      {
         cout << fitnessVec.at(i).at(j) << " + ";
      }
      cout << endl;
   }
}

// maybe run cauchy shake with smaller deviations
