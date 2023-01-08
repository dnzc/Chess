#include "Position.h"
#include "Move.h"

// doubly linked list
struct GametreeNode {

  // data
  Position& position;
  int score; // sum over all playouts of (0 for loss, 0.5 for draw, 1 for win)
  int playouts;

  // links to other nodes
  GametreeNode* children;
  GametreeNode* parent;

  // constructor
  GametreeNode(position) : position(position) {
    score = 0;
    playouts = 0;
    next = NULL;
    prev = NULL;
  }
}

class Engine {

  public:
    Engine();
    Move mcts(int numPlayouts);
    Move mcts(double timeLimitSeconds);

  private:
    Position position;
    void doOneMonteCarlo(GametreeNode root);

}

