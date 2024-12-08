#include <memory>
#include <userver/clients/dns/component.hpp>
#include <userver/clients/dns/resolver_fwd.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/common_component_list.hpp>
#include <userver/components/common_server_component_list.hpp>
#include <userver/components/dump_configurator.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/auth/auth_checker_factory.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/secdist/component.hpp>
#include <userver/storages/secdist/provider_component.hpp>
#include <userver/testsuite/dump_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/ugrpc/client/client_factory_component.hpp>
#include <userver/ugrpc/client/common_component.hpp>
#include <userver/ugrpc/client/middlewares/deadline_propagation/component.hpp>
#include <userver/ugrpc/client/middlewares/log/component.hpp>
#include <userver/ugrpc/server/middlewares/congestion_control/component.hpp>
#include <userver/ugrpc/server/middlewares/deadline_propagation/component.hpp>
#include <userver/ugrpc/server/middlewares/log/component.hpp>
#include <userver/ugrpc/server/server_component.hpp>
#include <userver/utils/daemon_run.hpp>
#include "components/auth_checker/auth_checker_bearer_factory/component.hpp"
#include "components/user_manager/component.hpp"
#include "handlers/internal/v1/user/handler.hpp"

int main(int argc, char* argv[]) {
  userver::server::handlers::auth::RegisterAuthCheckerFactory(
      "bearer",
      std::make_unique<components::auth_checker::AuthCheckerBearerFactory>());

  auto component_list =
      userver::components::MinimalServerComponentList()
          .Append<userver::congestion_control::Component>()
          .Append<userver::ugrpc::server::ServerComponent>()
          .Append<userver::server::handlers::Ping>()
          .Append<userver::components::DefaultSecdistProvider>()
          .Append<userver::components::TestsuiteSupport>()
          .Append<userver::components::HttpClient>()
          .Append<userver::server::handlers::TestsControl>()
          .Append<userver::clients::dns::Component>()
          .Append<userver::components::DumpConfigurator>()
          .Append<userver::components::Postgres>("postgres-db-1")
          .Append<userver::components::Secdist>()
          .Append<components::user::Component>()
          .Append<handlers::user_v1_internal::Handler>();

  return userver::utils::DaemonMain(argc, argv, component_list);
}
