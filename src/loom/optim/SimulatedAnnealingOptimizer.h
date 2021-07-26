// Copyright 2016, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Patrick Brosi <brosi@informatik.uni-freiburg.de>

#ifndef LOOM_OPTIM_SIMULATEDANNEALINGOPTIMIZER_H_
#define LOOM_OPTIM_SIMULATEDANNEALINGOPTIMIZER_H_

#include "loom/config/TransitMapConfig.h"
#include "loom/graph/OrderCfg.h"
#include "loom/optim/HillClimbOptimizer.h"
#include "loom/optim/ILPEdgeOrderOptimizer.h"
#include "loom/optim/NullOptimizer.h"
#include "loom/optim/OptGraph.h"
#include "loom/optim/OptGraphScorer.h"
#include "loom/optim/Optimizer.h"

using std::exception;
using std::string;

namespace loom {
namespace optim {

class SimulatedAnnealingOptimizer : public HillClimbOptimizer {
 public:
  SimulatedAnnealingOptimizer(const config::Config* cfg, const Scorer* scorer)
      : HillClimbOptimizer(cfg, scorer){};

  virtual int optimizeComp(OptGraph* og, const std::set<OptNode*>& g,
                           graph::HierarOrderCfg* c, size_t depth) const;

 private:
  double getScore(OptGraph* og, OptEdge* e, OptOrderCfg& cur) const;
};
}  // namespace optim
}  // namespace loom

#endif  // LOOM_OPTIM_SIMULATEDANNEALINGOPTIMIZER_H_