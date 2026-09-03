#pragma warning disable OPENAI001 // Suppress experimental API warnings for Responses API usage.
#pragma warning disable MAAI001 // Microsoft.Agents.AI.Harness is experimental.

using Azure.AI.Projects;
using Azure.Identity;
using Microsoft.Agents.AI;
using Microsoft.Agents.AI.Foundry.Hosting;
using Microsoft.Extensions.AI;

var projectEndpointValue = Environment.GetEnvironmentVariable("FOUNDRY_PROJECT_ENDPOINT")?.Trim()
    ?? throw new InvalidOperationException("FOUNDRY_PROJECT_ENDPOINT is not set.");
if (!Uri.TryCreate(projectEndpointValue, UriKind.Absolute, out var projectEndpoint)
    || projectEndpoint.Scheme != Uri.UriSchemeHttps
    || !projectEndpoint.AbsolutePath.Contains("/api/projects/", StringComparison.OrdinalIgnoreCase))
{
    throw new InvalidOperationException(
        "FOUNDRY_PROJECT_ENDPOINT must be an HTTPS Foundry Project URL that includes " +
        "'/api/projects/<project-name>'.");
}
var deployment = Environment.GetEnvironmentVariable("AZURE_AI_MODEL_DEPLOYMENT_NAME") ?? "gpt-5-mini";

var credential = new ChainedTokenCredential(
    new AzureCliCredential(),
    new AzureDeveloperCliCredential(),
    new ManagedIdentityCredential(ManagedIdentityId.SystemAssigned));

IChatClient chatClient = new AIProjectClient(projectEndpoint, credential)
    .GetProjectOpenAIClient()
    .GetResponsesClient()
    .AsIChatClient(deployment);

// AsHarnessAgent wraps the chat client with the Microsoft Agent Framework Harness: function
// invocation, per-service-call chat history persistence, planning, and web search come pre-configured.
AIAgent agent = chatClient.AsHarnessAgent(new HarnessAgentOptions
{
    Name = "HelloHarnessAgent",
    ChatOptions = new ChatOptions
    {
        Instructions = "You are the C# Harness hosted agent sample, running on .NET 10 with the Microsoft Agent " +
                      "Framework Harness. When greeted or asked who you are, identify yourself as the C# Harness " +
                      "hosted agent. Keep your answers brief.",
    },
});

var builder = WebApplication.CreateBuilder(args);
builder.Services.AddFoundryResponses(agent);

var app = builder.Build();
app.MapFoundryResponses();
app.Run();
