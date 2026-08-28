#pragma once

#include <string>
#include <string_view>

namespace foundry_agent
{

    class Config final
    {
    public:
        static constexpr std::string_view DefaultModel = "gpt-5-mini";
        static constexpr std::string_view TokenScope = "https://ai.azure.com/.default";
        static constexpr std::string_view DefaultInstructions =
            "You are a friendly assistant. Keep your answers brief.";

        Config(
            std::string projectEndpoint,
            std::string modelDeployment,
            std::string instructions = std::string{DefaultInstructions});

        static Config FromEnvironment(
            std::string instructions = std::string{DefaultInstructions});

        [[nodiscard]] const std::string &ProjectEndpoint() const noexcept;
        [[nodiscard]] const std::string &ModelDeployment() const noexcept;
        [[nodiscard]] const std::string &Instructions() const noexcept;
        [[nodiscard]] std::string ResponsesUrl() const;

    private:
        std::string projectEndpoint_;
        std::string modelDeployment_;
        std::string instructions_;
    };

} // namespace foundry_agent
