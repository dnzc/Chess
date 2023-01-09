#include "Position.h"
#include "Move.h"
#include "MoveGenerator.h"
#include <vector>
#include <string>
#include <memory>

struct MCTSNode {

  MCTSNode(Position& pos, Move move);

  // data
  Position pos;
  Move move;
  double score; // sum over all playouts of (0 for loss, 0.5 for draw, 1 for win)
  double playouts; // number of playouts; so (score/playouts) is win percentage

  // links to other nodes
  std::vector< std::shared_ptr<MCTSNode> > children;
  std::shared_ptr<MCTSNode> parent;

};

// GROUP A SKILL - complex OOP
class Engine {

  public:
    Engine();
    Engine(std::string FEN);
    void makeMove(Move move);
    Move MCTS(int numSteps, bool verbose);
    Move MCTS(double timeLimitSeconds, bool verbose);
    Position getPos();
    std::vector<Move> getLegalMoves();

  private:
    Position m_pos;
    MoveGenerator m_gen;
    std::shared_ptr<MCTSNode> m_root;
    void doOneStep();

    std::vector<Move> m_prevMoves;

};
