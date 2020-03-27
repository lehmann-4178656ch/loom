// Copyright 2017, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Patrick Brosi <brosi@informatik.uni-freiburg.de>

#ifndef OCTI_OCTILINEARIZER_H_
#define OCTI_OCTILINEARIZER_H_

#include <unordered_set>
#include <vector>
#include "octi/basegraph/BaseGraph.h"
#include "octi/basegraph/GridGraph.h"
#include "octi/combgraph/CombGraph.h"
#include "octi/combgraph/Drawing.h"
#include "shared/linegraph/LineGraph.h"
#include "util/graph/Dijkstra.h"

namespace octi {

using octi::basegraph::GeoPens;
using octi::basegraph::GeoPensMap;
using octi::basegraph::GridEdge;
using octi::basegraph::GridEdgePL;
using octi::basegraph::GridGraph;
using octi::basegraph::GridNode;
using octi::basegraph::GridNodePL;
using octi::basegraph::NodeCost;
using octi::basegraph::Penalties;

using shared::linegraph::LineEdge;
using shared::linegraph::LineGraph;
using shared::linegraph::LineNode;

using octi::combgraph::CombEdge;
using octi::combgraph::CombGraph;
using octi::combgraph::CombNode;
using octi::combgraph::Drawing;
using octi::combgraph::EdgeOrdering;

using util::graph::Dijkstra;

typedef Dijkstra::EList<GridNodePL, GridEdgePL> GrEdgList;
typedef Dijkstra::NList<GridNodePL, GridEdgePL> GrNdList;
typedef std::pair<std::set<GridNode*>, std::set<GridNode*>> RtPair;
typedef std::map<CombNode*, std::pair<size_t, size_t>> SettledPos;

enum BaseGraphType { OCTIGRID, GRID };

// exception thrown when no planar embedding could be found
struct NoEmbeddingFoundExc : public std::exception {
  const char* what() const throw() {
    return "Could not find planar embedding for input graph.";
  }
};

// comparator for nodes, based on degree
struct NodeCmp {
  bool operator()(CombNode* a, CombNode* b) {
    return a->pl().getRouteNumber() < b->pl().getRouteNumber();
  }
};

typedef std::priority_queue<CombNode*, std::vector<CombNode*>, NodeCmp> NodePQ;

struct GraphMeasures {
  double maxNodeDist;
  double minNodeDist;
  double avgNodeDist;
  double maxEdgeLength;
  double minEdgeLength;
  double avgEdgeLength;
  size_t maxDeg;
};

struct GridCost : public Dijkstra::CostFunc<GridNodePL, GridEdgePL, float> {
  GridCost(float inf) : _inf(inf) {}
  virtual float operator()(const GridNode* from, const GridEdge* e,
                           const GridNode* to) const {
    UNUSED(from);
    UNUSED(to);
    return e->pl().cost();
  }

  float _inf;

  virtual float inf() const { return _inf; }
};

struct GridCostGeoPen
    : public Dijkstra::CostFunc<GridNodePL, GridEdgePL, float> {
  GridCostGeoPen(float inf, const GeoPens* geoPens)
      : _inf(inf), _geoPens(geoPens) {}
  virtual float operator()(const GridNode* from, const GridEdge* e,
                           const GridNode* to) const {
    UNUSED(from);
    UNUSED(to);
    return e->pl().cost() + (*_geoPens)[e->pl().getId()];
  }

  float _inf;
  const GeoPens* _geoPens;

  virtual float inf() const { return _inf; }
};

struct BaseGraphHeur
    : public Dijkstra::HeurFunc<GridNodePL, GridEdgePL, float> {
  BaseGraphHeur(basegraph::BaseGraph* g, const std::set<GridNode*>& to)
      : g(g), to(0) {
    if (to.size() == 1) this->to = *to.begin();

    cheapestSink = std::numeric_limits<float>::infinity();

    for (auto n : to) {
      size_t i = 0;
      for (; i < 8; i++) {
        float sinkCost = g->getEdg(n->pl().getPort(i), n)->pl().cost();
        if (sinkCost < cheapestSink) cheapestSink = sinkCost;
        auto neigh = g->getNeighbor(n, i);
        if (neigh && to.find(neigh) == to.end()) {
          hull.push_back(n->pl().getX());
          hull.push_back(n->pl().getY());
          break;
        }
      }
      for (size_t j = i; j < 8; j++) {
        float sinkCost = g->getEdg(n->pl().getPort(j), n)->pl().cost();
        if (sinkCost < cheapestSink) cheapestSink = sinkCost;
      }
    }
  }

  float operator()(const GridNode* from, const std::set<GridNode*>& to) const {
    const_cast<GridNode*>(from)->pl().visited = true;
    if (to.find(from->pl().getParent()) != to.end()) return 0;

    float ret = std::numeric_limits<float>::infinity();

    for (size_t i = 0; i < hull.size(); i += 2) {
      float tmp = g->heurCost(from->pl().getParent()->pl().getX(),
                              from->pl().getParent()->pl().getY(), hull[i],
                              hull[i + 1]);
      if (tmp < ret) ret = tmp;
    }

    return ret + cheapestSink;
  }

  octi::basegraph::BaseGraph* g;
  GridNode* to;
  std::vector<size_t> hull;
  float cheapestSink;
};

class Octilinearizer {
 public:
  Octilinearizer(BaseGraphType baseGraphType) : _baseGraphType(baseGraphType) {}
  double draw(LineGraph* in, LineGraph* out, basegraph::BaseGraph** gg,
              const Penalties& pens, double gridSize, double borderRad,
              bool deg2heur, double maxGrDist, bool restrLocSearch,
              double enfGeoCourse,
              const std::vector<util::geo::Polygon<double>>& obstacles);

  double draw(const CombGraph& cg, const util::geo::DBox& box, LineGraph* out,
              basegraph::BaseGraph** gg, const Penalties& pens, double gridSize,
              double borderRad, double maxGrDist, bool restrLocSearch,
              double enfGeoCourse,
              const std::vector<util::geo::Polygon<double>>& obstacles);

  double drawILP(LineGraph* in, LineGraph* out, basegraph::BaseGraph** gg,
                 const Penalties& pens, double gridSize, double borderRad,
                 bool deg2heur, double maxGrDist, bool noSolve,
                 double enfGeoPens, const std::string& path);

 private:
  BaseGraphType _baseGraphType;

  basegraph::BaseGraph* newBaseGraph(const util::geo::DBox& bbox,
                                     double cellSize, double spacer,
                                     const Penalties& pens) const;

  void removeEdgesShorterThan(LineGraph* g, double d);

  void writeNdCosts(GridNode* n, CombNode* origNode, CombEdge* e,
                    basegraph::BaseGraph* g);

  void settleRes(GridNode* startGridNd, GridNode* toGridNd,
                 basegraph::BaseGraph* gg, CombNode* from, CombNode* to,
                 const GrEdgList& res, CombEdge* e);

  std::vector<CombEdge*> getOrdering(const CombGraph& cg, bool randr) const;

  bool draw(const std::vector<CombEdge*>& order, basegraph::BaseGraph* gg,
            Drawing* drawing, double cutoff, double maxGrDist,
            const GeoPensMap* geoPensMap);
  bool draw(const std::vector<CombEdge*>& order, const SettledPos& settled,
            basegraph::BaseGraph* gg, Drawing* drawing, double cutoff,
            double maxGrDist, const GeoPensMap* geoPensMap);

  SettledPos getNeighbor(const SettledPos& pos, const std::vector<CombNode*>&,
                         size_t i) const;

  RtPair getRtPair(CombNode* frCmbNd, CombNode* toCmbNd,
                   const SettledPos& settled, basegraph::BaseGraph* gg,
                   double maxGrDist);

  std::set<GridNode*> getCands(CombNode* cmBnd, const SettledPos& settled,
                               basegraph::BaseGraph* gg, double maxDis);
};

}  // namespace octi

#endif  // OCTI_OCTILINEARIZER_H_
