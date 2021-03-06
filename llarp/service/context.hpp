#ifndef LLARP_SERVICE_CONTEXT_HPP
#define LLARP_SERVICE_CONTEXT_HPP

#include <handlers/tun.hpp>
#include <net/net.hpp>
#include <service/config.hpp>
#include <service/endpoint.hpp>

#include <unordered_map>

namespace llarp
{
  namespace service
  {
    /// holds all the hidden service endpoints we own
    struct Context
    {
      Context(AbstractRouter *r);
      ~Context();

      void
      Tick(llarp_time_t now);

      /// stop all held services
      bool
      StopAll();

      util::StatusObject
      ExtractStatus() const;

      bool
      hasEndpoints();

      /// DRY refactor
      llarp::service::Endpoint *
      getFirstEndpoint();

      bool
      FindBestAddressFor(const llarp::AlignedBuffer< 32 > &addr, bool isSNode,
                         huint32_t &);

      /// DRY refactor
      llarp::handlers::TunEndpoint *
      getFirstTun();

      /// punch a hole to get ip range from first tun endpoint
      llarp_tun_io *
      getRange();

      struct mapAddressAll_context
      {
        llarp::service::Address serviceAddr;
        llarp::Addr localPrivateIpAddr;
      };

      struct endpoint_iter
      {
        void *user;
        llarp::service::Endpoint *endpoint;
        size_t index;
        bool (*visit)(struct endpoint_iter *);
      };

      bool
      iterate(struct endpoint_iter &i);

      /// function visitor returns false to prematurely break iteration
      void
      ForEachService(std::function< bool(const std::string &,
                                         const std::unique_ptr< Endpoint > &) >
                         visit);

      /// hint at possible path usage and trigger building early
      bool
      Prefetch(const llarp::service::Address &addr);

      bool
      MapAddressAll(const llarp::service::Address &addr,
                    llarp::Addr &localPrivateIpAddr);

      /// add default endpoint with options
      bool
      AddDefaultEndpoint(
          const std::unordered_multimap< std::string, std::string > &opts);

      /// add endpoint via config
      bool
      AddEndpoint(const Config::section_t &conf, bool autostart = false);

      /// stop and remove an endpoint by name
      /// return false if we don't have the hidden service with that name
      bool
      RemoveEndpoint(const std::string &name);

      bool
      StartAll();

     private:
      AbstractRouter *m_Router;
      std::unordered_map< std::string, std::unique_ptr< Endpoint > >
          m_Endpoints;
      std::list< std::unique_ptr< Endpoint > > m_Stopped;
    };
  }  // namespace service
}  // namespace llarp
#endif
