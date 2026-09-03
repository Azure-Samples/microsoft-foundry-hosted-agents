#pragma warning disable OPENAI001 // Suppress experimental API warnings for Responses API usage.
#pragma warning disable MAAI001 // Microsoft.Agents.AI.Harness is experimental.

using Azure.AI.Projects;
using Azure.Identity;
using Microsoft.Agents.AI;
using Microsoft.Extensions.AI;

var endpoint = Environment.GetEnvironmentVariable("FOUNDRY_PROJECT_ENDPOINT")
    ?? throw new InvalidOperationException("Set FOUNDRY_PROJECT_ENDPOINT environment variable");
var deployment = Environment.GetEnvironmentVariable("AZURE_AI_MODEL_DEPLOYMENT_NAME") ?? "gpt-5-mini";

IChatClient chatClient = new AIProjectClient(new Uri(endpoint), new AzureCliCredential())
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
        Instructions = "You are a friendly assistant. Keep your answers brief.",
    },
});

Console.WriteLine(await agent.RunAsync("Hello! Tell me a fun fact about .NET."));
