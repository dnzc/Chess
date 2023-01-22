#include "Engine.h"
#include "Move.h"
#include "Util.h"
#include <math.h>
#include <random>
#include <vector>
#include <iostream>
#include <string>
#include <chrono>

MCTSNode::MCTSNode(Position& pos, Move move) : pos(pos), move(move) {
  score = 0;
  playouts = 0;
  parent = nullptr;
}

Engine::Engine() {
  m_root = std::shared_ptr<MCTSNode>(new MCTSNode(m_pos, Move(-1, -1, empty, false, false, false))); // dummy move
}

Engine::Engine(std::string FEN) {
  Position p(FEN);
  m_pos = p;
  m_root = std::shared_ptr<MCTSNode>(new MCTSNode(m_pos, Move(-1, -1, empty, false, false, false))); // dummy move
}
Position Engine::getPos() {
  return m_pos;
}

std::vector<Move> Engine::getLegalMoves() {
  return m_gen.genMoves(m_pos, false);
}

void Engine::makeMove(Move move) {
  m_pos.makeMove(move);
  m_prevMoves.push_back(move);
  m_root = std::shared_ptr<MCTSNode>(new MCTSNode(m_pos, move));
}

// GROUP A SKILL: complex user-defined algorithms
void Engine::doOneMonteCarloStep(bool alphaBeta, std::chrono::time_point<std::chrono::steady_clock> startTime_ms) {

  // GROUP A SKILL - tree traversal
  // SELECTION
  std::shared_ptr<MCTSNode> curNode = m_root;
  while(curNode->children.size()>0) {
    // choose the child node with the largest value of w_i/n_i + sqrt(c*ln(n_{i-1})/n_i)
    // where c is an adjustable constant to control the exploitation / exploration ratio
    // note: 0.000001 is added to n_i since dividing by 0 is undefined
    std::shared_ptr<MCTSNode> nextNode;
    double maxVal = -1;
    for(auto child : curNode->children) {
      double val = child->score / (child->playouts+0.000001) + sqrt(2*log2(curNode->playouts) / (child->playouts+0.000001) );
      if(val>maxVal) {
        maxVal = val;
        nextNode = child;
      }
    }
    curNode = nextNode;
  }

  // EXPANSION
  // create new nodes, but only if the current one has at least one playout
  if(curNode->playouts > 0) {
    std::vector<Move> legalMoves = m_gen.genMoves(curNode->pos, false);
    if(legalMoves.size()>0) {
      for(Move move : legalMoves) {
        Position nextPos = curNode->pos;
        nextPos.makeMove(move);
        std::shared_ptr<MCTSNode> newNode = std::shared_ptr<MCTSNode>(new MCTSNode(nextPos, move));
        newNode->parent = curNode;
        curNode->children.push_back(newNode);
      }
      // pick random child
      curNode = curNode->children[std::rand() % curNode->children.size()];
    }
  }

  // SIMULATION
  Position p = curNode->pos;
  // 1 for loss, 0.5 for draw, 0 for win (from this position)
  // since e.g. if current position is checkmate, then result is 1 because the previous node wants to go to this node
  double result;

  if(alphaBeta) {
    double eval = minimaxAB(p, startTime_ms, m_inf, 2, -m_inf, m_inf); 
    result = 0.5 + 0.5*tanh(-0.15*eval); // positive eval means result should be closer to 0
  } else result = playout(p);
  

  // BACKPROPAGATION
  // travel back up the tree, updating the information
  while(true) {
    curNode->score += result;
    curNode->playouts++;
    // flip the result because the player flips between black and white
    result = 1-result;
    curNode = curNode->parent;
    if(curNode==nullptr) break; 
  }

}

double Engine::playout(Position& p) {
  while(true) {
    std::vector<Move> legalMoves = m_gen.genMoves(p, false);

    // terminal conditions
    if(legalMoves.size()==0)
      return m_gen.getCheckingPieces(p).getBits()==0 ? 0.5 : 1; // if no legal moves, then stalemate if not being checked, else loss
    if(p.getPlysSince50()>50) // if the 50 move rule has been exceeded, it is probably a draw, so evaluate the playout as a draw to save time
      return 0.5;
    if(p.getWhiteOccupancy().popcnt()==1) { // if white only has a king left
      bool isWhite = p.isWhiteToMove();
      if(p.getPieces(br).popcnt()) return isWhite ? 1 : 0; // rook endgame
      if(p.getPieces(bq).popcnt()) return isWhite ? 1 : 0; // queen endgame
      if(p.getPieces(bb).popcnt() > 1) return isWhite ? 1 : 0; // two bishops endgame
      if(p.getPieces(bb).popcnt() && p.getPieces(bn).popcnt()) return isWhite ? 1 : 0; // bishop+knight endgame
      if(p.getPieces(bb).popcnt() && p.getPieces(bn).popcnt()) return isWhite ? 1 : 0; // two knights endgame
      Bitboard occ = p.getBlackOccupancy();
      if(occ.popcnt()==1) return 0.5; // only kings left
      else if(occ.popcnt()==2 && (p.getPieces(bn)|p.getPieces(bb)).popcnt() == 1) return 0.5; // black only has one bishop, or knight
    }
    // same as above, for black
    if(p.getBlackOccupancy().popcnt()==1) {
      bool isBlack = !p.isWhiteToMove();
      if(p.getPieces(wr).popcnt()) return isBlack ? 1 : 0; // rook endgame
      if(p.getPieces(wq).popcnt()) return isBlack ? 1 : 0; // queen endgame
      if(p.getPieces(wb).popcnt() && p.getPieces(wn).popcnt()) return isBlack ? 1 : 0; // bishop+knight endgame
      Bitboard occ = p.getWhiteOccupancy();
      // note: only kings case has been handled above
      if(occ.popcnt()==2 && (p.getPieces(wn)|p.getPieces(wb)).popcnt() == 1) return 0.5;
    }

    // play a random legal move
    p.makeMove(legalMoves[std::rand() % legalMoves.size()]);
  }
}

int getTimeElapsed(std::chrono::time_point<std::chrono::steady_clock> begin) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count();
}

// alpha beta minimax
double Engine::minimaxAB(Position& p, std::chrono::time_point<std::chrono::steady_clock> startTime_ms, int timeLimit_ms, int depth, double alpha, double beta) {
  std::vector<Move> legalMoves = m_gen.genMoves(p, false);
  if(legalMoves.size() == 0) {
    return m_gen.getCheckingPieces(p).getBits()==0 ? 0 : -m_inf;
  }

  if(depth == 0) return capturesAB(p, startTime_ms, timeLimit_ms, alpha, beta);

  order(p, legalMoves);

  for(Move move : legalMoves) {
    Position newPos = p;
    newPos.makeMove(move);
    double evaluation = -minimaxAB(newPos, startTime_ms, timeLimit_ms, depth-1, -beta, -alpha);
    if(evaluation >= beta) return beta+0.1; // add 0.1 so that the move which actually achieved beta is preferred
                                            // (otherwise, if a move falls into mate in one then the engine might still play it as it will be evaluated as 0 if there exists another move that is evaluated at 0 instead of -inf)
    if(evaluation > alpha) alpha = evaluation;
    if(getTimeElapsed(startTime_ms) >= timeLimit_ms) return 0;
  }

  return alpha;
}

double Engine::capturesAB(Position& p, std::chrono::time_point<std::chrono::steady_clock> startTime_ms, int timeLimit_ms, double alpha, double beta) {
  // captures aren't forced, so check the eval before making a capture
  // otherwise, if only bad captures are available then this will evaluate the position as bad, even if other good moves exist
  double evaluation = eval(p);
  if(evaluation >= beta) return beta;
  if(evaluation > alpha) alpha = evaluation;

  std::vector<Move> captureMoves = m_gen.genMoves(p, true);
  order(p, captureMoves);

  for(Move move : captureMoves) {
    Position newPos = p;
    newPos.makeMove(move);
    double evaluation = -capturesAB(newPos, startTime_ms, timeLimit_ms, -beta, -alpha);
    if(evaluation >= beta) return beta;
    if(evaluation > alpha) alpha = evaluation;
    if(getTimeElapsed(startTime_ms) >= timeLimit_ms) return 0;
  }

  return alpha;

}

void Engine::order(Position& p, std::vector<Move>& moves) {
  std::sort(moves.begin(), moves.end(), [&](const Move& m1, const Move& m2) -> bool {
    int score1 = 0;
    PieceType capturedPiece1 = p.whichPiece(m1.end);
    // reward capturing valuable pieces with less valuable ones
    if(capturedPiece1 != empty) score1 += 10 * m_pieceValues[capturedPiece1] - m_pieceValues[m1.piece];
    // pawn promotions are probably good
    if(m1.promotion) score1 += m_pieceValues[m1.promotion];

    // same for second move
    int score2 = 0;
    PieceType capturedPiece2 = p.whichPiece(m2.end);
    if(capturedPiece2 != empty) score2 += 10 * m_pieceValues[capturedPiece2] - m_pieceValues[m2.piece];
    if(m2.promotion) score2 += m_pieceValues[m2.promotion];

    return score1 > score2;
  });
}

// positive if current player is winning, negative otherwise
double Engine::eval(Position& p) {
  double evaluation = 0;
  bool isWhite = p.isWhiteToMove();
  // material count
  int whitePawns = p.getPieces(wp).popcnt() * m_pieceValues[wp];
  int whiteOther = p.getPieces(wn).popcnt() * m_pieceValues[wn]
                 + p.getPieces(wb).popcnt() * m_pieceValues[wb]
                 + p.getPieces(wr).popcnt() * m_pieceValues[wr]
                 + p.getPieces(wq).popcnt() * m_pieceValues[wq];
  int blackPawns = p.getPieces(bp).popcnt() * m_pieceValues[bp];
  int blackOther = p.getPieces(bp).popcnt() * m_pieceValues[bp]
                 + p.getPieces(bn).popcnt() * m_pieceValues[bn]
                 + p.getPieces(bb).popcnt() * m_pieceValues[bb]
                 + p.getPieces(br).popcnt() * m_pieceValues[br]
                 + p.getPieces(bq).popcnt() * m_pieceValues[bq];
  evaluation = blackPawns + blackOther - whitePawns - whiteOther;
  if(isWhite) evaluation *= -1;

  // count non-pawn enemy pieces to determine whether endgame
  double endgameWeight = 1 - (double)(isWhite ? blackOther : whiteOther)/29; // initially there are 29 points worth of non-pawn material

  // in endgames, reward positions where the enemy king is at the board edge
  int enemyKing = p.getPieces(isWhite ? bk : wk).getLsb();
  int enemyRank = enemyKing>>3;
  int enemyFile = enemyKing&7;
  double distFromCentre = m_centreDist[enemyRank] + m_centreDist[enemyFile];
  evaluation += 0.2 * distFromCentre * endgameWeight;

  // in endgames, reward positions where our king is close to enemy king
  int ourKing = p.getPieces(isWhite ? wk : bk).getLsb();
  int ourRank = ourKing>>3;
  int ourFile = ourKing&7;
  double distBetween = abs(ourRank - enemyRank) + abs(ourFile - enemyFile);
  evaluation += 0.1 * (14-distBetween) * endgameWeight;

  return evaluation;
}

Move Engine::MCTS(int timeLimit_ms, bool alphaBeta, bool verbose) {
  auto begin = std::chrono::steady_clock::now();
  while(getTimeElapsed(begin) < timeLimit_ms) {
    doOneMonteCarloStep(alphaBeta, begin);
  }
  // return the move with the most number of playouts
  if(m_root->children.size()==0) return Move(-1, -1, empty, false, false, false); // dummy move
  std::sort(m_root->children.begin(), m_root->children.end(), [](const std::shared_ptr<MCTSNode> a, const std::shared_ptr<MCTSNode> b) -> bool {return a->playouts > b->playouts;});
  if(verbose) {
    std::cout << "Monte Carlo win rates for each move: (format: score/playouts)\n";
    for(auto child : m_root->children) {
      std::cout << "  " << (char)((child->move.start&7)+'a') << (child->move.start>>3)+1
        << (char)((child->move.end&7)+'a') << (child->move.end>>3)+1
        << ": " << child->score << "/" << child->playouts << "\n";
    }
  }
  return m_root->children[0]->move;

}

Move Engine::minimax(int timeLimit_ms, bool verbose) {
  auto begin = std::chrono::steady_clock::now();
  std::vector<Move> legalMoves =  m_gen.genMoves(m_pos, false);
  if(legalMoves.size()==0) return Move(-1, -1, empty, false, false, false); // dummy move
  std::vector< std::pair<Move,double> > moves; // move, eval
  std::vector< std::pair<Move,double> > lastCompletedSearch;
  moves.clear();
  for(Move move : legalMoves) {
    Move m = move;
    moves.push_back(std::make_pair(m, 0));
  }

  // iterative deepening
  int curDepth = 0;
  while(true) {
    double alpha = -m_inf;
    bool timeLimitReached = false;
    for(int i=0; i<moves.size(); ++i) {
      Position p = m_pos;
      p.makeMove(moves[i].first);
      double eval = -minimaxAB(p, begin, timeLimit_ms, curDepth, -m_inf, -alpha);
      if(getTimeElapsed(begin) >= timeLimit_ms) {
        timeLimitReached = true;
        break;
      }
      moves[i].second = eval;
      if(eval > alpha) {
        alpha = eval;
      }
    }
    if(timeLimitReached) break;
    // sort the legal moves by the evals, for the next iterative deepening
    std::sort(moves.begin(), moves.end(), [](const std::pair<Move,double> a, const std::pair<Move,double> b) -> bool {
      return a.second > b.second;
    });
    // search was completed at this depth, update the last completed search
    lastCompletedSearch.clear();
    for(auto i : moves) {
      Move m = i.first;
      lastCompletedSearch.push_back(std::make_pair(m, i.second));
    }

    curDepth++;
  }

  if(verbose) {
    Move m = lastCompletedSearch[0].first;
    double bestEval = lastCompletedSearch[0].second;
    std::cout << "Depth " << curDepth << " minimax best move: " << (char)((m.start&7)+'a') << (m.start>>3)+1
      << (char)((m.end&7)+'a') << (m.end>>3)+1
      << " (eval " << bestEval << ")\n";
  }

  return lastCompletedSearch[0].first;
}

int Engine::isGameOver() { // 0 if no, 1 if draw, 2 if checkmate
    if(m_gen.genMoves(m_pos, false).size()==0)
      return m_gen.getCheckingPieces(m_pos).getBits()==0 ? 1 : 2; 
    return 0;
}
