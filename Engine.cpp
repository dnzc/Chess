#include "Engine.h"
#include "Move.h"
#include "Util.h"
#include <math.h>
#include <random>
#include <vector>
#include <iostream>
#include <string>

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
  return m_gen.genMoves(m_pos);
}

void Engine::makeMove(Move move) {
  m_pos.makeMove(move);
  m_prevMoves.push_back(move);
  // GROUP A SKILL - tree
  // prune the game tree: loop through children, deleting moves that weren't played
  bool moveExistsInTree = false;
  std::shared_ptr<MCTSNode> nextRoot;
  for(auto child : m_root->children) {
    if(child->move.start == move.start && child->move.end == move.end && child->move.promotion == move.promotion) {
      moveExistsInTree = true;
      nextRoot = child;
      break;
    }
  }
  if(moveExistsInTree) {
    m_root = nextRoot;
    m_root->parent = nullptr;
  }
  else
    m_root = std::shared_ptr<MCTSNode>(new MCTSNode(m_pos, move));
}

// GROUP A SKILL: complex user-defined algorithms
void Engine::doOneStep() {

  // TODO: don't do MCTS if known endgame
  // TODO: say "checkmate" instead of segfaulting

  // GROUP A SKILL - tree traversal
  // SELECTION
  std::shared_ptr<MCTSNode> curNode = m_root;
  while(curNode->children.size()>0) {
    // choose the child node with the largest value of w_i/n_i + sqrt(c*ln(n_{i-1})/n_i)
    // where c is theoretically sqrt(2), but can be tweaked to control the exploration-exploitation ratio
    // note: 0.001 is added to n_i since dividing by 0 is undefined
    std::shared_ptr<MCTSNode> nextNode;
    double maxVal = -1;
    for(auto child : curNode->children) {
      double val = child->score / (child->playouts+0.001) + sqrt(2*log2(curNode->playouts) / (child->playouts+0.001) );
      if(val>maxVal) {
        maxVal = val;
        nextNode = child;
      }
    }
    curNode = nextNode;
  }

  // EXPANSION
  // choose random legal move, create new nodes
  std::vector<Move> legalMoves = m_gen.genMoves(curNode->pos);
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

  // SIMULATION
  // perform a "playout"
  Position p = curNode->pos;
  double result = 0; // 1 for loss, 0.5 for draw, 0 for win (from this position)
  // e.g. if current position is checkmate, then result is 1 because the previous node wants to go to this node
  while(true) {
    std::vector<Move> legalMoves = m_gen.genMoves(p);
    // terminal conditions
    // TODO: implement endgame table instead of this
    if(legalMoves.size()==0) {
      result = m_gen.getCheckingPieces(p).getBits()==0 ? 0.5 : 1; // if no legal moves, then stalemate if not being checked, else loss
      break;
    }
    if(p.getWhiteOccupancy().popcnt()==1) { // if white only has a king left
      bool isWhite = p.isWhiteToMove();
      if(p.getPieces(bq).getBits()) {result = isWhite ? 1 : 0; break;} // queen endgame
      if(p.getPieces(br).getBits()) {result = isWhite ? 1 : 0; break;} // rook endgame
      Bitboard occ = p.getBlackOccupancy();
      if(occ.popcnt()==1) {result = 0.5; break;} // only kings left
      if(occ.popcnt()==2 && (p.getPieces(bn)|p.getPieces(bb)).popcnt() == 1) {result = 0.5; break;} // black only has one bishop, or knight
      if(occ.popcnt()==3 && p.getPieces(bn).getBits() && p.getPieces(bb).getBits()) {result = isWhite ? 1 : 0; break;} // black has king, bishop and knight
    }
    // same as above, for black
    if(p.getBlackOccupancy().popcnt()==1) { // if white only has a king left
      bool isBlack = !p.isWhiteToMove();
      if(p.getPieces(wq).getBits()) {result = isBlack ? 1 : 0; break;}
      if(p.getPieces(wr).getBits()) {result = isBlack ? 1 : 0; break;}
      Bitboard occ = p.getBlackOccupancy();
      // note: only kings case has been handled above
      if(occ.popcnt()==2 && (p.getPieces(wn)|p.getPieces(wb)).popcnt() == 1) {result = 0.5; break;}
      if(occ.popcnt()==3 && p.getPieces(wn).getBits() && p.getPieces(wb).getBits()) {result = isBlack ? 1 : 0; break;}
    }

    // play a random legal move
    Move move = legalMoves[std::rand() % legalMoves.size()];
    p.makeMove(move);
  }

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

Move Engine::MCTS(int numSteps, bool verbose) {
  for(int i=0; i<numSteps; ++i) {
    doOneStep();
  }
  // return the move with the most number of playouts
  if(m_root->children.size()==0) return Move(-1, -1, empty, false, false, false); // dummy move
  auto choice = m_root->children[0];
  for(auto child : m_root->children) {
    if(verbose) {
      std::cout << (char)((child->move.start&7)+'a') << (child->move.start>>3)+1
        << (char)((child->move.end&7)+'a') << (child->move.end>>3)+1
        << ": " <<  child->score << "/" << child->playouts << "\n";
    }
    if(child->playouts > choice->playouts) choice = child;
  }
  return choice->move;

}
