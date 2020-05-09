#include <tm_kit/infra/Environments.hpp>
#include <tm_kit/infra/TerminationController.hpp>
#include <tm_kit/infra/RealTimeMonad.hpp>

#include <tm_kit/basic/ByteData.hpp>
#include <tm_kit/basic/VoidStruct.hpp>
#include <tm_kit/basic/TrivialBoostLoggingComponent.hpp>
#include <tm_kit/basic/real_time_clock/ClockComponent.hpp>
#include <tm_kit/basic/real_time_clock/ClockImporter.hpp>

#include <tm_kit/transport/BoostUUIDComponent.hpp>
#include <tm_kit/transport/multicast/MulticastComponent.hpp>
#include <tm_kit/transport/multicast/MulticastImporterExporter.hpp>
#include <tm_kit/transport/rabbitmq/RabbitMQComponent.hpp>
#include <tm_kit/transport/rabbitmq/RabbitMQImporterExporter.hpp>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

using namespace dev::cd606::tm;
using namespace boost::program_options;

int main(int argc, char **argv) {
    options_description desc("allowed options");
    desc.add_options()
        ("help", "display help message")
        ("transport", value<std::string>(), "mcast or rabbitmq")
        ("topic", value<std::string>(), "the topic to listen for, for rabbitmq, it can use rabbitmq wild card syntax, for mcast, it can be omitted(all topics), a simple string, or \"r/.../\" containing a regex")
        ("address", value<std::string>(), "the address to listen on")
        ("summaryPeriod", value<int>(), "print summary every this number of seconds")
        ("printPerMessage", "whether to print per message")
    ;
    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (vm.count("help")) {
        std::cout << desc << '\n';
        return 0;
    }
    if (!vm.count("transport")) {
        std::cerr << "No transport given!\n";
        return 1;
    }
    std::string transport = vm["transport"].as<std::string>();
    if (transport != "mcast" && transport != "rabbitmq") {
        std::cerr << "Transport must be mcast or rabbitmq!\n";
        return 1;
    }
    std::string rabbitMQTopic;
    std::variant<
        transport::multicast::MulticastComponent::NoTopicSelection
        , std::string
        , std::regex
    > multicastTopic;
    if (transport == "rabbitmq") {
        if (!vm.count("topic")) {
            rabbitMQTopic = "#";
        } else {
            rabbitMQTopic = vm["topic"].as<std::string>();
        }
    } else if (transport == "mcast") {
        if (!vm.count("topic")) {
            multicastTopic = transport::multicast::MulticastComponent::NoTopicSelection {};
        } else {
            std::string topic = vm["topic"].as<std::string>();
            if (boost::starts_with(topic, "r/") && boost::ends_with(topic, "/") && topic.length() > 3) {
                multicastTopic = std::regex {topic.substr(2, topic.length()-3)};
            } else {
                multicastTopic = topic;
            }
        }
    }
    if (!vm.count("address")) {
        std::cerr << "No address given!\n";
        return 1;
    }
    auto address = transport::ConnectionLocator::parse(vm["address"].as<std::string>());

    std::optional<int> summaryPeriod = std::nullopt;
    if (vm.count("summaryPeriod")) {
        summaryPeriod = vm["summaryPeriod"].as<int>();
    }
    bool printPerMessage = vm.count("printPerMessage");

    if (!summaryPeriod && !printPerMessage) {
        std::cerr << "This program must do something, either print summary message per period or print info per message\n";
        return 1;
    }

    using TheEnvironment = infra::Environment<
        infra::CheckTimeComponent<true>,
        basic::TrivialBoostLoggingComponent,
        basic::real_time_clock::ClockComponent,
        transport::BoostUUIDComponent,
        transport::rabbitmq::RabbitMQComponent,
        transport::multicast::MulticastComponent
    >;

    using M = infra::RealTimeMonad<TheEnvironment>;
    using R = infra::MonadRunner<M>;

    TheEnvironment env;
    R r(&env);

    {
        auto importer =
                (transport == "rabbitmq")
            ?
                transport::rabbitmq::RabbitMQImporterExporter<TheEnvironment>
                ::createImporter(address, rabbitMQTopic)
            :
                transport::multicast::MulticastImporterExporter<TheEnvironment>
                ::createImporter(address, multicastTopic)
            ;

        struct SharedState {
            std::mutex mutex;
            uint64_t messageCount;
            SharedState() : mutex(), messageCount(0) {}
        };
        std::shared_ptr<SharedState> sharedState = std::make_shared<SharedState>();

        auto perMessage = M::simpleExporter<basic::ByteDataWithTopic>([sharedState,printPerMessage](M::InnerData<basic::ByteDataWithTopic> &&data) {
            if (printPerMessage) {
                std::ostringstream oss;
                oss << data.timedData.value;
                data.environment->log(infra::LogLevel::Info, oss.str());
            }
            {
                std::lock_guard<std::mutex> _(sharedState->mutex);
                ++(sharedState->messageCount);
            }
        });

        r.exportItem("perMessage", perMessage, r.importItem("importer", importer));

        if (summaryPeriod) {
            auto clockImporter = 
                basic::real_time_clock::ClockImporter<TheEnvironment>
                ::createRecurringClockConstImporter<basic::VoidStruct>(
                    env.now()
                    , env.now()+std::chrono::hours(24)
                    , std::chrono::seconds(*summaryPeriod)
                    , basic::VoidStruct {}
                );
            auto perClockUpdate = M::simpleExporter<basic::VoidStruct>([sharedState](M::InnerData<basic::VoidStruct> &&data) {
                std::ostringstream oss;
                {
                    std::lock_guard<std::mutex> _(sharedState->mutex);
                    oss << "Got " << sharedState->messageCount << " messages";
                }
                data.environment->log(infra::LogLevel::Info, oss.str());
            });
            r.exportItem("perClockUpdate", perClockUpdate, r.importItem("clockImporter", clockImporter));
        }
    }

    r.finalize();

    infra::terminationController(infra::TerminateAfterDuration {std::chrono::hours(24)});
}