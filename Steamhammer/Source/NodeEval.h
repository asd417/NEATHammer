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

class Network {
public:
	virtual void						Activate(std::vector<double>& inputValues);
	virtual const std::vector<double>&	getOutputVector();
	virtual void						AddNodeEval(const json& nodeEval);
	virtual void						FinishInitializing();
	virtual void						Reset();
	virtual bool						IsNodeEvalEmpty();
};

class RecurrentNetwork : public Network {
public:
	RecurrentNetwork(json& inodes, json& onodes);
	void AddNodeEval(const json& nodeEval) override;
	void FinishInitializing() override;
	void Reset() override;

	virtual void Activate(std::vector<double>& inputValues) override
	{
		if (inputValues.size() != inputNodes.size()) throw std::invalid_argument("Input Vector size does not match Network Input size!");
		auto& iv = valuesArray[active];
		auto& ov = valuesArray[1 - active];
		active = 1 - active;

		for (int i = 0; i < inputNodes.size(); i++) {
			iv[inputNodes[i]] = inputValues[i];
			ov[inputNodes[i]] = inputValues[i];
		}
		for (const NodeEval& ne : nodeEvals) {
			std::vector<double> node_inputs{};
			for (Connection c : ne.connections) {
				node_inputs.push_back(iv[c.inputNode] * c.weight);
			}
			double sum = ne.aggrFuncVal(node_inputs);
			ov[ne.node] = ne.actiFuncVal(ne.bias + ne.response * sum);
		}
		for (int o : outputNodes)
		{
			outputs.push_back(ov[o]);
		}
	}
	const std::vector<double>& RecurrentNetwork::getOutputVector() override;
	bool IsNodeEvalEmpty() override;

private:
	int active = 0;
	std::vector<int> inputNodes;
	std::vector<int> outputNodes;
	std::vector<NodeEval> nodeEvals;
	std::array<std::unordered_map<int, double>,2> valuesArray;

	std::vector<double> outputs;
};

class FeedForwardNetwork : public Network {
public:
	FeedForwardNetwork(json& inodes, json& onodes);
	void FinishInitializing() override;
	void AddNodeEval(const json& nodeEval) override;
	void Reset() override;
	void Activate(std::vector<double>& inputValues) override
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

	const std::vector<double>& getOutputVector() override;
	bool IsNodeEvalEmpty() override;
	int getOutputCount() const {
		return outputCount;
	}
private:
	std::vector<int> inputNodes;
	std::vector<int> outputNodes;
	int outputCount = 0;
	std::vector<NodeEval> nodeEvals;

	//Used for Activation
	std::unordered_map<int, double> values;
	
	std::vector<double> outputs;
};