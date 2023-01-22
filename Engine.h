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
    Move MCTS(int timeLimit_ms, bool alphaBeta, bool verbose);
    Move minimax(int timeLimit_ms, bool verbose);
    Position getPos();
    std::vector<Move> getLegalMoves();
    int isGameOver(); // 0 for no, 1 for draw, 2 for checkmate

  private:
    Position m_pos;
    MoveGenerator m_gen;
    std::shared_ptr<MCTSNode> m_root;
    void doOneMonteCarloStep(bool alphaBeta, std::chrono::time_point<std::chrono::steady_clock> startTime_ms);
    double playout(Position& p);
    double minimaxAB(Position& p, std::chrono::time_point<std::chrono::steady_clock> startTime_ms, int timeLimit_ms, int depth, double alpha, double beta);
    double capturesAB(Position& p, std::chrono::time_point<std::chrono::steady_clock> startTime_ms, int timeLimit_ms, double alpha, double beta);

    void order(Position& p, std::vector<Move>& moves);

    double eval(Position& p);

    double m_inf = 100000000;
    double m_pieceValues[12] = {1, 3, 3, 5, 9, 0, 1, 3, 3, 5, 9, 0}; // wp, wn, wb, etc (kings n/a)
    double m_centreDist[8] = {3, 2, 1, 0, 0, 1, 2, 3}; // distance to centre for each file/rank

    std::vector<Move> m_prevMoves;

};
