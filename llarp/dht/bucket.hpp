#ifndef LLARP_DHT_BUCKET_HPP
#define LLARP_DHT_BUCKET_HPP

#include <dht/kademlia.hpp>
#include <dht/key.hpp>
#include <util/status.hpp>

#include <map>
#include <set>
#include <vector>

namespace llarp
{
  namespace dht
  {
    template < typename Val_t >
    struct Bucket
    {
      using BucketStorage_t = std::map< Key_t, Val_t, XorMetric >;
      using Random_t        = std::function< uint64_t() >;

      Bucket(const Key_t& us, Random_t r) : nodes(XorMetric(us)), random(r){};

      util::StatusObject
      ExtractStatus() const
      {
        util::StatusObject obj{};
        for(const auto& item : nodes)
        {
          obj.Put(item.first.ToHex(), item.second.ExtractStatus());
        }
        return obj;
      }

      size_t
      size() const
      {
        return nodes.size();
      }

      struct SetIntersector
      {
        bool
        operator()(const typename BucketStorage_t::value_type& lhs,
                   const Key_t& rhs)
        {
          return lhs.first < rhs;
        }

        bool
        operator()(const Key_t& lhs,
                   const typename BucketStorage_t::value_type& rhs)
        {
          return lhs < rhs.first;
        }
      };

      bool
      GetRandomNodeExcluding(Key_t& result,
                             const std::set< Key_t >& exclude) const
      {
        std::vector< typename BucketStorage_t::value_type > candidates;
        std::set_difference(nodes.begin(), nodes.end(), exclude.begin(),
                            exclude.end(), std::back_inserter(candidates),
                            SetIntersector());

        if(candidates.empty())
        {
          return false;
        }
        result = candidates[random() % candidates.size()].first;
        return true;
      }

      bool
      FindClosest(const Key_t& target, Key_t& result) const
      {
        Key_t mindist;
        mindist.Fill(0xff);
        for(const auto& item : nodes)
        {
          auto curDist = item.first ^ target;
          if(curDist < mindist)
          {
            mindist = curDist;
            result  = item.first;
          }
        }
        return nodes.size() > 0;
      }

      bool
      GetManyRandom(std::set< Key_t >& result, size_t N) const
      {
        if(nodes.size() < N || nodes.empty())
        {
          llarp::LogWarn("Not enough dht nodes, have ", nodes.size(), " want ",
                         N);
          return false;
        }
        if(nodes.size() == N)
        {
          std::transform(nodes.begin(), nodes.end(),
                         std::inserter(result, result.end()),
                         [](const auto& a) { return a.first; });

          return true;
        }
        size_t expecting = N;
        size_t sz        = nodes.size();
        while(N)
        {
          auto itr = nodes.begin();
          std::advance(itr, random() % sz);
          if(result.insert(itr->first).second)
          {
            --N;
          }
        }
        return result.size() == expecting;
      }

      bool
      FindCloseExcluding(const Key_t& target, Key_t& result,
                         const std::set< Key_t >& exclude) const
      {
        Key_t maxdist;
        maxdist.Fill(0xff);
        Key_t mindist;
        mindist.Fill(0xff);
        for(const auto& item : nodes)
        {
          if(exclude.count(item.first))
          {
            continue;
          }

          auto curDist = item.first ^ target;
          if(curDist < mindist)
          {
            mindist = curDist;
            result  = item.first;
          }
        }
        return mindist < maxdist;
      }

      bool
      GetManyNearExcluding(const Key_t& target, std::set< Key_t >& result,
                           size_t N, const std::set< Key_t >& exclude) const
      {
        std::set< Key_t > s(exclude.begin(), exclude.end());

        Key_t peer;
        while(N--)
        {
          if(!FindCloseExcluding(target, peer, s))
          {
            return false;
          }
          s.insert(peer);
          result.insert(peer);
        }
        return true;
      }

      void
      PutNode(const Val_t& val)
      {
        auto itr = nodes.find(val.ID);
        if(itr == nodes.end() || itr->second < val)
        {
          nodes[val.ID] = val;
        }
      }

      void
      DelNode(const Key_t& key)
      {
        auto itr = nodes.find(key);
        if(itr != nodes.end())
        {
          nodes.erase(itr);
        }
      }

      bool
      HasNode(const Key_t& key) const
      {
        return nodes.find(key) != nodes.end();
      }

      void
      Clear()
      {
        nodes.clear();
      }

      BucketStorage_t nodes;
      Random_t random;
    };
  }  // namespace dht
}  // namespace llarp
#endif
