// Copyright 2016, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Authors: Patrick Brosi <brosip@informatik.uni-freiburg.de>

#include <cassert>
#include "./Node.h"
#include "./Edge.h"
#include "./TransitGraph.h"
#include "./../geo/BezierCurve.h"
#include "gtfsparser/gtfs/Stop.h"
#include "../graph/EdgeTripGeom.h"
#include "../graph/OrderingConfiguration.h"

using namespace transitmapper;
using namespace graph;
using namespace gtfsparser;

// _____________________________________________________________________________
util::geo::Point NodeFront::getTripOccPos(const gtfs::Route* r) const {
  for (auto e : edges) {
    for (auto& etg : *e->getEdgeTripGeoms()) {
      TripOccWithPos to = etg.getTripsForRoute(r);
      if (to.first) {
        double p = 0;
        //if (etg.getGeomDir() != n) {
        //  p = (etg.getWidth() + etg.getSpacing()) * to.second + etg.getWidth()/2;
        //} else {
          p = (etg.getWidth() + etg.getSpacing()) * (etg.getTripsUnordered().size() - 1 - to.second) + etg.getWidth()/2;
        //}

        double pp = p / geom.getLength();

        return geom.getPointAt(pp).p;
      }
    }
  }

  // TODO: handle not-found case
}

// _____________________________________________________________________________
util::geo::Point NodeFront::getTripOccPos(const gtfs::Route* r, const Configuration& c) const {
  return getTripOccPosUnder(r, c, 0, 0);
}

// _____________________________________________________________________________
util::geo::Point NodeFront::getTripOccPosUnder(const gtfs::Route* r,
    const graph::Configuration& c, const EdgeTripGeom* g, const Ordering* order) const {
  for (auto e : edges) {
    for (auto& etg : *e->getEdgeTripGeoms()) {
      TripOccWithPos to;

      if (&etg == g) {
        to = etg.getTripsForRouteUnder(r, *order);
      } else {
        to = etg.getTripsForRouteUnder(r, c.find(&etg)->second);
      }

      if (to.first) {
        double p = 0;
        //if (etg.getGeomDir() != n) {
        //  p = (etg.getWidth() + etg.getSpacing()) * to.second + etg.getWidth()/2;
        //} else {
          p = (etg.getWidth() + etg.getSpacing()) * (etg.getTripsUnordered().size() - 1 - to.second) + etg.getWidth()/2;
        //}

        double pp = p / geom.getLength();

        return geom.getPointAt(pp).p;
      }
    }
  }
  // TODO: handle not-found case
}

// _____________________________________________________________________________
Node::Node(util::geo::Point pos) : _pos(pos) {
}

// _____________________________________________________________________________
Node::Node(double x, double y) : _pos(x, y) {
}

// _____________________________________________________________________________
Node::Node(util::geo::Point pos, gtfs::Stop* s) : _pos(pos) {
  if (s) _stops.insert(s);
}

// _____________________________________________________________________________
Node::Node(double x, double y, gtfs::Stop* s) : _pos(x, y) {
  if (s) _stops.insert(s);
}

// _____________________________________________________________________________
Node::~Node() {
  for (Edge* e : _adjListOut) {
    e->getFrom()->removeEdge(e);
    e->getTo()->removeEdge(e);
    _adjListIn.erase(e); // catch edge to itself case
    delete e;
  }

  for (Edge* e : _adjListIn) {
    e->getFrom()->removeEdge(e);
    e->getTo()->removeEdge(e);
    delete e;
  }
}

// _____________________________________________________________________________
void Node::addStop(gtfs::Stop* s) {
  _stops.insert(s);
}

// _____________________________________________________________________________
const std::set<gtfs::Stop*>& Node::getStops() const {
  return _stops;
}

// _____________________________________________________________________________
void Node::addEdge(Edge* e) {
  if (e->getFrom() == this) _adjListOut.insert(e);
  if (e->getTo() == this) _adjListIn.insert(e);
}

// _____________________________________________________________________________
void Node::removeEdge(Edge* e) {
  if (e->getFrom() == this) _adjListOut.erase(e);
  if (e->getTo() == this) _adjListIn.erase(e);
}

// _____________________________________________________________________________
const util::geo::Point& Node::getPos() const {
  return _pos;
}

// _____________________________________________________________________________
void Node::setPos(const util::geo::Point& p) {
  _pos = p;
}

// _____________________________________________________________________________
void Node::addMainDir(NodeFront f) {
  _mainDirs.push_back(f);
}

// _____________________________________________________________________________
const NodeFront* Node::getNodeFrontFor(const Edge* e) const {
  for (auto& nf : getMainDirs()) {
    if (std::find(nf.edges.begin(), nf.edges.end(), e) != nf.edges.end()) {
      return &nf;
    }
  }

  return 0;
}

// _____________________________________________________________________________
double Node::getScore() const {
  std::vector<InnerGeometry> igs = getInnerGeometries(false);

  double score = 0;

  for (size_t i = 0; i < igs.size(); i++) {
    for (size_t j = 0; j < igs.size(); j++) {
      if (j == i) continue;  // don't check against itself

      if (igs[j].geom.distTo(igs[i].geom) < 1) {
        score += .5;
      }
    }
  }

  return score / sqrt(_adjListIn.size() + _adjListOut.size());
}

// _____________________________________________________________________________
double Node::getScore(const Configuration& c) const {
  std::vector<InnerGeometry> igs = getInnerGeometries(c);

  double score = 0;

  for (size_t i = 0; i < igs.size(); i++) {
    for (size_t j = 0; j < igs.size(); j++) {
      if (j == i) continue;  // don't check against itself

      if (igs[j].geom.distTo(igs[i].geom) < 1) {
        score += .5;
      }
    }
  }

  return score / sqrt(_adjListIn.size() + _adjListOut.size());
}

// _____________________________________________________________________________
double Node::getScoreUnder(const graph::Configuration& c, const EdgeTripGeom* g,
    const std::vector<size_t>* order) const {


  std::vector<InnerGeometry> igs = getInnerGeometriesUnder(c, g, order);

  double score = 0;

  for (size_t i = 0; i < igs.size(); i++) {
    for (size_t j = 0; j < igs.size(); j++) {
      if (j == i) continue;  // don't check against itself

      if (igs[j].geom.distTo(igs[i].geom) < 1) {
        score += .5;
      }
    }
  }

  return score / sqrt(_adjListIn.size() + _adjListOut.size());
}

// _____________________________________________________________________________
double Node::getAreaScore(const Configuration& c, const EdgeTripGeom* g,
  const Ordering* order)
const {
  double ret = getScoreUnder(c, g, order);

  for (auto e : _adjListIn) {
    ret += e->getFrom()->getScoreUnder(c, g, order);
  }

  for (auto e : _adjListOut) {
    ret += e->getTo()->getScoreUnder(c, g, order);
  }

  return ret;
}

// _____________________________________________________________________________
double Node::getAreaScore(const Configuration& c) const {
  return getAreaScore(c, 0, 0);
}

// _____________________________________________________________________________
std::vector<Partner> Node::getPartner(const NodeFront* f, const gtfs::Route* r)
const {
  std::vector<Partner> ret;
  for (const auto& nf : getMainDirs()) {
    if (&nf == f) continue;

    for (const auto e : nf.edges) {
      for (const auto& etg : *e->getEdgeTripGeoms()) {
        // TODO: unordered access is fine here!!!
        for (const TripOccurance& to : etg.getTripsUnordered()) {
          if (to.route == r) {
            Partner p;
            p.front = &nf;
            p.edge = e;
            p.etg = &etg;
            p.route = to.route;
            ret.push_back(p);
          }
        }
      }
    }
  }
  return ret;
}

// _____________________________________________________________________________
std::vector<InnerGeometry> Node::getInnerGeometries(bool bezier) const {
  std::vector<InnerGeometry> ret;

  std::set<const gtfs::Route*> processed;
  for (size_t i = 0; i < getMainDirs().size(); i++) {
    const graph::NodeFront& nf = getMainDirs()[i];
    for (auto e : nf.edges) {
      for (auto etgIt = e->getEdgeTripGeoms()->begin();
            etgIt != e->getEdgeTripGeoms()->end(); etgIt++) {

        for (size_t i : etgIt->getTripOrdering()) {
          const TripOccurance& tripOcc = etgIt->getTripsUnordered()[i];
          if (!processed.insert(tripOcc.route).second) continue;
          util::geo::Point p = nf.getTripOccPos(tripOcc.route);
          std::vector<graph::Partner> partners = getPartner(&nf, tripOcc.route);

          if (partners.size() == 0) continue;

          util::geo::Point pp = partners[0].front->getTripOccPos(partners[0].route);

          if (bezier) {
            util::geo::Point b = p;
            util::geo::Point c = pp;
            std::pair<double, double> slopeA, slopeB;

            double d = nf.geom.distTo(partners[0].front->geom) / 2;

            if (nf.refEtg->getGeomDir() == this) {
              slopeA = nf.refEtg->getGeom().getSlopeBetweenDists(nf.refEtg->getGeom().getLength() - 5, nf.refEtg->getGeom().getLength());
            } else {
              slopeA = nf.refEtg->getGeom().getSlopeBetweenDists(5, 0);
            }

            if (partners[0].front->refEtg->getGeomDir() == this) {
              slopeB = partners[0].front->refEtg->getGeom().getSlopeBetweenDists(partners[0].front->refEtg->getGeom().getLength() - 5, partners[0].front->refEtg->getGeom().getLength());
            } else {
              slopeB = partners[0].front->refEtg->getGeom().getSlopeBetweenDists(5, 0);
            }


            b = util::geo::Point(p.get<0>() + slopeA.first * d, p.get<1>() + slopeA.second * d);
            c = util::geo::Point(pp.get<0>() + slopeB.first * d, pp.get<1>() + slopeB.second * d);

            geo::BezierCurve bc(p, b, c, pp);
            ret.push_back(InnerGeometry(bc.render(5), partners[0].route, &*etgIt));
          } else {
            geo::PolyLine line(p, pp);
            ret.push_back(InnerGeometry(line, partners[0].route, &*etgIt));
          }
        }
      }
    }
  }

  return ret;
}

// _____________________________________________________________________________
std::vector<InnerGeometry> Node::getInnerGeometries(const graph::Configuration& c)
const {
  return getInnerGeometriesUnder(c, 0, 0);
}

// _____________________________________________________________________________
std::vector<InnerGeometry> Node::getInnerGeometriesUnder(const graph::Configuration& c,
    const EdgeTripGeom* g,
    const graph::Ordering* order) const {
  std::vector<InnerGeometry> ret;

  std::set<const gtfs::Route*> processed;
  for (size_t i = 0; i < getMainDirs().size(); i++) {
    const graph::NodeFront& nf = getMainDirs()[i];
    for (auto e : nf.edges) {
      for (auto etgIt = e->getEdgeTripGeoms()->begin();
            etgIt != e->getEdgeTripGeoms()->end(); etgIt++) {

        const std::vector<size_t>* ordering = 0;

        if (&*etgIt == g) {
          ordering = order;
        } else {
          ordering = &c.find(&*etgIt)->second;
        }

        for (size_t i : *ordering) {
          const TripOccurance& tripOcc = etgIt->getTripsUnordered()[i];
          if (!processed.insert(tripOcc.route).second) continue;

          util::geo::Point p = nf.getTripOccPosUnder(
            tripOcc.route,
            c,
            g,
            order
          );

          std::vector<graph::Partner> partners = getPartner(&nf, tripOcc.route);

          if (partners.size() == 0) continue;

          util::geo::Point pp = partners[0].front->getTripOccPosUnder(
            partners[0].route,
            c,
            g,
            order
          );

          geo::PolyLine line(p, pp);
          ret.push_back(InnerGeometry(line, partners[0].route, &*etgIt));
        }
      }
    }
  }

  return ret;
}

// _____________________________________________________________________________
util::geo::Polygon Node::getConvexFrontHull(double d) const {
  util::geo::MultiLine l;

  if (getMainDirs().size() != 2) {
    for (auto& nf : getMainDirs()) {
      l.push_back(nf.geom.getLine());
    }
  } else {
    // for two main dirs, take average
    std::vector<const geo::PolyLine*> pols;
    geo::PolyLine a = getMainDirs()[0].geom;
    geo::PolyLine b = getMainDirs()[1].geom;

    assert(a.getLine().size() > 1);
    assert(b.getLine().size() > 1);

    if (util::geo::dist(a.getLine()[0], b.getLine()[0]) > util::geo::dist(a.getLine()[1], b.getLine()[0])) {
      a.reverse();
    }

    pols.push_back(&a);
    pols.push_back(&b);
    l.push_back(geo::PolyLine::average(pols).getLine());
  }

  util::geo::MultiPolygon ret;
  double pointsPerCircle = 36;
  boost::geometry::strategy::buffer::distance_symmetric<double> distanceStrat(d);
  boost::geometry::strategy::buffer::join_round joinStrat(pointsPerCircle);
  boost::geometry::strategy::buffer::end_round endStrat(pointsPerCircle);
  boost::geometry::strategy::buffer::point_circle circleStrat(pointsPerCircle);
  boost::geometry::strategy::buffer::side_straight sideStrat;

  if (l.size() > 1) {
    util::geo::Polygon hull;
    boost::geometry::convex_hull(l, hull);
    boost::geometry::buffer(hull, ret, distanceStrat, sideStrat, joinStrat, endStrat, circleStrat);
  } else {
    boost::geometry::buffer(l, ret, distanceStrat, sideStrat, joinStrat, endStrat, circleStrat);
  }


  assert(ret.size() == 1);
  return ret[0];
}
