#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <random>
#include <chrono>
#include <algorithm>

// Utility function to trim whitespace from both ends of a string
inline std::string trim(const std::string& str) {
	auto is_not_space = [](char ch) { return !std::isspace(ch); };

	// Find the first and last non-whitespace characters
	auto start = std::ranges::find_if(str, is_not_space);
	auto end = std::ranges::find_if(str.rbegin(), str.rend(), is_not_space).base();

	// If the string is entirely whitespace, return an empty string
	if (start >= end) {
		return "";
	}

	return std::string(start, end);
}


// Represents a single production rule with a weight and a computed probability
struct Rule {
	char predecessor;
	std::string successor;
	double weight;
	double probability;
};

// Container for the set of rules
class RuleSet {
public:
	// Adds a rule to the set
	void addRule(const Rule& rule) {
		std::cout << "Adding rule " << rule.predecessor << "(" << rule.weight << ") -> " << rule.successor << "\n";
		rules[rule.predecessor].push_back(rule);
		normalizeProbabilities(rule.predecessor);
	}

	void setAxiom(const std::string &aAxiom) {
		std::cout << "Setting axiom " << aAxiom << "\n";
		mAxiom = aAxiom;
	}

	std::string axiom() const {
		return mAxiom;
	}

	// Selects a rule nondeterministically based on provided probability
	std::string applyRule(char predecessor, double randomValue) const {
		auto it = rules.find(predecessor);
		if (it == rules.end()) {
			return "";
		}
		const auto& possibleRules = it->second;
		double cumulativeProbability = 0.0;

		for (const auto& rule : possibleRules) {
			cumulativeProbability += rule.probability;
			if (randomValue <= cumulativeProbability) {
				return rule.successor;
			}
		}
		return ""; // Should never reach here if probabilities are set correctly
	}

	// Loads the rule set from a text file
	static RuleSet loadFromFile(fs::path aRuleFile) {
		std::cout << "Loading L-system rule set " << aRuleFile << "\n";
		if (!fs::exists(aRuleFile)) {
			throw std::runtime_error("File " + aRuleFile.string() + " does not exist!");
		}
		RuleSet ruleSet;
		std::ifstream file(aRuleFile);
		std::string line;

		if (std::getline(file, line)) {
			line = trim(line);
			ruleSet.setAxiom(line);
		}
		while (std::getline(file, line)) {
			line = trim(line);
			if (line.empty() || line[0] == '#') {
				continue;
			}
			Rule rule = parseRule(line);
			ruleSet.addRule(rule);
		}
		return ruleSet;
	}

private:
	std::unordered_map<char, std::vector<Rule>> rules;
	std::string mAxiom;

	static Rule parseRule(const std::string& line) {
		Rule rule;
		size_t pos1 = line.find('(');
		size_t pos2 = line.find(')');
		size_t pos3 = line.find("->");

		rule.predecessor = line.at(0);
		rule.weight = std::stod(line.substr(pos1 + 1, pos2 - pos1 - 1));
		rule.successor = line.substr(pos3 + 3); // +3 to account for "->" and the space

		return rule;
	}

	void normalizeProbabilities(char predecessor) {
		auto& possibleRules = rules[predecessor];
		double totalWeight = 0.0;
		for (const auto& rule : possibleRules) {
			totalWeight += rule.weight;
		}
		for (auto& rule : possibleRules) {
			rule.probability = rule.weight / totalWeight;
		}
	}
};

// Generates a string using the L-system rules over a number of iterations
std::string generateLSystemString(const RuleSet& ruleSet, int iterations, int seed) {
	std::string current = ruleSet.axiom();
	std::default_random_engine rng(seed);
	std::uniform_real_distribution<double> dist(0.0, 1.0);

	for (int i = 0; i < iterations; ++i) {
		std::string next;
		for (const char& c : current) {
			double randomValue = dist(rng);
			std::string s(1, c);
			if (ruleSet.applyRule(c, randomValue).empty()) {
				next += c; // Keep the character if no rule is found
			} else {
				next += ruleSet.applyRule(c, randomValue);
			}
		}
		current = next;
	}

	return current;
}

// // Example usage
// int main() {
//     // Load rules from file
//     RuleSet ruleSet = RuleSet::loadFromFile("rules.txt");
//
//     // Generate a string using the L-system rules
//     std::string axiom = "A"; // Axiom
//     int iterations = 4;
//
//     std::string result = generateLSystemString(ruleSet, axiom, iterations);
//     std::cout << "Resulting L-system string: " << result << std::endl;
//
//     return 0;
// }
//
