#include "foundry_agent/agent.hpp"
#include "foundry_agent/config.hpp"
#include "foundry_agent/errors.hpp"

#include <cstdlib>
#include <iostream>

int main()
{
    try {
        auto agent = foundry_agent::Agent::CreateDefault(
            foundry_agent::Config::FromEnvironment());
        const auto response = agent.Run("Hello! Tell me a fun fact about C++.");
        std::cout << response.text << '\n';
        return EXIT_SUCCESS;
    } catch (const foundry_agent::ServiceError& error) {
        std::cerr << "Foundry service error (" << error.StatusCode() << "): "
                  << error.what() << '\n';
    } catch (const foundry_agent::AgentError& error) {
        std::cerr << "Agent error: " << error.what() << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Unexpected error: " << error.what() << '\n';
    }
    return EXIT_FAILURE;
}
