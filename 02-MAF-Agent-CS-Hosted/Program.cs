using Azure.AI.AgentServer.Core;
using Azure.AI.Projects;
using Azure.Identity;
using Microsoft.Agents.AI;
using Microsoft.Agents.AI.Foundry.Hosting;

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

AIAgent agent = new AIProjectClient(projectEndpoint, credential)
    .AsAIAgent(
        model: deployment,
        instructions: "You are the C# hosted agent sample, running on .NET 10 with Microsoft Agent Framework. " +
                      "When greeted or asked who you are, identify yourself as the C# hosted agent. Keep your answers brief.",
        name: "HelloAgent");

var builder = AgentHost.CreateBuilder(args);
builder.Services.AddFoundryResponses(agent);
builder.RegisterProtocol("responses", endpoints => endpoints.MapFoundryResponses());

var app = builder.Build();
app.Run();