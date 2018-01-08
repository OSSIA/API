#pragma once
#include <ossia/dataflow/audio_parameter.hpp>
#include <boost/range/algorithm/lexicographical_compare.hpp>
#include <ossia/network/common/path.hpp>
#include <ossia/detail/logger.hpp>

namespace ossia
{

struct node_sorter
{
    const std::vector<graph_node*>& node_order;
    const execution_state& st;

    bool compare(const graph_node* lhs, const graph_node* rhs) const
    {
      for(std::size_t i = 0, N = node_order.size(); i < N; i++)
      {
        if(node_order[i] == lhs)
          return true;
        else if(node_order[i] == rhs)
          return false;
        else
          continue;
      }
      throw std::runtime_error("lhs and rhs have to be found");
    }

    bool operator()(const graph_node* lhs, const graph_node* rhs) const
    {
      // This sorting method ensures that if for instance
      // node A produces "/a" and node B consumes "/a",
      // A executes before B.
      bool c1 = lhs->has_port_inputs();
      bool c2 = rhs->has_port_inputs();
      if (c1 && !c2)
        return true;
      else if (!c1 && c2)
        return false;
      else if (c1 && c2)
        // the nodes are already sorted through the toposort
        // so we can just keep their original order
        return compare(lhs, rhs);

      bool l1 = lhs->has_local_inputs(st);
      bool l2 = rhs->has_local_inputs(st);

      if (l1 && !l2)
        return true;
      else if (!l1 && l2)
        return false;
      else if (l1 && l2)
        return compare(lhs, rhs);

      bool g1 = lhs->has_global_inputs();
      bool g2 = rhs->has_global_inputs();
      if (g1 && !g2)
        return true;
      else if (!g1 && g2)
        return false;
      else if (g1 && g2)
        return compare(lhs, rhs);

      return compare(lhs, rhs);
    }
};

template<typename Graph_T>
struct init_node_visitor
{
    inlet& in;
    graph_edge& edge;
    execution_state& e;

    void operator()(immediate_glutton_connection) const
    {
      if (edge.out_node->enabled())
      {
        Graph_T::copy(*edge.out, in);
      }
      else
      {
        // todo delay, etc
        Graph_T::pull_from_parameter(in, e);
      }
    }

    void operator()(immediate_strict_connection) const
    {
      // if it's a strict connection then the other node
      // is necessarily enabled
      Graph_T::copy(*edge.out, in);
    }

    void operator()(delayed_glutton_connection& con) const
    {
      // TODO If there is data...
      // Else...
      Graph_T::copy(con.buffer, con.pos, in);
      con.pos++;
    }

    void operator()(delayed_strict_connection& con) const
    {
      Graph_T::copy(con.buffer, con.pos, in);
      con.pos++;
    }

    void operator()(dependency_connection) const
    {
    }

    void operator()() const
    {
    }
};

struct env_writer
{
    outlet& out;
    graph_edge& edge;
    execution_state& e;
    bool operator()(const immediate_glutton_connection&) const
    {
      return !edge.in_node->enabled();
    }
    bool operator()(const immediate_strict_connection& con) const
    {
      using rs = immediate_strict_connection::required_sides_t;
      switch(con.required_sides)
      {
        case rs::inbound:
        case rs::outbound:
          // TODO
          break;
        default:
        case rs::both:
          // Nothing to do : copied on "input" phase if the node is active,
          // or no need to copy if the node isn't
          break;
      }
      return false;
    }
    bool operator()(delayed_glutton_connection& con) const
    {
      // Copy to the buffer
      if (con.buffer && out.data && con.buffer.which() == out.data.which())
      {
        eggs::variants::apply(copy_data{}, out.data, con.buffer);
      }
      return false;
    }
    bool operator()(delayed_strict_connection& con) const
    {
      // Copy to the buffer
      if (con.buffer && out.data && con.buffer.which() == out.data.which())
      {
        eggs::variants::apply(copy_data{}, out.data, con.buffer);
      }
      return false;
    }
    bool operator()(const dependency_connection&) const
    {
      return false;
    }
    bool operator()() const
    {
      return false;
    }
};

}
