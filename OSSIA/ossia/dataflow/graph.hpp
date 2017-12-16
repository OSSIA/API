#pragma once
#include <ossia/dataflow/graph_edge.hpp>
#include <ossia/dataflow/graph_node.hpp>
#include <ossia/dataflow/dataflow.hpp>
#include <ossia/editor/scenario/time_value.hpp>
#include <boost/bimap.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/range/adaptors.hpp>
#include <ossia/dataflow/graph_ordering.hpp>
#include <boost/container/flat_set.hpp>
class DataflowTest;
namespace ossia
{

using graph_t = boost::
    adjacency_list<boost::vecS, boost::vecS, boost::directedS, node_ptr, std::shared_ptr<graph_edge>>;

using graph_vertex_t = graph_t::vertex_descriptor;
using graph_edge_t = graph_t::edge_descriptor;

template <typename T, typename U>
using bimap = boost::bimap<T, U>;
using node_bimap = bimap<graph_vertex_t, node_ptr>;
using edge_bimap = bimap<graph_edge_t, std::shared_ptr<graph_edge>>;
using node_bimap_v = node_bimap::value_type;
using edge_bimap_v = edge_bimap::value_type;

using edge_map_t
    = std::unordered_map<graph_edge*, std::shared_ptr<graph_edge>>;

enum class node_ordering
{
  topological, temporal, hierarchical
};

template<typename Ordering>
class OSSIA_EXPORT graph_base
{
  private:

  using node_set = std::vector<graph_node*>;
  using node_flat_set = boost::container::flat_set<graph_node*>;

  using Comparator = node_sorter<Ordering>;

  template<typename Comp_T>
  static void tick(
      graph_base& g,
      execution_state& e,
      std::vector<graph_node*>& active_nodes,
      node_set& next_nodes,
      Comp_T&& comp)
  {
    while (!active_nodes.empty())
    {
      next_nodes.clear();

      // Find all the nodes for which the inlets have executed
      // (or without cables on the inlets)
      for(graph_node* node : active_nodes)
        if(node->can_execute(e))
          next_nodes.push_back(node);

      if (!next_nodes.empty())
      {
        std::sort(next_nodes.begin(), next_nodes.end(), std::move(comp));
        // First look if there is a replacement or reduction relationship between
        // the first n nodes
        // If there is, we run all the nodes

        // If there is not we just run the first node
        graph_node& first_node = **next_nodes.begin();
        g.init_node(first_node, e);
        if(first_node.start_discontinuous()) {
          first_node.requested_tokens.front().start_discontinuous = true;
          first_node.set_start_discontinuous(false);
        }
        if(first_node.end_discontinuous()) {
          first_node.requested_tokens.front().end_discontinuous = true;
          first_node.set_end_discontinuous(false);
        }

        for(auto request : first_node.requested_tokens) {
          first_node.run(request, e);
          first_node.set_prev_date(request.date);
        }

        first_node.set_executed(true);
        g.teardown_node(first_node, e);
        active_nodes.erase(ossia::find(active_nodes, &first_node));
      }
      else
      {
        break; // nothing more to execute
      }
    }
  }

  void get_sorted_nodes(const graph_t& gr)
  {
    // Get a total order on nodes
    active_nodes.clear();
    topo_order.clear();
    topo_order.reserve(m_nodes.size());
    active_nodes.reserve(m_nodes.size());

    // TODO this should be doable with a single vector
    boost::topological_sort(gr, std::back_inserter(topo_order));

    for(auto vtx : topo_order)
    {
      auto node = gr[vtx].get();
      if(node->enabled())
        active_nodes.push_back(node);
    }
  }

  void get_enabled_nodes(const graph_t& gr)
  {
    active_nodes.clear();

    auto vtx = boost::vertices(gr);
    active_nodes.reserve(m_nodes.size());

    for(auto it = vtx.first; it != vtx.second; ++it)
    {
      auto node = gr[*it].get();
      if(node->enabled())
        active_nodes.push_back(node);
    }
  }


public:
  ~graph_base()
  {
    clear();
  }

  void add_node(node_ptr n)
  {
    auto vtx = boost::add_vertex(n, m_graph);
    m_nodes.insert(node_bimap_v{vtx, std::move(n)});
  }
  void remove_node(const node_ptr& n)
  {
    for(auto& port : n->inputs())
      for(auto edge : port->sources)
        disconnect(edge);
    for(auto& port : n->outputs())
      for(auto edge : port->targets)
        disconnect(edge);

    auto it = m_nodes.right.find(n);
    if (it != m_nodes.right.end())
    {
      auto vtx = boost::vertices(m_graph);
      if(std::find(vtx.first, vtx.second, it->second) != vtx.second)
      {
        boost::clear_vertex(it->second, m_graph);
        boost::remove_vertex(it->second, m_graph);
      }
      m_nodes.right.erase(it);
    }
  }

  void connect(const std::shared_ptr<graph_edge>& edge)
  {
    auto it1 = m_nodes.right.find(edge->in_node);
    auto it2 = m_nodes.right.find(edge->out_node);
    if (it1 != m_nodes.right.end() && it2 != m_nodes.right.end())
    {
      // TODO check that two edges can be added
      auto res = boost::add_edge(it1->second, it2->second, edge, m_graph);
      if (res.second)
      {
        m_edges.insert(edge_bimap_v{res.first, edge});
        m_edge_map.insert(std::make_pair(edge.get(), edge));
      }
    }
  }
  void disconnect(const std::shared_ptr<graph_edge>& edge)
  {
    if(edge)
    {
      auto it = m_edges.right.find(edge);
      if (it != m_edges.right.end())
      {
        auto edg = boost::edges(m_graph);
        if(std::find(edg.first, edg.second, it->second) != edg.second)
          boost::remove_edge(it->second, m_graph);
        m_edge_map.erase(edge.get());
      }
      edge->clear();
    }
  }
  void disconnect(graph_edge* edge)
  {
    auto ptr_it = m_edge_map.find(edge);
    if(ptr_it != m_edge_map.end())
    {
      disconnect(ptr_it->second);
    }
  }

  void clear()
  {
    // TODO clear all the connections, ports, etc, to ensure that there is no
    // shared_ptr loop
    for (auto& node : m_nodes.right)
    {
      node.first->clear();
    }
    for (auto& edge : m_edges.right)
    {
      edge.first->clear();
    }
    m_nodes.clear();
    m_edges.clear();
    m_graph.clear();
    m_edge_map.clear();
    m_time = 0;
  }

  void state()
  {
    execution_state e;
    state(e);
    e.commit();
  }
  void state(execution_state& e)
  {
    try
    {
      // TODO in the future, temporal_graph, space_graph that can be used as
      // processes.

      // Filter disabled nodes (through strict relationships).
      m_enabled_cache.clear();
      m_enabled_cache.reserve(m_nodes.size());

      for(auto it = boost::vertices(m_graph).first; it != boost::vertices(m_graph).second; ++it)
      {
        ossia::graph_node& ptr = *m_graph[*it];
        if(ptr.enabled()) {
          m_enabled_cache.insert(&ptr);
        }
      }

      disable_strict_nodes_rec(m_enabled_cache);

      // Start executing the nodes
      if constexpr(std::is_same<Ordering, topological_ordering>::value)
      {
        get_sorted_nodes(m_graph);
        tick(*this, e, active_nodes, m_order_cache, node_sorter<>{topological_ordering{active_nodes}, e});
      }
      else
      {
        get_enabled_nodes(m_graph);
        tick(*this, e, active_nodes, m_order_cache, node_sorter<Ordering>{Ordering{}, e});
      }

      for (auto& node : m_nodes.right)
      {
        ossia::graph_node& n = *node.first;
        n.set_executed(false);
        n.requested_tokens.clear();
        for (const outlet_ptr& out : node.first->outputs())
        {
          if (out->data)
            eggs::variants::apply(clear_data{}, out->data);
        }
      }
    }
    catch (const boost::not_a_dag& e)
    {
      ossia::logger().error("Execution graph is not a DAG.");
      return;
    }
  }

  void
  disable_strict_nodes(const node_flat_set& enabled_nodes, node_flat_set& ret)
  {
    for (graph_node* node : enabled_nodes)
    {
      for (const auto& in : node->inputs())
      {
        for (const auto& edge : in->sources)
        {
          assert(edge->out_node);

          if (immediate_strict_connection* sc = edge->con.target<immediate_strict_connection>())
          {
            if ((sc->required_sides
                 & immediate_strict_connection::required_sides_t::outbound)
                && node->enabled() && !edge->out_node->enabled())
            {
              ret.insert(node);
            }
          }
          else if (delayed_strict_connection* delay = edge->con.target<delayed_strict_connection>())
          {
            const auto n = ossia::apply(data_size{}, delay->buffer);
            if (n == 0 || delay->pos >= n)
              ret.insert(node);
          }
        }
      }

      for (const auto& out : node->outputs())
      {
        for (const auto& edge : out->targets)
        {
          assert(edge->in_node);

          if (auto sc = edge->con.target<immediate_strict_connection>())
          {
            if ((sc->required_sides
                 & immediate_strict_connection::required_sides_t::inbound)
                && node->enabled() && !edge->in_node->enabled())
            {
              ret.insert(node);
            }
          }
        }
      }
    }
  }
  /*
  static set<graph_node*> disable_strict_nodes(const set<node_ptr>& n)
  {
    using namespace boost::adaptors;
    auto res = (n | transformed([](const auto& p) { return p.get(); }));
    return disable_strict_nodes(set<graph_node*>{res.begin(), res.end()});
  }
  */

  void disable_strict_nodes_rec(node_flat_set& cur_enabled_node)
  {
    do
    {
      m_disabled_cache.clear();
      disable_strict_nodes(cur_enabled_node, m_disabled_cache);
      for (graph_node* n : m_disabled_cache)
      {
        if(!n->requested_tokens.empty())
          n->set_prev_date(n->requested_tokens.back().date);
        n->disable();

        cur_enabled_node.erase(n);
      }

    } while (!m_disabled_cache.empty());
  }


  static void copy_from_local(const data_type& out, inlet& in)
  {
    if (out.which() == in.data.which() && out && in.data)
    {
      eggs::variants::apply(copy_data{}, out, in.data);
    }
  }

  static void copy(const delay_line_type& out, std::size_t pos, inlet& in)
  {
    if (out.which() == in.data.which() && out && in.data)
    {
      eggs::variants::apply(copy_data_pos{pos}, out, in.data);
    }
  }

  static void copy(const outlet& out, inlet& in)
  {
    copy_from_local(out.data, in);
  }

  static void copy_to_local(
      const data_type& out, const destination& d, execution_state& in)
  {
    in.insert(destination_t{&d.address()}, out);
  }

  static void copy_to_global(
      const data_type& out, const destination& d, execution_state& in)
  {
    // TODO
  }

  static void pull_from_parameter(inlet& in, execution_state& e)
  {
    apply_to_destination(in.address, e, [&] (ossia::net::parameter_base* addr) {
      if (in.scope & port::scope_t::local)
      {
        e.find_and_copy(*addr, in);
      }
      else
      {
        e.copy_from_global(*addr, in);
      }
    });
  }

  void init_node(graph_node& n, execution_state& e)
  {
    // Clear the outputs of the node
    for (const outlet_ptr& out : n.outputs())
    {
      if (out->data)
        eggs::variants::apply(clear_data{}, out->data);
    }

    // Copy from environment and previous ports to inputs
    for (const inlet_ptr& in : n.inputs())
    {
      if (!in->sources.empty())
      {
        for (auto edge : in->sources)
        {
          ossia::apply(init_node_visitor<graph_base>{*in, *edge, e}, edge->con);
        }
      }
      else
      {
        pull_from_parameter(*in, e);
      }
    }
  }



  void teardown_node(const graph_node& n, execution_state& e)
  {
    for (const inlet_ptr& in : n.inputs())
    {
      if (in->data)
        eggs::variants::apply(clear_data{}, in->data);
    }

    // Copy from output ports to environment
    for (const outlet_ptr& out : n.outputs())
    {
      bool all_targets_disabled = !out->targets.empty() && ossia::all_of(out->targets, [] (auto tgt) {
        return tgt->in_node && !tgt->in_node->enabled();
      });

      if (out->targets.empty() || all_targets_disabled)
      {
        out->write(e);
      }
      else
      {
        // If the following target has been deactivated
        for (auto tgt : out->targets)
        {
          ossia::apply(env_writer{*out, *tgt, e}, tgt->con);
        }
      }
    }
  }

  node_bimap m_nodes;
  edge_bimap m_edges;

  graph_t m_graph;

  edge_map_t m_edge_map;

  time_value m_time{};
  node_set m_order_cache;

  node_flat_set m_enabled_cache;
  node_flat_set m_disabled_cache;
  std::vector<graph_node*> active_nodes;
  std::vector<graph_vertex_t> topo_order;

  friend struct inlet;
  friend struct outlet;
  friend class ::DataflowTest;
};


class graph : public graph_base<topological_ordering>
{ public: using graph_base<topological_ordering>::graph_base; };



}
