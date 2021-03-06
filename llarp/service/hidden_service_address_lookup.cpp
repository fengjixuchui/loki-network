#include <service/hidden_service_address_lookup.hpp>

#include <dht/messages/findintro.hpp>
#include <service/endpoint.hpp>

namespace llarp
{
  namespace service
  {
    HiddenServiceAddressLookup::HiddenServiceAddressLookup(Endpoint* p,
                                                           HandlerFunc h,
                                                           const Address& addr,
                                                           uint64_t tx)
        : IServiceLookup(p, tx, "HSLookup"), remote(addr), handle(h)
    {
    }

    bool
    HiddenServiceAddressLookup::HandleResponse(
        const std::set< IntroSet >& results)
    {
      LogInfo("found ", results.size(), " for ", remote.ToString());
      if(results.size() > 0)
      {
        IntroSet selected;
        for(const auto& introset : results)
        {
          if(selected.OtherIsNewer(introset) && introset.A.Addr() == remote)
            selected = introset;
        }
        return handle(remote, &selected, endpoint);
      }
      return handle(remote, nullptr, endpoint);
    }

    routing::IMessage*
    HiddenServiceAddressLookup::BuildRequestMessage()
    {
      routing::DHTMessage* msg = new routing::DHTMessage();
      msg->M.emplace_back(new dht::FindIntroMessage(txid, remote, 0));
      return msg;
    }

  }  // namespace service
}  // namespace llarp
