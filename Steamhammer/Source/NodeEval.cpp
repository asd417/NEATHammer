#include "NodeEval.h"

#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <functional>
#include <string>

NodeEval::NodeEval(int nodeID, const std::string& activationFunction, const std::string& aggregationFunction, double bias, double response) :
    node{ nodeID }, bias{ bias }, response{response}
{
    actiFuncVal = getActivationFunction(activationFunction.c_str());
    aggrFuncVal = getAggregationFunction(aggregationFunction.c_str());   
}

void NodeEval::AddConnection(int n, double w)
{
    connections.push_back({ n, w });
}


FeedForwardNetwork::FeedForwardNetwork(json& inodes, json& onodes)
{
    for (int i : inodes) {
        values[i] = 0.0f;
        inputNodes.push_back(i);
    }

    for (int o : onodes) {
        values[o] = 0.0f;
        outputCount++;
        outputNodes.push_back(o);
    }
}

void FeedForwardNetwork::FinishInitializing()
{
}

void FeedForwardNetwork::AddNodeEval(const json& nodeEval)
{
    NodeEval ne = { nodeEval["node"], nodeEval["activation_function"], nodeEval["aggregation_function"] , nodeEval["bias"], nodeEval["response"]};
    for (auto& c : nodeEval["inputs"]) {
        ne.AddConnection(c["inode"], c["weight"]);
    }
    nodeEvals.push_back(ne);
}

void FeedForwardNetwork::Reset()
{
}

const std::vector<double>& FeedForwardNetwork::getOutputVector()
{
    return outputs;
}

bool FeedForwardNetwork::IsNodeEvalEmpty()
{
    return nodeEvals.size() == 0;
}


double sum_aggregation(const std::vector<double>& values) {
    return std::accumulate(values.begin(), values.end(), 0.0);
}

double max_aggregation(const std::vector<double>& values) {
    return *std::max_element(values.begin(), values.end());
}

double min_aggregation(const std::vector<double>& values) {
    return *std::min_element(values.begin(), values.end());
}

double maxabs_aggregation(const std::vector<double>& values) {
    return *std::max_element(values.begin(), values.end(),
        [](double a, double b) { return std::fabs(a) < std::fabs(b); });
}

double median_aggregation(std::vector<double> values) {
    std::sort(values.begin(), values.end());
    size_t size = values.size();
    if (size % 2 == 0) {
        return (values[size / 2 - 1] + values[size / 2]) / 2.0f;
    }
    else {
        return values[size / 2];
    }
}

double mean_aggregation(const std::vector<double>& values) {
    return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
}

///////////////////////////////////////////Activation Functions///////////////////////////////////////////

double abs_activation(double x) {
    return std::fabs(x);
}

double clamped_activation(double x) {
    return std::max((double) -1.0f, std::min((double)1.0f, x));
}

double cube_activation(double x) {
    return x * x * x;
}

double exp_activation(double x) {
    //x = std::max((double)-60.0, std::min((double)60.0, x));
    return std::exp(x);
}

double gauss_activation(double x) {
    //x = std::max((double)-3.4f, std::min((double)3.4f, x));
    return exp(-5.0f * x * x);
}

double hat_activation(double x) {
    return std::max((double)0.0f, 1 - abs(x));
}

double identity_activation(double x) {
    return x;
}

double inv_activation(double x) {
    try {
        x = 1.0f / x;
    }
    catch (std::exception e) {
        return 0;
    }
    return x;
}

double log_activation(double x) {
    x = std::max(1e-7,x);
    return exp(x);
}

double relu_activation(double x) {
    x = x > 0 ? x : 0;
    return x;
}

double sigmoid_activation(double x) {
    //x = std::max((double) -60.0f, std::min((double) 60.0f, 5.0f * x));
    return (double)1.0f / ((double)1.0f + std::exp(-x));
}

double sin_activation(double x) {
    //x = std::max((double) -60.0f, std::min((double)60.0f, 5.0 * x));
    return std::sin(x);
}

double softplus_activation(double x) {
    //x = std::max(-60.0, std::min(60.0, 5.0 * x));
    return 0.2 * log(1 + exp(x));
}

double square_activation(double x) {
    return x * x;
}

double tanh_activation(double x) {
    //x = std::max((double)-60.0f, std::min((double)60.0f, 2.5f * x));
    return std::tanh(x);
}


// Map string to activation function pointer
ActiFuncPtr NodeEval::getActivationFunction(const char* name) {
    if (!strcmp(name, "abs_activation")) return abs_activation;
    if (!strcmp(name, "clamped_activation")) return clamped_activation;
    if (!strcmp(name, "cube_activation")) return cube_activation;
    if (!strcmp(name, "exp_activation")) return exp_activation;
    if (!strcmp(name, "gauss_activation")) return gauss_activation;
    if (!strcmp(name, "hat_activation")) return hat_activation;
    if (!strcmp(name, "identity_activation")) return identity_activation;
    if (!strcmp(name, "inv_activation")) return inv_activation;
    if (!strcmp(name, "log_activation")) return log_activation;
    if (!strcmp(name, "relu_activation")) return relu_activation;
    if (!strcmp(name, "sigmoid_activation")) return sigmoid_activation;
    if (!strcmp(name, "sin_activation")) return sin_activation;
    if (!strcmp(name, "softplus_activation")) return softplus_activation;
    if (!strcmp(name, "square_activation")) return square_activation;
    if (!strcmp(name, "tanh_activation")) return tanh_activation;
    throw std::invalid_argument("Unknown activation function");
    return nullptr;
}

// Map string to aggregation function pointer
AggrFuncPtr NodeEval::getAggregationFunction(const char* name) {
    if (!strcmp(name, "sum_aggregation")) return sum_aggregation;
    if (!strcmp(name, "max_aggregation")) return max_aggregation;
    if (!strcmp(name, "min_aggregation")) return min_aggregation;
    if (!strcmp(name, "maxabs_aggregation")) return maxabs_aggregation;
    if (!strcmp(name, "median_aggregation")) return median_aggregation;
    if (!strcmp(name, "mean_aggregation")) return mean_aggregation;
    throw std::invalid_argument("Unknown aggregation function");
    return nullptr;
}

RecurrentNetwork::RecurrentNetwork(json& inodes, json& onodes)
{
    for (int i : inodes) {
        inputNodes.push_back(i);
    }
    for (int o : onodes) {
        outputNodes.push_back(o);
    }
    outputCount = onodes.size();
    for (auto& v : valuesArray)
    {
        for (int i : inodes) {
            v[i] = 0.0f;
        }

        for (int o : onodes) {
            v[o] = 0.0f;
        }
    }
}

void RecurrentNetwork::AddNodeEval(const json& nodeEval)
{
    NodeEval ne = { nodeEval["node"], nodeEval["activation_function"], nodeEval["aggregation_function"] , nodeEval["bias"], nodeEval["response"] };
    for (auto& c : nodeEval["inputs"]) {
        ne.AddConnection(c["inode"], c["weight"]);
    }
    nodeEvals.push_back(ne);
}

/// <summary>
/// Call when all node_evals have been added
/// </summary>
void RecurrentNetwork::FinishInitializing()
{
    for (auto& v : valuesArray)
    {
        for (auto& ne : nodeEvals)
        {
            v[ne.node] = 0.0f;
            for (auto& c : ne.connections)
            {
                v[c.inputNode] = 0.0f;
            }
        }
    }
}

void RecurrentNetwork::Reset()
{
    std::vector<int> keys;
    for (auto& v : valuesArray)
    {
        for (auto& k : v)
        {
            keys.push_back(k.first);
        }
    }
    for (auto& k : keys)
    {
        valuesArray[0][k] = 0.0f;
        valuesArray[1][k] = 0.0f;
    }
    active = 0;
}

const std::vector<double>& RecurrentNetwork::getOutputVector()
{
    return outputs;
}


bool RecurrentNetwork::IsNodeEvalEmpty()
{
    return nodeEvals.size() == 0;
}

void Network::Activate(const std::vector<double>& inputValues)
{
}

const std::vector<double>& Network::getOutputVector()
{
    throw std::exception("Properly inherit this class instead");
    return {};
}

void Network::AddNodeEval(const json& nodeEval)
{
}

void Network::FinishInitializing()
{
}

void Network::Reset()
{
}

bool Network::IsNodeEvalEmpty()
{
    return false;
}
