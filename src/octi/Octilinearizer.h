// Copyright 2017, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Patrick Brosi <brosi@informatik.uni-freiburg.de>

#ifndef OCTI_OCTILINEARIZER_H_
#define OCTI_OCTILINEARIZER_H_

#include <unordered_set>
#include <vector>
#include "octi/combgraph/CombGraph.h"
#include "octi/gridgraph/GridGraph.h"
#include "octi/transitgraph/TransitGraph.h"
#include "util/graph/Dijkstra.h"

namespace octi {

using octi::gridgraph::GridGraph;
using octi::gridgraph::GridNode;
using octi::gridgraph::GridEdge;
using octi::gridgraph::GridNodePL;
using octi::gridgraph::GridEdgePL;
using octi::gridgraph::Penalties;

using octi::transitgraph::TransitGraph;
using octi::transitgraph::TransitNode;
using octi::transitgraph::TransitEdge;
using octi::transitgraph::EdgeOrdering;

using octi::combgraph::CombGraph;
using octi::combgraph::CombNode;
using octi::combgraph::CombEdge;

using util::graph::Dijkstra;

struct GraphMeasures {
  double maxNodeDist;
  double minNodeDist;
  double avgNodeDist;
  double maxEdgeLength;
  double minEdgeLength;
  double avgEdgeLength;
  size_t maxDeg;
};

struct GridCost : public Dijkstra::CostFunc<GridNodePL, GridEdgePL, double> {
  virtual double operator()(const GridNode* from, const GridEdge* e,
                            const GridNode* to) const {
    UNUSED(from);
    UNUSED(to);
    return e->pl().cost();
  }

  virtual double inf() const { return std::numeric_limits<double>::infinity(); }
};

struct GridHeur : public Dijkstra::HeurFunc<GridNodePL, GridEdgePL, double> {
  GridHeur(GridGraph* g, GridNode* from, const std::set<GridNode*>& to)
      : g(g), from(from), to(0) {
    if (to.size() == 1) {
      this->to = *to.begin();
    }

    for (auto n : to) {
      auto coords = g->getNodeCoords(n);

      for (size_t i = 0; i < 8; i++) {
        auto neigh = g->getNeighbor(coords.first, coords.second, i);
        if (neigh && to.find(neigh) == to.end()) {
          hull.push_back(g->getNodeCoords(n));
          break;
        }
      }
    }
  }

  double operator()(const GridNode* from, const std::set<GridNode*>& to) const {
    if (to.find(from->pl().getParent()) != to.end()) return 0;

    size_t ret = std::numeric_limits<size_t>::max();
    auto xy = g->getNodeCoords(from->pl().getParent());

    for (auto t : hull) {
      size_t temp = g->heurCost(xy.first, xy.second, t.first, t.second);
      if (temp < ret) ret = temp;
    }

    return ret;
  }

  octi::gridgraph::GridGraph* g;
  GridNode* from;
  GridNode* to;
  std::vector<std::pair<size_t, size_t> > hull;
};

class Octilinearizer {
 public:
  Octilinearizer() {}
  TransitGraph draw(TransitGraph* g, GridGraph** gg, const Penalties& pens);

 private:
  void normalizeCostVector(double* vec) const;
  double getMaxDis(CombNode* to, CombEdge* e, double gridSize);
  void removeEdgesShorterThan(TransitGraph* g, double d);
  void writeEdgeOrdering(CombGraph* target);
  EdgeOrdering getEdgeOrderingForNode(CombNode* n) const;
  EdgeOrdering getEdgeOrderingForNode(
      CombNode* n, bool useOrigNextNode,
      const std::map<CombNode*, DPoint>& newPos) const;
  void buildPolylineFromRes(const std::vector<GridEdge*>& l,
                            PolyLine<double>& res);
  double getCostFromRes(const std::vector<GridEdge*>& l);
  void addResidentEdges(gridgraph::GridGraph* g, CombEdge* e,
                        const std::vector<GridEdge*>& res);
  size_t changesTopology(CombNode* n, DPoint p,
                         const std::map<CombNode*, DPoint>& newPos) const;
};

}  // namespace octi

#endif  // OCTI_OCTILINEARIZER_H_