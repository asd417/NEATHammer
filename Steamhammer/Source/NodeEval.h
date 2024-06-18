#pragma once
#include <map>
#include <string>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Activation function pointer type
using ActiFuncPtr = std::function<double(double)>;

// Aggregation function pointer type
using AggrFuncPtr = std::function<double(const std::vector<double>&)>;

struct Connection{
	int inputNode;
	double weight;
};

class NodeEval {
public:
	NodeEval(int nodeID, const std::string& activationFunction, const std::string& aggregationFunction, double bias, double response);
	~NodeEval() = default;

	void AddConnection(int, double);

	int node;
	double bias;
	double response;
	ActiFuncPtr actiFuncVal;
	AggrFuncPtr aggrFuncVal;
	
	std::vector<Connection> connections;
private:
	static ActiFuncPtr getActivationFunction(const char*);
	static AggrFuncPtr getAggregationFunction(const char*);
};

class FeedForwardNetwork {
public:
	FeedForwardNetwork(json& inodes, json& onodes);

	void AddNodeEval(const json& nodeEval);

	template<typename T>
	void Activate(std::vector<T>& inputValues)
	{
		outputs.clear();
		if (inputValues.size() != inputNodes.size()) throw std::invalid_argument("Input Vector size does not match Network Input size!");
		for (int i = 0; i < inputNodes.size(); i++) {
			values[inputNodes[i]] = inputValues[i];
		}
		for (const NodeEval& ne : nodeEvals) {
			std::vector<double> node_inputs{};
			for (Connection c : ne.connections) {
				node_inputs.push_back(values[c.inputNode] * c.weight);
			}
			double sum = ne.aggrFuncVal(node_inputs);
			values[ne.node] = ne.actiFuncVal(ne.bias + ne.response * sum);
		}
		for (int o : outputNodes)
		{
			outputs.push_back(values[o]);
		}
	}

	const std::vector<double>& getOutputVector();
	bool IsNodeEvalEmpty();
private:
	std::vector<int> inputNodes;
	std::vector<int> outputNodes;
	std::vector<NodeEval> nodeEvals;

	//Used for Activation
	std::unordered_map<int, double> values;
	
	std::vector<double> outputs;
};