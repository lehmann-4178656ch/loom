// Copyright 2016, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Patrick Brosi <brosi@informatik.uni-freiburg.de>

#ifndef SKELETONBUILDER_OUTPUT_GEOJSONOUTPUT_H_
#define SKELETONBUILDER_OUTPUT_GEOJSONOUTPUT_H_

#include <string>
#include <ostream>
#include "transitmap/config/TransitMapConfig.h"
#include "transitmap/util/Geo.h"
#include "./../graph/Graph.h"
#include "transitmap/graph/Edge.h"
#include "transitmap/geo/PolyLine.h"
#include "json/json.hpp"

using json = nlohmann::json;

using namespace transitmapper;

namespace skeletonbuilder {

class JsonOutput {

 public:
  JsonOutput(const config::Config* cfg);
  void print(const graph::Graph& outG);

 private:
  const config::Config* _cfg;
};

}

#endif // SKELETONBUILDER_OUTPUT_GEOJSONOUTPUT_H_