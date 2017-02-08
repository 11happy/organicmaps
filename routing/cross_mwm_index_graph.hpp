#pragma once

#include "routing/cross_mwm_index_graph.hpp"
#include "routing/num_mwm_id.hpp"
#include "routing/routing_mapping.hpp"
#include "routing/segment.hpp"

#include "base/math.hpp"

#include <map>
#include <memory>
#include <set>
#include <vector>

namespace routing
{
/// \brief Getting information for cross mwm routing.
class CrossMwmIndexGraph final
{
public:
  CrossMwmIndexGraph(std::shared_ptr<NumMwmIds> numMwmIds, RoutingIndexManager & indexManager)
    : m_indexManager(indexManager), m_numMwmIds(numMwmIds)
  {
  }

  /// \brief Transition segment is a segment which is crossed by mwm border. That means
  /// start and finsh of such segment have to lie in different mwms. If a segment is
  /// crossed by mwm border but its start and finish lie in the same mwm it's not
  /// a transition segment.
  /// For most cases there is only one transition segment for a transition feature.
  /// Transition segment at the picture below is marked as *===>*.
  /// Transition feature is a feature which contains one or more transition segments.
  /// Transition features at the picture below is marked as *===>*------>*.
  /// Every transition feature is duplicated in two neighbouring mwms.
  /// The duplicate is called "twin" at the picture below.
  /// For most cases a transition feature contains only one transition segment.
  /// -------MWM0---------------MWM1----------------MWM2-------------
  /// |                   |                 |                       |
  /// | F0 in MWM0   *===>*------>*    *===>*------>*  F2 of MWM1   |
  /// | Twin in MWM1 *===>*------>*    *===>*------>*  Twin in MWM2 |
  /// |                   |                 |                       |
  /// | F1 in MWM0      *===>*---->*      *===>*--->*  F3 of MWM1   |
  /// | Twin in MWM1    *===>*---->*      *===>*--->*  Twin in MWM2 |
  /// |                   |                 |                       |
  /// ---------------------------------------------------------------
  /// There are two kinds of transition segments:
  /// * enter transition segments are enters to their mwms;
  /// * exit transition segments are exits from their mwms;
  /// \returns true if |s| is an exit (|isOutgoing| == true) or an enter (|isOutgoing| == false)
  /// transition segment.
  bool IsTransition(Segment const & s, bool isOutgoing);

  /// \brief Fills |twins| with duplicates of |s| transition segment in neighbouring mwm.
  /// For most cases there is only one twin for |s|.
  /// If |s| is an enter transition segment fills |twins| with appropriate exit transition segments.
  /// If |s| is an exit transition segment fills |twins| with appropriate enter transition segments.
  /// \note GetTwins(...) shall be called only if IsTransition(s, ...) returns true.
  void GetTwins(Segment const & s, std::vector<Segment> & twins) const;

  /// \brief Fills |edges| with edges outgoing from |s| (ingoing to |s|).
  /// If |isOutgoing| == true then |s| should be an enter transition segment.
  /// In that case |edges| is filled with all edges starting from |s| and ending at all reachable
  /// exit transition segments of the mwm of |s|.
  /// If |isOutgoing| == false then |s| should be an exit transition segment.
  /// In that case |edges| is filled with all edges starting from all reachable
  /// enter transition segments of the mwm of |s| and ending at |s|.
  /// Weight of each edge is equal to weight of the route form |s| to |SegmentEdge::m_target|
  /// if |isOutgoing| == true and from |SegmentEdge::m_target| to |s| otherwise.
  void GetEdgeList(Segment const & s, bool isOutgoing, std::vector<SegmentEdge> & edges) const;

private:
  struct TransitionSegments
  {
    std::set<Segment> m_ingoing;
    std::set<Segment> m_outgoing;
  };

  RoutingIndexManager & m_indexManager;
  std::shared_ptr<NumMwmIds> m_numMwmIds;

  std::map<NumMwmId, TransitionSegments> m_transitionCache;
};
}  // routing
